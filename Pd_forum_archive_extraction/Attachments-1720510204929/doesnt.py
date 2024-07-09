try:
	import pyext
except:
	print "ERROR: This script must be loaded by the PD/Max pyext external"
from testclass import *

orchestra1 = [['x1',['a',1]],['x2',['c',11]]]
orchestra2 = [['y1',['e',2]],['y2',['g',22]]]
orchestra = []

class Test(pyext._class):
    _inlets=1
    _outlets=1
    
    def __init__(self, *args):
        global orchestra
        print str(args[0])
        orchestra = eval(str(args[0]))
        for i in orchestra:
            j = (i[0], i[1][0], i[1][1])
            exec str(i[0])+' = Instrument'+str(j)
    
    def float_1(self,f):
        global orchestra
        if f > 10:
            exec str(orchestra[0][0])+'.setVal('+str(f)+')'
        else:
            exec str(orchestra[1][0])+'.setVal('+str(f)+')'

