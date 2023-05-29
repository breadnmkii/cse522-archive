/* ns_child_exec.c

    This program was retrieved by Marion Sudvarg
    for Washington University's CSE 522S course on January 22, 2021,
    from the original listing at https://lwn.net/Articles/533492/
    posted January 21, 2013 by mkerrisk

    Besides additional comments throughout the code, the program matches the original.

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   Create a child process that executes a shell command in new namespace(s).   
*/
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

/* Displays usage information if command-line arguments are not entered correctly. */
static void
usage(char *pname)
{
    fprintf(stderr, "Usage: %s [options] cmd [arg...]\n", pname);
    fprintf(stderr, "Options can be:\n");
    fprintf(stderr, "    -i   new IPC namespace\n");
    fprintf(stderr, "    -m   new mount namespace\n");
    fprintf(stderr, "    -n   new network namespace\n");
    fprintf(stderr, "    -p   new PID namespace\n");
    fprintf(stderr, "    -u   new UTS namespace\n");
    fprintf(stderr, "    -U   new user namespace\n");
    fprintf(stderr, "    -v   Display verbose messages\n");
    exit(EXIT_FAILURE);
}

static int              /* Start function for cloned child */
childFunc(void *arg)
{

    char **argv = arg;

    /* Execute command, passing any additional arguments */
    execvp(argv[0], &argv[0]);
    errExit("execvp");
}

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */

int
main(int argc, char *argv[])
{
    int flags, opt, verbose;
    pid_t child_pid;

    flags = 0;
    verbose = 0;

    /* Parse command-line options. The initial '+' character in
       the final getopt() argument prevents GNU-style permutation
       of command-line options. That's useful, since sometimes
       the 'command' to be executed by this program itself
       has command-line options. We don't want getopt() to treat
       those as options to this program. */

    while ((opt = getopt(argc, argv, "+imnpuUv")) != -1) {
        switch (opt) {
        case 'i': flags |= CLONE_NEWIPC;        break;
        case 'm': flags |= CLONE_NEWNS;         break;
        case 'n': flags |= CLONE_NEWNET;        break;
        case 'p': flags |= CLONE_NEWPID;        break;
        case 'u': flags |= CLONE_NEWUTS;        break;
        case 'U': flags |= CLONE_NEWUSER;       break;
        case 'v': verbose = 1;                  break;
        default:  usage(argv[0]);
        }
    }

    /*
        Create a child process, using clone(), creating new namespaces
        as specified by command-line arguments.
        In the absense of the CLONE_VM flag,
        the child process receives its own virtual address space
        (as it would with a fork() call), but unlike fork,
        it begins executing in the childFunc function,
        and therefore uses a new stack.

        It additionally passes the address of the next command-line argument,
        after the arguments that were parsed, to the child function.
    */
    child_pid = clone(childFunc,
                    child_stack + STACK_SIZE,
                    flags | SIGCHLD, &argv[optind]);
    if (child_pid == -1)
        errExit("clone");

    if (verbose)
        printf("%s: PID of child created by clone() is %ld\n",
                argv[0], (long) child_pid);

    /* Parent falls through to here */

    if (waitpid(child_pid, NULL, 0) == -1)      /* Wait for child */
        errExit("waitpid");

    if (verbose)
        printf("%s: terminating\n", argv[0]);
    exit(EXIT_SUCCESS);
}