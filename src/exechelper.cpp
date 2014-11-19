#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[]) {
	int i;

	if(ioctl(STDIN_FILENO, TIOCSCTTY, NULL) < 0) {
		perror("ioctl TIOCSCTTY failed");
		return EXIT_FAILURE;
	}
	for(i = 0; i < argc-1; i++)
		argv[i] = argv[i+1];
	argv[i] = NULL;
	if(argv[0] != NULL)
		execvp(argv[0], argv);
	return EXIT_FAILURE;
}
