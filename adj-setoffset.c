/* ADJ_SETOFFSET test
 *              by: John Stultz <john.stultz@linaro.org>
 *              (C) Copyright Linaro 2014
 *              Licensed under the GPLv2
 *
 *  This test validates ADJ_SETOFFSET supports reasonable valid
 *  timespecs and poperly errors out on invalid ones.
 *
 *  Usage: adj-setoffset
 *
 *  To build:
 *	$ gcc adj-setoffset.c -o adj-setoffset -lrt
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
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

extern char *optarg;

#define NSEC_PER_SEC 1000000000L

#ifndef ADJ_SETOFFSET
#define ADJ_SETOFFSET 0x0100
#endif 

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

#define NUM_VALID 7
#define NUM_INVALID 12

struct timespec valid_vals[NUM_VALID] = {
	{0,0},
	/* pair adjustments so net change is zero */
	{0,500},
	{0,-500},

	{0,500},
	{-1,NSEC_PER_SEC-500},

	{500, 0},
	{-500,0},
};

struct timespec invalid_vals[NUM_INVALID] = {
	{0, NSEC_PER_SEC},
	{0, NSEC_PER_SEC + 10},

	{0, -NSEC_PER_SEC},
	{0, -NSEC_PER_SEC - 10},

	{500, NSEC_PER_SEC},
	{500, NSEC_PER_SEC + 10},

	{500, -NSEC_PER_SEC},
	{500, -NSEC_PER_SEC - 10},

	{-500, NSEC_PER_SEC},
	{-500, NSEC_PER_SEC + 10},

	{-500, -NSEC_PER_SEC},
	{-500, -NSEC_PER_SEC - 10},
};


int main(int argc, char** argv) 
{
	struct timex tx;
	int ret;
	int i;

	clear_time_state();

	/* Set the leap second insert flag */
	tx.modes = ADJ_SETOFFSET | ADJ_NANO;

	printf("Testing ADJ_SETOFFSET: ");
	for (i=0; i < NUM_VALID; i++) {
		tx.time.tv_sec = valid_vals[i].tv_sec;
		tx.time.tv_usec = valid_vals[i].tv_nsec;

		ret = adjtimex(&tx);
		if (ret < 0 ) {
			printf("FAIL\n");
			printf("Error: adjtimex(ADJ_SETOFFSET, %ld:%ld)\n",
				valid_vals[i].tv_sec,valid_vals[i].tv_nsec);
			return -1;
		}
	}

	for (i=0; i < NUM_INVALID; i++) {
		tx.time.tv_sec = invalid_vals[i].tv_sec;
		tx.time.tv_usec = invalid_vals[i].tv_nsec;
		ret = adjtimex(&tx);
		if (ret >= 0 ) {
			printf("FAIL\n");
			printf("Error: invalid adjtimex(ADJ_SETOFFSET, %ld:%ld)\n",
				invalid_vals[i].tv_sec,invalid_vals[i].tv_nsec);
			return -1;
		}
	}
	printf("PASS\n");
	return 0;
}
