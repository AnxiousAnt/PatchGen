#! /usr/bin/env python

import sys, string

def escaped(l):
    for c in '\\"' + "'":
        words = string.split(l, c)
        l = words[0]
        for word in words[1:]:
            l = l + '\\' + c + word
    return l

for line in sys.stdin.readlines():
    l = string.strip(line)
    if l:
        if l[0] == '#':
            print '//' + l[1:]
        else:
            print 'sys_gui("' + escaped(line[:-1]) + '\\n");'
    else:
        print
