
try:
	import pyext
except:
	print "ERROR: This script must be loaded by the PD pyext external"
	sys.exit()

import imaplib
import email




mail = ''
user = 'user'
passw = 'pass'
subject = ''
uidsList = ''
data = ''

class box(pyext._class):
    
        
    # number of inlets and outlets
	_inlets=1
	_outlets=2
      

        
        # constructor
	def __init__(self,*args):
		if len(args) == 2: 
			global user, passw
			user = args[0]
			passw = "%s" % args[1]
                        

     	def subject_1(self, *args):
		global subject
		subject = " ".join(str(e) for e in args)
		print "Subject changed to '",subject,"'"
		#print args

        def login_1(self, *args):
		global mail, user, passw
		if len(args) == 2: 
			user = args[0]
			passw = "%s" % args[1]

		print "Logging to gmail as", user
		mail = imaplib.IMAP4_SSL('imap.gmail.com')
		mail.login( user, passw)
		mail.list()
		mail.select("inbox") # connect to inbox.

	def logout_1(self):
		print "Logging out from gmail..."
		global mail
		mail.close()
		mail.logout() 
  
	def check_1(self):
		global mail, subject, uidsList, data
		print "________________________________________________"
		print "Checking mail for SUBJECT ", subject
		tmp =  '(HEADER Subject "%s")' % subject
		result, data = mail.uid('search', None, tmp) # search and return uids instead
		#print data
		str = ''.join(data)
                if (len(str)>0):
                	uidsList = str.split(' ')
			#print mylist
			size = len(uidsList)
		else:
			size = 0
		
		if(size > 0 ):
			latest_email_uid = data[0].split()[-1]
			result, data = mail.uid('fetch', latest_email_uid, '(RFC822)')
			raw_email = data[0][1] # here's the body, which is raw text of the whole email
		# including headers and alternate payloads
			email_message = email.message_from_string(raw_email)
			#print email.utils.parseaddr(email_message['From'])  
			self._outlet(2,email.utils.parseaddr(email_message['From']))
			self._outlet(2,uidsList)
		#else:
			#print "No emails ;("

		print "Found ", size, "emails."
		self._outlet(1,float(size))
		
		

	def delete_1(self,argv):
		global mail
		print "Deleting mail ", argv
		mail.uid('STORE', argv, '+FLAGS', '\\Deleted')
		mail.expunge()

	def deleteall_1(self):
		global mail, subject, uidsList
		print "Deleting all mail with subject '",subject ,"'"
		for n in uidsList:
			mail.uid('STORE', n, '+FLAGS', '\\Deleted')
		mail.expunge()

	def list_1(self):
		global uidsList
		print uidsList
		
		

