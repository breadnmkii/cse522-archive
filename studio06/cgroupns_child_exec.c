/* cgroupns_child_exec.c

    This program is based on the original userns_child_exec.c,
    which was retrieved for Washington University's CSE 522S course on January 23, 2021,
    from the original listing at https://lwn.net/Articles/539940/ posted February 27, 2013 by mkerrisk

    Details for userns_child_exec.c:
        Copyright 2013, Michael Kerrisk
        Licensed under GNU General Public License v2 or later

        Create a child process that executes a shell command in new
        namespace(s); allow UID and GID mappings to be specified when
        creating a user namespace.

        Note that the mappings are irrelevant if not creating a new user namespace.

        Uses a pipe to synchronize parent and child,
        ensuring that any mappings occur before child exec's a new program

    This program adds an additional flag to create a cgroups namespace.
    If that flag is specified, the program reads in an additional command-line argument,
    which specifies a cgroup.procs file,
    into which it will place itself before the call to clone().
*/
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

struct child_args {
    char **argv;        /* Command to be executed by child, with arguments */
    int    pipe_fd[2];  /* Pipe used to synchronize parent and child */
};

static int verbose;

/* Displays usage information if command-line arguments are not entered correctly. */
static void
usage(char *pname)
{
    fprintf(stderr, "Usage: %s [options] cmd [arg...]\n\n", pname);
    fprintf(stderr, "Create a child process that executes a shell command "
            "in a new user namespace,\n"
            "and possibly also other new namespace(s).\n\n");
    fprintf(stderr, "Options can be:\n\n");
#define fpe(str) fprintf(stderr, "    %s", str);
    fpe("-i          New IPC namespace\n");
    fpe("-m          New mount namespace\n");
    fpe("-n          New network namespace\n");
    fpe("-p          New PID namespace\n");
    fpe("-u          New UTS namespace\n");
    fpe("-C          New cgroups namespace\n");
    fpe("-U          New user namespace\n");
    fpe("-M uid_map  Specify UID map for user namespace\n");
    fpe("-G gid_map  Specify GID map for user namespace\n");
    fpe("            If -M or -G is specified, -U is required\n");
    fpe("-v          Display verbose messages\n");
    fpe("\n");
    fpe("Map strings for -M and -G consist of records of the form:\n");
    fpe("\n");
    fpe("    ID-inside-ns   ID-outside-ns   len\n");
    fpe("\n");
    fpe("A map string can contain multiple records, separated by commas;\n");
    fpe("the commas are replaced by newlines before writing to map files.\n");

    exit(EXIT_FAILURE);
}

/* Update the mapping file 'map_file', with the value provided in
   'mapping', a string that defines a UID or GID mapping. A UID or
   GID mapping consists of one or more newline-delimited records
   of the form:

       ID_inside-ns    ID-outside-ns   length

   Requiring the user to supply a string that contains newlines is
   of course inconvenient for command-line use. Thus, we permit the
   use of commas to delimit records in this string, and replace them
   with newlines before writing the string to the file. */

static void
update_map(char *mapping, char *map_file)
{
    int fd, j;
    size_t map_len;     /* Length of 'mapping' */

    /* Replace commas in mapping string with newlines */

    map_len = strlen(mapping);
    for (j = 0; j < map_len; j++)
        if (mapping[j] == ',')
            mapping[j] = '\n';

    fd = open(map_file, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "open %s: %s\n", map_file, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (write(fd, mapping, map_len) != map_len) {
        fprintf(stderr, "write %s: %s\n", map_file, strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(fd);
}

static void setgroups_deny(char * setgroups_file)
{
    int fd;
    const char * deny = "deny";
    fd = open(setgroups_file, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "open %s: %s\n", setgroups_file, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (write(fd, deny, strlen(deny)) != strlen(deny)) {
        fprintf(stderr, "write %s: %s\n", setgroups_file, strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(fd);
}

static void
joinCgroup(const char * path) {
	int pid, fd, len, ret;

	/* Get PID and format as string */
	const int pid_len = 16;
	char pidstr[pid_len];
	pid = getpid();
	len = snprintf(pidstr, pid_len, "%d", pid);

	/* Open cgroup.procs file specified in path */
	fd = open(path, O_WRONLY);	
	if (fd == -1) {
		fprintf(stderr, "Failure in child: could not open %s for writing\n", path);
		errExit("");
	}

	/* Write PID into cgroup.procs file */
	printf("Writing %s into %s\n", pidstr, path);
	ret = write(fd, pidstr, len);
	if (ret < len) {
		fprintf(stderr, "Failure in child: could not write PID %s to file %s\n", pidstr, path);
		errExit("");
	}
	close(fd);
}

static int              /* Start function for cloned child */
childFunc(void *arg)
{
    struct child_args *args = (struct child_args *) arg;
    char ch;

    /* Wait until the parent has updated the UID and GID mappings. See
       the comment in main(). We wait for end of file on a pipe that will
       be closed by the parent process once it has updated the mappings. */

    close(args->pipe_fd[1]);    /* Close our descriptor for the write end
                                   of the pipe so that we see EOF when
                                   parent closes its descriptor */
    if (read(args->pipe_fd[0], &ch, 1) != 0) {
        fprintf(stderr, "Failure in child: read from pipe returned != 0\n");
        exit(EXIT_FAILURE);
    }

    /* Execute a shell command */

    execvp(args->argv[0], args->argv);
    errExit("execvp");
}

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */

int
main(int argc, char *argv[])
{
    int flags, opt;
    pid_t child_pid;
    struct child_args args;
    char *uid_map, *gid_map;
    char map_path[PATH_MAX], setgroups_path[PATH_MAX];

    /* Parse command-line options. The initial '+' character in
       the final getopt() argument prevents GNU-style permutation
       of command-line options. That's useful, since sometimes
       the 'command' to be executed by this program itself
       has command-line options. We don't want getopt() to treat
       those as options to this program. */

    flags = 0;
    verbose = 0;
    gid_map = NULL;
    uid_map = NULL;
    while ((opt = getopt(argc, argv, "+imnpuUCM:G:v")) != -1) {
        switch (opt) {
        case 'i': flags |= CLONE_NEWIPC;        break;
        case 'm': flags |= CLONE_NEWNS;         break;
        case 'n': flags |= CLONE_NEWNET;        break;
        case 'p': flags |= CLONE_NEWPID;        break;
        case 'u': flags |= CLONE_NEWUTS;        break;
	case 'C': flags |= CLONE_NEWCGROUP;	break;
        case 'v': verbose = 1;                  break;
        case 'M': uid_map = optarg;             break;
        case 'G': gid_map = optarg;             break;
        case 'U': flags |= CLONE_NEWUSER;       break;
        default:  usage(argv[0]);
        }
    }

    /* -M or -G without -U is nonsensical */
    if ((uid_map != NULL || gid_map != NULL) &&
            !(flags & CLONE_NEWUSER))
        usage(argv[0]);

    /* If joining cgroups namespace*/
    if ( flags & CLONE_NEWCGROUP ) {
	    joinCgroup(argv[optind]);
	    optind++;
    }

    args.argv = &argv[optind];

    /* We use a pipe to synchronize the parent and child, in order to
       ensure that the parent sets the UID and GID maps before the child
       calls execve(). This ensures that the child maintains its
       capabilities during the execve() in the common case where we
       want to map the child's effective user ID to 0 in the new user
       namespace. Without this synchronization, the child would lose
       its capabilities if it performed an execve() with nonzero
       user IDs (see the capabilities(7) man page for details of the
       transformation of a process's capabilities during execve()). */

    if (pipe(args.pipe_fd) == -1)
        errExit("pipe");

    /* Create the child in new namespace(s) */

    child_pid = clone(childFunc, child_stack + STACK_SIZE,
                      flags | SIGCHLD, &args);
    if (child_pid == -1)
        errExit("clone");

    /* Parent falls through to here */

    if (verbose)
        printf("%s: PID of child created by clone() is %ld\n",
                argv[0], (long) child_pid);

    /* Update the UID and GID maps in the child */

    if (uid_map != NULL) {
        snprintf(map_path, PATH_MAX, "/proc/%ld/uid_map",
                (long) child_pid);
        update_map(uid_map, map_path);
    }
    if (gid_map != NULL) {
        snprintf(map_path, PATH_MAX, "/proc/%ld/gid_map",
                (long) child_pid);
        snprintf(setgroups_path, PATH_MAX, "/proc/%ld/setgroups",
                (long) child_pid);
	setgroups_deny(setgroups_path);
        update_map(gid_map, map_path);
    }

    /* Close the write end of the pipe, to signal to the child that we
       have updated the UID and GID maps */

    close(args.pipe_fd[1]);

    if (waitpid(child_pid, NULL, 0) == -1)      /* Wait for child */
        errExit("waitpid");

    if (verbose)
        printf("%s: terminating\n", argv[0]);

    exit(EXIT_SUCCESS);
}