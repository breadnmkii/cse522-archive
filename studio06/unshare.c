/* unshare.c 

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   Modified unshare script with attribution to original
   namespace changing demonstration script.
*/

#define _GNU_SOURCE
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
    int flags;
    pid_t thispid;
    char *new_hostname = "bread-n-pi";
    char read_hostname[sizeof(char) * (strlen(new_hostname)+1)];    // +1 for null term

    flags = CLONE_NEWUTS;   // specify uts namespace

    // unshare namespace
    if (unshare(flags) == -1)
        errExit("unshare");

    // print PID
    thispid = getpid();
    printf("Process pid is %d\n", thispid);

    // change hostname of new namespace
    sethostname(new_hostname, sizeof(char) * strlen(new_hostname));
    // read new hostname
    gethostname(read_hostname, sizeof(read_hostname));
    printf("New hostname is %s\n", read_hostname);

    while(1) {}
}