#ifndef NODEPROBE_ICMPMAP_H
#define NODEPROBE_ICMPMAP_H

#include "types.h"
#include <string.h>
#include <sys/socket.h>
#include <linux/icmp.h>

typedef struct {
	int sockfd;
	int timeout;
	unsigned short id;
	unsigned char src_ip[4];
} icmpman_context_t;

status_t icmpman_create_context(icmpman_context_t *restrict context);

#endif
