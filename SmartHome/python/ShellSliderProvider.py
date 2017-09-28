# -*- coding: utf-8 -*-
import copy
import logging
import subprocess


class ShellSliderProvider:
    def __init__(self, config):
        self.logger = logging.getLogger("ShellSliderProvider")
        self.config = config

        self.sliders = copy.deepcopy(self.config['sliders'])

    def registerSliders(self, addSliderFunction):
        for name, slider in self.sliders.iteritems():
            addSliderFunction(name, self.getSliderValue, self.setSliderValue, "shell")

    def getSliderValue(self, name):
        slider = self.sliders[name]
        return slider['value']

    def setSliderValue(self, name, value):
        slider = self.sliders[name]
        command = slider['command']

        if value < slider['min']:
            value = slider['min']

        if value > slider['max']:
            value = slider['max']

        process = subprocess.Popen(command, stdin=subprocess.PIPE, shell=True)
        out, err = process.communicate(str(value))

        if process.returncode != 0:
            raise Exception("Slider command {0} returned code {1}".format(command, process.returncode))

        slider['value'] = value

    def initSliders(self):
        self.logger.info("Initializing sliders")
        for name, slider in self.sliders.iteritems():
            self.setSliderValue(name, slider['value'])

    def start(self):
        self.logger.info("Starting")
        self.initSliders()
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.logger.info("Stopped")

