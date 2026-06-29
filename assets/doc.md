# NodeProbe API Reference

NodeProbe is used by including a single header and linking against the shared library.

```c
#include "nodeprobe.h"
```

### How to link it?
It is possible to build the library with *pcc* or *gcc*.  
It also possible to link the library with *musl* or *glibc*.  
The Makefile detects the libc automatically.

The second part of library name indicates its libc.  
if the name is *libnodeprobe-musl.so* it means you can only use it with musl libc.  
The same think happens to *libnodeprobe-glibc.so*

#### Linking with *pcc*
```sh
pcc main.c -Wl,--library-path=./,--library=nodeprobe-musl,-rpath=./ -o main
# or if the name is libnodeprobe-glibc.so
pcc main.c -Wl,--library-path=./,--library=nodeprobe-glibc,-rpath=./ -o main
```

#### Linking with *gcc*
```sh
gcc main.c -L./ -lnodeprobe-musl -Wl,-rpath=./ -o main
# or if the name is libnodeprobe-glibc.so
gcc main.c -L./ -lnodeprobe-glibc -Wl,-rpath=./ -o main
```

**All functions require `CAP_NET_RAW` or root privileges at runtime, since they operate on raw `AF_PACKET` sockets.**

### Important Global Type
Return type used by all module functions.

```c
typedef enum {
	SUCCESS,   /* operation completed successfully */
	FAILURE,   /* generic failure */
	TIMEOUT,   /* no response received within the timeout window */
	INVALID,   /* inside usage not relative to outside */
	ERRALOC,   /* memory allocation failed */
	ERRTIME,   /* clock_gettime failed */
	ERRSOCK,   /* socket creation failed */
	ERRBIND,   /* socket bind failed */
	ERRSEND,   /* sendto failed */
	ERRRECV,   /* recvfrom failed */
	ERRCLOS,   /* socket close failed */
	ERRPOLL    /* poll failed */
} status_t;
```

---

### How to discover hosts?
*arpman* module in NodeProbe provides three functions to this matter. So how to use it?

#### Types
```c
typedef struct {
	int sockfd;              /* managed by arpman_create_context / arpman_delete_context */
	int ifindex;             /* network interface index (from if_nametoindex()) */
	int timeout;             /* operation timeout in ms*/
	unsigned char src_mac[6]; /* MAC address of the sending interface */
	unsigned char out_mac[6]; /* destination MAC for the Ethernet frame (usually broadcast: ff:ff:ff:ff:ff:ff) */
	unsigned char src_ip[4];  /* source IPv4 address */
	unsigned char dst_ip[4];  /* target IPv4 address to resolve */
} arpman_context_t;
```

#### Functions
Calling `arpman_create_context()` to initiate the module is required.

```c
status_t arpman_create_context(arpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `arpman_context_t`; The `ifindex` member is necessary to have a valid value and the function sets the `sockfd` |

Return Values
| Status | Description |
| --- | --- |
| ERRSOCK | `socket()` failed; most of the time the program doesn't have root access |
| ERRBIND | `bind()` failed; most of the time the `ifindex` has invalid value |
| SUCCESS | the call was successful |

---

The main function for host discovery is the `arpman_mac_request()`.

```c
status_t arpman_mac_request(arpman_context_t *restrict context, uint8_t *mac);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `arpman_context_t`; all the members should have value for this function, it means `timeout`, `src_mac`, `out_mac`, `src_ip` and `dst_ip` |
| mac | A pointer to a buffer with 6 bytes size; the function fills this buffer with host MAC address if the host exists, you need this mac for other modules because they make their own Ethernet Frame and don't let the *kernel* to resolv the MAC of the host |

Return Values
| Status | Description |
| --- | --- |
| ERRALOC | `calloc()` failed; probably you're out of memory |
| ERRSEND | failed to send the frame over the network; make sure you pass write pointer to the function, check the logfile to troubleshoot |
| ERRTIME | calling `clock_gettime()` was unsuccessful |
| ERRPOLL | calling `poll()` was unsuccessful or pfd.revents had wrong value |
| TIMEOUT | timeout to receive ARP Response from the host; it means there is no host at target IP |
| ERRRECV | failed to receive frame from the network |
| SUCCESS | the call was successful and MAC address of the host have been copied in the `mac` buffer |

---

You must call the `arpman_delete_context()` after you done to free resources

```c
status_t arpman_delete_context(arpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `arpman_context_t` that you've create with `arpman_create_context()` function |

Return Values
| Status | Description |
| --- | --- |
| ERRCLOS | `close()` failed to close the socket; weird problem |
| SUCCESS | the resources have completely freed |

---

#### Example
```c
#include "nodeprobe.h"
#include <stdint.h>
#include <stdio.h>
#include <net/if.h>

#define IFNAME "wlan0"
#define TIMEOUT 2000
#define SRC_MAC {0xa2, 0x25, 0xce, 0x17, 0xe7, exa7} // A2:25:CE:17:E7:A7
#define DST_MAC {0xff, 0xff, 0xff, 0xff, 0xff, 0xff} // FF:FF:FF:FF:FF:FF
#define SRC_IP {192, 168, 1, 1} // 192.168.1.1
#define IP_RANGE_START 1
#define IP_RANGE_END 224

int main(int argc, char **argv)
{
	unsigned int ifindex = if_nametoindex(IFNAME);
	if (ifindex == 0) {
		perror("failed to get interface index");
		return 1;
	}

	arpman_context_t arp_context = {
		.ifindex = (int) ifindex,
		.timeout = TIMEOUT,
		.src_mac = SRC_MAC,
		.out_mac = DST_MAC,
		.src_ip = SRC_IP,
		.dst_ip = {192, 168, 2, 0}
	};

	uint8_t mac[6];

	// creating context

	if (arpman_create_context(&arp_context) != SUCCESS) {
		perror("arpman_create_context() failed");
		return 2;
	}

	// discovering hosts from range 192.168.2.1 to 192.168.2.224

	for (uint8_t i = (uint8_t) IP_RANGE_START; i <= (uint8_t) IP_RANGE_END; i++) {
		arp_context.dst_ip[3] = i;
		switch (arpman_mac_request(&arp_context, mac)) {
		case SUCCESS :
			printf("[ALIVE] MAC address of the host with IP 192.168.2.%d is %X:%X:%X:%X:%X:%X\n", i, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			break;
		case TIMEOUT :
			printf("[DEAD] there is no host with IP 192.168.2.%d\n", i);
			break;
		default :
			printf("[UKNOWN] an error occured\n");
			break;
		}
	}

	// deleting the context and freeing resources

	arpman_delete_context(&arp_context);
	return 0;
}
```

---

### How to ping the host?
The *icmpman* module required.  
This module create an Ethernet Frame plus IPv4 Datagram and ICMP-ECHO message, and the *id* field in icmp echo header is the `id` member in `icmpman_context_t`, it means you must use different values per packet.

#### Types
```c
typedef struct {
	int sockfd;              /* managed by icmpman_create_context / icmpman_delete_context */
	int ifindex;             /* network interface index */
	int timeout;             /* operation timeout in ms*/
	unsigned short id;       /* ICMP echo identifier, used to match replies to requests */
	size_t mtu_size;         /* MTU of the interface, use `ioctl` with `SIOCGIFMTU`*/
	unsigned char src_mac[6];
	unsigned char dst_mac[6];
	unsigned char src_ip[4];
	unsigned char dst_ip[4];
} icmpman_context_t;
```

#### Functions
Call `icmpman_create_context()` to initiate the module and allocate resources.

```c
status_t icmpman_create_context(icmpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `icmpman_context_t` which its `ifindex` member must have a valid value before call |

Return Values
| Status | Description |
| --- | --- |
| ERRSOCK | the function failed to create socket; you need `CAP_NET_RAW` |
| ERRBIND | `bind()` failed to bind on the interface you've passed |
| SUCCESS | context successfully created |

---

The main function to perform an echo-reply icmp operation.

```c
status_t icmpman_echo_request(icmpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `icmpman_context_t`; all member must have a valid value before call |

Return Values
| Status | Description |
| --- | --- |
| ERRALOC | `calloc()` failed; probably you're out of memory |
| ERRSEND | failed to send the frame over the network; make sure you pass write pointer to the function, check the logfile to troubleshoot |
| ERRTIME | calling `clock_gettime()` was unsuccessful |
| ERRPOLL | calling `poll()` was unsuccessful or pfd.revents had wrong value |
| TIMEOUT | timeout to receive ICMP Reply from the host; it means the host isn't *pingable* |
| ERRRECV | failed to receive frame from the network |
| SUCCESS | the call was successful and the host is *pingable* |

---

To free allocated resources you must call the function below

```c
status_t icmpman_delete_context(icmpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `icmpman_context_t` that you've create with `icmpman_create_context()` function |

Return Values
| Status | Description |
| --- | --- |
| ERRCLOS | `close()` failed to close the socket |
| SUCCESS | the resources have completely freed |

---

#### Example
```c
#include "nodeprobe.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define IFNAME "wlan0"
#define TIMEOUT 2000
#define SRC_MAC {0xa2, 0x25, 0xce, 0x17, 0xe7, 0xa7} // A2:25:CE:17:E7:A7
#define DST_MAC {0x5e, 0x79, 0xe0, 0x4e, 0x10, 0x86} // 5E:79:E0:4E:10:86 : the MAC address you've resolved via arpman module
#define SRC_IP {192, 168, 1, 1} // 192.168.1.1
#define DST_IP {192, 168, 2, 51} // 192.168.2.51

int main(int argc, char **argv)
{
	unsigned int ifindex = if_nametoindex(IFNAME);
	if (ifindex == 0) {
		perror("failed to get interface index");
		return 1;
	}

	srand((int) time(NULL));
	icmpman_context_t icmp_context = {
		.ifindex = ifindex,
		.timeout = TIMEOUT,
		.id = (unsigned short) rand();
		.src_mac = SRC_MAC,
		.dst_mac = DST_MAC,
		.src_ip = SRC_IP,
		.dst_ip = DST_IP
	};

	// creating context

	if (icmpman_create_context(&icmp_context) != SUCCESS) {
		perror("icmpman_create_context() failed");
		return 2;
	}

	// get mtu size
	
	struct ifreq ifr = {
		.ifr_name = IFNAME
	};
	if (ioctl(icmp_context.sockfd, SIOCGIFMTU, &ifr) == -1) {
		perror("ioctl() failed to get mtu size of the interface");
		return 3;
	}
	icmp_context.mtu_size = (size_t) ifr.ifr_mtu;

	// pinging the host with IP 192.168.2.51

	switch (icmpman_echo_request(&icmp_context)) {
	case SUCCESS :
		printf("[PINGABLE] successful icmp echo-reply with host at IP 192.168.2.51\n");
		break;
	case TIMEOUT :
		printf("[UNPINGABLE] the host at IP 192.168.2.51 didn't response to our icmp echo message\n");
		break;
	default :
		printf("[UKNOWN] an error occured\n");
		break;
	}

	// deleting the context and freeing resources

	icmpman_delete_context(&icmp_context);
	return 0;
}
```

---

### How to detect active TCP services on specific host?
The *tcpman* module is required for this operation.

#### Types
```c
typedef struct {
	int sockfd;
	int ifindex;
	int timeout;
	size_t mtu_size;
	unsigned char src_mac[6];
	unsigned char dst_mac[6];
	unsigned char src_ip[4];
	unsigned char dst_ip[4];
	unsigned short src_port;
	unsigned short dst_port;
} tcpman_context_t;
```

### Functions
You must call the function below to initiate the module.

```c
status_t tcpman_create_context(tcpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `tcpman_context_t` which its `ifindex` member must have a valid value before call |

Return Values
| Status | Description |
| --- | --- |
| ERRSOCK | the function failed to create socket; you need `CAP_NET_RAW` |
| ERRBIND | `bind()` failed to bind on the interface you've passed |
| SUCCESS | context successfully created |

---

The main function to perform the actions declared below

```c
status_t tcpman_sync_request(tcpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `tcpman_context_t`; all member must have a valid value before call |

Return Values
| Status | Description |
| --- | --- |
| ERRALOC | `calloc()` failed; probably you're out of memory |
| ERRSEND | failed to send the frame over the network; make sure you pass write pointer to the function, check the logfile to troubleshoot |
| ERRTIME | calling `clock_gettime()` was unsuccessful |
| ERRPOLL | calling `poll()` was unsuccessful or pfd.revents had wrong value |
| TIMEOUT | timeout to receive TCP segment from the host; it means a firewall probably dropped the TCP SYN |
| ERRRECV | failed to receive frame from the network |
| FAILURE | there is no active service on the target port |
| SUCCESS | the call was successful and there is an active service on target port |

---

To free allocated resources you must call the function below

```c
status_t tcpman_delete_context(tcpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `tcpman_context_t` that you've create with `tcpman_create_context()` function |

Return Values
| Status | Description |
| --- | --- |
| ERRCLOS | `close()` failed to close the socket |
| SUCCESS | the resources have completely freed |

---

#### Example
```c
#include "nodeprobe.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define IFNAME "wlan0"
#define TIMEOUT 2000
#define SRC_MAC {0xa2, 0x25, 0xce, 0x17, 0xe7, 0xa7} // A2:25:CE:17:E7:A7
#define DST_MAC {0x5e, 0x79, 0xe0, 0x4e, 0x10, 0x86} // 5E:79:E0:4E:10:86 : the MAC address you've resolved via arpman module
#define SRC_IP {192, 168, 1, 1} // 192.168.1.1
#define DST_IP {192, 168, 2, 51} // 192.168.2.51
#define DST_PORT 22 // SSH port

int main(int argc, char **argv)
{
	unsigned int ifindex = if_nametoindex(IFNAME);
	if (ifindex == 0) {
		perror("failed to get interface index");
		return 1;
	}

	srand((int) time(NULL));
	tcpman_context_t tcp_context = {
		.ifindex = ifindex,
		.timeout = TIMEOUT,
		.src_mac = SRC_MAC,
		.dst_mac = DST_MAC,
		.src_ip = SRC_IP,
		.dst_ip = DST_IP,
		.src_port = (rand() % 0xffffffff),
		.dst_port = DST_PORT
	};

	// creating context

	if (tcpman_create_context(&tcp_context) != SUCCESS) {
		perror("tcpman_create_context() failed");
		return 2;
	}

	// get mtu size
	
	struct ifreq ifr = {
		.ifr_name = IFNAME
	};
	if (ioctl(tcp_context.sockfd, SIOCGIFMTU, &ifr) == -1) {
		perror("ioctl() failed to get mtu size of the interface");
		return 3;
	}
	tcp_context.mtu_size = (size_t) ifr.ifr_mtu;

	// detect active service on ssh port of the host at IP 192.168.2.51

	switch (tcpman_sync_request(&tcp_context)) {
	case SUCCESS :
		printf("[ACTIVE] a ssh server detected on host at IP 192.168.2.51\n");
		break;
	case FAILURE :
		printf("[DEACTIVE] there is no ssh server on host at IP 192.168.2.51\n");
	case TIMEOUT :
		printf("[DROPPED] a firewall has dropped the packet\n");
		break;
	default :
		printf("[UKNOWN] an error occured\n");
		break;
	}

	// deleting the context and freeing resources

	tcpman_delete_context(&tcp_context);
	return 0;
}
```

---

### How to detect active UDP services on specific host?
The *udpman* module is required for this operation.

#### Types
```c
typedef struct {
	int sockfd;
	int ifindex;
	int timeout;
	size_t mtu_size;
	unsigned char src_mac[6];
	unsigned char dst_mac[6];
	unsigned char src_ip[4];
	unsigned char dst_ip[4];
	unsigned short src_port;
	unsigned short dst_port;
} udpman_context_t;
```

### Functions
You must call the function below to initiate the module.

```c
status_t udpman_create_context(udpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `udpman_context_t` which its `ifindex` member must have a valid value before call |

Return Values
| Status | Description |
| --- | --- |
| ERRSOCK | the function failed to create socket; you need `CAP_NET_RAW` |
| ERRBIND | `bind()` failed to bind on the interface you've passed |
| SUCCESS | context successfully created |

---

The main function to perform the actions declared below

```c
status_t udpman_udp_request(udpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `udpman_context_t`; all member must have a valid value before call |

Return Values
| Status | Description |
| --- | --- |
| ERRALOC | `calloc()` failed; probably you're out of memory |
| ERRSEND | failed to send the frame over the network; make sure you pass write pointer to the function, check the logfile to troubleshoot |
| ERRTIME | calling `clock_gettime()` was unsuccessful |
| ERRPOLL | calling `poll()` was unsuccessful or pfd.revents had wrong value |
| ERRRECV | failed to receive frame from the network |
| FAILURE | there is no active service on the target port; but is also possible that the ICMP Unreachable message have lost in network; therefore, it's better to repeat this operation several times |
| SUCCESS | the call was successful and there is an active service on target port |

---

To free allocated resources you must call the function below

```c
status_t udpman_delete_context(udpman_context_t *restrict context);
```

Arguments
| Argument | Description |
| --- | --- |
| context | A pointer to `udpman_context_t` that you've create with `udpman_create_context()` function |

Return Values
| Status | Description |
| --- | --- |
| ERRCLOS | `close()` failed to close the socket |
| SUCCESS | the resources have completely freed |

---

#### Example
```c
#include "nodeprobe.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define IFNAME "wlan0"
#define TIMEOUT 2000
#define SRC_MAC {0xa2, 0x25, 0xce, 0x17, 0xe7, 0xa7} // A2:25:CE:17:E7:A7
#define DST_MAC {0x5e, 0x79, 0xe0, 0x4e, 0x10, 0x86} // 5E:79:E0:4E:10:86 : the MAC address you've resolved via arpman module
#define SRC_IP {192, 168, 1, 1} // 192.168.1.1
#define DST_IP {192, 168, 2, 51} // 192.168.2.51
#define DST_PORT 67 // DHCP port

int main(int argc, char **argv)
{
	unsigned int ifindex = if_nametoindex(IFNAME);
	if (ifindex == 0) {
		perror("failed to get interface index");
		return 1;
	}

	srand((int) time(NULL));
	udpman_context_t udp_context = {
		.ifindex = ifindex,
		.timeout = TIMEOUT,
		.src_mac = SRC_MAC,
		.dst_mac = DST_MAC,
		.src_ip = SRC_IP,
		.dst_ip = DST_IP,
		.src_port = (rand() % 0xffffffff),
		.dst_port = DST_PORT
	};

	// creating context

	if (udpman_create_context(&udp_context) != SUCCESS) {
		perror("udpman_create_context() failed");
		return 2;
	}

	// get mtu size
	
	struct ifreq ifr = {
		.ifr_name = IFNAME
	};
	if (ioctl(udp_context.sockfd, SIOCGIFMTU, &ifr) == -1) {
		perror("ioctl() failed to get mtu size of the interface");
		return 3;
	}
	udp_context.mtu_size = (size_t) ifr.ifr_mtu;

	// detect active service on dhcp port of the host at IP 192.168.2.51

	switch (udpman_udp_request(&udp_context)) {
	case SUCCESS :
		printf("[ACTIVE] a dhcp server detected on host at IP 192.168.2.51 (it's possible that a firewall silently has dropped the packet)\n");
		break;
	case FAILURE :
		printf("[DEACTIVE] there is no dhcp server on host at IP 192.168.2.51\n");
	default :
		printf("[UKNOWN] an error occured\n");
		break;
	}

	// deleting the context and freeing resources

	udpman_delete_context(&udp_context);
	return 0;
}
```

### My Interface
I'm currently developing a TUI Interface for this library with Python and [textual](https://github.com/Textualize/textual) framework and it's called [*NodeProbe-TUI*](https://github.com/wizrd00/NodeProbe-TUI).
