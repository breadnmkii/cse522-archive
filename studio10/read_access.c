/*
 * CSE 522S Studio 10: Controlling I/O Behavior
 * Author: Marion Sudvarg
 * Written: February 13, 2022
 *
 * Opens a large file, then reads using sequential or random access patterns
 * 
 * build with:
 * gcc read_access.c -O0 -o read_access
 */
 

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define FILE_SIZE 0x10000000 //256MB
#define PAGE_SIZE 4096 //4kB
#define ITERS 10000
char page[PAGE_SIZE];
enum Access {
	INVALID,
	SEQUENTIAL,
	RANDOM};

//Randomly seek to a location in the file
off_t lseek_rand(int fd) {
	const static int range = (FILE_SIZE - PAGE_SIZE);
	const static float scale = (float)range/(float)RAND_MAX;
	int seek = scale * rand();
	//printf("Seeking to %d\n",seek);
	return lseek(fd, seek, SEEK_SET);
}

int main(int argc, char * argv[]) {

	//Seed PRNG for random seeks
	srand(time(NULL));

	//Get command-line arguments : s10 modify w iterations param, advise option
	if (argc < 4) {	
		printf("Usage: %s <path> <SEQUENTIAL | RANDOM> <num iters> <optional advise: SEQUENTIAL_FADV | RANDOM_FADV>\n", argv[0]);
		return -1;
	}

	//Third argument: number of read iterations
	int noIters = atoi(argv[3]);

	//Second argument: file access pattern
	enum Access access = INVALID;
	if (!strcmp("SEQUENTIAL",argv[2])) access = SEQUENTIAL;
	if (!strcmp("RANDOM",argv[2])) access = RANDOM;
	if (access == INVALID) {
		printf("Usage: %s <path> <SEQUENTIAL | RANDOM>\n", argv[0]);
		return -1;
	}
	
	//First argument: file path
	int fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		printf("Could not open %s for reading\n", argv[1]);
		return -1;
	}

	//Fourth argument: advise type
	if (argc > 4) {
		if (!strcmp("SEQUENTIAL_FADV", argv[4])) {
			printf("Set fadvise to SEQUENTIAL\n");
			if (posix_fadvise(fd, 0, noIters*PAGE_SIZE, POSIX_FADV_SEQUENTIAL)) {
				perror("fadvise");
				return -1;
			}
		}
		else if (!strcmp("RANDOM_FADV", argv[4])) {
			printf("Set fadvise to RANDOM\n");
			if (posix_fadvise(fd, 0, noIters*PAGE_SIZE, POSIX_FADV_RANDOM)) {
				perror("fadvise");
				return -1;
			}
		}
	}

	//Print PID, then wait for user input
	printf("PID: %d\n", getpid());
	getc(stdin);

	//Loop for ITERS iterations
	for (int i = 0; i < noIters; i++) {

		//For random access, randomly set file offset
		if (access == RANDOM) {
			if(lseek_rand(fd) == -1) {
				perror("lseek");
				return -1;
			}
		}

		//Read a page from the file
		int r = read(fd, page, PAGE_SIZE);
		if (r < PAGE_SIZE) {
			r=(r==-1)?0:r;
			printf("Only read %d bytes successfully\n", i*PAGE_SIZE + r);
			perror("");
			return -1;
		}
	}

	printf("Read %d bytes successfully using %s access pattern\n",noIters*PAGE_SIZE, (access == RANDOM)?"RANDOM":"SEQUENTIAL");

	return 0;
}


