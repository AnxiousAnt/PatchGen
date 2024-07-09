#!/usr/bin/python
# 
#This is public domain


import sys, osc

osc.init()

osc.sendMsg("/sip",[int(sys.argv[3])],sys.argv[1],int(sys.argv[2]))
