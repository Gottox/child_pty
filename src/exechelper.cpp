#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[]) {
	if(argc < 2) {
		fputs("usage: exechelper COMMAND ARGUMENTS...\n", stderr);
	} else if(ioctl(STDIN_FILENO, TIOCSCTTY, NULL) < 0) {
		perror("ioctl TIOCSCTTY");
	} else {
		memmove(argv, argv+1, --argc * sizeof(char*));
		argv[argc] = NULL;
		execvp(argv[0], argv);
		perror("execvp");
	}
	return EXIT_FAILURE;
}
