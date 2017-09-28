# -*- coding: utf-8 -*-
import json
import logging
import subprocess


class ShellEventHandlerProvider:
    def __init__(self,  config):
        self.logger = logging.getLogger("ShellEventHandlerProvider")
        self.config = config

    def handle(self, event, script):
        self.logger.debug("Handling event {0} with script {1}".format(event, script))

        jsonData = json.dumps(event['data']) if event['data'] else None

        process = subprocess.Popen(script, event['type'], event['name'], stdin=subprocess.PIPE)
        process.communicate(jsonData)

        if process.returncode != 0:
            raise Exception("Handler script {0} returned code {1}".format(script, process.returncode))

    def registerEventHandlers(self, addEventHandlerFunction):
        for handlerCfg in self.config['handlers']:
            type, name, script = handlerCfg['type'], handlerCfg['name'], handlerCfg['script']

            def handler(event):
                self.handle(event, script)

            addEventHandlerFunction(handler, type, name)

    def start(self):
        self.logger.info("Starting")
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.logger.info("Stopped")

