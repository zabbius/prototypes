# -*- coding: utf-8 -*-
import json
import logging
import subprocess


class ShellEventHandler:
    def __init__(self, eventManager, config):
        self.logger = logging.getLogger("ShellEventHandler")
        self.config = config

        self.eventManager = eventManager

        self.handlers = []

    def handle(self, event, script):

        self.logger.debug("Handling event {0} with script {1}".format(event, script))

        jsonData = json.dumps(event['data']) if event['data'] else None

        process = subprocess.Popen(script, event['type'], event['name'], stdin=subprocess.PIPE)
        process.communicate(jsonData)

        if process.returncode != 0:
            raise Exception("Handler script {0} returned code {1}".format(script, process.returncode))

    def start(self):
        self.logger.info("Starting")
        self.logger.debug("Registering event handlers")

        for handlerCfg in self.config['handlers']:
            type, name, script = handlerCfg['type'], handlerCfg['name'], handlerCfg['script']

            def handler(event):
                self.handle(event, script)

            self.eventManager.addEventHandler(handler, type, name)
            self.handlers.append((handler, type, name))

        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")

        self.logger.debug("Unregistering event handlers")
        for handler, type, name in self.handlers:
            self.eventManager.delEventHandler(handler, type, name)

        self.logger.info("Stopped")

    def getStatus(self):
        return {}
