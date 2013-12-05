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
import matplotlib.dates as mdates

#make sure we've got a recent version fo matplotlib
if (StrictVersion(matplotlib.__version__) < StrictVersion('1.0')):
	print "Error: This tool requires matplotlib 1.0 or greater"
	sys.exit(-1)



#helpers for tweaking chart size/fonts
normchart	= {"dpi":100}

chart_opts = normchart



x = []
act = []
kern = []
ntp = []
freq = []
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
	freq.append(float(drift))


left, width = 0.1, 0.8
rect1 = [left, 0.2, width, 0.7]
rect3 = [left, 0.1, width, 0.1]

fig = plt.figure()

ax = fig.add_axes(rect1)

ax.plot(x, act, 'g', label="actual offset")
ax.plot(x, kern, 'b', label="kern offset")
ax.plot(x, ntp, 'r', label="ntp offset")

plt.ylabel("offset (sec)")

bx = fig.add_axes(rect3, sharex=ax)
bx.plot(x,freq, 'y')

plt.ylabel("ppm adj")

legend = ax.legend()
legend.get_frame().set_alpha(0.5)
for label in legend.get_texts():
	label.set_fontsize('small')


ax.grid(True)
bx.grid(True)


for label in ax.get_xticklabels():
	label.set_visible(False)

for label in bx.get_xticklabels():
	label.set_rotation(30)
	label.set_horizontalalignment('right')

for label in ax.get_yticklabels():
	label.set_fontsize('small')

for label in bx.get_yticklabels():
	label.set_fontsize('small')

bx.fmt_xdata = mdates.DateFormatter('%Y-%m-%d')

#prune bottom label so we don't have y axis label overlaps
ax.get_yticklabels()[0].set_visible(False)


fig.autofmt_xdate()
plt.margins(0.05, 0.5)
plt.subplots_adjust(bottom=0.15)
plt.savefig(filename+'.png', dpi=chart_opts["dpi"])
#plt.show()


