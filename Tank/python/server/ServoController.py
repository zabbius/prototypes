import logging
import os
import subprocess
import time
import six

class ServoController:
    def __init__(self, config):
        self.logger = logging.getLogger("ServoController")
        
        self.servodPath = config['servodPath']
        self.ctrlFile = config['ctrlFile']
        
        self.minValue = config['minValue']
        self.maxValue = config['maxValue']

        self.range = (self.maxValue - self.minValue) / 2
        self.zero = (self.maxValue + self.minValue) / 2
        
        self.servos = config['Servos']
        self.servod = None
        
        self.config = config;

    def start(self):
        if self.servod is not None:
            raise RuntimeError("Servo controller already started")

        self.logger.info("Starting")
                        
        pins = []
        n = 0
        for name in six.iterkeys(self.servos):
            servo = self.servos[name]
            pins.append(str(servo['pin']))
            servo['index'] = n
            n = n + 1
            self.servos[name] = servo
            
        pinList = ','.join(pins)
            
        args = [self.servodPath, '-f', '--min={0}'.format(self.minValue), '--max={0}'.format(self.maxValue), '--p1pins={0}'.format(pinList)]
        
        self.logger.info("Starting servoblaster with args: {0}".format(args))
        self.servod = subprocess.Popen(args)
        self.logger.info("servoblaster started")

        n = 0
        while not os.path.exists(self.ctrlFile) and n < 10:
            n += 1
            time.sleep(0.1)
            
        
        self.logger.info("Setting initial values")
        for name in six.iterkeys(self.servos):
            servo = self.servos[name]
            self.setServoPosition(name, servo['value'])
        
        self.logger.info("Started")
        
    def stop(self):
        if self.servod is None:
            return

        self.logger.info("Stopping")
        self.servod.terminate()
        self.servod.wait()
        self.servod = None
        self.logger.info("Stopped")
    
    def getServoPosition(self, name):
        if name not in self.servos:
            raise IndexError("Wrong servo name: {0}".format(name))
        
        return self.servos[name]['value']
        
    def getStatus(self):
        servoStatus = {}
        
        for name in six.iterkeys(self.servos):
            servoStatus[name] = self.servos[name].copy()
        
        return servoStatus
        
    
    def setServoPosition(self, name, value):
        if name not in self.servos:
            raise IndexError("Wrong servo name: {0}".format(name))
        
        servo = self.servos[name]

        value += servo['trim']

        if value > servo['maxlimit']:
            value = servo['maxlimit']
        if value < servo['minlimit']:
            value = servo['minlimit']
        
        valueOut = int(self.range * value + self.zero)
        
        with open(self.ctrlFile, "w") as f:
            f.write("{0}={1}\n".format(servo['index'], valueOut))

        servo['value'] = value
        self.servos[name] = servo
        
    def setMany(self, positions):
        for name, value in positions.iteritems():
            self.setServoPosition(name, value)
        
        
class ServoCommandHandler:
    def __init__(self, servoController):
        self.servoController = servoController
        
    def handleCommand(self, cmd, args):
        if cmd != 'servo':
            raise NotImplementedError("Wrong command: {0}".format(cmd))
        
        action = args.get('act')

        if action == 'set':
            name = args.get('name')
            value = float(args.get('value'))
            self.servoController.setServoPosition(name, value)
            
            if args.get('_protocol', '') == 'udp':
                return { '_noAnswer': True }
            
            return True

        elif action == 'setmany':
            values = args.get('values')
            
            servodict = {}

            for item in values.split(','):
                name, value = item.split(':', 1)
                servodict[name] = float(value)
                
            self.servoController.setMany(servodict)

            if args.get('_protocol', '') == 'udp':
                return { '_noAnswer': True }
            
            return True
            
            
        elif action == 'get':
            name = args.get('name')
            value = self.servoController.getServoPosition(name)
            return { 'servoName': name, 'servoValue': value }

        elif action == 'status':
            return { 'servoStatus': self.servoController.getStatus() }

        elif action == 'start':
            self.servoController.start()
            return True
        elif action == 'stop':
            self.servoController.stop()
            return True

        else:
            raise NotImplementedError("Wrong action: {0}".format(action))
        