# NodeProbe

![Language](https://img.shields.io/badge/language-C99-blue?style=flat-square)
![Standard](https://img.shields.io/badge/standard-POSIX-lightgrey?style=flat-square)
![Platform](https://img.shields.io/badge/platform-Linux-orange?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)
![libc](https://img.shields.io/badge/libc-musl%20%7C%20glibc-yellow?style=flat-square)
![Layer](https://img.shields.io/badge/layer-L2%20%2F%20L3%20%2F%20L4-red?style=flat-square)

NodeProbe is a modular C library for low-level network probing and host discovery in local networks. It operates directly at the **data-link layer** using raw sockets (`AF_PACKET`), bypassing the kernel's network stack to give the library full control over frame construction and protocol behavior.

Each module is designed to be **independent** and **thread-safe** through per-instance context structs — making NodeProbe suitable for multi-threaded scanners where multiple probes run concurrently without shared state.

> Full API documentation: [`assets/doc.md`](assets/doc.md)

---

## Modules

| Module | Protocol | Socket | Purpose |
|--------|----------|--------|---------|
| `arpman` | ARP | `AF_PACKET` / `ETH_P_ARP` | Host discovery — resolves IP → MAC |
| `icmpman` | ICMPv4 | `AF_PACKET` / `ETH_P_IP` | Reachability — sends Echo Request |
| `tcpman` | TCP | `AF_PACKET` / `ETH_P_IP` | TCP service detection — sends raw SYN, detects active services on each host|
| `udpman` | UDP | `AF_PACKET` / `ETH_P_IP` | UDP service detection — sends raw UDP datagram, listens for ICMP unreachable |
| `logman` | — | — | Structured logger with `mmap`|

---

## Design

- **Context-based API** — every module exposes a `*_context_t` struct. The caller fills in the target, source, interface, and timeout fields before calling the module's functions. No global state.
- **Raw socket per module** — each context owns its socket. When multiple threads each hold their own context, they filter incoming frames that aren't belonging to their request.
- **Frame construction in userspace** — Ethernet, IP, and protocol headers are built manually. `icmpman`, `tcpman` and `udpman` use `AF_PACKET` to avoid triggering unnecessary kernel-level ARP lookups for already-resolved MACs.
- **Deadline-aware timeout** — `poll()` is used for receive blocking. Timeout management via `clock_gettime()` is planned for the multi-packet receive path to enforce a hard per-operation deadline rather than a per-call timeout.
- **Dual libc support** — the Makefile detects musl vs. glibc at build time and names the output accordingly (`libnodeprobe-musl.so` / `libnodeprobe-glibc.so`).

---

## Building

**Requirements:**
- Linux kernel with `AF_PACKET` support
- `pcc` or `gcc`
- musl or glibc
- `CAP_NET_RAW` capability at runtime (or run as root)

```sh
# default (uses pcc, auto-detects libc)
make

# with gcc
make CC=gcc

# clean
make clean
```

Output: `libnodeprobe-musl.so` or `libnodeprobe-glibc.so` in the project root.

---

## License

MIT — see [`LICENSE`](LICENSE)
