# -*- coding: utf-8 -*-
import json
import logging

import sqlite3
import datetime
import threading
import traceback


from python_modules.common_utils import SafeTimer

EVENT_STATE_NEW = 'NEW'
EVENT_STATE_HANDLED = 'HANDLED'
EVENT_STATE_HANDLING = 'HANDLING'
EVENT_STATE_ERROR = 'ERROR'


class EventManager:
    def __init__(self, config):
        self.logger = logging.getLogger("EventManager")
        self.config = config

        self.dbPath = self.config['database']
        self.db = None
        self.dbLock = threading.Lock()

        self.cleanTimer = SafeTimer(self.onCleanTimer, config['clean_interval'], 'event_clean_timer')
        self.handleTimer = SafeTimer(self.onHandleTimer, config['handle_interval'], 'event_handle_timer')

        self.handlers = set()

    def addEventHandler(self, handler, type=None, name=None):
        self.handlers.add((type, name, handler))

    def delEventHandler(self, handler, type=None, name=None):
        self.handlers.remove((type, name, handler))

    def clearEventHandlers(self):
        self.handlers.clear()

    def executeQuery(self, *args, **kwargs):
        with self.dbLock:
            result = self.db.execute(*args, **kwargs)
            self.db.commit()
            return result

    def handleEvent(self, event):
        self.executeQuery("UPDATE event SET state = ?;", (EVENT_STATE_HANDLING,))

        success = True

        for type, name, handler in self.handlers:
            if type and type != event['type']:
                continue
            if name and name != event['name']:
                continue

            self.logger.debug("Handling event {0} with handler {1}".format(event, handler))

            try:
                handler(event)
            except Exception as e:
                self.logger.error("Exception caught while handling event {0}: {1}\n{2}"
                                  .format(event, e, traceback.format_exc()))
                success = False

        if success:
            self.executeQuery("UPDATE event SET state = ?;", (EVENT_STATE_HANDLED,))
        else:
            self.executeQuery("UPDATE event SET state = ?, retries = retries + 1;", (EVENT_STATE_ERROR,))

    def onHandleTimer(self):
        self.logger.debug("Handling new events")
        events = self.fetchEvents(
            self.executeQuery("SELECT * FROM event WHERE state NOT IN (?, ?);",
                            (EVENT_STATE_HANDLED, EVENT_STATE_ERROR)))

        for event in events:
            self.handleEvent(event)

        self.logger.debug("Retrying to handle error events")
        events = self.fetchEvents(
            self.executeQuery("SELECT * FROM event WHERE state = ? AND retries < ?;",
                            (EVENT_STATE_ERROR, self.config['handle_retry_count'])))

        for event in events:
            self.handleEvent(event)

    def onCleanTimer(self):
        thresholdTime = datetime.datetime.now() - datetime.timedelta(days=self.config['event_store_days'])
        self.logger.debug("Deleting old events")
        self.executeQuery("DELETE FROM event WHERE time < ?;", (thresholdTime,))

    def addEvent(self, type, name, data=None):
        if data is not None:
            data = json.dumps(data)

        time = datetime.datetime.now()
        self.executeQuery("INSERT INTO event (type, name, data, time, state) VALUES (?, ?, ?, ?, ?)",
                        (type, name, data, time, EVENT_STATE_NEW))

    def fetchEvents(self, cursor):
        result = []
        for row in cursor:
            event = {'id': row['id'], 'type': row['type'], 'name': row['name'],
                     'time': row['time'], 'data': row['data'], 'state': row['state'], 'retries': row['retries']}

            if event['data']:
                event['data'] = json.loads(event['data'])

            result.append(event)

        return result

    def getAllEvents(self):
        return self.fetchEvents(self.executeQuery("SELECT * FROM event ORDER BY id DESC;"))

    def getEventsByFilter(self, type = None, name = None):
        query = "SELECT * FROM event WHERE 1"
        params = {}

        if type is not None:
            query += " AND type = :type"
            params['type'] = type

        if name is not None:
            query += " AND name = :name"
            params['name'] = name

        query += " ORDER BY id DESC;"

        return self.fetchEvents(self.executeQuery(query, params))

    def start(self):
        self.logger.info("Starting")
        self.logger.debug("Connecting to {0}".format(self.dbPath))
        self.db = sqlite3.connect(self.dbPath, check_same_thread=False)
        self.db.row_factory = sqlite3.Row
        self.handleTimer.start(False)
        self.cleanTimer.start(False)
        self.logger.info("Started")

    def stop(self):
        self.logger.info("Stopping")
        self.clearEventHandlers()
        self.handleTimer.stop()
        self.cleanTimer.stop()
        if self.db:
            self.db.close()
            self.db = None

        self.logger.info("Stopped")

    def getStatus(self):
        return {}


class EventCommandHandler:
    def __init__(self, eventManager):
        self.eventManager = eventManager

    def handleCommand(self, cmd, args):
        if cmd != 'event':
            raise NotImplementedError("Wrong command: {0}".format(cmd))

        action = args.get('act')

        if action == 'add':
            type = args.get('type')
            name = args.get('name')

            data = args.get('_httpBody', None)
            if data:
                try:
                    jsonData = json.loads(data)
                    data = jsonData
                except:
                    pass

            self.eventManager.addEvent(type, name, data)
            return True

        elif action == 'get':
            type = args.get('type', None)
            name = args.get('name', None)

            events = self.eventManager.getEventsByFilter(type, name)

            return { 'events': events }

        elif action == 'status':
            return {'eventStatus': self.eventManager.getStatus()}
        elif action == 'start':
            self.eventManager.start()
            return True
        elif action == 'stop':
            self.eventManager.stop()
            return True
        else:
            raise NotImplementedError("Wrong action: {0}".format(action))
