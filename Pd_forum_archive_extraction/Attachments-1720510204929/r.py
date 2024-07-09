import pyext
class r(pyext._class):
    _inlets=0
    _outlets=1

    def __init__(self, *args):
        r = ''
        for arg in args:
            r += str(arg)+'-'
        self._bind(r[0:-1], self.recv)
        
    def recv(self, *args):
        if len(args)==2:
            self._outlet(1, str(args[0]), (str(args[1]),))
        elif len(args)==1:
            self._outlet(1, args)
        else:
            self._outlet(1, args)
        
