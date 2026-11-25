# NodeProbe

![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)
![C99](https://img.shields.io/badge/C-C99-blue?style=flat-square)
![Platform](https://img.shields.io/badge/platform-Linux-orange?style=flat-square)

**ARP, ICMP, TCP and UDP host discovery for local networks: thread-safe C99 library with a Textual TUI.**
NodeProbe is a modular C library for low-level network probing and host discovery in local networks. It operates directly at the **data-link layer** using raw sockets (`AF_PACKET`), bypassing the kernel's network stack to give the library full control over frame construction and protocol behavior.

![NodeProbe TUI demo](assets/demo.gif)

> The library is fully standalone and documented: **[API reference (`assets/doc.md`)](assets/doc.md)**

---

## Requirements

- Linux (raw `AF_PACKET` sockets)
- Python 3.12+ (for the TUI)
- `make` and a C compiler: `clang` or `gcc`
- Root or `CAP_NET_RAW` at runtime, because every probe sends raw frames

On Debian : `sudo apt install build-essential pipx`

## Install

**System-wide (recommended)**

```sh
sudo pipx install --global git+https://github.com/wizrd00/NodeProbe.git
sudo nodeprobe
```

**Per user**

```sh
pipx install git+https://github.com/wizrd00/NodeProbe.git
sudo "$(command -v nodeprobe)"
```

The trade-off: a global install puts the command on `sudo`'s path (`sudo nodeprobe` just works) and keeps the files root-owned, but needs a recent `pipx` (`--global`, check `pipx --version`). A per-user install needs no privileges to install, but `sudo` cannot see `~/.local/bin`, so running it as root takes the longer command above, and root ends up executing files your own user can modify.

**Development**

```sh
git clone https://github.com/wizrd00/NodeProbe.git && cd NodeProbe
make                                  # builds libnodeprobe-glibc.so in the repository root
pip install textual psutil
cd tui && sudo python3 -m nodeprobe_tui
```

`NODEPROBE_LIB=/path/to/libnodeprobe-glibc.so` overrides the library lookup.

---

## The library

NodeProbe is a modular C99 library for low-level host discovery and service probing. It builds Ethernet, IP and transport headers in user space and talks to the wire through raw `AF_PACKET` sockets. It has **no dependency besides libc**, and **it does not need the TUI**.

| Module | Protocol | Purpose |
| --- | --- | --- |
| `arpman` | ARP | Host discovery: resolve IP to MAC, silence means no host |
| `icmpman` | ICMPv4 | Reachability: echo request and round-trip time |
| `tcpman` | TCP | Service detection with a raw SYN (open, closed, filtered) |
| `udpman` | UDP | Service detection from the reply or ICMP unreachable |
| `logman` | n/a | Thread-safe structured logger over an `mmap`'d ring buffer |

**Design**

- **Context-based API:** each module owns its socket and state in a `*_context_t` struct. No global state, so concurrent probes need no shared locks.
- **Frames built by hand:** `icmpman`, `tcpman` and `udpman` reuse the MAC that `arpman` already resolved, so the kernel never repeats the ARP lookup for the same host.
- **Per-operation timeouts** with `poll()`, and a status code for every failure mode (`status_t`).
- **Unit-tested without a network:** socket calls are replaced by fakes at link time (see `test/`, built on Unity).

**A complete example: is `192.168.1.1` alive?**

```c
#include "nodeprobe.h"
#include <stdio.h>
#include <net/if.h>

int main(void)
{
	arpman_context_t ctx = {
		.ifindex = (int) if_nametoindex("eth0"),
		.timeout = 400, /* ms */
		.src_mac = {0x02, 0xfc, 0x00, 0x00, 0x00, 0x01},
		.out_mac = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, /* broadcast */
		.src_ip = {192, 168, 1, 10},
		.dst_ip = {192, 168, 1, 1}
	};
	uint8_t mac[6];

	if (arpman_create_context(&ctx) != SUCCESS)
		return 1; /* usually: no root / CAP_NET_RAW */

	status_t status = arpman_mac_request(&ctx, mac);
	if (status == SUCCESS)
		printf("host is up: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	else if (status == TIMEOUT)
		puts("nobody answered: no host at that IP");

	arpman_delete_context(&ctx);
	return 0;
}
```

```sh
clang -std=c99 arp_demo.c -Iinclude -L. -lnodeprobe-glibc -Wl,-rpath=. -o arp_demo
sudo ./arp_demo
```

ICMP, TCP, UDP and logging are covered the same way in **[`assets/doc.md`](assets/doc.md)**, with types, return values and examples for every function.

### Build your own frontend

The TUI below is one consumer of the library, not part of it. Because the library is a plain shared object with a small C API, it can be bound from almost any language (the TUI does it with `ctypes`) and has much more potential than a terminal app: a **graphical desktop interface** (GTK, Qt), a web dashboard, a CLI for scripting, or a monitoring agent. Everything a frontend needs (probing, timing, status codes, logging) is already in the library.

---

## The TUI

A Wireshark-style terminal interface on top of the library: pick an interface and a range, choose the probes, and watch hosts appear live.

![Live scan](assets/scan.gif)

- Live host table, sortable by any column, with a details tree per host (ARP, ICMP, TCP, UDP)
- Filter by IP, MAC, port or service name
- Event log, progress bar, rescan one host or the whole range
- Range formats: `192.168.1.0/24`, `192.168.1.10-50`, `10.0.0.1 10.0.0.7`
- JSON export, light and dark themes

| Key | Action |
| --- | --- |
| `Ctrl+S` / `Enter` | Start the scan |
| `F5` / `r` | Rescan everything / the selected host |
| `x` | Stop |
| `u` | Show or hide unreachable hosts |
| `/` | Filter |
| `e` | Export to `nodeprobe-<time>.json` |
| `c` | Clear the log |
| `t` | Switch theme |
| `Esc` | Back |
| `q` | Quit |

---

## How this was built

I wrote the important part of this project myself, completely, patiently and obsessively: the C library (every protocol module, the context design, the logger and the tests) and the Python wrapper that binds it. The Textual UI layer was written by an AI assistant (Claude) under my direction. I decided what it should do and how it should look, and handed over the part I had no interest in building by hand. It was a pragmatic call: the result is a finished, usable tool instead of a great library with no interface.

## Layout

```
source/ include/    the C library
test/               unit tests (Unity + fake sockets)
assets/doc.md       API reference
tools/logshow.py    reads logman's binary logs
tui/nodeprobe_tui/  the Textual interface
```

## License

MIT, see [`LICENSE`](LICENSE).
