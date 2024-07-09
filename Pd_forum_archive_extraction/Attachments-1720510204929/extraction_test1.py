# this file will look slightly different for every library
# write everything into a database like format... (NOT YET!!!)
# OBJECTNAME | OPT OTHER NAME | HELPFILE-NAME | DESCRIPTION | TAGS | LIBRARYINFO | ARGUMENTS | INLETS/OUTLETS | EXAMPLES | SEE ALSO
# objectname is the string name (for example "plus" for "+")
# opt other name = abbreviation, +, 
# helpfilename (can be different than object name), 
# description = short description
# libraryinfo: purepd or GEM + author + licence
# arguments
# inlets/outlets
# examples : what this object is for...
# see also/similar objects
# junk
#
# use X coordinate to sort comments???

import re
import os

print 'searching all files for comments'

searchPattern = "#X text \d+ \d+ " # search for comments
replacePattern = searchPattern    # what can be deleted of the comments
deleteNLPattern = r'\r'            # delete cr
fileExtension = ".*[.]pd$"        # files to look for
separator = "|"                    # separate colomns
separator2 = "___"                # separate junk
fileTo = "test_to.txt"

p = re.compile(searchPattern)    
rep = re.compile(replacePattern)
nldel = re.compile(deleteNLPattern)
fe = re.compile(fileExtension)

f2 = open(fileTo, "w")

for fileName in os.listdir("."):
    m = fe.search(fileName)
    if m:                            # only if is it a pd-file
        f = open(fileName, "r")
        first = 1                    # suppose the first entry is the description
        for line in f:
            m = p.search(line)     # search in line for pd-comment pattern
            if m:
                line = nldel.sub("",line)
                if first == 1:                    # help needed! how to sort other than by appearance? write to array?
                    f2.write("\n" + fileName + separator) #need \n for every file but the first.
                    newText = rep.sub("",line)    # this should delete the trailing pattern
                    f2.write(newText[:-1])        # this writes without closing nl
                    f2.write(separator)
                    first = 0
                else:
                    newText = rep.sub("",line)
                    f2.write(newText[:-1])
                    f2.write(separator2)
        f.close()
f2.close()

#done...