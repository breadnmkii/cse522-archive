
/******************************************************************************
*
* enable_ccnt_522.c
*
* This Linux kernel module, when loaded,
* enables the ARM Cortex-A53 cycle counter
* on every core of the CPU.
*
* Modified November 29, 2021 by Marion Sudvarg
*
******************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <asm/perfmon.h>

// Kernel Math includes
#include <linux/math64.h>
#include <linux/limits.h>

static bool cycles_64_bit = 1;
static bool count_every_64 = 0;

static void enable_ccnt(void *info) {
        pmccntr_enable_once(cycles_64_bit, count_every_64);
}

static int enable_ccnt_init(void) {
	int i;
	uint64_t buf[100];	

	uint64_t min = U64_MAX;
	uint64_t max = 0;
	uint64_t sum = 0;
	uint64_t stddev = 0;
	uint64_t mean;
	uint64_t start, end, elapsed_t;

        on_each_cpu(enable_ccnt,NULL,0);

        printk (KERN_INFO "Cycle Counter Enabled\n");

	// Measure elapsed time between pmcntr reads
	for (i=0; i<100; ++i) {
		start = pmccntr_get();
		end = pmccntr_get();
		
		// Ensure no overflow
		if (end < start) {
			--i;
			continue;
		}
		
		elapsed_t = end - start;
		buf[i] = elapsed_t;

		// Calculate min and max
		if (elapsed_t > max) {
			max = elapsed_t;
		}
		if (elapsed_t < min) {
			min = elapsed_t;
		}
		// Accumulate sum
		sum += elapsed_t;
		printk("start %lu end %lu elapsed %lu sum %lu\n", (unsigned long)start, (unsigned long)end, (unsigned long)elapsed_t, (unsigned long)sum);
	}

	// Calculate mean
	mean = div_u64(sum, 100);
	
	// Calculate stddev
	printk("stddev initial value: %lu\n", (unsigned long)stddev);
	for (i=0; i<100; i++){
		printk("buf[i]: %lu\n", (unsigned long)buf[i]);
		printk("sqr diff sum: %lu\n", (unsigned long)stddev);
        	if(buf[i] > mean){
            		stddev += (buf[i]-mean)*(buf[i]-mean);
        	} else {
            		stddev += (mean-buf[i])*(mean-buf[i]);
        	}
        }
	stddev = int_sqrt64( div_u64(stddev, 100) );

	printk(KERN_INFO "min: %lu, max: %lu, mean: %lu, stddev: %lu\n", (unsigned long)min, (unsigned long)max, (unsigned long)mean, (unsigned long)stddev);

    return 0;
}

static void enable_ccnt_exit(void) {
}

module_init(enable_ccnt_init);
module_exit(enable_ccnt_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Marion Sudvarg");
MODULE_DESCRIPTION("Enable ARM Cortex-A53 Cycle Counter");
