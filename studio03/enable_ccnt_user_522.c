#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include "perfmon.h"

int main() {
    int i;
	unsigned long buf[100];	

	unsigned long min = ULONG_MAX;
	unsigned long max = 0;
	unsigned long sum = 0;
	unsigned long stddev = 0;
	unsigned long mean;
	unsigned long start, end, elapsed_time;

    printf("Cycle Counter Enabled\n");

	// Measure elapsed time between pmcntr reads
	for (i=0; i<100; ++i) {
		start = pmccntr_get();
		end = pmccntr_get();
		
		// Ensure no overflow
		if (end < start) {
			--i;
			continue;
		}
		
		elapsed_time = end - start;
		buf[i] = elapsed_time;

		// Calculate min and max
		if (elapsed_time > max) {
			max = elapsed_time;
		}
		if (elapsed_time < min) {
			min = elapsed_time;
		}
		// Accumulate sum
		sum += elapsed_time;
		printf("start %lu end %lu elapsed %lu sum %lu\n", (unsigned long)start, (unsigned long)end, (unsigned long)elapsed_time, (unsigned long)sum);
	}

	// Calculate mean
	mean = sum/100;
	
	// Calculate stddev
	printf("stddev initial value: %lu\n", (unsigned long)stddev);
	for (i=0; i<100; i++){
		printf("buf[i]: %lu\n", (unsigned long)buf[i]);
		printf("sqr diff sum: %lu\n", (unsigned long)stddev);
        	if(buf[i] > mean){
            		stddev += (buf[i]-mean)*(buf[i]-mean);
        	} else {
            		stddev += (mean-buf[i])*(mean-buf[i]);
        	}
        }
	stddev = sqrt(stddev/100);

	printf("min: %lu, max: %lu, mean: %lu, stddev: %lu\n", (unsigned long)min, (unsigned long)max, (unsigned long)mean, (unsigned long)stddev);

    return 0;
}