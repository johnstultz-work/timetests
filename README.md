timetests
=========

Linux Timekeeping Tests

These are a collection of timekeeping tests I've used over
the years along with some new ones, assembled to try to
provide some basic sanity testing of the timekeeping
code.

I've tried to keep each test independent as possible,
with no heavyweight testing infrastructure, so its
easy to pluck a single test out if needed. Not all
the tests succeed at this, but most of them do.


I've developed and tested this on Ubuntu 12.04,
so your milage may vary on other distros.

To run:
	./runall.sh


I also reccomend using the trinity test suite:
	http://codemonkey.org.uk/projects/trinity/


If have any fixes or even suggestions for new tests, please
email me!

	John Stultz <johnstul@us.ibm.com>
