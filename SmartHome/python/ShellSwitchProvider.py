# -*- coding: utf-8 -*-
import copy
import logging
import subprocess


class ShellSwitchProvider:
    def __init__(self, config):
        self.logger = logging.getLogger("ShellSwitchProvider")
        self.config = config

        self.switches = copy.deepcopy(self.config['switches'])

    def registerSwitches(self, addSwitchFunction):
        for name, switch in self.switches.iteritems():
            addSwitchFunction(name, self.getSwitchValue, self.setSwitchValue, "shell")

    def getSwitchValue(self, name):
        switch = self.switches[name]
        return switch['value']

    def setSwitchValue(self, name, value):
        switch = self.switches[name]
        command = switch['command']

        realValue = value
        if switch['invert']:
            realValue = not realValue

        realValue = "1" if realValue else "0"

        process = subprocess.Popen(command, stdin=subprocess.PIPE, shell=True)
        out, err = process.communicate(realValue)

        if process.returncode != 0:
            raise Exception("Switch command {0} returned code {1}".format(command, process.returncode))

        switch['value'] = value

    def initSwitches(self):
        self.logger.info("Initializing switches")
        for name, switch in self.switches.iteritems():
            self.setSwitchValue(name, switch['value'])


    def start(self):
        self.logger.info("Starting")
        self.initSwitches()
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.logger.info("Stopped")

