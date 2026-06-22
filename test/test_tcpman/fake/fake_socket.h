#ifndef _FAKE_SOCKET_H
#define _FAKE_SOCKET_H

#include "types.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

ssize_t sendto(int sockfd, const void *buf, size_t size, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t recvfrom(int sockfd, void *buf, size_t size, int flags, struct sockaddr *restrict src_addr, socklen_t *restrict addrlen);

#endif
