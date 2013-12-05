#!/usr/bin/env python
# by: John Stultz <john.stultz@linaro.org>
# Copyright 2013 Linaro Limited
#
# Licensed under the GPLv2
#
# This tool requires matplotlib 1.0 or greater to be installed!

import time
import string
import sys
from pylab import *
from datetime import *
from distutils.version import StrictVersion

#make sure we've got a recent version fo matplotlib
if (StrictVersion(matplotlib.__version__) < StrictVersion('1.0')):
	print "Error: This tool requires matplotlib 1.0 or greater"
	sys.exit(-1)



#helpers for tweaking chart size/fonts
normchart	= {"dpi":75}

chart_opts = normchart



x = []
act = []
kern = []
ntp = []

filename = sys.argv[1]

#Get the commit data
datafile = open(filename, "r")
for line in datafile.read().split("\n"):
	if "key" in line:
		continue
	if not "," in line:
		continue

	(datestr,dateval,act_off,kern_off,ntp_off,drift,status) = line.split(",")

	date = datetime.fromtimestamp(float(dateval))

	x.append(date)
	act.append(float(act_off))
	kern.append(float(kern_off))
	ntp.append(float(ntp_off))


fig, ax = plt.subplots()

ax.plot(x, act, 'g.-', label="actual offset")
ax.plot(x, kern, 'b.-', label="kern offset")
ax.plot(x, ntp, 'r.-', label="ntp offset")

fig.autofmt_xdate()

legend = ax.legend()
for label in legend.get_texts():
    label.set_fontsize('small')

plt.margins(0.05, 0.5)
plt.subplots_adjust(bottom=0.15)
plt.grid(True)
plt.savefig(filename+'.png', dpi=chart_opts["dpi"])
#plt.show()


