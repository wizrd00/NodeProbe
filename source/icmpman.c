#include "icmpman.h"

uint16_t calc_ipv4_checksum(ipv4_header_t *ip)
{}

uint16_t calc_icmpv4_checksum(icmpv4_echo_header_t *icmp)
{}

status_t icmpman_create_context(icmpman_context_t *restrict context)
{
	status_t _stat = SUCCESS;
	struct sockaddr_ll addr = {
		.sll_family = AF_PACKET,
		.sll_protocol = htons(ETH_P_IP),
		.sll_ifindex = context->ifindex
	};
	int tmp_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	CHECK_NOTEQUAL(tmp_sockfd, -1, ERRSOCK, "socket() failed to create socket; %s", strerror(errno));
	CHECK_NOTEQUAL_CLOSE(bind(tmp_sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll)), -1, ERRBIND, tmp_sockfd, "bind() failed to bind to socket with fd = %d; %s", tmp_sockfd, strerror(errno));
	context->sockfd = tmp_sockfd;
	return _stat;
}

status_t icmpman_delete_context(icmpman_context_t *restrict context)
{
	status_t _stat = SUCCESS;
	CHECK_NOTEQUAL(close(context->sockfd), -1, "close() failed to close socket with fd = %d; %s", context->sockfd, strerror(errno));
	return _stat;
}

status_t icmpman_echo_request(icmpman_context_t *restrict context, uint32_t ip)
{
	status_t _stat = SUCCESS;
	size_t offset = 0;
	unsigned char frame[FRAME_SIZE];
	unsigned char *buffer = (unsigned char *) calloc(context->mtu_size, sizeof(unsigned char));
	CHECK_NOTEQUAL(buffer, NULL, ERRALOC, "calloc() failed to allocate %zu bytes from heap; %s", context->mtu_size, strerror(errno));
	ethernet_header_t eth_header = ETHERNET_DEFAULT_HEADER();
	ipv4_header_t ip_header = IPV4_DEFAULT_HEADER();
	icmpv4_echo_header_t icmp_header = ICMPV4_DEFAULT_HEADER();
	ip_header.chksum = calc_ipv4_checksum(&ip_header);
	icmp_header.chksum = calc_icmpv4_checksum(&icmp_header);
	memcpy((void *) frame, (void *) &eth_header, sizeof(ethernet_header_t));
	offset += sizeof(ethernet_header_t);
	memcpy((void *) (frame + offset), (void *) &ip_header, sizeof(ipv4_header_t));
	offset += sizeof(ipv4_header_t);
	memcpy((void *) (frame + offset), (void *) &icmp_header, sizeof(icmpv4_echo_header_t));
	// TODO
}
