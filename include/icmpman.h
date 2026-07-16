#ifndef NODEPROBE_ICMPMAP_H
#define NODEPROBE_ICMPMAP_H

#include "types.h"
#include "contexts.h"
#include "utils/checksum.h"
#include "utils/calc.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include <sys/socket.h>

#define ICMPMAN_FRAME_SIZE (sizeof(ethernet_header_t) + sizeof(ipv4_header_t) + sizeof(icmpv4_echo_header_t))

#define ICMPV4_ECHO_REQUEST_DEFAULT_ADDR() {\
	.sll_family = AF_PACKET,\
	.sll_protocol = htons(ETH_P_IP),\
	.sll_ifindex = context->ifindex,\
	.sll_halen = (unsigned char) 6,\
	.sll_addr = {context->dst_mac[0], context->dst_mac[1], context->dst_mac[2], context->dst_mac[3], context->dst_mac[4], context->dst_mac[5]}\
}

typedef struct {
	int sockfd;
	int ifindex;
	int timeout;
	unsigned short id;
	size_t mtu_size;
	unsigned char src_mac[6];
	unsigned char dst_mac[6];
	unsigned char src_ip[4];
	unsigned char dst_ip[4];
} icmpman_context_t;

status_t icmpman_create_context(icmpman_context_t *restrict context);

status_t icmpman_delete_context(icmpman_context_t *restrict context);

status_t icmpman_echo_request(icmpman_context_t *restrict context);

#endif
