#!/usr/bin/python

# module level:
for i in (1,):
    exec "mx = 1"
    my = 2

# fine:
print "mx: ", mx
print "my: ", my


# function level:

def func():
    for i in (1,):
        exec "fx = 1"
        fy = 2

func()

# Errors:
try:
    print "fx: ", fx
except NameError, e:
    print "NameError: ", e

try:
    print "fy: ", fy
except NameError, e:
    print "NameError: ", e
   

# class level

class cl(object):
        
    for i in (1,):
        exec "dx = 1"
        dy = 2

    def __init__(self):
        for i in (1,):
            exec "cx = 1"
            cy = 2
            print "cx inside __init__: ", cx

    def fail(self):
        try:
            print "cx: ", cx
        except NameError, e:
            print "NameError: ", e

        try:
            print "cy: ", cy
        except NameError, e:
            print "NameError: ", e
    
    # okay:
    print "dx: ", dx
    # fails:
    try:
        print "cx: ", cx
    except NameError, e:
        print "NameError @ class level: ", e
        print "Note: No 'global' here!"
   
   
c = cl()
c.fail()
