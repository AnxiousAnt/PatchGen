class Instrument:
    def __init__(self, name, q1, q2):
        self.name = name 
        self.quality1 = q1
        self.quality2 = q2
        self.val = 999
    def setVal(self, val):
        self.val = val
        print self.name, self.val
        return self.val
