/*
 * CSE 522S Studio 10: Controlling I/O Behavior
 * Author: Marion Sudvarg
 * Written: February 13, 2022
 *
 * Opens a large file, then writes using sequential or random access patterns
 * 
 * build with:
 * gcc write_access.c -O0 -o write_access
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

//Fills page of memory with cycle of ascii characters 32-126 inclusive
void fill_page(void) {
	for (int i = 0; i < PAGE_SIZE; i++) {
		page[i] = (char)(i%(126-32+1) + 32);
	}
}

int main(int argc, char * argv[]) {

	//Seed PRNG for random seeks
	srand(time(NULL));

	//Get command-line arguments
	if (argc < 3) {
		printf("Usage: %s <path> <SEQUENTIAL | RANDOM>\n", argv[0]);
		return -1;
	}

	//Second argument: file access pattern
	enum Access access = INVALID;
	if (!strcmp("SEQUENTIAL",argv[2])) access = SEQUENTIAL;
	if (!strcmp("RANDOM",argv[2])) access = RANDOM;
	if (access == INVALID) {
		printf("Usage: %s <path> <SEQUENTIAL | RANDOM>\n", argv[0]);
		return -1;
	}

	//First argument: file path
	int fd = open(argv[1], O_WRONLY);
	if (fd == -1) {
		printf("Could not open %s for writing\n", argv[1]);
		return -1;
	}

	//Fill memory page with contents for writing to file
	fill_page();

	//Print PID, then wait for user input
	printf("PID: %d\n", getpid());
	getc(stdin);

	//Loop for ITERS iterations
	for (int i = 0; i < ITERS; i++) {

		//For random access, randomly set file offset
		if (access == RANDOM) {
			if(lseek_rand(fd) == -1) {
				perror("lseek");
				return -1;
			}
		}

		//Read a page from the file
		int r = write(fd, page, PAGE_SIZE);
		if (r < PAGE_SIZE) {
			r=(r==-1)?0:r;
			printf("Only wrote %d bytes successfully\n", i*PAGE_SIZE + r);
			perror("");
			return -1;
		}
	}

	printf("Wrote %d bytes successfully using %s access pattern\n",ITERS*PAGE_SIZE, (access == RANDOM)?"RANDOM":"SEQUENTIAL");

	return 0;
}


