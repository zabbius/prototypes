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

    def addSensor(self, name, getFunction, interval=None, type=None, info=None):
        if name in self.sensors:
            raise RuntimeError("Sensor {0} already exists".format(name))

        if interval is not None:
            interval = datetime.timedelta(seconds=interval)

        if not info:
            info = {}

        sensor = { 'get': getFunction, 'interval': interval, 'value': None,
                   'last_update': None, 'type': type, 'info': info }

        self.sensors[name] = sensor

    def delSensor(self, name):
        if name not in self.sensors:
            raise RuntimeError("Wrong sensor name: {0}".format(name))

        self.sensors.pop(name)

    def updateSensor(self, name, sensor):
        self.logger.debug("Updating sensor {0}".format(name))

        now = datetime.datetime.now()

        if sensor['interval'] is not None and sensor['last_update'] is not None:
            nextTime = sensor['last_update'] + sensor['interval']
            if nextTime < now:
                return
        try:
            sensor['value'] = sensor['get'](name)
            sensor['last_update'] = now

        except Exception as e:
            sensor['value'] = None
            self.logger.error("Exception caught while updating sensor {0}: {1}\n{2}"
                              .format(name, e, traceback.format_exc()))

    def onUpdateTimer(self):
        self.logger.debug("Updating sensors")

        for name, sensor in self.sensors.iteritems():
            self.updateSensor(name, sensor)

    def getSensorValue(self, name):
        if name not in self.sensors:
            raise RuntimeError("Wrong sensor name: {0}".format(name))

        sensor = self.sensors.get(name)

        return sensor['value']

    def start(self):
        self.logger.info("Starting")
        self.updateTimer.start(True)
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.updateTimer.stop()
        self.clearSensors()
        self.logger.info("Stopped")

    def clearSensors(self):
        self.logger.info("Clearing sensors")
        self.sensors.clear()

    def getStatus(self):
        sensorStatus = {}

        for name, sensor in self.sensors.iteritems():
            sensorStatusItem = { 'value': sensor['value'],
                                 'last_update': str(sensor['last_update'])}

            for k, v in sensor['info'].iteritems():
                if k not in sensorStatusItem:
                    sensorStatusItem[k] = v

            sensorStatus[name] = sensorStatusItem

        return sensorStatus


class SensorCommandHandler:
    def __init__(self, sensorManager):
        self.sensorManager = sensorManager

    def handleCommand(self, cmd, args):
        if cmd != 'sensor':
            raise NotImplementedError("Wrong command: {0}".format(cmd))

        action = args.get('act')

        if action == 'get':
            name = args.get('name')
            value = self.sensorManager.getSensorValue(name)
            return {'sensorName': name, 'sensorValue': value}

        elif action == 'status':
            return {'sensorStatus': self.sensorManager.getStatus()}

        elif action == 'start':
            self.sensorManager.start()
            return True
        elif action == 'stop':
            self.sensorManager.stop()
            return True

        else:
            raise NotImplementedError("Wrong action: {0}".format(action))
