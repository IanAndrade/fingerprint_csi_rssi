#include <Arduino.h>
#include "functions.h"


unsigned long return_end_time(unsigned long time_millis)
{
	unsigned long start_time;
	unsigned long end_time;
	start_time = millis();
	// Avoid overflow problem
	if ((ULONG_MAX - start_time) <= time_millis)
		end_time = ULONG_MAX;
	else
		end_time = start_time + time_millis;

	return end_time;
}


