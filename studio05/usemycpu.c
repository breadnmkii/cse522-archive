#include <unistd.h>
#include <stdio.h>

int main() {
	double rpi;
	pid_t pid = getpid();
	printf("pid: %d\n", (int)pid);
	while (1) {
		rpi = 22/7;
	}
}
