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
		while True:
			try:
				try:
					answer_sets = getAnswerSets(s)
				except RuntimeWarning as e:
					print e.args[0]
				except RuntimeError as e:
					print e.args[0]
				except SyntaxError as e:
					print e.args[0]
				
				processAnswerSets(answer_sets)
				
				input = getInput()
				
				sendInput(s, input)
			except socket.error:
				print "Socket was closed."
				break
		closeSocket(s)
		return 0
#	except Exception, err:
#		sys.stderr.write('ERROR: %s\n' % str(err))
#		return 1

def connectToSocket(s):
	try:
		s.connect((opt.host, opt.port))
	except socket.error:
		raise EnvironmentError("Could not connect to %s:%d" % (opt.host, opt.port))
	
def getAnswerSets(s):
	answer_sets = []
	while True:
		try:
			output = recv_until(s, '\0')
		except socket.error:
			raise EnvironmentError("Socket was closed.")
		
		for line in output.splitlines():
			matched = False
			for i in range(len(PARSER)):
				match = PARSER[i].match(line)
				if match != None:
					matched = True
					if i == 0 and opt.debug:
						print "Found answer set for step %s." % match.group(1)
					elif i == 1:
						answer_sets.append(line.split())
					elif i == 2:
						return answer_sets
					elif i == 3:
						raise RuntimeWarning(line)
					elif i == 4:
						raise RuntimeError(line)
			if not matched:
				raise SyntaxError("Unkown output received from server: %s" % line)
	return answer_sets

def recv_until(s, delimiter):
	global data_old

	if opt.debug:
		print "Receiving data from socket..."

	data_list = data_old.split(delimiter, 1)
	
	if len(data_list) > 1:
		data_old = data_list[1]
		return data_list[0]
	else:
		while True:
			data = ''
			try:
				data = s.recv(2048)
			except socket.error:
				if opt.debug:
					print "Socket was closed. Returning last received data..."
				return data + data_old
			data = data_old + data
			data_list = data.split(delimiter, 1)
			if len(data_list) > 1:
				data = data_list[0]
				data_old = data_list[1]
				break;
			data_old = data
	
	return data

def processAnswerSets(answer_sets):
	i = 1
	for answer_set in answer_sets:
		print "Answer: %d" % i
		printAnswerSet(answer_set)
		i += 1

def printAnswerSet(answer_set):
	"""prints the answer set"""
	plan = {}
	# stores all predicates in dictionary plan
	for predicate in answer_set:
		match = re.match(".*[(,]([0-9]+)\)$", predicate)
		if match != None:
			t = int(match.group(1))
			if t in plan:
				plan[t].append(predicate)
			else:
				plan[t] = [predicate]
		else:
			if 'B' in plan:
				plan['B'].append(predicate)
			else:
				plan['B'] = [predicate]
	
	# get predicate lenghts
	row_len = {}
	for time in plan:
		plan[time].sort()
		row = 0
		for predicate in plan[time]:
			if row in row_len:
				if len(predicate) > row_len[row]:
					row_len[row] = len(predicate)
			else:
				row_len[row] = len(predicate)
			row += 1

	# prints predicates in rows
	if 'B' in plan:
		printRow(plan, row_len, 'B')
	for time in plan:
		if time != 'B':
			printRow(plan, row_len, time)

def printRow(plan, row_len, time):
	time_str = str(time).rjust(len(str(len(plan))))
	print "  %s. " % time_str,
	row = 0
	for predicate in plan[time]:
		 print predicate.ljust(row_len[row]+1),
		 row += 1
	print ""

def getInput():
	print "Please enter new information, '#endstep.' on its own line to end:"
	input = ''
	
	while True:
		line = sys.stdin.readline()
		
		if line == "#endstep.\n":
			break
		elif line == "#stop.\n":
			input += line
			break
		elif re.match("^-?[a-z_][a-zA-Z0-9_]*(\(.+\))?\.$", line) != None:
			input += line
		else:
			print "Warning: Unknown input."
	
	return input

def sendInput(s, input):
	s.send('%s\0' % input)
	if re.match("#stop\.", input) != None:
		endProgram(s)

def endProgram(s):
	closeSocket(s)
	if opt.debug:
		print "Exiting Program..."
	sys.exit(0)

def closeSocket(s):
	if opt.debug:
		print "Closing socket..."
	try:
		s.shutdown(1)
		s.close()
	except socket.error:
		print "Socket was already closed."

if __name__ == '__main__':
	if sys.version < '2.6':
		print 'You need at least python 2.6'
		sys.exit(1)
	sys.exit(main())

