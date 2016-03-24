#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void usage(void)
{
	printf("Almost there... just trying to make sure you can execute arbitrary commands....\n");
	printf("Write 'Give me a...' on my stdin, wait 2 seconds, and then write '... flag!'.\n");
	printf("Do not include the quotes. Each part is a different line.\n");
}

int main(int argc, const char *argv[])
{
	char buf[1024] = {0};
	int fd;
	time_t t1;
	time_t t2;

	if (read(STDIN_FILENO, &buf, sizeof(buf)) <= 0) {
		usage();
		return 1;
	}

	if (strncmp(buf, "Give me a...", 12)) {
		usage();
		return 1;
	}

	t1 = time(NULL);

	if (read(STDIN_FILENO, &buf, sizeof(buf)) <= 0) {
		perror("read");
		return 1;
	}

	if (strncmp(buf, "... flag!", 9)) {
		printf("FAIL: Write '... flag!' on my stdin.\n");
		return 1;
	}

	t2 = time(NULL);

	if ((t2 - t1) >= 2) {
		if ((fd = open("/flag", O_RDONLY)) == -1) {
			perror("open");
			return 1;
		}

		if (read(fd, &buf, sizeof(buf)) < 0) {
			perror("read");
			return 1;
		}

		printf("Flag: %s\n", buf);
	} else {
		printf("FAIL: Wait at least 2 seconds before you write the second part!\n");
		return 1;
	}

	return 0;
}
