# -*- coding: utf-8 -*-
import logging
import subprocess


class ShellSensorProvider:
    def __init__(self, sensorManager, config):
        self.logger = logging.getLogger("ShellSensorProvider")
        self.config = config

        self.sensorManager = sensorManager

        self.sensors = self.config['sensors']

    def getSensorValue(self, id, script):
        process = subprocess.Popen(script, id, stdout=subprocess.PIPE)
        out, err = process.communicate()

        if process.returncode != 0:
            raise Exception("Sensor script {0} returned code {1}".format(script, process.returncode))

        return out


    def start(self):
        self.logger.info("Starting")
        self.logger.debug("Registering sensors")

        for id, sensor in self.sensors.iteritems:
            script = sensor['script']
            interval = sensor.get('interval', None)

            def handler():
                self.getSensorValue(id, script)

            self.sensorManager.addSensor(id, sensor['name'], handler, interval)

        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")

        self.logger.debug("Unregistering event handlers")
        for id in self.sensors.iterkeys():
            self.sensorManager.delSensor(id)

        self.logger.info("Stopped")

    def getStatus(self):
        return {}
