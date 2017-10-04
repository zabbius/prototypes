#!/usr/bin/python2
# -*- coding: utf-8 -*-

import logging

from ArduinoController import ArduinoController
from ControlManager import ControlCommandHandler, ControlManager
from EventManager import EventManager, EventCommandHandler
from SensorManager import SensorManager, SensorCommandHandler
from ShellControlProvider import ShellControlProvider
from ShellEventHandlerProvider import ShellEventHandlerProvider
from ShellSensorProvider import ShellSensorProvider
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

        self.controlManager = ControlManager(config['ControlManager'])
        self.controlCommandHandler = ControlCommandHandler(self.controlManager)
        self.httpHandler.setHandler('control', self.controlCommandHandler.handleCommand)

        self.sensorManager = SensorManager(config['SensorManager'])
        self.sensorCommandHandler = SensorCommandHandler(self.sensorManager)
        self.httpHandler.setHandler('sensor', self.sensorCommandHandler.handleCommand)

        self.http_port = int(config['http_port'])
        self.httpServer = DispatchedBackgroundHTTPServer(('', self.http_port),
                                                         self.httpHandler.getRequestHandlerClass(), self.httpDispatcher)
        self.httpServer.allow_reuse_address = True

        self.arduinoController = ArduinoController(config['ArduinoController'])

        self.shellSensorProvider = ShellSensorProvider(config['ShellSensorProvider'])
        self.shellControlProvider = ShellControlProvider(config['ShellControlProvider'])

        self.eventManager = EventManager(config['EventManager'])
        self.eventCommandHandler = EventCommandHandler(self.eventManager)

        self.shellEventHandlerProvider = ShellEventHandlerProvider(config['ShellEventHandlerProvider'])
        self.httpHandler.setHandler('event', self.eventCommandHandler.handleCommand)

    def start(self):
        self.logger.info("Starting")
        self.httpDispatcher.start()
        self.httpServer.startServer()

        self.arduinoController.start()
        self.shellSensorProvider.start()
        self.shellControlProvider.start()

        self.arduinoController.registerControls(self.controlManager.addControl)
        self.shellControlProvider.registerControls(self.controlManager.addControl)

        self.shellSensorProvider.registerSensors(self.sensorManager.addSensor)

        self.controlManager.start()
        self.sensorManager.start()

        self.shellEventHandlerProvider.registerEventHandlers(self.eventManager.addEventHandler)

        self.eventManager.start()

        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.httpServer.stopServer()
        self.httpDispatcher.stop()

        self.eventManager.stop()

        self.controlManager.stop()
        self.sensorManager.stop()

        self.arduinoController.stop()
        self.shellSensorProvider.stop()
        self.shellControlProvider.stop()

        self.logger.info("Stopped")

    def getStatus(self):
        status = { 'http_port': self.http_port,
                   'ControlManager': self.controlManager.getStatus(),
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
