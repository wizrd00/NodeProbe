#ifndef NODEPROBE_HEADERS_H
#define NODEPROBE_HEADERS_H

#include <stdint.h>

typedef struct {
	uint16_t htype;
	uint16_t ptype;
	uint8_t hlen;
	uint8_t plen;
	uint16_t op;
	uint8_t sha[6];
	uint8_t spa[4];
	uint8_t tha[6];
	uint8_t tpa[4];
} arp_inet_header_t;

typedef struct {
	uint8_t ver_ihl
	uint8_t tos;
	uint16_t tlen;
	uint16_t id;
	uint16_t flag_frag;
	uint8_t ttl;
	uint8_t proto;
	uint16_t chksum;
	uint32_t src_addr;
	uint32_t dst_addr;
	uint8_t opts[];
} ipv4_header_t;

typedef struct {
	uint16_t src_port;
	uint16_t dst_port;
	uint32_t seq;
	uint32_t ack;
	uint8_t ofst;
	uint8_t flag;
	uint16_t win;
	uint16_t chksum;
	uint16_t urgt;
	uint8_t opts[];
} tcp_header_t;

typedef struct {
	uint16_t src_port;
	uint16_t dst_port;
	uint16_t len;
	uint16_t chksum;
} udp_header_t;

typedef struct {
	uint32_t src_addr;
	uint32_t dst_addr;
	uint8_t proto;
	uint8_t len;
} pseudo_header_t;

#endif
