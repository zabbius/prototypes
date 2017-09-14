#!/usr/bin/python2

import serial
import sys

ser = serial.Serial('/dev/ttyUSB0', 19200, timeout=0.1)

ser.write(sys.argv[1] + "\r\n")
print ser.read(20480)
