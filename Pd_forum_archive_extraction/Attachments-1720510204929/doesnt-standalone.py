from testclass import *

orchestra1 = [['x1',['a',1]],['x2',['c',11]]]
orchestra2 = [['y1',['e',2]],['y2',['g',22]]]
orchestra = []
        

class Test(object):
    
    def __init__(self, *args):
        global orchestra
        print str(args[0])
        orchestra = eval(str(args[0]))
        print orchestra
        for i in orchestra:
            j = (i[0], i[1][0], i[1][1])
            exec str(i[0])+' = Instrument'+str(j)
    
    def float_1(self,f):
        global orchestra
        if f > 10:
            print str(orchestra[0][0])+'.setVal('+str(f)+')'
            exec str(orchestra[0][0])+'.setVal('+str(f)+')'
        else:
            print str(orchestra[1][0])+'.setVal('+str(f)+')'
            exec str(orchestra[1][0])+'.setVal('+str(f)+')'

t = Test(orchestra1)
t.float_1(9)
