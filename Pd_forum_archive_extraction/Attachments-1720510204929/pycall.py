import pyext
import string

class pymod(pyext._class):
    
    _inlets = 1
    _outlets = 1
    
    # constructor
    def __init__(self, mod):
        mod = str(mod)
        self.mod  = __import__(mod, globals(),  locals(), [])
        self.mod_string = mod
    
    def _anything_1(self, func, *args):
        """inlet to accept everything"""
        # TODO this needs proper error checking ... or maybe not.
        func = str(func)
        args = [str(x) for x in args]
        
        # HACK: replace the string "[BS]" with a backslash, that cannot be input in Pd:
        args = [string.replace(x, "[BS]",'\\') for x in args]
        
        if hasattr(self.mod, func):
            result = None
            if callable(getattr(self.mod, func)):
                result = getattr(self.mod, func)(*args)
            else:
                result = getattr(self.mod, func)
            self._outlet(1, result)
        else:
            print "No such attribute: %s.%s" % (self.mod_string, func)
