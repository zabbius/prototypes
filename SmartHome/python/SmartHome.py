#!/usr/bin/python2
# -*- coding: utf-8 -*-

import logging

from ArduinoController import ArduinoController
from EventManager import EventManager, EventCommandHandler
from SwitchController import SwitchController, SwitchCommandHandler
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

        self.switchController = SwitchController(config['SwitchController'])
        self.switchCommandHandler = SwitchCommandHandler(self.switchController)

        self.httpHandler.setHandler('switch', self.switchCommandHandler.handleCommand)

        self.http_port = int(config['http_port'])
        self.httpServer = DispatchedBackgroundHTTPServer(('', self.http_port),
                                                         self.httpHandler.getRequestHandlerClass(), self.httpDispatcher)
        self.httpServer.allow_reuse_address = True

        self.arduinoController = ArduinoController(config['ArduinoController'])

        self.eventManager = EventManager(config['EventManager'])
        self.eventCommandHandler = EventCommandHandler(self.eventManager)

        self.httpHandler.setHandler('event', self.eventCommandHandler.handleCommand)

    def start(self):
        self.logger.info("Starting")
        self.httpDispatcher.start()
        self.httpServer.startServer()

        self.arduinoController.start()

        self.switchController.addAll(self.arduinoController.getSwitches().keys(), self.arduinoController.setSwitch,
                                     self.arduinoController.getSwitch, 'arduino')

        self.switchController.start()

        self.eventManager.start()

        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.eventManager.stop()

        self.arduinoController.stop()

        self.httpServer.stopServer()
        self.httpDispatcher.stop()
        self.logger.info("Stopped")

    def getStatus(self):
        status = { 'http_port': self.http_port,
                   'SwitchController': self.switchController.getStatus() }
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
