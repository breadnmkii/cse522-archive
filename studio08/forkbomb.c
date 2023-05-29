#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>

int main() {
    void *temp;
    while (1)
    {
        temp = malloc(5000);
        sleep(2);
        fork();
    }
    return 0;
}