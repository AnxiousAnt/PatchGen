# Python script to search the udev file system for devices
# Copyright (C) 2005  Tim Blechmann
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.


from os import popen

def find (*args):
	name = ""
	for token in args:
		name+=" " + str(token)
	name = name.strip()
	
	for i in range(255):
		pipe = popen('udevinfo -a -p /sys/class/input/event%d' % i)

		line = pipe.readline()
		while line:
			if name in line:
				return i

			line = pipe.readline()
		pipe.close()
	return -1
