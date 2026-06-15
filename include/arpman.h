#ifndef NODEPROBE_ARPMAN_H
#define NODEPROBE_ARPMAN_H

#include "types.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

typedef struct {
	int sockfd;
	int ifindex;
} arpman_context_t;

status_t arpman_create_context(arpman_context_t *restrict context);

status_t arpman_delete_context(arpman_context_t *restrict context);

#endif
