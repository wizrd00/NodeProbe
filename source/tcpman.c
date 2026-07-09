#include "tcpman.h"

static uint16_t calc_ipv4_checksum(ipv4_header_t *ip)
{
	return checksum((uint8_t *) ip, sizeof(ipv4_header_t));
}

static uint16_t calc_tcp_checksum(tcp_header_t *tcp, ipv4_pseudo_header_t *pseudo)
{
	uint8_t data[sizeof(tcp_header_t) + sizeof(ipv4_pseudo_header_t)];
	memcpy((void *) data, (void *) tcp, sizeof(tcp_header_t));
	memcpy((void *) (data + sizeof(tcp_header_t)), (void *) pseudo, sizeof(ipv4_pseudo_header_t));
	return checksum(data, sizeof(data));
}

static status_t check_sync_response(tcp_header_t *res_tcp, tcp_header_t *req_tcp)
{
	status_t _stat = SUCCESS;
	bool match = (res_tcp->dst_port == req_tcp->src_port);
	if (!match)
		return _stat = INVALID;
	_stat = ((res_tcp->flag & 0x04) == 0x04) ? FAILURE : ((res_tcp->flag & 0x12) == 0x12) ? SUCCESS : INVALID;
	return _stat;
}

status_t tcpman_create_context(tcpman_context_t *restrict context)
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

status_t tcpman_delete_context(tcpman_context_t *restrict context)
{
	status_t _stat = SUCCESS;
	CHECK_NOTEQUAL(close(context->sockfd), -1, ERRCLOS, "close() failed to close socket with fd = %d; %s", context->sockfd, strerror(errno));
	return _stat;
}

status_t tcpman_sync_request(tcpman_context_t *restrict context)
{
	status_t _stat = SUCCESS;
	int timeout = context->timeout;
	size_t offset = 0;
	unsigned char frame[TCPMAN_FRAME_SIZE];
	unsigned char *buffer = (unsigned char *) calloc(context->mtu_size, sizeof(unsigned char));
	CHECK_NOTEQUAL(buffer, NULL, ERRALOC, "calloc() failed to allocate %zu bytes; %s", context->mtu_size, strerror(errno));
	uint32_t src_ip = ((uint32_t) context->src_ip[0] | (uint32_t) context->src_ip[1] << 8 | (uint32_t) context->src_ip[2] << 16 | (uint32_t) context->src_ip[3] << 24);
	uint32_t dst_ip = ((uint32_t) context->dst_ip[0] | (uint32_t) context->dst_ip[1] << 8 | (uint32_t) context->dst_ip[2] << 16 | (uint32_t) context->dst_ip[3] << 24);
	ethernet_header_t req_eth_header = ETHERNET_DEFAULT_HEADER(context->src_mac, context->dst_mac);
	ipv4_header_t req_ip_header = IPV4_DEFAULT_HEADER(TCPMAN_FRAME_SIZE - sizeof(ethernet_header_t), PROTO_TCP, src_ip, dst_ip);
	tcp_header_t req_tcp_header = TCP_SYNC_DEFAULT_HEADER(context->src_port, context->dst_port);
	ipv4_pseudo_header_t pseudo_header = IPV4_PSEUDO_DEFAULT_HEADER(src_ip, dst_ip, PROTO_TCP, sizeof(tcp_header_t));
	struct sockaddr_ll req_addr = TCP_SYNC_REQUEST_DEFAULT_ADDR();
	struct timespec start_tp, now_tp;
	struct pollfd pfd = {
		.fd = context->sockfd,
		.events = POLLIN
	};
	req_ip_header.chksum = htons(calc_ipv4_checksum(&req_ip_header));
	req_tcp_header.chksum = htons(calc_tcp_checksum(&req_tcp_header, &pseudo_header));
	memcpy((void *) frame, (void *) &req_eth_header, sizeof(ethernet_header_t));
	offset += sizeof(ethernet_header_t);
	memcpy((void *) (frame + offset), (void *) &req_ip_header, sizeof(ipv4_header_t));
	offset += sizeof(ipv4_header_t);
	memcpy((void *) (frame + offset), (void *) &req_tcp_header, sizeof(tcp_header_t));
	ssize_t sendto_ret = sendto(context->sockfd, (void *) frame, TCPMAN_FRAME_SIZE, 0, (struct sockaddr *) &req_addr, sizeof(struct sockaddr_ll));
	CHECK_NOTEQUAL_FREE(sendto_ret, (ssize_t) -1, ERRSEND, buffer, "sendto() failed and returned -1 on socket with fd = %d; %s", context->sockfd, strerror(errno));
	CHECK_NOTEQUAL(clock_gettime(CLOCK_REALTIME, &start_tp), -1, ERRTIME, "clock_gettime() failed to get time; %s", strerror(errno));
	while (1) {
		CHECK_NOTEQUAL(clock_gettime(CLOCK_REALTIME, &now_tp), -1, ERRTIME, "clock_gettime() failed to get time; %s", strerror(errno));
		timeout -= CONVERT_TIMESPEC(now_tp) - CONVERT_TIMESPEC(start_tp);
		if (timeout < 0)
			break;
		switch (poll(&pfd, (nfds_t) 1, timeout)) {
		case -1 :
			CHECK_STAT_FREE(ERRPOLL, buffer, "poll() failed; %s", strerror(errno));
		case 0 :
			CHECK_STAT_FREE(TIMEOUT, buffer, "poll() timeout after %d ms", context->timeout);
		case 1 :
			if ((pfd.revents & POLLIN) != POLLIN)
				CHECK_STAT_FREE(ERRPOLL, buffer, "poll() failed and pfd.revents = %d", pfd.revents);
		}
		ssize_t recvfrom_ret = recvfrom(context->sockfd, (void *) buffer, context->mtu_size, 0, NULL, NULL);
		CHECK_NOTEQUAL_FREE(recvfrom_ret, (ssize_t) -1, ERRRECV, buffer, "recvfrom() failed and returned -1 on socket with fd = %d; %s", context->sockfd, strerror(errno));
		offset = 0;
		if ((size_t) recvfrom_ret < sizeof(ethernet_header_t))
			continue;
		if (!CHECK_INCLUDE_IPV4_DATAGRAM((ethernet_header_t *) buffer))
			continue;
		recvfrom_ret -= (ssize_t) sizeof(ethernet_header_t);
		offset += sizeof(ethernet_header_t);
		if ((size_t) recvfrom_ret < sizeof(ipv4_header_t))
			continue;
		if (!CHECK_INCLUDE_TCP_SEGMENT((ipv4_header_t *) (buffer + offset)))
			continue;
		recvfrom_ret -= (ssize_t) sizeof(ipv4_header_t);
		offset += sizeof(ipv4_header_t);
		if ((size_t) recvfrom_ret < sizeof(tcp_header_t))
			continue;
		if ((_stat = check_sync_response((tcp_header_t *) (buffer + offset), &req_tcp_header)) != INVALID)
			break;
	}
	free((void *) buffer);
	return _stat;
}
