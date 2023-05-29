#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    int go;
    pid_t mypid;
<<<<<<< HEAD
    char *argv[] = {"time", "./parallel_dense_mm", "500", NULL};
=======
    const char argv[2];
>>>>>>> 1e7d08ce03762f6a4c93447a758d1b0a8ebf7146
    
    mypid = getpid();
    printf("My PID: %d\n", mypid);
    printf("Press any number to continue...");
    scanf("%d", &go);

<<<<<<< HEAD
    execvp(argv[0], argv);
=======
    argv[0] = "time";
    argv[1] = "500";

    execvp("./parallel_dense_mm", argv);
>>>>>>> 1e7d08ce03762f6a4c93447a758d1b0a8ebf7146

    return 0;
}