#!/usr/bin/python

import sys
import socket

try:
	import readline
	print "Good: readline support available."
except:
	print "No readline support available."
	pass

True, False = 1==1, 1==0

class PdSender(socket.socket):
	"""Class that thinly wraps "socket" for Pd purposes. 
Methods: 
	Connect(host, port)
	Run(prompt)
	Exit
"""
	def __init__(self, host, port):
		self._port = int(port)
		self._host = host
		socket.socket.__init__(self)

	def Connect(self):
		try:
			self.connect((self._host, self._port))
		except socket.error, err:
			print "Error connecting to %s:%d: %s" % (self._host, self._port, err)
			sys.exit()

	def Run(self, prompt="""<pd> $ """):
		print """
Type "quit" or EOF to exit. """
		command = ""
		while True:
			try:
				command = raw_input(prompt)
			except EOFError:
				self.Exit()
			if command == "quit":
				self.Exit()
			else:
				command += ";"
				self.send(command)
				
	def Exit(self):
		self.close()
		print "\nHave a good time."
		sys.exit()


if __name__ == "__main__":
	port = 3000
	host = "localhost"

	# parse args, build prompt
	arglen = len(sys.argv) 
	if arglen == 3:
		host = sys.argv[1]
		port = int(sys.argv[2])
	elif arglen > 3 or arglen == 2:
		print """Usage: %s host port""" % sys.argv[0]
		sys.exit()
	print "Connecting to: %s:%s" % (host, port)
	prompt = """<pd @ %s:%s> $ """ % (host, port)

	
	pd = PdSender( host, port )
	pd.Connect()
	# Example for direct sending:
	# pd.send("something else;")
	pd.Run(prompt)
	
	
