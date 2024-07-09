#!/usr/bin/env python
"""
Simple example of sending a counter via TCP to Pure Data on localhost. In PD the
[netreceive] object must use the same port specified by TCP_PORT. Python sockets
part based on Python tcp documentation.
"""
import socket
import time
import sys
import random

# TCP stuff
TCP_IP = '127.0.0.1'
TCP_PORT = 54321
BUFFER_SIZE = 1024
INTERVAL_STEP = 0.001
INTERVAL_MAX = 0.12
INTERVAL_MIN = 0.01
CHOKE_POINT = 0.08
INTERVAL = INTERVAL_MAX # secondsA

# Try to open the connection to PD
print ("Opening connection on port %d") % (TCP_PORT)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print ("Connencting...")
try:
    s.connect((TCP_IP, TCP_PORT))
    print "Connected. Press CTRL + C to exit."
    s.send("python Hello from the python client")
except socket.error as err:
    # Exit on error
    print "Could not connect. Aborting.\nError: %s" % (err)
    sys.exit(1)

while True:
    try:
        # We send a 'begin' and 'end' which we route in Pd
        print "sending..."
        s.send("begin;")
        for x in range(0,10000):
            # Format messages ready for [tabwrite]: [value index(
            s.send( ("%f %d;") % (random.random(), x))
        time.sleep (INTERVAL)
        INTERVAL = INTERVAL - INTERVAL_STEP
        print("Interval is now %f" % (INTERVAL))
        s.send("end;")
        if INTERVAL < CHOKE_POINT:
            print('Pd is probably @@@@@ \033[91mCHOKING\033[0m @@@@ now ...')
        else:
            print('Pd is porbaly ok and !! \033[92mRESPONSIVE\033[0m ! now...')
        if (INTERVAL < INTERVAL_MIN) or (INTERVAL >= INTERVAL_MAX):
            INTERVAL_STEP = INTERVAL_STEP * -1
            print '\033[96m>>>>>>>>>> Inverting interval step... <<<<<\033[0m'
        s.send("end;")
    except KeyboardInterrupt:
        # Press CTRL + C to stop.
        s.send("python Bye from python;")
        time.sleep(0.2)
        print "Closing connection"
        s.close()
        break
try:
    s.close()
except:
    pass

