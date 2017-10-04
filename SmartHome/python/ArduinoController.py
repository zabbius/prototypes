# -*- coding: utf-8 -*-
import copy
import logging
import time
import traceback

from python_modules.command_utils.serial_client_connector import SerialClientConnector

INITIAL_TIMEOUT = 5.0


class ArduinoController:
    def __init__(self, config):
        self.logger = logging.getLogger("ArduinoController")
        self.port = config['port']
        self.speed = config['speed']
        self.timeout = config['timeout']

        self.initRequests = config['Init']
        self.deinitRequests = config['Deinit']

        self.connector = None

        self.config = config

        self.switches = copy.deepcopy(config['Switches'])

    def sendInitRequests(self):
        self.logger.info("Sending initial requests")
        while True:
            try:
                for request in self.initRequests:
                    self.connector.requestAndCheck(request['cmd'], request['args'])
                break
            except Exception, ex:
                self.logger.error("{0}: Exception caught: {1}\n{2}".format(self, ex, traceback.format_exc()))
                time.sleep(1)

    def sendDeinitRequests(self):
        self.logger.info("Sending final requests")
        for request in self.deinitRequests:
            self.connector.requestAndCheck(request['cmd'], request['args'])

    def initSwitches(self):
        self.logger.info("Initializing switches")
        for name, switch in self.switches.iteritems():
            self.connector.requestAndCheck('pin', { 'act': 'dmode', 'mode': 'output', 'pin': switch['pin'] })
            self.setSwitchValue(name, switch['value'])

    def deinitSwitches(self):
        self.logger.info("Deinitialinitializing switches")
        for name, switch in self.switches.iteritems():
            self.connector.requestAndCheck('pin', { 'act': 'dmode', 'mode': 'input', 'pin': switch['pin'] })

    def getSwitchValue(self, name):
        switch = self.switches[name]
        return switch['value']

    def setSwitchValue(self, name, value):
        switch = self.switches[name]
        realValue = value
        if switch['invert']:
            realValue = not realValue

        realValue = 1 if realValue else 0

        self.connector.requestAndCheck('pin', {'act': 'dwrite', 'value': realValue, 'pin': switch['pin']})
        switch['value'] = value

    def registerControls(self, addControlFunction):
        for name, switch in self.switches.iteritems():
            addControlFunction(name, self.getSwitchValue, self.setSwitchValue, "switch")

    def showMessage(self, message):
        self.connector.requestAndCheck('display', {'act': 'message', 'text': message})

    def showAlarm(self, alarm):
        self.connector.requestAndCheck('display', {'act': 'alarm', 'text': alarm})

    def start(self):
        if self.connector is not None:
            raise RuntimeError("Arduino controller already started")

        self.logger.info("Starting")
        self.connector = SerialClientConnector(self.port, self.speed, self.timeout, INITIAL_TIMEOUT)
        self.sendInitRequests()
        self.initSwitches()

        self.logger.info("Started")

    def stop(self):
        if self.connector is None:
            return

        self.logger.info("Stopping")
        self.deinitSwitches()
        self.sendDeinitRequests()

        self.connector.dispose()
        self.connector = None
        self.logger.info("Stopped")
