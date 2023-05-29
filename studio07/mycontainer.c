// Custom container program implementations for use in simple_init.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>
#include <sys/syscall.h>

#include "mycontainer.h"

static int
pivot_root(const char *new_root, const char *put_old) {
    return syscall(SYS_pivot_root, new_root, put_old);
}

int makeContainer(const char *path) {

    // Recursively set all mounts from root as private
    mount("", "/", "", MS_BIND | MS_REC, NULL);
    
    // Bind mount specified directory to itself as a mount-point
    mount(path, path, "", MS_BIND, NULL);

    // Change working dir to supplied path
    chdir(path);

    // Bind mount all basic root directories (listed in handout)
    // filesystem types located in /proc/filesystems
    // MS_REMOUNT | MS_BIND | MS_RDONLY
    mount("/bin", "bin", NULL, MS_BIND | MS_RDONLY, NULL);
    mount("/dev/console", "dev/console", NULL, MS_BIND, NULL);
    mount("/dev/pts", "dev/pts", NULL, MS_BIND, NULL);
    mount("/dev/shm", "dev/shm", NULL, MS_BIND, NULL);
    mount("/etc/hostname", "etc/hostname", NULL, MS_BIND, NULL);
    mount("/etc/hosts", "etc/hosts", NULL, MS_BIND, NULL);
    mount("/lib", "lib", NULL, MS_BIND | MS_RDONLY, NULL);
    mount("proc","proc","proc", 0, NULL);
    mount("/sys", "sys", "sysfs", MS_BIND | MS_RDONLY, NULL);
    mount("/usr/bin", "usr/bin", NULL, MS_BIND | MS_RDONLY, NULL);
    mount("/usr/lib","usr/lib",NULL, MS_BIND | MS_RDONLY, NULL);

    // Make old-root subdirectory
    if (mkdir("old-root", S_IRWXU) < 0) {
        if (errno == EEXIST) {
            perror("NOTE");
        }
        else {
            perror("mkdir");
            return -1;
        }
    }
    // and swap with new container root (old-root is tmp)
    if (pivot_root(".", "old-root") < 0) {
        perror("pivot_root");
        return -1;
    }



}

