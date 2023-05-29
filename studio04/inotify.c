#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>

#define BUF_LEN 4096
#define CHECKFLAG(event,flag) if (event->mask & flag) printf("%s ", #flag);

int main(int argc, char *argv[]){
	char str[200];
	int length;
	
    if(argc != 2) {
        printf("Usage: %s <file path>\n", argv[0]);
        return -1;
    }

    int fd;
    if ((fd = inotify_init1(0)) == -1)
    {
        int errsv = errno;
        printf("inotify_init1 failed, errno: %d\n", errsv);
        return -1;
    }

    int watch_fd;
    if ((watch_fd = inotify_add_watch(fd, argv[1], IN_ALL_EVENTS)) == -1)
    {
        int errsv = errno;
        printf("inotify_add_watch failed, errno: %d\n", errsv);
        return -1;
    }

    printf("inotify_add_watch success. inotify file descriptor: %d, descriptor for water: %d, file path: %s\n", fd, watch_fd, argv[1]);

    struct pollfd fds[2];
    int timeout_msecs = 500;
    fds[0].fd = fd;
    fds[0].events = POLLIN;
	fds[1].fd = 0;//STDIN_FILENO;
	fds[1].events = POLLIN;
	
    char buf[BUF_LEN] __attribute__((aligned(__alignof__(struct inotify_event))));

    while (1)
    {
        int poll_ret = poll(fds, 2, timeout_msecs);
        if(poll_ret > 0) {
            if(fds[0].revents & POLLIN){
                ssize_t len, i = 0;
                len = read(fd, buf, BUF_LEN);
                //printf("len: %d\n", len);
                while(i < len) {
                    struct inotify_event *event = (struct inotify_event *) &buf[i];
                    printf ("wd=%d mask=%d cookie=%d len=%d dir=%s\n", event->wd, event->mask, event->cookie, event->len, (event->mask & IN_ISDIR) ? "yes" : "no");
                    if (event->len)
                        printf ("name=%s\n", event->name);
                    CHECKFLAG(event, IN_ACCESS)
                    CHECKFLAG(event, IN_MODIFY)
                    CHECKFLAG(event, IN_ATTRIB)
                    CHECKFLAG(event, IN_CREATE)
                    CHECKFLAG(event, IN_DELETE)
                    CHECKFLAG(event, IN_OPEN)
                    CHECKFLAG(event, IN_CLOSE)
                    i += sizeof(struct inotify_event) + event->len;
                }
            }
            if(fds[1].revents & POLLIN){
				printf("Standard input stream ready.\n");
				// typing ...
				fgets(str, sizeof(str), stdin);
				length = strlen(str);
				if(str[length-1] == '\n'){
					str[length-1] = '\0';
				}
				printf("The string is: %s\n", str);
				int watch_fd2;
				if ((watch_fd2 = inotify_add_watch(fd, str, IN_ALL_EVENTS)) == -1)
				{
					int errsv = errno;
					printf("standard add_watch failed, errno: %d\n", errsv);
					//return -1;
				}
				
			}
        }
    }

    return 0;
}
