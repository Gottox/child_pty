#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
	int rv;

	if(argc < 2) {
		fputs("usage: exechelper COMMAND ARGUMENTS...\n", stderr);
	} else if(fcntl(3, F_SETFD, fcntl(3, F_GETFD) | FD_CLOEXEC) >= 0 &&
			ioctl(STDIN_FILENO, TIOCSCTTY, NULL) >= 0) {
		memmove(argv, argv+1, --argc * sizeof(char*));
		argv[argc] = NULL;
		execvp(argv[0], argv);
	}
	printf("%i\n", errno);
	rv = htonl(errno);
	write(3, &rv, sizeof(errno));
	return EXIT_FAILURE;
}
