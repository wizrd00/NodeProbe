#include "fake_socket.h"

arp_inet_header_t *header;

ssize_t sendto(int sockfd, const void *buf, size_t size, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	printf("Enter fake sendto() -> ");
	header = (arp_inet_header_t *) buf;
	printf("DONE\n");
	return (ssize_t) size;
}

ssize_t recvfrom(int sockfd, void *buf, size_t size, int flags, struct sockaddr *restrict src_addr, socklen_t *restrict addrlen)
{
	printf("Enter fake recvfrom() -> ");
	arp_inet_header_t res = {
		.htype = 1,
		.ptype = 0x800,
		.hlen = 6,
		.plen = 4
	};
	switch (sockfd) {
	case 1 :
		printf("DONE\n");
		return (ssize_t) size - (ssize_t) 1;
	case 2 :
		printf("DONE\n");
		return (ssize_t) -1;
	default :
		res.op = 2;
		res.sha[0] = 0x12;
		res.sha[1] = 0x23;
		res.sha[2] = 0x34;
		res.sha[3] = 0x45;
		res.sha[4] = 0x56;
		res.sha[5] = 0x67;

		res.spa[0] = header->tpa[0];
		res.spa[1] = header->tpa[1];
		res.spa[2] = header->tpa[2];
		res.spa[3] = header->tpa[3];

		res.tha[0] = header->sha[0];
		res.tha[1] = header->sha[1];
		res.tha[2] = header->sha[2];
		res.tha[3] = header->sha[3];
		res.tha[4] = header->sha[4];
		res.tha[5] = header->sha[5];

		res.tpa[0] = header->spa[0];
		res.tpa[1] = header->spa[1];
		res.tpa[2] = header->tpa[2];
		res.tpa[3] = header->tpa[3];

		memcpy(buf, (void *) &res, size);
		printf("DONE\n");
		return (ssize_t) size;
	}
}
