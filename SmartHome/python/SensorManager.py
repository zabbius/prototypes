# -*- coding: utf-8 -*-
import copy
import logging
import traceback

import datetime

from python_modules.common_utils import SafeTimer


class SensorManager:
    def __init__(self, config):
        self.logger = logging.getLogger("SensorManager")
        self.config = config

        self.updateTimer = SafeTimer(self.onUpdateTimer, config['update_interval'], 'sensor_update_timer')

        self.sensors = {}

    def addSensor(self, id, name, handler, interval = None):
        if interval is not None:
            interval = datetime.timedelta(seconds=interval)

        self.sensors[id] = { 'id': id, 'name': name, 'handler': handler, 'interval': interval, 'value': None,
                             'last_update': None }

    def delSensor(self, id):
        del self.sensors['id']

    def updateSensor(self, sensor):
        self.logger.debug("Updating sensor {0}".format(sensor['name']))

        now = datetime.datetime.now()

        if sensor['interval'] is not None:
            nextTime = sensor['last_update'] + sensor['interval']
            if nextTime < now:
                return
        try:
            sensor['value'] = sensor['handler']()
            sensor['last_update'] = now

        except Exception as e:
            sensor['value'] = None
            self.logger.error("Exception caught while updating sensor {0}: {1}\n{2}"
                              .format(sensor['id'], e, traceback.format_exc()))

    def onUpdateTimer(self):
        self.logger.debug("Updating sensors")

        for sensor in self.sensors:
            self.updateSensor(sensor)

    def start(self):
        self.logger.info("Starting")
        self.updateTimer.start(False)
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.updateTimer.stop()
        self.logger.info("Stopped")

    def getStatus(self):
        sensors = {}

        for id, sensor in self.sensors.iteritems():
            sensor = copy.copy(sensor)
            sensor['interval'] = sensor['interval'].total_seconds()
            sensor['last_update'] = str(sensor['last_update'])

            sensors[id] = sensor

        return sensors
