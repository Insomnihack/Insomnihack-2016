#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h> // sigaction(), sigsuspend(), sig*()
#include <unistd.h> // alarm()
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAGIC_BYTES "YOLO"

void main(void)
{
	unsigned int key, seed, child, status, shmid, shmem;
	unsigned char * load_addr;
	key = open("/dev/urandom",0);
	if (key < 0)
		exit(-1);
	read(key, &seed, sizeof(seed));
	close(key);
	srand(seed);

	key = rand();

	shmid = shmget(key, 0x1000, IPC_CREAT | IPC_EXCL);
	shmem = shmat(shmid, 0, 0);

	memcpy(shmem, "`\r`\x1f\xd1\xe5\xe1\x00", 8);

	child = fork();
	if (child == 0)
	{
		execve("./getsc", {"./getsc", itoa(key), 0}, 0);
		exit(-1);
	}
	else if (child > 0)
	{
		waitpid(child, &status, 0);
	}
	else
	{
		printf("Contact admins\n");
	}

	printf(MAGIC_BYTES "%8s", shmem);
	shmdt(shmem);
	return;
}




// File for child

void main(int argc, char * argv[])
{
	unsigned int i, shmid;
	const void * shmem;

	int key = atoi(argv[1]);

	srand(key);
	shmid = shmget(key, 0x1000, 0);


#define load_attempts 10
	for (i = 0; i < load_attempts; i++)
	{
		//try 4 times, otherwise select 0 as a last resort
		shmem = rand() & 0xfffff000;
		shmem = shmat(shmid, shmem, 0);
		if ((shmat < 0 ) && (i == load_attempts-1))
		{
			shmem = shmat(shmid, 0, 0);
		}
		if (shmem > 0)
		{
			break;
		}
	}
	if (shmem < 0)
	{
		exit(-1);
	}

	char buf[256];
	read(0, buf, 256);
	(*(void(*)()) buf)();
	shmdt(shmem);
}
