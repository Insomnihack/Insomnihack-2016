#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void main(void)
{
//	char *argv[] = {"./qemu-arm", "-L", "./", "-nopie", "-nx", "-wxorx", "./retoasted", NULL};
	char *argv[] = {"./qemu-arm", "-L", "./", "-nx", "-wxorx", "./retoasted", NULL};
	char *envp[] =
	{
        "HOME=/",
        "USER=retoasted",
	"FLAG=plopilol",
        0
	};

	chdir("/home/retoasted/challenge");
	if (access("dev/urandom", F_OK) != 0)
	{
		printf("Ooops, contact admins\n");
		exit(-1);
	}
	if ( chroot("/home/retoasted/challenge") == 0 )
		execve(argv[0], argv, envp);
	printf("Error chrooting\n");
	exit(-1);
}
