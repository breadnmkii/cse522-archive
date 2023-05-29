#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <errno.h>
#include <stdbool.h>
char storeValue[255];
FILE* ptr;
void myInterruptHandler (int signum) { 
    printf("Inotify interrupted");
    fprintf(ptr, storeValue);
    fputs(storeValue, ptr);
    fclose(ptr);
    exit(1);
 }
int main(){
	
    char ch;
    char buff[255];
    
	int storeFlag = 1;
	char oldValue[255];
	
		ptr = fopen("/proc/sys/fs/inotify/max_user_watches", "r");
		if (NULL == ptr) {
			printf("file can't be opened \n");
		}
		//printf("The number of max user watch \n");
		fscanf(ptr, "%s", buff);
		printf("max user watch : %s\n", buff );
		
		if(storeFlag){
			memcpy(storeValue, buff, 255);
			storeFlag = 0;
			memcpy(oldValue, buff, 255);
			printf("store value: %s\n", storeValue);
		}
		
	typedef void (*sighandler_t)(int);
    sighandler_t oldHandler = signal(SIGINT, myInterruptHandler);
    while(true){
		printf("Ctrl + C can't kill me!!\n");
		sleep(1000);
    }
    //Change back to the old handler
    //signal(SIGINT, oldHandler);
    //alternatively:  signal(SIGINT, SIG_DFL);
 
    fclose(ptr);
    
    return 0;
}
