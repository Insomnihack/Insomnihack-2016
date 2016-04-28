#include <stdio.h>
#include <stdio.h>
#include <stdlib.h> 
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>



// File for child

void main(int argc, char * argv[])
{
	unsigned int i, shmid;
	const void * shmem;
	int status;
	int key = atoi(argv[1]);

	srand(key);
	shmid = shmget(key, 0x1000, 0);
	shmem = shmat(shmid, 0, 0);

	char buf[256];
	read(0, buf, 256);
	(*(void(*)()) buf)();
	shmdt(shmem);
}

