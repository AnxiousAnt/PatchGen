# py/pyext - python script objects for PD and MaxMSP
#
# Copyright (c) 2002-2005 Thomas Grill (gr@grrrr.org)
# For information on usage and redistribution, and for a DISCLAIMER OF ALL
# WARRANTIES, see the file, "license.txt," in this distribution.  
#

"""debug nbx"""

try:
	import pyext
except:
	print "ERROR: This script must be loaded by the PD/Max pyext external"


class nbx(pyext._class):
	"""..."""

	_inlets = 1
	_outlets = 0

	def __init__(self,*args):
		"""Class constructor"""
		print "\n\n\n\n\n\ninit\n\n\n\n\n\n"

	def bind_1(self):
		print "binding 's_float'"
		self._bind("s_float", self.recv)
		print "binding 's_nbx'"
		self._bind("s_nbx", self.recv)

	def bang_1(self):
		print "sending a bang to 'r_float'"
		self._send("r_float")
		print "sending a bang to 'r_nbx'"
		self._send("r_nbx")

	def recv(self, arg):
		"""receive function"""
		print arg

	def __del__(self):
		"""Class destructor"""
		pass
