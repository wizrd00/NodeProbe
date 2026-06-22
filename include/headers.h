#ifndef NODEPROBE_HEADERS_H
#define NODEPROBE_HEADERS_H

#include <stdint.h>

#define PROTO_ETHERNET 0x0001
#define PROTO_ARP 0x0806
#define PROTO_IPV4 0x0800
#define PROTO_ICMPV4 0x0001
#define PROTO_TCP 0x0006

#define ARP_REQ 1
#define ARP_RES 2

#define ETHERNET_DEFAULT_HEADER(_src_mac, _dst_mac) {\
	.dst_addr = {(uint8_t) _dst_mac[0], (uint8_t) _dst_mac[1], (uint8_t) _dst_mac[2], (uint8_t) _dst_mac[3], (uint8_t) _dst_mac[4], (uint8_t) _dst_mac[5]},\
	.src_addr = {(uint8_t) _src_mac[0], (uint8_t) _src_mac[1], (uint8_t) _src_mac[2], (uint8_t) _src_mac[3], (uint8_t) _src_mac[4], (uint8_t) _src_mac[5]},\
	.type = htons((uint16_t) PROTO_IPV4) \
}

#define ARP_REQUEST_DEFAULT_HEADER(_src_mac, _src_ip, _dst_ip) {\
		.htype = htons((uint16_t) PROTO_ETHERNET),\
		.ptype = htons((uint16_t) PROTO_IPV4),\
		.hlen = (uint8_t) 6,\
		.plen = (uint8_t) 4,\
		.op = htons((uint16_t) ARP_REQ),\
		.sha = {(uint8_t) _src_mac[0], (uint8_t) _src_mac[1], (uint8_t) _src_mac[2], (uint8_t) _src_mac[3], (uint8_t) _src_mac[4], (uint8_t) _src_mac[5]},\
		.spa = {(uint8_t) _src_ip[0], (uint8_t) _src_ip[1], (uint8_t) _src_ip[2], (uint8_t) _src_ip[3]},\
		.tha = {(uint8_t) 0, (uint8_t) 0, (uint8_t) 0, (uint8_t) 0, (uint8_t) 0, (uint8_t) 0},\
		.tpa = {(uint8_t) _dst_ip[0], (uint8_t) _dst_ip[1], (uint8_t) _dst_ip[2], (uint8_t) _dst_ip[3]}\
}

#define IPV4_DEFAULT_HEADER(_len, _proto, _src_ip, _dst_ip) {\
	.ver_ihl = ((uint8_t) 4 << 4) | ((uint8_t) 5),\
	.tos = (uint8_t) 0,\
	.tlen = htons((uint16_t) _len),\
	.id = htons((uint16_t) 0x1337),\
	.flag_frag = htons((uint16_t) 0x4000),\
	.ttl = (uint8_t) 0x40,\
	.proto = (uint8_t) _proto,\
	.chksum = (uint16_t) 0,\
	.src_addr = (uint32_t) _src_ip,\
	.dst_addr = (uint32_t) _dst_ip\
}

#define ICMPV4_ECHO_DEFAULT_HEADER(_id) {\
	.type = (uint8_t) 8,\
	.code = (uint8_t) 0,\
	.chksum = (uint16_t) 0,\
	.id = htons((uint16_t) _id),\
	.seq = (uint16_t) 0\
}

#define TCP_SYNC_DEFAULT_HEADER(_src_port, _dst_port) {\
	.src_port = htons((uint16_t) _src_port),\
	.dst_port = htons((uint16_t) _dst_port),\
	.seq = htonl((uint32_t) 0x1337),\
	.ack = (uint32_t) 0,\
	.ofst = (uint8_t) (5 << 4),\
	.flag = (uint8_t) 0x02,\
	.win = (uint16_t) 0xffff,\
	.chksum = (uint16_t) 0,\
	.urgt = (uint16_t) 0\
}

#define IPV4_PSEUDO_DEFAULT_HEADER(_src_ip, _dst_ip, _proto, _len) {\
	.src_ip = (uint32_t) _src_ip,\
	.dst_ip = (uint32_t) _dst_ip,\
	.zero = (uint8_t) 0,\
	.proto = (uint8_t) _proto,\
	.len = htons((uint16_t) _len)\
}

typedef struct {
	uint8_t dst_addr[6];
	uint8_t src_addr[6];
	uint16_t type;
} ethernet_header_t;

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
	uint8_t ver_ihl;
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
	uint8_t type;
	uint8_t code;
	uint16_t chksum;
	uint16_t id;
	uint16_t seq;
} icmpv4_echo_header_t;

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
	uint32_t src_ip;
	uint32_t dst_ip;
	uint8_t zero;
	uint8_t proto;
	uint16_t len;
} ipv4_pseudo_header_t;

#endif
