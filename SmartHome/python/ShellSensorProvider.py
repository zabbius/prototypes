# -*- coding: utf-8 -*-
import copy
import logging
import subprocess


class ShellSensorProvider:
    def __init__(self, config):
        self.logger = logging.getLogger("ShellSensorProvider")
        self.config = config
        self.sensors = self.config['sensors']

    def registerSensors(self, addSwitchFunction):
        for name, sensor in self.sensors.iteritems():
            sensorInfo = copy.copy(sensor)
            sensorInfo.pop('interval', None)
            sensorInfo.pop('command', None)
            addSwitchFunction(name, self.getSensorValue, sensor.get('interval', None), "shell", sensorInfo)

    def getSensorValue(self, name):
        sensor = self.sensors[name]
        command = sensor['command']

        process = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
        out, err = process.communicate()

        if process.returncode != 0:
            raise Exception("Sensor command {0} returned code {1}".format(command, process.returncode))

        return out

    def start(self):
        self.logger.info("Starting")
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.logger.info("Stopped")

