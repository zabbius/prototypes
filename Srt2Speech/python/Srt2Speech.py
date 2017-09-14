#!/usr/bin/python2
# -*- coding: utf-8 -*-

import codecs
import httplib
import logging
import os
import pipes
import shlex
import shutil
import subprocess
import sys
import uuid

import pysrt

FRAGMENT_FORMAT = "-t s16 -c 1 -r 22050"


class Str2Speech:
    def __init__(self, config):
        self.logger = logging.getLogger("Srt2Speech")
        self.config = config

    def readSubtitles(self):
        if self.config['input'] == '-':
            srtFile = codecs.getreader(self.config['encoding'])(sys.stdin)
            self.logger.info("Reading subtitles from STDIN")
        else:
            srtFile = codecs.open(self.config['input'], 'Ur', encoding=self.config['encoding'])
            self.logger.info("Reading subtitles from {0}".format(self.config['input']))

        subtitles = [{'start': s.start.ordinal, 'end': s.end.ordinal, 'text': s.text.replace('\n', ' ')}
                     for s in pysrt.stream(srtFile)]
        subtitles = sorted(subtitles, key=lambda s: s['start'])

        prev = None
        for sub in subtitles:
            sub['available'] = sub['duration'] = sub['end'] - sub['start']
            if prev:
                prev['available'] = sub['start'] - prev['start']
            prev = sub

        self.logger.info("Got {0} subtitles ".format(len(subtitles)))
        return subtitles

    def getSpeech(self, text, rate, voice):
        self.logger.debug(u"Getting speech with rate {0:.2f} and voice {1}: {2}".format(rate, voice, text))
        ssml = u'<speak><voice name="{0}"><prosody rate="{1}">{2}</prosody></voice></speak>'.format(voice, rate, text)
        headers = {'Content-Type': 'text/plain'}
        connection = httplib.HTTPConnection(self.config['server'])
        connection.request('POST', "/speech?ssml=1", ssml.encode('utf-8'), headers)
        response = connection.getresponse()
        speech = response.read()
        connection.close()

        sampleRate = int(response.getheader('X-Speech-Sample-Rate'))

        duration = len(speech) / 2 * 1000 / sampleRate

        return speech, duration, sampleRate

    def generateSpeech(self, subtitles, tempdir):
        self.logger.info("Generating speech for subtitles")

        shift = 0

        for sub in subtitles:
            sub['speech_start'] = sub['start'] + shift
            sub['speech_available'] = sub['available'] - shift

            rate = self.config['rate']
            voice = self.config['voice']
            available = sub['speech_available']

            speech, duration, sampleRate = self.getSpeech(sub['text'], rate, voice)

            if duration - available > self.config['threshold']:
                accel = self.config['maxaccel']

                if available > 0:
                    accel = float(duration) / available
                    if accel > self.config['maxaccel']:
                        accel = self.config['maxaccel']

                rate *= accel

                speech, duration, sampleRate = self.getSpeech(sub['text'], rate, voice)

                shift = duration - available

                if shift < self.config['threshold']:
                    shift = 0

            else:
                shift = 0

            sub['speech_end'] = sub['speech_start'] + duration
            sub['speech_sample_rate'] = sampleRate

            path = tempdir + "/{0:08}-{1:08}.pcm".format(sub['speech_start'], sub['speech_end'])

            with open(path, "wb") as f:
                f.write(speech)

            sub['speech_path'] = path

    def mixFragments(self, subsToMix, start, end, tempdir):
        files = " ".join(
            ["-t s16 -c 1 -r {0} {1}".format(s['speech_sample_rate'], s['speech_path']) for s in subsToMix])
        delays = " ".join(["{0:.3f}".format((s['speech_start'] - start) / 1000.0) for s in subsToMix])
        mixedPath = tempdir + "/mixed_{0:08}-{1:08}.pcm".format(start, end)

        cmd = "sox {0} {1} {2} {3} pad 0 {4:.3f} delay {5} remix - norm trim 0 {4:.3f}".format(
            "-M" if len(subsToMix) > 1 else "", files, FRAGMENT_FORMAT, mixedPath, (end - start) / 1000.0, delays)

        self.logger.debug("Mixing {0}".format(mixedPath))
        subprocess.check_call(shlex.split(cmd), stdout=None, stderr=None, stdin=None)

        return mixedPath

    def mixSpeech(self, subtitles, tempdir):
        self.logger.info("Mixing speech fragments")

        mixedFragments = []

        subsToMix = []
        start = end = 0

        for sub in subtitles:
            if sub['speech_start'] < end:
                subsToMix.append(sub)
            else:
                if subsToMix:
                    mixedPath = self.mixFragments(subsToMix, start, end, tempdir)
                    mixedFragments.append(mixedPath)

                start = end
                subsToMix = [ sub ]

            end = sub['speech_end']

        if subsToMix:
            mixedPath = self.mixFragments(subsToMix, start, end, tempdir)
            mixedFragments.append(mixedPath)

        self.logger.debug("Concatenating fragments")
        resultPath = tempdir + "/result.pcm"

        with open(resultPath, "wb") as res:
            for path in mixedFragments:
                with open(path, 'rb') as f:
                    shutil.copyfileobj(f, res)

        return resultPath

    def writeResult(self, resultPath):
        if self.config['output'] == '-':
            outFile = sys.stdout
            self.logger.info("Writing result to STDOUT")
        else:
            outFile = open(self.config['output'], "wb")
            self.logger.info("Writing result to {0}".format(self.config['output']))

        cmd = "sox {0} {1} {2} -".format(FRAGMENT_FORMAT, resultPath, self.config['format'])

        subprocess.check_call(shlex.split(cmd), stderr=None, stdin=None, stdout=outFile)
        self.logger.info("Result saved")

    def run(self):
        tempdir = pipes.quote(self.config['tempdir'] + "/" + str(uuid.uuid4()))

        self.logger.info("Creating temp dir {0}".format(tempdir))
        os.mkdir(tempdir)
        try:
            subtitles = self.readSubtitles()
            self.generateSpeech(subtitles, tempdir)
            resultPath = self.mixSpeech(subtitles, tempdir)
            self.writeResult(resultPath)

        finally:
            self.logger.info("Recursively removing temp dir {0}".format(tempdir))
            shutil.rmtree(tempdir, ignore_errors=True)


if __name__ == "__main__":
    from python_modules.common_utils.server_launcher import UtilLauncher


    class Launcher(UtilLauncher):
        def createUtil(self, config):
            return Str2Speech(config['Srt2Speech'])

        def getConfigOverrideSection(self, config):
            return config['Srt2Speech']

        def addArgumentsToParser(self, parser):
            parser.add_argument("-f", "--format", help="Output file format")
            parser.add_argument("-d", "--tempdir", help="Directory for temporary files")
            parser.add_argument("-s", "--server", help="TTS server URL")
            parser.add_argument("-e", "--encoding", help="Input file encoding")
            parser.add_argument("-i", "--input", help="Input srt file")
            parser.add_argument("-o", "--output", help="Output sound file")
            parser.add_argument("-v", "--voice", help="Default voice")
            parser.add_argument("-r", "--rate", help="Default speech speed rate", type=float)
            parser.add_argument("-a", "--maxaccel", help="Maximum speech acceleration", type=float)
            parser.add_argument("-t", "--threshold", help="Duration threshold", type=float)

    Launcher().Run()
