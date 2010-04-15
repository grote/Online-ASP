#!/usr/bin/python
#########################################################################
#   Copyright 2010 Torsten Grote
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
##########################################################################

import sys
import socket
import re
from optparse import OptionParser

# Parse Command Line Options
usage = "usage: %prog [online.lp] [options]"
parser = OptionParser(usage=usage, version="%prog 0.1")
parser.add_option("-n", "--hostname", dest="host", help="Hostname of the online iClingo server", metavar="HOST")
parser.add_option("-p", "--port", dest="port", help="Port the online iClingo server is listening to", metavar="PORT")
parser.add_option("-d", "--debug", dest="debug", help="show debugging output", action="store_true")
parser.set_defaults(
	host = 'localhost',
	port = 25277,
	debug = False
)
(opt, args) = parser.parse_args()

data_old = ''

def main():
	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		
		connectToSocket(s)
		read(s)

		return 0
	except Exception, err:
		sys.stderr.write('ERROR: %s\n' % str(err))
		return 1

def connectToSocket(s):
	try:
		s.connect((opt.host, opt.port))
	except socket.error:
		raise EnvironmentError("Could not connect to %s:%d" % (opt.host, opt.port))
	
def read(s):
	while True:
		try:
			data = recv_until(s, '\0')
			# TODO send other stuff
			s.send('p(1,1).\0')
			print data
		except socket.error:
			print "Socket was closed."
			break

	try:
		s.shutdown(1)
		s.close()
	except socket.error:
		print "Socket was already closed."

def recv_until(s, delimiter):
	global data_old
	
	data = s.recv(1024)
	data = data_old + data
	data_list = data.split(delimiter, 1)
	data = data_list[0]
	
	if len(data_list) > 1:
		data_old = data_list[1]

	return data

if __name__ == '__main__':
	sys.exit(main())


