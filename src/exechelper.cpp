#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
	int rv, state_fd = atoi(argv[1]);

	if (fcntl(state_fd, F_SETFD, fcntl(state_fd, F_GETFD) | FD_CLOEXEC) >= 0 &&
			ioctl(STDIN_FILENO, TIOCSCTTY, NULL) >= 0) {
		argc -= 2;
		memmove(argv, argv + 2, argc * sizeof(char*));
		argv[argc] = NULL;
		execvp(argv[0], argv);
	}
	rv = htonl(errno);
	write(state_fd, &rv, sizeof(errno));
	pause();
	return EXIT_FAILURE;
}
