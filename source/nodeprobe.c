#include "nodeprobe.h"

int main(int argc, char **args)
{
	// The code below is just temporary for real testing of modules
	arpman_context_t context = {
		.ifindex = 3,
		.timeout = 20000,
		.src_ip = {(unsigned char) 10, (unsigned char) 28, (unsigned char) 42, (unsigned char) 207},
		.src_mac = {(unsigned char) 204, (unsigned char) 71, (unsigned char) 64, (unsigned char) 252, (unsigned char) 123, (unsigned char) 5},
		.out_mac = {(unsigned char) 94, (unsigned char) 121, (unsigned char) 224, (unsigned char) 78, (unsigned char) 16, (unsigned char) 134}

	};
	uint32_t ip;
	uint8_t mac[6];
	if (inet_pton(AF_INET, args[1], &ip) != 1) {
		printf("inet_pton() failed\n");
		return 1;
	}
	if (logman_create_context("./", 1024) == -1) {
		printf("logman_create_context() failed\n");
		return 1;
	}
	LOGT("Start nodeprobe");
	if (arpman_create_context(&context) != SUCCESS) {
		printf("arpman_create_context() failed\n");
		return 1;
	}
	if (arpman_mac_request(&context, ip, mac) != SUCCESS) {
		printf("arpman_request_mac() failed\n");
		return 1;
	} else {
		printf("The MAC Address = %x:%x:%x:%x:%x:%x\n", (int) mac[0], (int) mac[1], (int) mac[2], (int) mac[3], (int) mac[4], (int) mac[5]);
	}
	arpman_delete_context(&context);
	logman_delete_context();
	return 0;
}
