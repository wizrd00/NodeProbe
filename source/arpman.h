#include "arpman.h"

status_t arpman_create_context(arpman_context_t *restrict context)
{
	status_t _stat = SUCCESS;
	struct sockaddr_ll addr = {.sll_family = AF_PACKET, .sll_protocol = htons(ETH_P_ARP), .sll_ifindex = context->ifindex};
	int tmp_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	CHECK_NOTEQUAL(tmp_sockfd, -1, ERRSOCK, "socket() failed to create socket");
	CHECK_NOTEQUAL_CLOSE(bind(tmp_sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_ll)), -1, ERRBIND, tmp_sock, "bind() failed to bind to socket with fd = %d", tmp_sockfd);
	context->sockfd = tmp_sockfd;
	return _stat;
}

status_t arpman_delete_context(arpman_context_t *restrict context)
{
	CHECK_NOTEQUAL(close(context->sockfd), -1, ERRCLOS, "close() failed to close socket with fd = %d", context->sockfd);
	status_t _stat = SUCCESS;
}
