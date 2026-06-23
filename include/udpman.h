#ifndef NODEPROBE_UDPMAN_H
#define NODEPROBE_UDPMAN_H

#include "types.h"
#include "utils/checksum.h"
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

#define UDPMAN_FRAME_SIZE sizeof(ethernet_header_t) + sizeof(ipv4_header_t) + sizeof(udp_header_t)

#define UDP_CHECK_REQUEST_DEFAULT_ADDR() {\
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
	size_t mtu_size;
	unsigned char src_mac[6];
	unsigned char dst_mac[6];
	unsigned char src_ip[4];
	unsigned char dst_ip[4];
	unsigned short src_port;
	unsigned short dst_port;
} udpman_context_t;

status_t udpman_create_context(udpman_context_t *restrict context);

status_t udpman_delete_context(udpman_context_t *restrict context);

status_t udpman_udp_request(udpman_context_t *restrict context);

#endif
