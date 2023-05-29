#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

int main() {
    // Print pids
    printf("My pid is %d\n", getpid());
    printf("Parent pid is %d\n", getppid());

    char *mount_point = "/proc4"; // Note: not using arg for mount point

    // Mark root /proc as private
    mount(NULL, "/proc", NULL, MS_PRIVATE, NULL);
    printf("Marked /proc as private\n");

    // Mount pseudofs
    if (mount_point != NULL) {
        mkdir(mount_point, 0555);       /* Create directory for mount point */
        printf("Created mountpoint\n");
        // Mount childspaces's proc pseudofs
        if (mount("/proc", mount_point, "proc", 0, NULL) == -1)
            errExit("mount");
        printf("Mounting child's procfs at %s\n", mount_point);
    }

    // Reset root /proc as shared
    mount(NULL, "/proc", NULL, MS_SHARED, NULL);
    printf("Reset /proc as shared\n");

    execlp("ps", "ps", "u", (char *) NULL);
    errExit("execlp");  /* Only reached if execlp() fails */
}