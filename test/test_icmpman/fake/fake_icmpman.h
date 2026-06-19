#ifndef NODEPROBE_ICMPMAP_H
#define NODEPROBE_ICMPMAP_H

#include "types.h"
#include "utils/checksum.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fake_poll.h>
#include <fake_socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

#define FRAME_SIZE sizeof(ethernet_header_t) + sizeof(ipv4_header_t) + sizeof(icmpv4_echo_header_t)

#define ETHERNET_DEFAULT_HEADER() {\
	.dst_addr = {(uint8_t) context->out_mac[0], (uint8_t) context->out_mac[1], (uint8_t) context->out_mac[2], (uint8_t) context->out_mac[3], (uint8_t) context->out_mac[4], (uint8_t) context->out_mac[5]},\
	.src_addr = {(uint8_t) context->src_mac[0], (uint8_t) context->src_mac[1], (uint8_t) context->src_mac[2], (uint8_t) context->src_mac[3], (uint8_t) context->src_mac[4], (uint8_t) context->src_mac[5]},\
	.len = (uint16_t) FRAME_SIZE\
}

#define IPV4_DEFAULT_HEADER() {\
	.ver_ihl = ((uint8_t) 4 << 4) | ((uint8_t) 5),\
	.tos = (uint8_t) 0,\
	.tlen = (uint16_t) FRAME_SIZE - sizeof(ethernet_header_t),\
	.id = (uint16_t) context->id,\
	.flag_frag = (uint16_t) 0x4000,\
	.ttl = (uint8_t) 0x40,\
	.proto = (uint8_t) PROTO_ICMPV4,\
	.chksum = (uint16_t) 0,\
	.src_addr = {(uint8_t) context->src_ip[0], (uint8_t) context->src_ip[1], (uint8_t) context->src_ip[2], (uint8_t) context->src_ip[3]},\
	.dst_addr = ip\
}

#define ICMPV4_DEFAULT_HEADER() {\
	.type = (uint8_t) 8,\
	.code = (uint8_t) 0,\
	.chksum = (uint16_t) 0,\
	.id = (uint16_t) context->id,\
	.seq = (uint16_t) 0\
}

#define ICMPV4_ECHO_REQUEST_DEFAULT_ADDR() {\
	.sll_family = AF_PACKET,\
	.sll_protocol = htons(ETH_P_IP),\
	.sll_ifindex = context->ifindex,\
	.sll_halen = (unsigned char) 6,\
	.sll_addr = {context->out_mac[0], context->out_mac[1], context->out_mac[2], context->out_mac[3], context->out_mac[4], context->out_mac[5]}\
}

typedef struct {
	int sockfd;
	int ifindex;
	int timeout;
	unsigned short id;
	size_t mtu_size;
	unsigned char src_ip[4];
	unsigned char src_mac[6];
	unsigned char out_mac[6];
} icmpman_context_t;

status_t icmpman_create_context(icmpman_context_t *restrict context);

status_t icmpman_delete_context(icmpman_context_t *restrict context);

status_t icmpman_echo_request(icmpman_context_t *restrict context, uint32_t ip);

#endif
