/* ns_exec.c 

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   Modified script for joining namespaces.
*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

int
main(int argc, char *argv[])
{
    int fd;
    char *ns_path;
    char *pidStr;

    // require two args (pid, execvpe cmd)
    if (argc != 3) {
        fprintf(stderr, "%s <PID> <cmd>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int pidChars = strlen(argv[1]);

    // Compose namespace path
    ns_path = malloc(strlen("/proc//ns/uts") + pidChars);
    strncpy(ns_path, "/proc/", 6);
    strcat(ns_path, argv[1]);
    strcat(ns_path, "/ns/uts");
    printf("Namespace path: %s\n", ns_path);


    fd = open(ns_path, O_RDONLY);   /* Get descriptor for namespace */
    if (fd == -1)
        errExit("open");

    if (setns(fd, 0) == -1)         /* Join that namespace */
        errExit("setns");

    execvp(argv[2], &argv[2]);      /* Execute a command in namespace */
    errExit("execvp");
}
