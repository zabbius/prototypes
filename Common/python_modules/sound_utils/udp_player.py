# -*- coding: utf-8 -*-

import logging
import socket
import alsaaudio
import threading
import traceback

import six


class UdpPlayer:
    def __init__(self, ):
        self.logger = logging.getLogger("UdpPlayer")

    def __str__(self):
        return str(self.addr)

    def getListenAddr(self):
        return self.socket.getsockname()

    def start(self, host, port, sampleRate=8000, periodSize=100, channels=1, sampleFormat=alsaaudio.PCM_FORMAT_S16_LE):
        self.recvPackets = 0
        self.recvBytes = 0

        self.periodSize = periodSize

        self.addr = (host, port)
        self.sampleRate = sampleRate
        self.sampleFormat = sampleFormat
        self.channels = channels

        self.logger.info("{0} starting".format(self))

        self.logger.info("{0}: Creating socket".format(self))
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.settimeout(1)

        self.logger.info("{0}: Binding socket".format(self))
        self.socket.bind(self.addr)

        self.logger.info("{0}: Creating PCM playback".format(self))
        self.pcm = alsaaudio.PCM(alsaaudio.PCM_PLAYBACK)

        self.logger.info("{0}: Setting playback parameters, channels: {1}, rate: {2}, period: {3}, format: {4}".
                         format(self, self.channels, self.sampleRate, self.periodSize, self.sampleFormat))

        self.pcm.setchannels(self.channels)
        self.pcm.setrate(self.sampleRate)
        self.pcm.setformat(self.sampleFormat)
        self.pcm.setperiodsize(self.periodSize)

        self.queue = queue.Queue()

        self.stopThread = False

        self.logger.info("{0}: Starting threads".format(self))

        self.recvThread = threading.Thread(target=self.recvThreadProc, name="UdpPlayerRecvThread")
        self.playThread = threading.Thread(target=self.playThreadProc, name="UdpPlayerPlayThread")

        self.recvThread.daemon = True
        self.playThread.daemon = True

        self.recvThread.start()
        self.playThread.start()
        self.logger.info("{0} started".format(self))

    def stop(self):
        self.logger.info("{0} stopping".format(self))
        self.stopThread = True
        self.recvThread.join()
        self.playThread.join()

        self.socket.close()
        self.pcm.close()

        self.pcm = None;
        self.socket = None
        self.recvThread = None
        self.playThread = None

        self.logger.info("{0} stopped".format(self))

    def status(self):
        if self.socket is None:
            return {'started': False}

        return {'started': True,
                'periodSize': self.periodSize,
                'addr': self.addr,
                'sampleRate': self.sampleRate,
                'sampleFormat': self.sampleFormat,
                'channels': self.channels,
                'recvPackets': self.recvPackets,
                'recvBytes': self.recvBytes}

    def recvThreadProc(self):
        while not self.stopThread:
            try:
                data, addr = self.socket.recvfrom(1024 * 1024)
                self.recvPackets += 1
                self.recvBytes += len(data)
                self.queue.put(data)
            except socket.timeout:
                pass
            except Exception as ex:
                self.logger.warning("Exception caught in receive thread: {0}\n{1}".format(ex, traceback.format_exc()))

    def playThreadProc(self):
        while not self.stopThread:
            try:
                data = self.queue.get(True, 1)
                self.pcm.write(data)
            except queue.Empty:
                pass
            except Exception as ex:
                self.logger.warning("Exception caught in play thread: {0}\n{1}".format(ex, traceback.format_exc()))
