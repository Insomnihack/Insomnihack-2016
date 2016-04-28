#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SLICE 256
#define MAX_OVERHEAT 4
static unsigned long gcanary = 0;
static int overheat = 0;
static char gpassw[] = "How Large Is A Stack Of Toast?\n";

void checkpass(char * passw)
{
	int n = 0;
	long canary = gcanary;
	printf("Passphrase : ");
	n = read(0, passw, 32);
	if (n < 0)
		exit(-1);
	passw[n] = 0;
	if (strcmp(gpassw,passw) != 0)
	{
		printf("Access denied!\nNo toast today :-(\n");
		exit(-1);
	}
	if (canary != gcanary)
		exit(0);
	printf("Access granted!\n");
}

void show_bread(char * bread)
{
	int x;
	for (x=0; x<MAX_SLICE; x++)
	{
		printf("[%3hhu]", bread[x]);
		if ((x + 1) % 16 == 0 )
			printf("\n");
	}
}

void handle_bread(char * t_bread, int verbose)
{
	char rchoice[4];
	int i = 0;
	int choice;
	int temp;

	long canary = gcanary;

	do
	{
		if (overheat == MAX_OVERHEAT)
		{
			printf("The bread reserve tank is empty... Quitting\n");
			return;
		}
		if (verbose)
		{
			printf("Bread status: \n");
			show_bread(t_bread);
		}

		printf("Which slice do you want to heat?\n");
		read(0, &rchoice, 4);
		if ((rchoice[0] == 'q') || (rchoice[0] == 'x'))
			return;

		if ( sscanf(rchoice, "%d", &choice))
		{
			if (choice < MAX_SLICE)
			{
				printf("Toasting %d!\n", choice);
				temp = t_bread[choice] + (rand() % 256);
				if ( temp > 256 )
				{
					printf("Detected bread overheat, replacing\n");
					t_bread[choice] = 0;
					++overheat;
				}
				else
				{
					t_bread[choice] = temp;
				}
				++i;
			}
		}
	} while (i<MAX_SLICE+MAX_OVERHEAT);

	if (canary != gcanary)
		exit(-1);
	return;
}

typedef struct {
	int verbose;
	char t_bread[MAX_SLICE];
	char passw[32];
	int rndfd;
	long canary;
} MStack;

int main(int argc, char *argv[])
{
	MStack mstack;
	mstack.verbose=0;
	int seed;

	setvbuf(stdout, NULL, _IONBF, 0);

	memset(&mstack.t_bread,0,256);
	if (argc > 1)
		mstack.verbose = 1;

	mstack.rndfd = open("/dev/urandom",0); // gets overwritten by getname
	read(mstack.rndfd,&gcanary,sizeof(gcanary));
	mstack.canary = gcanary;

	printf("Welcome to Internet of Toaster!\nFeaturing \"Random Heat Distribution\" (patent pending)\n");
	checkpass(mstack.passw);
	printf("This next-gen toaster allows for %d slices of bread !\n", MAX_SLICE);
	printf("It also has a small tank of replacement bread if you burn one, which is a huge improvement over the netbsd-based models!\n");

	// printf("Reading seed from fd %d\n", mstack.rndfd);
	read(mstack.rndfd, &seed, 4);
	// printf("Red seed %08x\n", seed);

	srand(seed);

	handle_bread(mstack.t_bread, mstack.verbose);

	printf("Well, you've had your toasting frenzy!\nCheers\n");

	// which slice to heat ?
	if (mstack.canary != gcanary)
		exit(-1);
	return 0;
}
