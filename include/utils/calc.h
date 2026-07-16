#ifndef NODEPROBE_CALC_H
#define NODEPROBE_CALC_H

#include "types.h"
#include "utils/checksum.h"
#include <stdint.h>
#include <string.h>

static inline uint16_t calc_ipv4_checksum(ipv4_header_t *ip)
{
	return checksum((uint8_t *) ip, sizeof(ipv4_header_t));
}

static inline uint16_t calc_icmpv4_checksum(icmpv4_echo_header_t *icmp)
{
	return checksum((uint8_t *) icmp, sizeof(icmpv4_echo_header_t));
}

static inline uint16_t calc_tcp_checksum(tcp_header_t *tcp, ipv4_pseudo_header_t *pseudo)
{
	uint8_t data[sizeof(tcp_header_t) + sizeof(ipv4_pseudo_header_t)];
	memcpy((void *) data, (void *) tcp, sizeof(tcp_header_t));
	memcpy((void *) (data + sizeof(tcp_header_t)), (void *) pseudo, sizeof(ipv4_pseudo_header_t));
	return checksum(data, sizeof(data));
}

static inline uint16_t calc_udp_checksum(udp_header_t *udp, ipv4_pseudo_header_t *pseudo)
{
	uint8_t data[sizeof(udp_header_t) + sizeof(ipv4_pseudo_header_t)];
	memcpy((void *) data, (void *) udp, sizeof(udp_header_t));
	memcpy((void *) (data + sizeof(udp_header_t)), (void *) pseudo, sizeof(ipv4_pseudo_header_t));
	return checksum(data, sizeof(data));
}

#endif
