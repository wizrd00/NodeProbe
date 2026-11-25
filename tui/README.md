# NodeProbe TUI

A Textual interface for NodeProbe: ARP host discovery, ICMP echo, TCP SYN and UDP service probing from the terminal.

## Install

```sh
pipx install git+https://github.com/wizrd00/NodeProbe.git   # needs make, a C compiler and Python 3.12+
sudo "$(command -v nodeprobe)"                               # raw sockets need root or CAP_NET_RAW
```

From a checkout:

```sh
make                              # builds libnodeprobe-glibc.so in the repository root
pip install -r tui/requirements.txt   # or: pip install textual psutil
cd tui && sudo python3 -m nodeprobe_tui
```

`NODEPROBE_LIB=/path/to/libnodeprobe-glibc.so` overrides the library lookup.

## Keys

| Key | Action |
| --- | --- |
| `Ctrl+S` / `Enter` | start the scan (setup screen) |
| `F5` | rescan the whole range |
| `r` | rescan the selected host |
| `x` | stop the running scan |
| `u` | show / hide unreachable hosts |
| `/` | filter hosts (IP, MAC, port number or service name) |
| `e` | export the results to `nodeprobe-<time>.json` |
| `c` | clear the event log |
| `t` | switch theme |
| `Esc` | back |
| `q` | quit |

Click a column header in the host table to sort by it.
