import logging
import subprocess
import re
import time


class VideoController:
    def __init__(self, config):
        self.logger = logging.getLogger("VideoController")

        self.tinycamdPath = config['tinycamdPath']
        self.modeRegex = re.compile(r": frame\([0-9]+\) ([0-9]+)x([0-9]+): fps ([0-9]+)/([0-9]+)")

        self.ports = range(config['PortRange']['first'], config['PortRange']['last'])
        self.listenHost = config['listenHost']
        self.devicesConfig = config['Devices']

        self.captures = {}
        self.devices = {}

    def start(self):
        self.logger.info("Starting")
        self.devices = self.listDevices()
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.stopAllCaptures()
        self.logger.info("Stopped")

    def listDevices(self):
        self.logger.info("Probing video devices")
        result = {}
        for (name, device) in self.devicesConfig.iteritems():
            self.logger.info("Probing {0} ({1})".format(name, device))

            args = [self.tinycamdPath, "-P", "-d", device]
            self.logger.info("Starting tinycamd with args: {0}".format(args))
            p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            (output, err) = p.communicate()

            mjpegFound = False

            modes = []

            for line in output.split('\n'):
                line = line.strip()

                if mjpegFound:
                    if line.startswith("fmtDesc:"):
                        break

                    match = self.modeRegex.match(line)
                    if match is None:
                        continue

                    (width, height, secs, fps) = match.groups()

                    if secs == '1':
                        mode = {'width': int(width), 'height': int(height), 'maxfps': int(fps)}
                        modes.append(mode)
                        self.logger.debug("Mode found: {0}".format(mode))

                else:
                    if not line.startswith("fmtDesc:"):
                        continue

                    if line.lower().find("mjpeg") != -1 or line.lower().find("mjpg") != -1:
                        mjpegFound = True

            if len(modes) > 0:
                result[name] = {'device': device, 'modes': sorted(modes, lambda x, y: cmp(x['width'], y['width']))}

            return result

    def startCapture(self, name, width, height, fps):
        if name not in self.devices:
            raise RuntimeError("Invalid name: {0}".format(name))

        if name in self.captures:
            raise RuntimeError("Capture already started at {0}".format(name))

        if len(self.ports) == 0:
            raise RuntimeError("No more ports available")

        self.logger.info("Starting capture at {0}".format(name))

        device = self.devices[name]

        port = self.ports.pop(0)

        self.logger.info(
            "Starting video from {0} with width: {1}, height: {2}, fps: {3} at {4}:{5}".format(device, width, height,
                                                                                               fps, self.listenHost,
                                                                                               port))

        args = [self.tinycamdPath, '-d', device['device'], '-s', "{0}x{1}".format(width, height), '-f', str(fps), '-F',
                'mjpeg', '-p', "{0}:{1}".format(self.listenHost, port)]
        self.logger.info("Starting tinycamd with args: {0}".format(args))
        tinycamd = subprocess.Popen(args)
        self.logger.info("Waiting for tinycamd to open port")
        time.sleep(1)
        self.logger.info("tinycamd started")

        self.captures[name] = (tinycamd, device, width, height, fps, port)

        self.logger.info("Capture started at {0}".format(name))
        return port

    def stopCapture(self, name):
        if name not in self.captures:
            return

        (tinycamd, device, width, height, fps, port) = self.captures.pop(name)

        self.logger.info("Stopping capture at {0}".format(name))

        tinycamd.terminate()
        tinycamd.wait()

        self.logger.info("Capture stopped at {0}".format(name))
        self.ports.append(port)

    def stopAllCaptures(self):
        self.logger.info("Stopping all captures")
        for name in self.captures.iterkeys():
            self.stopCapture(name)

    def getStatus(self):
        result = []

        for (name, params) in self.captures.iteritems():
            (tinycamd, device, width, height, fps, port) = params
            result.append({'name': name, 'device': device, 'width': width, 'height': height, 'fps': fps, 'port': port})

        return {'devices': self.devices, 'captures': result}


class VideoCommandHandler:
    def __init__(self, videoController):
        self.videoController = videoController

    def handleCommand(self, cmd, args):
        if cmd != 'video':
            raise NotImplementedError("Wrong command: {0}".format(cmd))

        action = args.get('act')

        if action == 'startCapture':
            name = args.get('name')
            width = int(args.get('width'))
            height = int(args.get('height'))
            fps = int(args.get('fps'))
            port = self.videoController.startCapture(name, width, height, fps)

            return {'port': port}

        elif action == 'stopCapture':
            name = args.get('name')
            self.videoController.stopCapture(name)
            return True

        elif action == 'status':
            return {'videoStatus': self.videoController.getStatus()}

        elif action == 'start':
            self.videoController.start()
            return True
        elif action == 'stop':
            self.videoController.stop()
            return True

        else:
            raise NotImplementedError("Wrong action: {0}".format(action))
