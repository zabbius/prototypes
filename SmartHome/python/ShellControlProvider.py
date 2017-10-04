# -*- coding: utf-8 -*-
import copy
import logging
import subprocess


class ShellControlProvider:
    def __init__(self, config):
        self.logger = logging.getLogger("ShellControlProvider")
        self.config = config

        self.controls = copy.deepcopy(self.config['controls'])

        for name, control in self.controls.iteritems():
            if control['type'] == 'switch':
                control['prepare'] = self.prepareSwitchValue
            elif control['type'] == 'slider':
                control['prepare'] = self.prepareSliderValue
            elif control['type'] == 'text':
                control['prepare'] = self.prepareTextValue
            else:
                raise RuntimeError("Wrong control type : {0}".format(control['type']))

    def registerControls(self, addControlFunction):
        for name, control in self.controls.iteritems():
            if control['type'] in ['switch', 'text']:
                addControlFunction(name, self.getControlValue, self.setControlValue, control['type'])

            elif control['type'] == 'slider':
                addControlFunction(name, self.getControlValue, self.setControlValue, control['type'],
                                   min=control['min'], max=control['max'])

            else:
                raise RuntimeError("Wrong control type : {0}".format(control['type']))

    def getControlValue(self, name):
        control = self.controls[name]
        return control['value']

    def setControlValue(self, name, value):
        control = self.controls[name]

        value, valueToSend = control['prepare'](control, value)

        command = control['command']
        process = subprocess.Popen(command, stdin=subprocess.PIPE, shell=True)
        out, err = process.communicate(valueToSend)

        if process.returncode != 0:
            raise RuntimeError("Control command {0} returned code {1}".format(command, process.returncode))

        control['value'] = value

    @staticmethod
    def prepareSwitchValue(control, value):

        if control['type'] != 'switch':
            raise RuntimeError("Wrong control type : {0}".format(control['type']))

        if isinstance(value, str):
            value = value.lower() in ("yes", "true", "1")
        elif not isinstance(value, bool):
            value = bool(value)

        sendValue = value

        if control['invert']:
            sendValue = not sendValue

        sendValue = "1" if sendValue else "0"

        return value, sendValue


    @staticmethod
    def prepareSliderValue(control, value):
        value = int(value)

        if value < control['min']:
            value = control['min']

        if value > control['max']:
            value = control['max']

        return value, str(value)

    @staticmethod
    def prepareTextValue(control, value):
        if isinstance(value, str):
            return value, value

        return str(value), str(value)

    def initControls(self):
        self.logger.info("Initializing controls")
        for name, control in self.controls.iteritems():
            self.setControlValue(name, control['value'])

    def start(self):
        self.logger.info("Starting")
        self.initControls()
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.logger.info("Stopped")

