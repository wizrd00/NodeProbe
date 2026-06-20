#ifndef NODEPROBE_TCPMAN_H
#define NODEPROBE_TCPMAN_H

#include "types.h"
#include "utils/checksum.h"
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

#define FRAME_SIZE sizeof(ethernet_header_t) + sizeof(ipv4_header_t) + sizeof(tcp_header_t)

#define TCP_SYNC_REQUEST_DEFAULT_ADDR() {\
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

} tcpman_context_t;

status_t tcpman_create_context(tcpman_context_t *restrict context);

status_t tcpman_delete_context(tcpman_context_t *restrict context);

status_t tcpman_sync_request(tcpman_context_t *restrict context);

#endif
