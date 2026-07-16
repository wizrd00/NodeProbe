#include "arpman.h"

static status_t check_arp_response(arp_inet_header_t *res, arp_inet_header_t *req)
{
	status_t _stat = SUCCESS;
	bool match = (res->op == htons(ARP_RES)) && (memcmp(res->tha, req->sha, (size_t) 6) == 0) && (memcmp(res->tpa, req->spa, (size_t) 4) == 0) && (memcmp(res->spa, req->tpa, (size_t) 4) == 0);
	if (!match)
		_stat = FAILURE;
	return _stat;
}

status_t arpman_create_context(arpman_context_t *restrict context)
{
	status_t _stat = SUCCESS;
	CREATE_CONTEXT(context, SOCK_DGRAM, ETH_P_ARP);
	return _stat;
}

status_t arpman_delete_context(arpman_context_t *restrict context)
{
	status_t _stat = SUCCESS;
	DELETE_CONTEXT(context);
	return _stat;
}

status_t arpman_mac_request(arpman_context_t *restrict context, uint8_t *mac)
{
	status_t _stat = SUCCESS;
	int timeout = context->timeout;
	arp_inet_header_t res_header, req_header = ARP_REQUEST_DEFAULT_HEADER(context->src_mac, context->src_ip, context->dst_ip);
	struct sockaddr_ll req_addr = ARP_REQUEST_DEFAULT_ADDR();
	struct timespec start_tp, now_tp;
	struct pollfd pfd = {
		.fd = context->sockfd,
		.events = POLLIN
	};
	ssize_t sendto_ret = sendto(context->sockfd, (void *) &req_header, sizeof(arp_inet_header_t), 0, (struct sockaddr *) &req_addr, sizeof(struct sockaddr_ll));
	CHECK_NOTEQUAL(sendto_ret, (ssize_t) -1, ERRSEND, "sendto() failed and returned -1 on socket with fd = %d; %s", context->sockfd, strerror(errno));
	CHECK_EQUAL((size_t) sendto_ret, sizeof(arp_inet_header_t), ERRSEND, "sendto() failed and sent %zu bytes instead of %zu bytes", (size_t) sendto_ret, sizeof(arp_inet_header_t));
	CHECK_NOTEQUAL(clock_gettime(CLOCK_REALTIME, &start_tp), -1, ERRTIME, "clock_gettime() failed to get time; %s", strerror(errno));
	while (1) {
		CHECK_NOTEQUAL(clock_gettime(CLOCK_REALTIME, &now_tp), -1, ERRTIME, "clock_gettime() failed to get time; %s", strerror(errno));
		timeout = context->timeout - (CONVERT_TIMESPEC(now_tp) - CONVERT_TIMESPEC(start_tp));
		if (timeout < 0)
			break;
		switch (poll(&pfd, (nfds_t) 1, timeout)) {
		case -1 :
			CHECK_STAT(ERRPOLL, "poll() failed; %s", strerror(errno));
		case 0 :
			CHECK_STAT(TIMEOUT, "poll() timeout after %d ms", context->timeout);
		case 1 :
			if ((pfd.revents & POLLIN) != POLLIN)
				CHECK_STAT(ERRPOLL, "poll() failed and pfd.revents = %d", pfd.revents);
		}
		ssize_t recvfrom_ret = recvfrom(context->sockfd, (void *) &res_header, sizeof(arp_inet_header_t), 0, NULL, NULL);
		CHECK_NOTEQUAL(recvfrom_ret, (ssize_t) -1, ERRRECV, "recvfrom() failed and returned -1 on socket with fd = %d; %s", context->sockfd, strerror(errno));
		if (check_arp_response(&res_header, &req_header) == SUCCESS)
			break;
		LOGW("The received datagram is invalid, trying again...");
	}
	memcpy((void *) mac, (void *) res_header.sha, (size_t) 6);
	return _stat;

}
