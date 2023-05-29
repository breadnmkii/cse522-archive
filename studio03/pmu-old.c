#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <math.h>
#include <limits.h>

#define max_value  ULLONG_MAX;
#define min_value  0;
unsigned long long elasped_time[100];

static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags);
    return ret;
}
int
main(int argc, char **argv)
{
    struct perf_event_attr pe;
    pid_t mypid = getpid();
    printf(":%d\n ", mypid);
    long long count;
    int fd;
	unsigned long long min = max_value;
	unsigned long long max = min_value;
	unsigned long long sum = 0;
	unsigned long long mean = 0;
	unsigned long long stdiv = 0;
	
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.disabled = 0;
    pe.type = PERF_TYPE_HARDWARE;
    pe.exclude_hv = 1;
    pe.size = sizeof(struct perf_event_attr);
    pe.enable_on_exec = 1;
    pe.inherit = 1;
    pe.exclude_kernel = 1;
    pe.config = PERF_COUNT_HW_CPU_CYCLES;
    fd = perf_event_open(&pe, mypid, -1, -1, 0);
    

    
    for (int i = 0; i < 100; i++){
		ioctl(fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
		//printf("Measuring cycles count for this printf\n");
		ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
		
		read(fd, &count, sizeof(long long));
		printf("count: %lld\n", count);
		
		//int temp = end-start;
		if(count > 0){
			elasped_time[i] = count;
			if(count < min){
				min = count;
			}
			if(count > max){
				max = count;
			}
			sum += count;
		}
		
	}
    mean = sum/100;
	for (int i = 0; i < 100; i++){
        if(elasped_time[i] > mean){
            stdiv += pow(elasped_time[i] - mean , 2);
        }else{
            stdiv += pow(mean - elasped_time[i] , 2);
        }
        // printf("elasped: %llu: ", elasped_time[i]);
        // printf("pow: %llu: ", pow((elasped_time[i] - mean) , 2));
    }
    stdiv = sqrt(stdiv/100);
    printf("min: %llu: ", min);
    printf("max: %llu: ", max);
    printf("mean: %llu: ", mean);
    printf("stdiv: %llu: ", stdiv);
   

    close(fd);
}
