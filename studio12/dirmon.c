#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>

#define BUF_LEN 4096
#define handle_err() while (1);
#define CHECKFLAG_LOG(event, flag, logf) if (event->mask & flag) fprintf(logf, "%s ", #flag);

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Usage: %s <directory> <output log path>\n", argv[0]);
		handle_err();
	}

	// Initialize file descriptors
	int fd;
	if ( (fd = inotify_init()) == -1) {
		perror("init");
		handle_err();
	}

	int watchfd;
	if ( (watchfd = inotify_add_watch(fd, argv[1], IN_ALL_EVENTS)) == -1) {
		perror("watch");
		handle_err();
	}

	FILE *logfile;
	if ( (logfile = fopen(argv[2], "w")) == NULL) {
		perror("open");
		handle_err();
	}
	printf("Initialized log file\n");

	// Initialize polling
	struct pollfd fds[1];
	int timeout_msecs = 500;
	fds[0].fd = fd;
	fds[0].events = POLLIN;

	char buf[BUF_LEN] __attribute__((aligned(__alignof__(struct inotify_event))));
	
	// Polling
	printf("Logging events...\n");
	while (1) {
		if (poll(fds, 1, timeout_msecs) > 0) {
			// Received POLLIN events from inotify fd
			if (fds[0].revents & POLLIN) {
				// Read each inotify event struct
				struct inotify_event *event;
				ssize_t len, i = 0;
				len = read(fd, buf, BUF_LEN);

				while (i<len) {
					event = (struct inotify_event *) &buf[i];
					CHECKFLAG_LOG(event, IN_ACCESS, logfile)
                    CHECKFLAG_LOG(event, IN_MODIFY, logfile)
                    CHECKFLAG_LOG(event, IN_ATTRIB, logfile)
                    CHECKFLAG_LOG(event, IN_CREATE, logfile)
                    CHECKFLAG_LOG(event, IN_DELETE, logfile)
                    CHECKFLAG_LOG(event, IN_OPEN, logfile)
                    CHECKFLAG_LOG(event, IN_CLOSE, logfile)
					fprintf(logfile, "wd=%d mask=%d cookie=%d len=%d dir=%s\n", event->wd, event->mask, event->cookie, event->len, (event->mask & IN_ISDIR) ? "yes" : "no");
					if (event->len) {
                        fprintf (logfile, "name=%s\n", event->name);
					}

					fflush(logfile);
                    i += sizeof(struct inotify_event) + event->len;	// incrm to next event (include name length)
				}
			}
		}
	}
	
	return 0;
}
