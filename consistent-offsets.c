/* Validate consistent offsets between clockids
 *              by: john stultz (johnstul@us.ibm.com)
 *              (C) Copyright IBM 2012
 *              Licensed under the GPLv2
 *
 *  To build:
 *	$ gcc consistent-offsets.c -o consistent-offsets -lrt
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 * NOTE: This test still has some problems with false positives.
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <string.h>
#include <signal.h>

extern char *optarg;

#define NSEC_PER_SEC 1000000000ULL

#define CLOCK_REALTIME			0
#define CLOCK_MONOTONIC			1
#define CLOCK_PROCESS_CPUTIME_ID	2
#define CLOCK_THREAD_CPUTIME_ID		3
#define CLOCK_MONOTONIC_RAW		4
#define CLOCK_REALTIME_COARSE		5
#define CLOCK_MONOTONIC_COARSE		6
#define CLOCK_BOOTTIME			7
#define CLOCK_REALTIME_ALARM		8
#define CLOCK_BOOTTIME_ALARM		9
#define CLOCK_HWSPECIFIC		10
#define CLOCK_TAI			11
#define NR_CLOCKIDS			12

char *clockstring(int clockid)
{
	switch (clockid) {
	case CLOCK_REALTIME:
		return "CLOCK_REALTIME";
	case CLOCK_MONOTONIC:
		return "CLOCK_MONOTONIC";
	case CLOCK_PROCESS_CPUTIME_ID:
		return "CLOCK_PROCESS_CPUTIME_ID";
	case CLOCK_THREAD_CPUTIME_ID:
		return "CLOCK_THREAD_CPUTIME_ID";
	case CLOCK_MONOTONIC_RAW:
		return "CLOCK_MONOTONIC_RAW";
	case CLOCK_REALTIME_COARSE:
		return "CLOCK_REALTIME_COARSE";
	case CLOCK_MONOTONIC_COARSE:
		return "CLOCK_MONOTONIC_COARSE";
	case CLOCK_BOOTTIME:
		return "CLOCK_BOOTTIME";
	case CLOCK_REALTIME_ALARM:
		return "CLOCK_REALTIME_ALARM";
	case CLOCK_BOOTTIME_ALARM:
		return "CLOCK_BOOTTIME_ALARM";
	case CLOCK_TAI:
		return "CLOCK_TAI";
	};
	return "UNKNOWN_CLOCKID";
}


long long llabs(long long val)
{
	if (val<0)
		val = -val;
	return val;
}

unsigned long long ts_to_nsec(struct timespec ts)
{
	return ts.tv_sec * NSEC_PER_SEC + ts.tv_nsec;
}


long long diff_timespec(struct timespec start, struct timespec mid, struct timespec end)
{
	long long start_ns, mid_ns, end_ns;

	start_ns = ts_to_nsec(start);
	mid_ns = ts_to_nsec(mid);
	end_ns = ts_to_nsec(end);

	start_ns = (start_ns + end_ns)/2;

	return mid_ns - start_ns;


}

int test_offsets(int clockid, long long margin, int sleep, int runtime)
{
	struct timespec testclk, mono, bound, start;
	long long off1, off2, delta;

	clock_gettime(CLOCK_MONOTONIC, &mono);
	clock_gettime(clockid, &testclk);
	clock_gettime(CLOCK_MONOTONIC, &bound);
	off1 = diff_timespec(mono, testclk, bound);
	start = mono;

	while (1) {
		clock_gettime(CLOCK_MONOTONIC, &mono);
		clock_gettime(clockid, &testclk);
		clock_gettime(CLOCK_MONOTONIC, &bound);
		off2 = diff_timespec(mono, testclk, bound);
		delta = llabs(off2-off1);
		if (delta > margin) {
			printf("ERROR: Clock: %s Offset: %lld Delta: %lld Margin: %lld\n", clockstring(clockid), off2, delta, margin);
			return -1;
		}
		off1 = off2; 
		if (sleep)
			usleep(NSEC_PER_SEC/1000);

		if (diff_timespec(start, mono, start) > runtime * NSEC_PER_SEC)
			break;
	}
	return 0;
}


int main(int argc, char **argv)
{
	struct timespec real, mono, bound;
	long long off1, margin;
	int failed = 0, clockid, userclock=-1, maxclocks;
	int runtime = 30;
	int opt;


	/* Process arguments */
	while ((opt = getopt(argc, argv, "t:c:"))!=-1) {
		switch(opt) {
		case 't':
			runtime = atoi(optarg);
			break;
		case 'c':
			userclock = atoi(optarg);
			maxclocks = userclock+1;
			break;
		default:
			printf("Usage: %s [-t <secs>] [-c <clockid>]\n", argv[0]);
			printf("	-t: Number of seconds to run\n");
			printf("	-c: clockid to use (default, all clockids)\n");
			exit(-1);
		}
	}




	setbuf(stdout, NULL);

	clock_gettime(CLOCK_MONOTONIC, &mono);
	clock_gettime(CLOCK_REALTIME, &real);
	clock_gettime(CLOCK_MONOTONIC, &bound);

	margin = diff_timespec(mono, bound, mono) *1000;

	off1 = diff_timespec(mono, real, bound);

	if (userclock == -1) {
		userclock = CLOCK_REALTIME;
		maxclocks = NR_CLOCKIDS;
	}

	for (clockid = userclock; clockid < maxclocks; clockid++) {

		/* Skip clockids that won't have constant offsets */
		if (clockid == CLOCK_MONOTONIC ||
				clockid == CLOCK_PROCESS_CPUTIME_ID ||
				clockid == CLOCK_THREAD_CPUTIME_ID ||
				clockid == CLOCK_MONOTONIC_RAW ||
				clockid == CLOCK_REALTIME_COARSE ||
				clockid == CLOCK_MONOTONIC_COARSE ||
				clockid == CLOCK_HWSPECIFIC)
			continue;


		printf("Offset %-34s: ", clockstring(clockid));

		if (test_offsets(clockid, margin, 0, runtime)) {
			failed = 1;
			break;
		}
		if (test_offsets(clockid, margin, 1, runtime)) {
			failed = 1;
			break;
		}

		printf("PASSED\n");
	}

	if (failed)
		printf("FAILED\n");
	return -failed;
}
