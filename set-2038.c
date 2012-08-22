/* Time bounds setting test
 *		by: john stultz (johnstul@us.ibm.com)
 *		(C) Copyright IBM 2012
 *		Licensed under the GPLv2
 *
 *  NOTE: This is a meta-test which sets the time to edge cases then
 *  uses other tests to detect problems. Thus this test requires that
 *  the inconsistency-check and nanosleep tests be present in the same
 *  directory it is run from.
 *
 *  To build:
 *	$ gcc set-2038.c -o set-2038 -lrt
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
 */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define NSEC_PER_SEC 1000000000LL

#define KTIME_MAX	((long long)~((unsigned long long)1 << 63))
#define KTIME_SEC_MAX	(KTIME_MAX / NSEC_PER_SEC)

#define YEAR_1901 (-0x7fffffffL)
#define YEAR_1970 1
#define YEAR_2038 0x7fffffffL			/*overflows 32bit time_t */
#define YEAR_2262 KTIME_SEC_MAX			/*overflows 64bit ktime_t */
#define YEAR_MAX  ((long long)((1ULL<<63)-1))	/*overflows 64bit time_t */

int is32bits(void)
{
       return (sizeof(long) == 4);
}

int settime(long long time)
{
	struct timeval now;
	int ret;

	now.tv_sec = (time_t)time;
	now.tv_usec  = 0;

	ret = settimeofday(&now, NULL);

	printf("Setting time to 0x%lx: %d\n",(long)time, ret);
	return ret;
}

int do_tests(void)
{
	int ret;

	system("date");
	ret = system("./inconsistency-check -c 0 -t 20");
	ret |= system("./nanosleep");
	return ret;

}

int main(void)
{
	int ret = 0;
	time_t start;

	start = time(0);

	/* First test that crazy values don't work */
	if (!settime(YEAR_1901)) {
		ret = -1;
		goto out;
	}
	if (!settime(YEAR_MAX)){
		ret = -1;
		goto out;
	}
	if (!is32bits() && !settime(YEAR_2262)) {
		ret = -1;
		goto out;
	}

	/* Now test behavior near edges */
	settime(YEAR_1970);
	if (ret = do_tests())
		goto out;

	settime(YEAR_2038-600);
	if (do_tests())
		goto out;

	/* The rest of the tests are 64bit only */
	if (is32bits())
		goto out;

	settime(YEAR_2262-600);
	if (do_tests())
		goto out;

	/* Test rollover behavior 32bit edge */
	settime(YEAR_2038-10);
	if (do_tests())
		goto out;

out:
	/* restore clock */
	settime(start);
	return ret;
}

