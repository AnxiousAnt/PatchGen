#! /usr/bin/env python

"""List pd dlls.

usage: pddlls [extradir]
extradir: directory to be scanned for pd externals and abstractions (default 
current)
"""

import os
import sys

verbose = 1

# Main program: parse command line and start processing
def main():
    global verbose
    extradir = './' # default is current dir
    if sys.argv[1:]: # otherwise get it from the command line
        extradir = sys.argv[1]
    if not os.path.isdir(extradir): # exit if it doesn't exist
        print "directory doesn't exist", repr(extradir)
        sys.exit(2)
    listpdexternsinsubdirs(extradir)

# Recursively list pd externals and abstractions in extradir
def listpdexternsinsubdirs(extradir):
    item = 0 # count of valid externs
    for root, dirs, files in os.walk(extradir): #recursively walk the 
directory
        for name in files:
            extension = str.lower(name) # make name lower case
            pathname = os.path.join(root, name) # make the full path
            pname = pathname.partition('/')[2] # relative to extradir
            pdtype = 'not' # default not an external
            if pname.endswith('.pd'):
                pdtype = 'abstraction'
            elif pname.endswith('.dll'): # not necessarily pd though...
                pdtype = 'MSW external'
            elif pname.endswith('.pd_linux'):
                pdtype = 'linux external'
            elif pname.endswith('.pd_darwin'):
                pdtype = 'OSX external'
            if pdtype != 'not': # a valid object
                item += 1
                if verbose:
                    print '%04d-->' % (item), pdtype, pname

if __name__ == '__main__':
    main()

