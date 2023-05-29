#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    if(argc < 2 || argc > 3) {
        printf("Usage: %s <source path> [target path]\n", argv[0]);
        return -1;
    }

    char *path_to = (argc == 3) ? argv[2] : basename(argv[1]);

    if(link(argv[1], path_to) < 0){
        int errsv = errno;
        printf("link failed, errno: %d", errsv);
        return -1;
    }

    return 0;
}