#!/usr/bin/python

# drift-log.py 
#    (C) Copyright IBM 2005
#    by John Stultz johnstul@us.ibm.com
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.


import commands
import os
import sys
import string
import time

def ntpdate_offset(ntpdate_cmd, server):
	#get actual offset
	(stat, cmd) = commands.getstatusoutput(ntpdate_cmd + ' -uq ' + server)
	
	if (stat != 0):
		print "Bad ntpdate output:", cmd
		sys.exit(-1)

	line = string.split(cmd)
	return string.atof(line[-2])


def ntpdate_set(ntpdate_cmd, server):
	(stat, cmd) = commands.getstatusoutput(ntpdate_cmd + ' -ub ' + server)
	line = string.split(cmd)
	if len(line) != 8:
		print "Time was set."
	else:
		print "Time could not be set."

def ntpdc_sysinfo_server(ntpdc_cmd):
	(stat, cmd) = commands.getstatusoutput(ntpdc_cmd + ' -c sysinfo')
	line = string.split(cmd)
	
	server = ""
	if len(line) != 4:
		server = line[2]

	return server


def ntpdc_kerninfo_offset(ntpdc_cmd):
	#get ntpd's view of the world
	(stat, cmd) = commands.getstatusoutput(ntpdc_cmd + ' -c kerninfo')
	line = string.split(cmd)

	offset = 0
	if len(line) > 18:
		offset = line[2]

	return offset


def ntpdc_kerninfo_freq(ntpdc_cmd):
	#get ntpd's view of the world
	(stat, cmd) = commands.getstatusoutput(ntpdc_cmd + ' -c kerninfo')
	line = string.split(cmd)
	drift = 0
	if len(line) > 18:
		drift = line[6]

	return drift

def ntpdc_kerninfo_status(ntpdc_cmd):
	#get ntpd's view of the world
	(stat, cmd) = commands.getstatusoutput(ntpdc_cmd + ' -c kerninfo')
	line = string.split(cmd)

	status = ""
	if len(line) > 18:
		status = line[17] + " " + line[18]

	return status


def ntpdc_peers_offset(ntpdc_cmd):
	#get ntpd's view of the world
	(stat, cmd) = commands.getstatusoutput(ntpdc_cmd + ' -c peers')
	line = string.split(cmd)

	offset = line[-2]

	return offset



def find_cmd(possibilities):
	for path in possibilities:
		if os.path.exists(path):
			return path
	print "Cannot fine command: ", possibilities
	sys.exit(1)
	




server_default = "yourserverhere"
sleep_time_default  = 60

set_time = 0
server = ""
sleep_time = 0
loops = -1

#parse args
for arg in sys.argv[1:]:
	if arg == "-s":
		set_time = 1
	elif server == "":
		server = arg
	elif sleep_time == 0:
		sleep_time = string.atoi(arg)
	elif loops == -1:
		loops = string.atoi(arg)

if server == "":
	server = server_default
if sleep_time == 0:
	sleep_time = sleep_time_default

print "Checking", server, "every", sleep_time, "secs", loops , "times"


#Find utility paths
ntpdate_possibilities = ["/usr/sbin/ntpdate", "/usr/bin/ntpdate"]
ntpdc_possibilities = ["/usr/sbin/ntpdc", "/usr/bin/ntpdc"]

ntpdate_cmd = find_cmd(ntpdate_possibilities)
ntpdc_cmd = find_cmd(ntpdc_possibilities)

#get current ntpd server name
ntpdc_server = ntpdc_sysinfo_server(ntpdc_cmd)
ip1 = string.split(commands.getoutput('host ' + server))
ip2 = string.split(commands.getoutput('host ' + ntpdc_server))
if ip1[-1] != ip2[-1]:
	print "Warning: specified server", server, 
	print "is not the current NTP server", ntpdc_server

# set the time
if set_time == 1:
	ntpdate_settime(ntpdate_cmd, server)
		
	
#start logging
print "key: date, secs, actual offset, kernel offset, ntp offset, ntp drift, ntp status"
print "================================================================================"

i = 0
while 1:
	i = i + 1
	if (loops == -1):
		i = 0
	elif (i > loops):
		break
	
	#get time right now
	now_time = time.time()
	datestr = time.strftime("%d %b %Y %H:%M:%S", time.localtime(now_time))
	print datestr,",", now_time, ",",

	print ntpdate_offset(ntpdate_cmd, server), ",",

	print ntpdc_kerninfo_offset(ntpdc_cmd), ",", 

	print ntpdc_peers_offset(ntpdc_cmd), ",",

	print ntpdc_kerninfo_freq(ntpdc_cmd), ",",

	print ntpdc_kerninfo_status(ntpdc_cmd)

	sys.stdout.flush()
	
	time.sleep(sleep_time)
