#!/usr/bin/env python
"""
Simple example of sending a counter via TCP to Pure Data on localhost. In PD the
[netreceive] object must use the same port specified by TCP_PORT. Python sockets
part based on Python tcp documentation.
"""
import socket
import time
import sys

# TCP stuff
TCP_IP = '127.0.0.1'
TCP_PORT = 54321
BUFFER_SIZE = 1024
# A counter
counter = 0

# Try to open the connection to PD
print ("Opening connection on port %d") % (TCP_PORT)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print ("Connencting...")
try:
    s.connect((TCP_IP, TCP_PORT))
    print "Connected. Press CTRL + C to exit."
except socket.error as err:
    # Exit on error
    print "Could not connect. Aborting.\nError: %s" % (err)
    sys.exit(1)

# A dictionary for possibly multiple messages
messages = {}
s.send ("Hello from Python!;")

while True:
    try:
        messages['count'] = counter
        messages['foo'] = counter * 2
        for k in messages.keys():
            # Here the semicolon (";") is important!! See http://en.wikipedia.org/wiki/FUDI
            message_string = ("%s %d;") % (k, messages[k])
            print "Sending %s" % (message_string)
            s.send(message_string)
        counter += 1
        time.sleep (1)
    except KeyboardInterrupt:
        # Press CTRL + C to stop.
        s.send("Bye from python;")
        time.sleep(0.2)
        print "Closing connection"
        s.close()
        break
try:
    s.close()
except:
    pass

