#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <math.h>
#include <limits.h>

unsigned long long elapsed_cycles[100];

static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags);
    return ret;
}

int main() {

	struct perf_event_attr pe;
	long long pmu_count;
	int pmu_fd;

	pid_t thispid = getpid();

	// Stats vars
	unsigned long long min = ULLONG_MAX;
	unsigned long long max = 0;
	unsigned long long sum = 0;
	unsigned long long mean = 0;
	unsigned long long stdiv = 0;

	// clear struct
	memset(&pe, 0, sizeof(struct perf_event_attr));

	// set perf event attributes
	pe.disabled = 0;
	pe.type = PERF_TYPE_HARDWARE;
	pe.exclude_hv = 1;
	pe.size = sizeof(struct perf_event_attr);
	pe.enable_on_exec = 1;
	pe.inherit = 1;
	pe.exclude_kernel = 0;
	pe.config = PERF_COUNT_HW_CPU_CYCLES;
	

	// pmu_fd = SYS_perf_event_open(&pe, procpid, -1, -1, 0);
	pmu_fd = perf_event_open(&pe, thispid, -1, -1, 0);
	if (pmu_fd == -1) {
		perror("perf event_open failed!\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < 100; i++){
		// Reset counter
		ioctl(pmu_fd, PERF_EVENT_IOC_RESET, 0);

		// Measure count between two successive reads?
		ioctl(pmu_fd, PERF_EVENT_IOC_ENABLE, 0);
		ioctl(pmu_fd, PERF_EVENT_IOC_DISABLE, 0);
		read(pmu_fd, &pmu_count, sizeof(pmu_count));
		printf("count: %lld\n", pmu_count);
		
		if(pmu_count > 0){
			elapsed_cycles[i] = pmu_count;
			if(pmu_count < min){
				min = pmu_count;
			}
			if(pmu_count > max){
				max = pmu_count;
			}
			sum += pmu_count;
		}
		
	}
    mean = sum/100;

	for (int i = 0; i < 100; i++){
        if(elapsed_cycles[i] > mean){
            stdiv += pow(elapsed_cycles[i] - mean , 2);
        }else{
            stdiv += pow(mean - elapsed_cycles[i] , 2);
        }
    }
    stdiv = sqrt(stdiv/100);
    printf("min: %llu, ", min);
    printf("max: %llu, ", max);
    printf("mean: %llu, ", mean);
    printf("stdiv: %llu\n", stdiv);
   
    close(pmu_fd);

}
