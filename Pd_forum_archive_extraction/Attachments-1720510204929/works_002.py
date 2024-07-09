from testclass import *

orchestra = [['x1',['a',1]],['x2',['c',11]]]

for i in orchestra:
    j = (i[0], i[1][0], i[1][1])
    print str(i[0])+' = Instrument'+str(j)
    exec  str(i[0])+' = Instrument'+str(j)

class Test(object):

    def float_1(self,f):
        if f > 10:
            print str(orchestra[0][0])+'.setVal('+str(f)+')'
            exec  str(orchestra[0][0])+'.setVal('+str(f)+')'
        else:
            print str(orchestra[1][0])+'.setVal('+str(f)+')'
            exec  str(orchestra[1][0])+'.setVal('+str(f)+')'


t = Test()
t.float_1(9)
t.float_1(11)

