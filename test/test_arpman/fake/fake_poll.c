#include "fake_poll.h"

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	switch (fds[0].fd) {
	case 4 :
		return 0;
	case 5 :
		return -1;
	default :
		fds[0].revents = fds[0].events;
		return 1;
	}
}
