from os import environ, kill, wait, read
from subprocess import Popen, PIPE
import sys
import signal
import asyncore
import asynchat
import socket
import select

# Copyright Chris McCormick, 2008
# LGPL

class PdSend(asynchat.async_chat):
	def __init__(self):
		self._cache = []
		self._socket = None
	
	def Connect(self, addr):
		self._socket = asyncore.dispatcher()
		self._socket.create_socket(socket.AF_INET, socket.SOCK_STREAM)
		self._socket.connect(addr)
		asynchat.async_chat.__init__(self, self._socket)
		[self.Send(d) for d in self._cache]
	
	def Send(self, data):
		if self._socket:
			asynchat.async_chat.push(self, " ".join([str(d) for d in data]) + ";\n")
		else:
			self._cache.append(data)

class PdReceive(asynchat.async_chat):
	def __init__(self, parent, localaddr=("127.0.0.1", 30322)):
		self._parent = parent
		self._socket = asyncore.dispatcher()
		self._socket.create_socket(socket.AF_INET, socket.SOCK_STREAM)
		self._socket.set_reuse_addr()
		self._socket.bind(localaddr)
		self._socket.handle_accept = self.handle_accept
		self._socket.listen(1)
	
	def handle_accept(self):
		conn, addr = self._socket.accept()
		asynchat.async_chat.__init__(self, conn)
		self._ibuffer = ""
		self.set_terminator(";\n")
		self._parent.Connect()
	
	def collect_incoming_data(self, data):
		self._ibuffer += data
	
	def found_terminator(self):
		data = self._ibuffer.split(" ")
		self._ibuffer = ""
		
		method = getattr(self._parent, 'Action_' + data[0], None)
		if method:
			method(data)
		else:
			print "no method for input", data

class PdStderr(asynchat.async_chat):
	def __init__(self, fd, parent):
		self._fd = fd
		self._parent = parent
		self._ibuffer = ""
		self.set_terminator("\n")
	
	def Update(self):
		for fd in select.select([self._fd], [], [], 0)[0]:
			self._ibuffer += fd.read(1)
			if len(self._ibuffer):
				if self._ibuffer[-1] == "\n":
					self._parent.Error(self._ibuffer[:-1])
					self._ibuffer = ""
			else:
				self._parent.PdDied()

class Pd:
	"""
		Spawns an instance of the Pure Data program and manages communications with it.
	"""
	errorCallbacks = {}
	def __init__(self, port=30321, nogui=True, open="_main.pd", cmd=None):
		"""
			nogui is a boolean which specifies whether to start Pd with or without a gui. Defaults to nogui=True
			open is a string which specifies a pd file to open on startup.
			cmd is a message to send to the pd interpreter on startup.
		"""
		# print " ".join(["pd", nogui and "-nogui" or "", open and "-open '%s'" % open or "", cmd and '-send "%s"' % cmd or ""])
		
		self.connectCallback = None
		
		args = ["pd", "-stderr"]
		if nogui:
			args.append("-nogui")
		
		if open:
			args.append("-open")
			args.append(open)
		
		if cmd:
			args.append("-send")
			args.append(cmd)
		
		self.pd = Popen(args, stderr=PIPE)
		
		if port:
			self.port = port
		
		self._pdSend = PdSend()
		self._pdReceive = PdReceive(self)
		self._pdStderr = PdStderr(self.pd.stderr, self)
	
	def Update(self):
		asyncore.poll()
		self._pdStderr.Update()
	
	def Send(self, msg):
		self._pdSend.Send(msg)
	
	def Connect(self):
		self._pdSend.Connect(("127.0.0.1", self.port))
	
	def Error(self, error):
		errors = error.split(" ")
		method = getattr(self, 'Error_' + errors[0], None)
		if method:
			method(errors)
		elif error in self.errorCallbacks:
			self.errorCallbacks[error]()
		else:
			print 'untrapped error: "' + error + '"'
	
	def PdDied(self):
		print "ARg, Pd died!"
	
	def Exit(self):
		#self.close()
		if sys.platform == "win32":
			# Kill the process using pywin32
			import win32api
			win32api.TerminateProcess(int(process._handle), -1)
		if sys.platform == "linux2":
			kill(self.pd.pid, signal.SIGINT)
			wait()

if __name__ == "__main__":
	from time import time, sleep
	from os import path, getcwd
	
	start = time()
	print "launching pd"
	pd = Pd(nogui = False, open = path.join(getcwd(), "test.pd"))
	pd.Send(["test message", 1, 2, 3])
	
	print "running a bunch of stuff for 30 seconds"
	while time() - start < 30:
	        #if time() - start > 20 and not pd.connected:
	        #       pd.Connect()
	        pd.Update()
		sleep(0.0001)
	
	print "exiting pd"
	pd.Exit()
	print "done"

