# NodeProbe-TUI

A Textual interface for the [NodeProbe](https://github.com/wizrd00/NodeProbe) C library — ARP host discovery, ICMP echo, TCP SYN and UDP service probing from the terminal.

## Run

```sh
pip install -r requirements.txt      # Python 3.12+
cp ../nodeprobe/libnodeprobe-glibc.so library/   # or libnodeprobe-musl.so on musl systems
sudo python3 _tui.py                 # raw sockets need root or CAP_NET_RAW
```

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
