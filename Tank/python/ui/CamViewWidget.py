from PyQt4 import QtGui
from PyQt4 import QtCore

import time

from CamViewWidgetUi import *


# noinspection PyAttributeOutsideInit
class CamViewWidget(QtGui.QWidget):
    def __init__(self, videoClient, parent=None):
        super(CamViewWidget, self).__init__(parent)
        self.ui = Ui_CamViewWidget()
        self.ui.setupUi(self)

        self.connect(self, QtCore.SIGNAL('onImage(QImage)'), self.setImage)
        self.connect(self, QtCore.SIGNAL('onFps(int)'), self.setFps)

        self.videoClient = videoClient

        self.connect(self.ui.comboDevice, QtCore.SIGNAL('currentIndexChanged(int)'), self.deviceChanged)
        self.connect(self.ui.comboMode, QtCore.SIGNAL('currentIndexChanged(int)'), self.modeChanged)

        self.connect(self.ui.btnStartStop, QtCore.SIGNAL('clicked()'), self.startStop)
        self.refreshDevices()
        self.started = False

    def refreshDevices(self):
        self.videoClient.refreshStatus()

        self.ui.comboDevice.clear()
        self.devices = []

        for (name, device) in self.videoClient.getDevices().iteritems():
            self.devices.append(device)
            self.ui.comboDevice.addItem(name)

    def deviceChanged(self, index):
        self.deviceName = str(self.ui.comboDevice.itemText(index))
        self.device = self.devices[index]

        self.ui.comboMode.clear()
        for mode in self.device['modes']:
            self.ui.comboMode.addItem("{0}x{1}, max {2} fps".format(mode['width'], mode['height'], mode['maxfps']))

    def modeChanged(self, index):
        self.mode = self.device['modes'][index]

        self.ui.spinFps.setMaximum(int(self.mode['maxfps']))

    def startStop(self):
        if self.started:
            self.stop()
        else:
            self.start()

    def start(self):
        if self.started:
            return

        name = self.deviceName
        width = self.mode['width']
        height = self.mode['height']
        fps = self.ui.spinFps.value()

        self.videoClient.startCapture(name, width, height, fps, self.onFrame)

        self.started = True
        self.ui.comboMode.setDisabled(self.started)
        self.ui.comboDevice.setDisabled(self.started)
        self.ui.spinFps.setDisabled(self.started)
        self.ui.btnStartStop.setText("Stop")

        self.frameCount = 0
        self.tick = time.time()

    def stop(self):
        if not self.started:
            return

        name = self.deviceName

        self.videoClient.stopCapture(name)

        self.started = False
        self.ui.comboMode.setDisabled(self.started)
        self.ui.comboDevice.setDisabled(self.started)
        self.ui.spinFps.setDisabled(self.started)
        self.ui.btnStartStop.setText("Start")

    def closeEvent(self, event):
        self.stop()
        QtGui.QWidget.closeEvent(self, event)

    def onFrame(self, frame):

        self.frameCount += 1
        tick = time.time()
        interval = tick - self.tick
        if interval > 1:
            self.emit(QtCore.SIGNAL('onFps(int)'), self.frameCount)
            self.frameCount = 0
            self.tick = tick

        image = QtGui.QImage.fromData(frame, "jpeg")

        self.emit(QtCore.SIGNAL('onImage(QImage)'), image)

    def setFps(self, fps):
        if not self.started:
            return

        self.ui.labelStatus.setText("Current FPS: {0}".format(fps))

    def setImage(self, image):
        if not self.started:
            return

        pixmap = QtGui.QPixmap.fromImage(image)
        self.ui.labelImage.setPixmap(pixmap)
