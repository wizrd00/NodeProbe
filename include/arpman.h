#ifndef NODEPROBE_ARPMAN_H
#define NODEPROBE_ARPMAN_H

#include "types.h"
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

#define ARP_REQ 1
#define ARP_RES 2

#define ARP_REQUEST_DEFAULT_HEADER() {\
		.htype = htons(PROTO_ETHERNET),\
		.ptype = htons(PROTO_IPV4),\
		.hlen = (uint8_t) 6,\
		.plen = (uint8_t) 4,\
		.op = htons(ARP_REQ),\
		.sha = {(uint8_t) context->src_mac[0], (uint8_t) context->src_mac[1], (uint8_t) context->src_mac[2], (uint8_t) context->src_mac[3], (uint8_t) context->src_mac[4], (uint8_t) context->src_mac[5]},\
		.spa = {(uint8_t) context->src_ip[0], (uint8_t) context->src_ip[1], (uint8_t) context->src_ip[2], (uint8_t) context->src_ip[3]},\
		.tha = {(uint8_t) 0, (uint8_t) 0, (uint8_t) 0, (uint8_t) 0, (uint8_t) 0, (uint8_t) 0},\
		.tpa = {(uint8_t) (ip & 0x000000ff), (uint8_t) ((ip & 0x0000ff00) >> 8), (uint8_t) ((ip & 0x00ff0000) >> 16), (uint8_t) (ip >> 24)}\
	}

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
	unsigned char src_ip[4];
	unsigned char src_mac[6];
	unsigned char out_mac[6];
} arpman_context_t;

status_t arpman_create_context(arpman_context_t *restrict context);

status_t arpman_delete_context(arpman_context_t *restrict context);

status_t arpman_mac_request(arpman_context_t *restrict context, uint32_t ip, uint8_t *mac);

#endif
