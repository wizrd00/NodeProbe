#include "fake_poll.h"

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	printf("Enter fake poll() -> ");
	switch (fds[0].fd) {
	case 4 :
		printf("DONE\n");
		return 0;
	case 5 :
		printf("DONE\n");
		return -1;
	default :
		fds[0].revents = fds[0].events;
		printf("DONE\n");
		return 1;
	}
}
