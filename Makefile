CFLAGS += -O3 -Wl,-no-as-needed
LDFLAGS += -lrt -lpthread
bins = inconsistency-check threadtest consistent-offsets leap-a-day	\
	leapcrash raw_skew change_skew set-2038 nanosleep		\
	clocksource-switch nsleep-lat mqueue-lat alarmtimer-suspend	\
	skew_consistency ntp-converge set-tai adj-setoffset		\
	set-timer-lat valid-adjtimex

all: ${bins}

clean:
	rm -f ${bins}

