/* Demo leapsecond deadlock
 *              by: john stultz (johnstul@us.ibm.com)
 *              (C) Copyright IBM 2012
 *              Licensed under the GPL
 *
 * This test demonstrates leapsecond deadlock that is possibe
 * on kernels from 2.6.26 to 3.3.
 *
 * WARNING: THIS WILL LIKELY HARDHANG SYSTEMS AND MAY LOSE DATA
 * RUN AT YOUR OWN RISK!
 *  To build:
 *	$ gcc leapcrash.c -o leapcrash -lrt
 */



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <string.h>
#include <signal.h>



/* clear NTP time_status & time_state */
void clear_time_state(void)
{
	struct timex tx;
	int ret;

	/*
	 * XXX - The fact we have to call this twice seems
	 * to point to a slight issue in the kernel's ntp state
	 * managment. Needs to be investigated further.
	 */

	tx.modes = ADJ_STATUS;
	tx.status = STA_PLL;
	ret = adjtimex(&tx);

	tx.modes = ADJ_STATUS;
	tx.status = 0;
	ret = adjtimex(&tx);
}

/* Make sure we cleanup on ctrl-c */
void handler(int unused)
{
	clear_time_state();
	exit(0);
}


int  main(void)
{
	struct timex tx;
	struct timespec ts;
	time_t next_leap;
	int count =0;

	setbuf(stdout, NULL);

	signal(SIGINT, handler);
	signal(SIGKILL, handler);
	printf("This runs for a few minutes. Press ctrl-c to stop\n");

	clear_time_state();


	/* Get the current time */
	clock_gettime(CLOCK_REALTIME, &ts);

	/* Calculate the next possible leap second 23:59:60 GMT */
	next_leap = ts.tv_sec;
	next_leap += 86400 - (next_leap % 86400);

	for (count = 0; count < 20; count++) {
		struct timeval tv;


		/* set the time to 2 seconds before the leap */
		tv.tv_sec = next_leap - 2;
		tv.tv_usec = 0;
		if (settimeofday(&tv, NULL)) {
			printf("Error: You're likely not running with proper (ie: root) permissions\n");
			exit(-1);
		}
		tx.modes = 0;
		adjtimex(&tx);

		/* hammer on adjtime w/ STA_INS */
		while (tx.time.tv_sec < next_leap + 1) {
			/* Set the leap second insert flag */
			tx.modes = ADJ_STATUS;
			tx.status = STA_INS;
			adjtimex(&tx);
		}
		clear_time_state();
		printf(".");
	}
	printf("PASSED\n");
	return 0;
}
