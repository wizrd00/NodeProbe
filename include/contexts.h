#ifndef NODEPROBE_CONTEXTS_H
#define NODEPROBE_CONTEXTS_H

#include "stats.h"
#include "checks.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>

#define CREATE_CONTEXT(_context, _sock_type, _sock_proto)\
	do {\
		struct sockaddr_ll addr = {\
			.sll_family = AF_PACKET,\
			.sll_protocol = htons(_sock_proto),\
			.sll_ifindex = _context->ifindex\
		};\
		int tmp_sockfd = socket(AF_PACKET, _sock_type, htons(_sock_proto));\
		CHECK_NOTEQUAL(tmp_sockfd, -1, ERRSOCK, "socket() failed to create socket; %s", strerror(errno));\
		CHECK_NOTEQUAL_CLOSE(bind(tmp_sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll)), -1, ERRBIND, tmp_sockfd, "bind() failed to bind to socket with fd = %d; %s", tmp_sockfd, strerror(errno));\
		_context->sockfd = tmp_sockfd;\
	} while (0)

#define DELETE_CONTEXT(_context)\
	do {\
		CHECK_NOTEQUAL(close(_context->sockfd), -1, ERRCLOS, "close() failed to close socket with fd = %d; %s", _context->sockfd, strerror(errno));\
	} while (0)

#endif
