# -*- coding: utf-8 -*-
import logging


class ControlManager:
    def __init__(self, config):
        self.logger = logging.getLogger("ControlManager")
        self.config = config
        self.controls = {}

    def addControl(self, name, getFunction, setFunction, type=None, **kwargs):
        if name in self.controls:
            raise RuntimeError("Control {0} already exists".format(name))

        self.controls[name] = { 'set': setFunction, 'get': getFunction, 'type': type, 'info': kwargs }

    def delControl(self, name):
        if name not in self.controls:
            raise RuntimeError("Wrong control name: {0}".format(name))

        self.controls.pop(name)

    def setControlValue(self, name, value):
        if name not in self.controls:
            raise RuntimeError("Wrong control name: {0}".format(name))

        control = self.controls.get(name)
        control['set'](name, value)

    def getControlValue(self, name):
        if name not in self.controls:
            raise RuntimeError("Wrong control name: {0}".format(name))

        control = self.controls.get(name)
        value = control['get'](name)

        return value

    def getControlStatus(self, name):
        if name not in self.controls:
            raise RuntimeError("Wrong control name: {0}".format(name))

        control = self.controls.get(name)
        value = control['get'](name)

        status = { 'name': name, 'value': value, 'type': control['type'] }
        for k, v in control['info'].iteritems():
            if k not in status:
                status[k] = v

        return status

    def start(self):
        self.logger.info("Starting")
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.clearControls()
        self.logger.info("Stopped")

    def clearControls(self):
        self.logger.info("Clearing controls")
        self.controls.clear()

    def getStatus(self):
        controlStatus = {}

        for name in self.controls.iterkeys():
            controlStatus[name] = self.getControlStatus(name)

        return controlStatus


class ControlCommandHandler:
    def __init__(self, controlManager):
        self.controlManager = controlManager

    def handleCommand(self, cmd, args):
        if cmd != 'control':
            raise NotImplementedError("Wrong command: {0}".format(cmd))

        action = args.get('act')

        if action == 'set':
            name = args.get('name')
            value = args.get('value')

            self.controlManager.setControlValue(name, value)

            if args.get('_protocol', '') == 'udp':
                return {'_noAnswer': True}

            return True

        elif action == 'get':
            name = args.get('name')
            return self.controlManager.getControlStatus(name)

        elif action == 'status':
            return {'controlStatus': self.controlManager.getStatus()}

        elif action == 'start':
            self.controlManager.start()
            return True

        elif action == 'stop':
            self.controlManager.stop()
            return True

        else:
            raise NotImplementedError("Wrong action: {0}".format(action))
