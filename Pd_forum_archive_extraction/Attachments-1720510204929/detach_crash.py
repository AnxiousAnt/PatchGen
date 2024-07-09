import pyext
import glob, os
import random
import time

class creation(pyext._class):
    _inlets=1
    _outlets=0

    def __init__(self):
        #self._detach(1)
        pass
    
    def on_1(self):
        self._send('gemwin', 'border', (1,))
        self._send('gemwin', 'offset', (0, 0))
        self._send('gemwin', 'create', ())
        self._send('gemwin', (1,))

        
