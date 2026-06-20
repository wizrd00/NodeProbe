#include "icmpman.h"

uint16_t calc_ipv4_checksum(ipv4_header_t *ip)
{
	return checksum((uint8_t *) ip, sizeof(ipv4_header_t));
}

uint16_t calc_icmpv4_checksum(icmpv4_echo_header_t *icmp)
{
	return checksum((uint8_t *) icmp, sizeof(icmpv4_echo_header_t));
}

status_t check_echo_response(ethernet_header_t *res_eth, ethernet_header_t *req_eth, ipv4_header_t *res_ip, ipv4_header_t *req_ip, icmpv4_echo_header_t *res_icmp, icmpv4_echo_header_t *req_icmp)
{
	status_t _stat = SUCCESS;
	bool match = (res_icmp->type == 0) && (res_icmp->code == 0) && (res_icmp->id == req_icmp->id);
	if (!match)
		_stat = FAILURE;
	return _stat;
}

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
	CHECK_NOTEQUAL(close(context->sockfd), -1, ERRCLOS, "close() failed to close socket with fd = %d; %s", context->sockfd, strerror(errno));
	return _stat;
}

status_t icmpman_echo_request(icmpman_context_t *restrict context)
{
	status_t _stat = SUCCESS;
	size_t offset = 0;
	unsigned char frame[FRAME_SIZE];
	unsigned char *buffer = (unsigned char *) calloc(context->mtu_size, sizeof(unsigned char));
	CHECK_NOTEQUAL(buffer, NULL, ERRALOC, "calloc() failed to allocate %zu bytes; %s", context->mtu_size, strerror(errno));
	uint32_t src_ip = ((uint32_t) context->src_ip[0] | (uint32_t) context->src_ip[1] << 8 | (uint32_t) context->src_ip[2] << 16 | (uint32_t) context->src_ip[3] << 24);
	uint32_t dst_ip = ((uint32_t) context->dst_ip[0] | (uint32_t) context->dst_ip[1] << 8 | (uint32_t) context->dst_ip[2] << 16 | (uint32_t) context->dst_ip[3] << 24);
	ethernet_header_t res_eth_header, req_eth_header = ETHERNET_DEFAULT_HEADER(context->src_mac, context->dst_mac);
	ipv4_header_t res_ip_header, req_ip_header = IPV4_DEFAULT_HEADER(FRAME_SIZE - sizeof(ethernet_header_t), src_ip, dst_ip);
	icmpv4_echo_header_t res_icmp_header, req_icmp_header = ICMPV4_ECHO_DEFAULT_HEADER(context->id);
	struct sockaddr_ll req_addr = ICMPV4_ECHO_REQUEST_DEFAULT_ADDR();
	struct pollfd pfd = {
		.fd = context->sockfd,
		.events = POLLIN
	};
	req_ip_header.chksum = htons(calc_ipv4_checksum(&req_ip_header));
	req_icmp_header.chksum = htons(calc_icmpv4_checksum(&req_icmp_header));
	memcpy((void *) frame, (void *) &req_eth_header, sizeof(ethernet_header_t));
	offset += sizeof(ethernet_header_t);
	memcpy((void *) (frame + offset), (void *) &req_ip_header, sizeof(ipv4_header_t));
	offset += sizeof(ipv4_header_t);
	memcpy((void *) (frame + offset), (void *) &req_icmp_header, sizeof(icmpv4_echo_header_t));
	ssize_t sendto_ret = sendto(context->sockfd, (void *) frame, FRAME_SIZE, 0, (struct sockaddr *) &req_addr, sizeof(struct sockaddr_ll));
	CHECK_NOTEQUAL_FREE(sendto_ret, (ssize_t) -1, ERRSEND, buffer, "sendto() failed to send ICMPv4 echo request on socket with fd = %d; %s", context->sockfd, strerror(errno));
	CHECK_EQUAL_FREE((size_t) sendto_ret, FRAME_SIZE, ERRSEND, buffer, "sendto() failed and sent %zu bytes instead of %zu bytes", (size_t) sendto_ret, FRAME_SIZE);
	while (1) {
		switch (poll(&pfd, (nfds_t) 1, context->timeout)) {
		case -1 :
			CHECK_STAT_FREE(ERRPOLL, buffer, "poll() failed; %s", strerror(errno));
		case 0 :
			CHECK_STAT_FREE(TIMEOUT, buffer, "poll() timeout after %d ms", context->timeout);
		case 1 :
			if ((pfd.revents & POLLIN) != POLLIN)
				CHECK_STAT_FREE(ERRPOLL, buffer, "poll() failed and pfd.revents = %d", pfd.revents);
		}
		ssize_t recvfrom_ret = recvfrom(context->sockfd, (void *) buffer, context->mtu_size, 0, NULL, NULL);
		CHECK_NOTEQUAL_FREE(recvfrom_ret, (ssize_t) -1, ERRRECV, buffer, "recvfrom() failed to receive ICMPv4 echo response on socket with fd = %d; %s", context->sockfd, strerror(errno));
		CHECK_EQUAL_FREE((size_t) recvfrom_ret, FRAME_SIZE, ERRRECV, buffer, "size of the received frame is %zu instead of %zu", (size_t) recvfrom, FRAME_SIZE);
		offset = 0;
		memcpy((void *) &res_eth_header, (void *) buffer, sizeof(ethernet_header_t));
		offset += sizeof(ethernet_header_t);
		memcpy((void *) &res_ip_header, (void *) (buffer + offset), sizeof(ipv4_header_t));
		offset += sizeof(ipv4_header_t);
		memcpy((void *) &res_icmp_header, (void *) (buffer + offset), sizeof(icmpv4_echo_header_t));
		if (check_echo_response(&res_eth_header, &req_eth_header, &res_ip_header, &req_ip_header, &res_icmp_header, &req_icmp_header) == SUCCESS)
			break;
		LOGW("the received frame is invalid, trying again...");
	}
	free((void *) buffer);
	return _stat;
}
