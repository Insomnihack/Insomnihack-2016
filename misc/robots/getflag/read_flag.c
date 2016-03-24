#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, const char *argv[])
{
	char buf[1024] = {0};
	int fd;

	if ((fd = open("/home/robots/getflag/flag", O_RDONLY)) == -1) {
		perror("open");
		return 1;
	}

	if (read(fd, &buf, sizeof(buf)) < 0) {
		perror("read");
		return 1;
	}

	printf("Flag: %s\n", buf);

	return 0;
}
