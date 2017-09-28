# -*- coding: utf-8 -*-
import logging


class SliderManager:
    def __init__(self, config):
        self.logger = logging.getLogger("SliderManager")
        self.config = config
        self.sliders = {}

    def addSlider(self, name, getFunction, setFunction, type=None):
        if name in self.sliders:
            raise RuntimeError("Slider {0} already exists".format(name))

        self.sliders[name] = {'set': setFunction, 'get': getFunction, 'type': type}

    def delSlider(self, name):
        if name not in self.sliders:
            raise RuntimeError("Wrong slider name: {0}".format(name))

        self.sliders.pop(name)

    def setSliderValue(self, name, value):
        if name not in self.sliders:
            raise RuntimeError("Wrong slider name: {0}".format(name))

        slider = self.sliders.get(name)
        slider['set'](name, value)

    def getSliderValue(self, name):
        if name not in self.sliders:
            raise RuntimeError("Wrong slider name: {0}".format(name))

        slider = self.sliders.get(name)
        value = slider['get'](name)

        return value

    def start(self):
        self.logger.info("Starting")
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.clearSliders()
        self.logger.info("Stopped")

    def clearSliders(self):
        self.logger.info("Clearing sliders")
        self.sliders.clear()

    def getStatus(self):
        sliderStatus = {}

        for name, slider in self.sliders.iteritems():
            sliderStatus[name] = {'type': slider['type'], 'value': self.getSliderValue(name)}

        return sliderStatus


class SliderCommandHandler:
    def __init__(self, sliderManager):
        self.sliderManager = sliderManager

    def handleCommand(self, cmd, args):
        if cmd != 'slider':
            raise NotImplementedError("Wrong command: {0}".format(cmd))

        action = args.get('act')

        if action == 'set':
            name = args.get('name')
            value = int(args.get('value'))

            self.sliderManager.setSliderValue(name, value)

            if args.get('_protocol', '') == 'udp':
                return {'_noAnswer': True}

            return True

        elif action == 'get':
            name = args.get('name')
            value = self.sliderManager.getSliderValue(name)
            return {'sliderName': name, 'sliderValue': value}

        elif action == 'status':
            return {'sliderStatus': self.sliderManager.getStatus()}

        elif action == 'start':
            self.sliderManager.start()
            return True
        elif action == 'stop':
            self.sliderManager.stop()
            return True

        else:
            raise NotImplementedError("Wrong action: {0}".format(action))
