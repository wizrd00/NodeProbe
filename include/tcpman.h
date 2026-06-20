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

typedef struct {
	int sockfd;
	int ifindex;
	int timeout;
	size_t mtu_size;
	unsigned char src_ip[4];
	unsigned char src_mac[6];
	unsigned char out_mac[6];
} tcpman_context_t;

status_t tcpman_create_context(tcpman_context_t *restrict context);

status_t tcpman_delete_context(tcpman_context_t *restrict context);

status_t tcpman_sync_request(tcpman_context_t *restrict context, uint16_t port);

#endif
