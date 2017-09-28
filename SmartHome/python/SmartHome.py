#!/usr/bin/python2
# -*- coding: utf-8 -*-

import logging

from ArduinoController import ArduinoController
from EventManager import EventManager, EventCommandHandler
from SensorManager import SensorManager, SensorCommandHandler
from ShellSensorProvider import ShellSensorProvider
from ShellSliderProvider import ShellSliderProvider
from ShellSwitchProvider import ShellSwitchProvider
from SliderManager import SliderManager, SliderCommandHandler
from SwitchManager import SwitchManager, SwitchCommandHandler
from python_modules.command_utils import HTTPCommandHandler
from python_modules.server_utils import DispatchedBackgroundHTTPServer
from python_modules.threading_utils import MultiThreadingDispatcher


class SmartHome:
    def __init__(self, config):
        self.logger = logging.getLogger("SmartHome")
        self.config = config

        httpThreadsNumber = int(config['http_threads_number'])
        self.httpDispatcher = MultiThreadingDispatcher("HttpDispatcher", httpThreadsNumber)

        self.httpHandler = HTTPCommandHandler()

        self.httpHandler.setHandler('ping', self.pingHandler)
        self.httpHandler.setHandler('status', self.statusHandler)

        self.switchManager = SwitchManager(config['SwitchManager'])
        self.switchCommandHandler = SwitchCommandHandler(self.switchManager)
        self.httpHandler.setHandler('switch', self.switchCommandHandler.handleCommand)

        self.sliderManager = SliderManager(config['SliderManager'])
        self.sliderCommandHandler = SliderCommandHandler(self.sliderManager)
        self.httpHandler.setHandler('slider', self.sliderCommandHandler.handleCommand)

        self.sensorManager = SensorManager(config['SensorManager'])
        self.sensorCommandHandler = SensorCommandHandler(self.sensorManager)
        self.httpHandler.setHandler('sensor', self.sensorCommandHandler.handleCommand)

        self.http_port = int(config['http_port'])
        self.httpServer = DispatchedBackgroundHTTPServer(('', self.http_port),
                                                         self.httpHandler.getRequestHandlerClass(), self.httpDispatcher)
        self.httpServer.allow_reuse_address = True

        self.arduinoController = ArduinoController(config['ArduinoController'])

        self.shellSensorProvider = ShellSensorProvider(config['ShellSensorProvider'])
        self.shellSwitchProvider = ShellSwitchProvider(config['ShellSwitchProvider'])
        self.shellSliderProvider = ShellSliderProvider(config['ShellSliderProvider'])

        self.eventManager = EventManager(config['EventManager'])
        self.eventCommandHandler = EventCommandHandler(self.eventManager)

        self.httpHandler.setHandler('event', self.eventCommandHandler.handleCommand)

    def start(self):
        self.logger.info("Starting")
        self.httpDispatcher.start()
        self.httpServer.startServer()

        self.arduinoController.start()
        self.shellSensorProvider.start()
        self.shellSwitchProvider.start()
        self.shellSliderProvider.start()

        self.arduinoController.registerSwitches(self.switchManager.addSwitch)

        self.shellSwitchProvider.registerSwitches(self.switchManager.addSwitch)
        self.shellSliderProvider.registerSliders(self.sliderManager.addSlider)
        self.shellSensorProvider.registerSensors(self.sensorManager.addSensor)

        self.switchManager.start()
        self.sliderManager.start()
        self.sensorManager.start()

        self.eventManager.start()

        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.httpServer.stopServer()
        self.httpDispatcher.stop()

        self.eventManager.stop()

        self.switchManager.stop()
        self.sliderManager.stop()
        self.sensorManager.stop()

        self.arduinoController.stop()
        self.shellSensorProvider.stop()
        self.shellSwitchProvider.stop()
        self.shellSliderProvider.stop()

        self.logger.info("Stopped")

    def getStatus(self):
        status = { 'http_port': self.http_port,
                   'SwitchManager': self.switchManager.getStatus(),
                   'SliderManager': self.sliderManager.getStatus(),
                   'SensorManager': self.sensorManager.getStatus(),
                   'EventManager':  self.eventManager.getStatus(),
                   }
        return status

    def statusHandler(self, cmd, args):
        return self.getStatus()

    def pingHandler(self, cmd, args):
        return True


if __name__ == "__main__":
    from python_modules.common_utils import ServiceLauncher


    class Launcher(ServiceLauncher):
        def createService(self, config):
            return SmartHome(config['SmartHome'])

        def addArgumentsToParser(self, parser):
            pass


    Launcher().Run()
