from time import time
import os
import os.path

from PodSix.Pd import Pd

start = time()
print "launching pd"
pd = Pd(nogui = False, open = os.path.join(os.getcwd(), "data", "test.pd"), port=30321)
def Start():
	pd.Send("My test message")
print "running a bunch of stuff for 30 seconds"
while time() - start < 30:
	#if time() - start > 20 and not pd.connected:
	#	pd.Connect()
	pd.Update()
	
	if "started: yo" in pd.messages:
		print "Running connect"
		pd.Connect(Start)
	
	if pd.messages:
		print pd.messages.pop()
	print time() - start
	# print "update"
print "exiting pd"
pd.Exit()
print "done"
