#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/inotify.h>

#define BUF_LEN 4096

void check_high(int fd) {
    int low = 0, high = 0;
    char buf[128];
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, sizeof(buf));
    sscanf(buf, "low %d\nhigh %d\n",&low, &high);
    printf("high: %d\n", high);
    return;
}

void check_populated(int fd) {
    int populated = 0;
    char buf[128];
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, sizeof(buf));
    sscanf(buf, "populated %d\n", &populated);
    printf("populated: %d\n", populated);
    return;
}


int main(int argc, char * argv[]) {
    if(argc!=3) {
        printf("Usage: %s [path to memory.events] [path to cgroup.events]\n", argv[0]);
        return -1;
    }

    char buf[BUF_LEN] __attribute__((aligned(__alignof__(struct inotify_event))));

    char *memevp = argv[1];
    char *cgevp = argv[2];

    int fd_mem, fd_cg;
    fd_mem = open(memevp, O_RDONLY);
    fd_cg = open(cgevp, O_RDONLY);

    if(fd_mem < 0 || fd_cg < 0) {
        printf("open failed, fd_mem: %d, fd_cg: %d", fd_mem, fd_cg);
        return -1;
    }

    int ifd = inotify_init();
    if(ifd<0) {
        printf("inotify_init failed");
        return -1;
    }
    
    int mem_wd = inotify_add_watch( ifd, memevp, IN_MODIFY);
    int cg_wd = inotify_add_watch( ifd, cgevp, IN_MODIFY);

    while (1)
    {
        /* code */
        ssize_t len, i = 0;
        len = read(ifd, buf, BUF_LEN);
        while(i < len) {
            struct inotify_event *event = (struct inotify_event *) &buf[i];
            printf ("wd=%d\n", event->wd);
            if(event->wd == mem_wd) {
                check_high(fd_mem);
            } else if (event->wd == cg_wd) {
                check_populated(fd_cg);
            }
            i += sizeof(struct inotify_event) + event->len;
        }
    }

    return 0;
}