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
answer_sets = {}
PARSER = [
	re.compile("^Step:\ (\d+)$"),
	re.compile("^(-?[a-z_][a-zA-Z0-9_]*(\(.+\))?\ *)+$"),
	re.compile("^Input:$"),
	re.compile("^Warning: (?P<series>.+)$"),
	re.compile("^Error: (?P<series>.+)$"),
]

def main():
#	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		
		connectToSocket(s)
		read(s)

		return 0
#	except Exception, err:
#		sys.stderr.write('ERROR: %s\n' % str(err))
#		return 1

def connectToSocket(s):
	try:
		s.connect((opt.host, opt.port))
	except socket.error:
		raise EnvironmentError("Could not connect to %s:%d" % (opt.host, opt.port))
	
def read(s):
	step = 0
	while True:
		try:
			step = getAnswerSets(s, step)

			print answer_sets
			
			input = getInput()

			s.send('%s\0' % input)
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
	
	data_list = data_old.split(delimiter, 1)
	
	if len(data_list) > 1:
		data_old = data_list[1]
		return data_list[0]
	else:
		while True:
			try:
				data = s.recv(2048)
			except socket.error:
				return data + data_old
			data = data_old + data
			data_list = data.split(delimiter, 1)
			if len(data_list) > 1:
				data = data_list[0]
				data_old = data_list[1]
				break;
			data_old = data
	
	return data


def getAnswerSets(s, step):
	global answer_sets
	new_step = 0

	while True:
		try:
			data = recv_until(s, '\0')
		except socket.error:
			 "Socket was closed."
		
		try:
			answer_set = parseOutput(data)
			if len(answer_set) != 2:
				raise RuntimeError("Malformed answer set received: %s" % repr(answer_set))
			elif not answer_set[0] in answer_sets:
				answer_sets[answer_set[0]] = []
			
			answer_sets[answer_set[0]].append(answer_set[1])
			
			if step == 0:
				step = answer_set[0]
			new_step = answer_set[0]
			if new_step != step:
				break
		except RuntimeWarning as e:
			print e.args[0]
		except RuntimeError as e:
			print e.args[0]
		except SyntaxError as e:
			print e.args[0]
		
	return new_step
	


def parseOutput(output):
	answer_set = []
	for line in output.splitlines():
		matched = False
		for i in range(len(PARSER)):
			match = PARSER[i].match(line)
			if match != None:
				matched = True
				if i == 0:
					answer_set.append(int(match.group(1)))
				elif i == 1:
					answer_set.append(line.split())
				elif i == 2:
					# TODO improve functions to stop here and return AS
					print "bla"
				elif i == 3:
					raise RuntimeWarning(line)
				elif i == 4:
					raise RuntimeError(line)
		if not matched:
			raise SyntaxError("Unkown output received from server: %s" % line)
	return answer_set

def getInput():
	input = ''
	
	while True:
		line = sys.stdin.readline()
		
		if line == "#endstep.\n":
			break
		elif line == "#stop.\n":
			input += line
			break
		elif re.match("^ *\w+\([\d\w, ]+\)\. *$", line) != None:
			input += line
		else:
			print "Warning: Unknown input."
	
	return input


if __name__ == '__main__':
	if sys.version < '2.6':
		print 'You need at least python 2.6'
		sys.exit(1)
	sys.exit(main())

