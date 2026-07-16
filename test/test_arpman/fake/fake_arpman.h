#ifndef NODEPROBE_ARPMAN_H
#define NODEPROBE_ARPMAN_H

#include "types.h"
#include "contexts.h"
#include <string.h>
#include <time.h>
#include <fake_poll.h>
#include <fake_socket.h>

#define ARP_REQUEST_DEFAULT_ADDR() {\
		.sll_family = AF_PACKET,\
		.sll_protocol = htons(ETH_P_ARP),\
		.sll_ifindex = context->ifindex,\
		.sll_hatype = htons(ARPHRD_ETHER),\
		.sll_halen = (unsigned char) 6,\
		.sll_addr = {context->out_mac[0], context->out_mac[1], context->out_mac[2], context->out_mac[3], context->out_mac[4], context->out_mac[5]}\
	};

typedef struct {
	int sockfd;
	int ifindex;
	int timeout;
	unsigned char src_mac[6];
	unsigned char out_mac[6];
	unsigned char src_ip[4];
	unsigned char dst_ip[4];
} arpman_context_t;

status_t arpman_create_context(arpman_context_t *restrict context);

status_t arpman_delete_context(arpman_context_t *restrict context);

status_t arpman_mac_request(arpman_context_t *restrict context, uint8_t *mac);

#endif
