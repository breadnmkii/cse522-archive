#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main() {
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Child
        printf("(Child) my_pid: %ld  parent_pid: %ld\n", getpid(), getppid());
        sleep(2);
        // Orphaned at this point
        printf("(Child) parent_pid: %ld\n", getppid());

    } else {
        // Parent
        printf("(Parent) child_pid: %ld  my_pid: %ld  parent_pid: %ld\n", pid, getpid(), getppid());
        sleep(1);
    }
    
    return 0;
}

