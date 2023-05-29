#include <stdio.h>
#include <unistd.h>

int main() {
    printf("my pid: %d\n", getpid());
    while (1) {}
    return 0;
}