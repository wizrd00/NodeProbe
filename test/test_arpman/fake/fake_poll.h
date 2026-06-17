#ifndef _FAKE_POLL_H
#define _FAKE_POLL_H

#include <stdio.h>
#include <poll.h>
#include <unistd.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);

#endif
