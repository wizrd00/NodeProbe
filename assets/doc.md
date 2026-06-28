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
    ERRSOCK,   /* socket creation failed */
    ERRBIND,   /* socket bind failed */
    ERRSEND,   /* sendto failed */
    ERRRECV,   /* recvfrom failed */
    ERRCLOS,   /* socket close failed */
    ERRPOLL    /* poll failed */
} status_t;
```

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
	unsigned char src_ip[4];  /* source IPv4 address
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
| context | A pointer to arpman_context_t; the `ifindex` member is necessary to have a valid value and the function sets the `sockfd` |

Return
| Status | Description |
| --- | --- |
| ERRSOCK | socket() failed; most of the time the program doesn't root access |
| ERRBIND | bind() failed; most of the time the `ifindex` is an invalid number |
| SUCCESS | the call was successful |
