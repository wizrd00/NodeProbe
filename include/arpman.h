#ifndef NODEPROBE_ARPMAN_H
#define NODEPROBE_ARPMAN_H

#include "types.h"
#include <stdlib.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

#define ARP_REQUEST_DEFAULT_HEADER() {\
		.htype = (uint16_t) PROTO_ETHERNET,\
		.ptype = (uint16_t) PROTO_IPV4,\
		.hlen = (uint8_t) 6,\
		.plen = (uint8_t) 4,\
		.op = (uint16_t) 1,\
		.sha = {\
			(uint8_t) context->mac[0],\
			(uint8_t) context->mac[1],\
			(uint8_t) context->mac[2],\
			(uint8_t) context->mac[3],\
			(uint8_t) context->mac[4],\
			(uint8_t) context->mac[5]\
		},\
		.spa = {\
			(uint8_t) context->ip[0],\
			(uint8_t) context->ip[1],\
			(uint8_t) context->ip[2],\
			(uint8_t) context->ip[3]\
		},\
		.tha = {\
			(uint8_t) 0,\
			(uint8_t) 0,\
			(uint8_t) 0,\
			(uint8_t) 0,\
			(uint8_t) 0,\
			(uint8_t) 0\
		},\
		.tpa = {\
			(uint8_t) (ip >> 24),\
			(uint8_t) ((ip & 0x00ff0000) >> 16),\
			(uint8_t) ((ip & 0x0000ff00) >> 8),\
			(uint8_t) (ip & 0x000000ff)\
		}\
	}

#define ARP_REQUEST_DEFAULT_ADDR() {\
		.sll_family = AF_PACKET,\
		.sll_protocol = htons(ETH_P_ARP),\
		.sll_ifindex = context->ifindex,\
		.sll_halen = (unsigned char) 6,\
		.sll_addr = {\
			(unsigned char) 0,\
			(unsigned char) 0,\
			(unsigned char) 0,\
			(unsigned char) 0,\
			(unsigned char) 0,\
			(unsigned char) 0\
		}\
	};

typedef struct {
	int sockfd;
	int ifindex;
	int timeout;
	char ip[4];
	char mac[6];
} arpman_context_t;

status_t arpman_create_context(arpman_context_t *restrict context);

status_t arpman_delete_context(arpman_context_t *restrict context);

status_t arpman_request_mac(arpman_context_t *restrict context, uint32_t ip, uint8_t *mac);

#endif
