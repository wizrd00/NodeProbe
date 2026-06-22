#include "fake_socket.h"

void *frame;
size_t frame_size;

ssize_t sendto(int sockfd, const void *buf, size_t size, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	printf("Enter fake sendto() -> ");
	frame = buf;
	tcp_header_t *tcp = (tcp_header_t *) ((char *) frame + sizeof(ethernet_header_t) + sizeof(ipv4_header_t));
	tcp->dst_port = tcp->src_port;
	tcp->flag = (uint8_t) (sockfd != 0) ? 0x12: 0x04;
	frame_size = size;
	printf("DONE\n");
	return (ssize_t) size;
}

ssize_t recvfrom(int sockfd, void *buf, size_t size, int flags, struct sockaddr *restrict src_addr, socklen_t *restrict addrlen)
{
	printf("Enter fake recvfrom() -> ");
	switch (sockfd) {
	case 1 :
		printf("DONE\n");
		return (ssize_t) size - (ssize_t) 1;
	case 2 :
		printf("DONE\n");
		return (ssize_t) -1;
	default :
		memcpy(buf, frame, frame_size);
		printf("DONE\n");
		return (ssize_t) frame_size;
	}
}
