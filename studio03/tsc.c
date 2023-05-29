#include <stdio.h>
#include <limits.h>
#include <math.h>

#define max_value  ULLONG_MAX;
#define min_value  0;

unsigned long long elasped_time[100];

static inline unsigned long long rdtsc_get(void) {
    unsigned long high, low;
    asm volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ( (unsigned long long) low) |
            ( ( (unsigned long long) high ) << 32 );
}
int main(){
    unsigned long long min = max_value;
    unsigned long long max = min_value;
    unsigned long long sum = 0;
    unsigned long long mean = 0;
    unsigned long long stdiv = 0;
    for (int i = 0; i < 100; i++){
        unsigned long long start = rdtsc_get();
        unsigned long long end = rdtsc_get();
        unsigned long long temp = end-start;
        if(temp > 0){
            elasped_time[i] = end-start;
            if(temp < min){
                min = temp;
            }
            if(temp > max){
                max = temp;
            }
            sum += temp;
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
    return 1;
}
