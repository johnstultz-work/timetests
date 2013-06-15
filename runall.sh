make
# correctness tests, observational only
echo
echo "Consistency check:"
echo "=================="
./inconsistency-check

# XXX Offset check gives too many false positives. Skip for now
#echo
#echo "Offset check:"
#echo "============="
#./consistent-offsets

echo
echo "Raw skew check:"
echo "==============="
./raw_skew

echo
echo "Nanosleep timers check:"
echo "======================="
./nanosleep

echo
echo "Nanosleep latency check:"
echo "======================="
./nsleep-rel-lat

echo
echo "Mqueue latency check:"
echo "======================="
./mqueue-lat

echo
echo "Alarmtimer suspend check:"
echo "========================="
./alarmtimer-suspend


# "destructive tests" that may change the date, insert leapseconds, etc
echo
echo "Adjtimex freq adjustment tests:"
echo "==============================="
./change_skew

echo
echo "Clocksource changing tests:"
echo "==============================="
./clocksource-switch

echo
echo "Leap second tests:"
echo "=================="
# test for known historic bugs
./leap-a-day -s -i 10
./leapcrash


# time value edge cases
echo
echo "Settimeofday edge cases:"
echo "========================"
./set-2038


# longer running stress tests
./threadtest -t 300 -n 16
./threadtest -t 300 -n 16 -i

