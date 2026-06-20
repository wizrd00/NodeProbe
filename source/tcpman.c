#include "tcpman.h"

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
	return _stat;
}

status_t tcpman_delete_context(tcpman_context_t *restrict context)
{
	status_t _stat = SUCCESS;
	CHECK_NOTEQUAL(close(context->sockfd), -1, ERRCLOS, "close() failed to close socket with fd = %d; %s", context->sockfd, strerror(errno));
	return _stat;
}

status_t tcpman_sync_request(tcpman_context_t *restrict context, uint16_t port)
{
	status_t _stat = SUCCESS;
}
