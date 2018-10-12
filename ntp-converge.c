/* ntp-convergence test
 *		by: John Stultz (john.stultz@linaro.org)
 *		(C) Copyright Linaro Inc 2013
 *		Licensed under the GPLv2
 *
 *  NOTE: This test kills ntpd, clears the drift file, injects a fixed offset,
 *  and then starts ntp and logs the behavior..
 *
 *  This test requires drift-log.py be present in the current directory
 *
 *  To build:
 *	$ cc -Wl,-no-as-needed  -lrt -lpthread  ntp-converge.c -o ntp-converge
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
#include <sys/time.h>
#include <sys/timex.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define NSEC_PER_SEC 		1000000000LL

#define ADJ_SETOFFSET		0x0100

#define CONF_FILE "/tmp/myntp.conf"
#define DRIFT_FILE "/tmp/myntp.drift"

//#define MINMAXPOLL "minpoll 4 maxpoll 4"
#define MINMAXPOLL ""

extern char *optarg;


int set_fixed_offset(long long nsec)
{
	struct timex tx;
	int ret;
	int neg = 0;

	if (nsec < 0) {
		neg = 1;
		nsec = -nsec;
	}

	tx.modes = ADJ_SETOFFSET;
	tx.time.tv_sec = nsec/NSEC_PER_SEC;
	tx.time.tv_usec = nsec%NSEC_PER_SEC;

	/*
	 * negative timespecs are strange, as the tv_nsec
	 * portion  must always be positive. So subtract it
	 * from NSEC_PER_SEC and drop one from the tv_sec.
	 */
	if (neg) {
		tx.time.tv_sec = -tx.time.tv_sec;
		if (tx.time.tv_usec) {
			tx.time.tv_usec = NSEC_PER_SEC - tx.time.tv_usec;
			tx.time.tv_sec--;
		}
	}

	/* we're not using ADJ_NANO, so convert to usecs */
	tx.time.tv_usec /= 1000;
	ret = adjtimex(&tx);

	return ret;
}

void clear_ntp_state(void)
{
	struct timex tx;
	int ret;

	tx.modes = ADJ_OFFSET;
	tx.offset = 0;
	ret = adjtimex(&tx);

	tx.modes = ADJ_FREQUENCY;
	tx.freq = 0;
	ret = adjtimex(&tx);

	tx.modes = ADJ_STATUS;
	tx.status = 0;
	ret = adjtimex(&tx);

}

pid_t run_driftlog(char *server, long runtime, char* logfile)
{
	pid_t pid;
	int ret;
	char buf[256];

	sprintf(buf, "./drift-log.py %s 60 %ld | tee %s", server, runtime, logfile);

	pid = fork();
	if (!pid) {
		ret = system(buf);
		exit(0);
	}
	return pid;
}


int set_time(char *server)
{
	pid_t pid;
	int ret;
	char buf[256];

	sprintf(buf, "ntpdate -b %s > /dev/null", server);
	ret = system(buf);
	return ret;
}


pid_t run_ntpd(void)
{
	pid_t pid;
	int ret;
	char buf[256];

	sprintf(buf, "ntpd -n  -c %s -f %s", CONF_FILE, DRIFT_FILE);

	pid = fork();
	if (!pid) {
		ret = system(buf);
		exit(0);
	}

	return pid;
}



void generate_ntp_conf(char *server)
{

	int fd;
	char* boilerplate = 
		"restrict -4 default kod notrap nomodify nopeer noquery\n"
		"restrict -6 default kod notrap nomodify nopeer noquery\n"
		"restrict 127.0.0.1\n"
		"restrict ::1\n";

	char buf[256];
	sprintf(buf, "server %s %s\n", server, MINMAXPOLL);

	fd = open(CONF_FILE, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
	if(fd < 0)
		exit(-1);
	write(fd, buf, strlen(buf));
	write(fd, boilerplate, strlen(boilerplate));
	close(fd);

	fd = open(DRIFT_FILE, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
	close(fd);
}

void cleanup_conf(void)
{
	unlink(CONF_FILE);
	unlink(DRIFT_FILE);
}

int main(int argc, char** argv)
{
	int ret, opt;
	pid_t pid;
	char *logfile = NULL;
	char* server = "91.189.89.199";
	long offset = 50000000; /*nsecs*/
	long runtime = 30; /*mins*/
	time_t now;


	/*
	 * XXX Still TODO:
	 *   - Need better error checking
	 *   - validate drift-log.py, ntpd, ntpdate are present
	 */

	/* Process arguments */
	while ((opt = getopt(argc, argv, "s:f:o:r:"))!=-1) {
		switch(opt) {
		case 's':
			server = optarg;
			break;
		case 'f':
			logfile = optarg;
			break;
		case 'o':
			offset = atoi(optarg);
			break;
		case 'r':
			runtime = atoi(optarg);
			break;
		default:
			printf("Usage: %s [-s <server>] [-o <offset>]\n", argv[0]);
			printf("	-s <server>: Use <server> for convergence test \n");
			printf("	-o <nsec>: Nsec offset that will be injected \n");
			printf("	-f <logfile>: Output file for logging\n");
			printf("	-r <min>: Runtime in minutes\n");
			exit(-1);
		}
	}


	/* Set the log file name to be based on the current time */
	if (!logfile) {
		now = time(NULL);
		logfile = malloc(256);
		if (!logfile)
			exit(-1);
		strftime(logfile, 256,"%d_%b_%Y_%T_%z.driftlog", localtime(&now));
	}

	if (getuid()) {
		printf("ERROR: Need to run this as root\n");
		exit(-1);
	}

	/* Kill ntpd, clear any state and set the time properly */
	ret = system("killall -9 ntpd");
	clear_ntp_state();
	set_time(server);


	/* inject fixed offset */
	set_fixed_offset(offset);

	/* put together a config and run ntp */
	generate_ntp_conf(server);
	run_ntpd();

	/* run driftlog */
	pid = run_driftlog(server, runtime, logfile);

	/* wait for driftlog to finish */
	printf("Waiting for logging to finish...\n");
	waitpid(pid, 0,0);

	/* Kill ntpd (again)*/
	ret = system("killall -9 ntpd");

	/* clean it all up */
	cleanup_conf();

	printf("DONE!\n");
	return 0;
}
