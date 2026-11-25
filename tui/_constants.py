TITLE = "NodeProbe-TUI"
MAN = "Manual"
ARPMAN_TIMEOUT = 400 #ms
ICMPMAN_TIMEOUT = 500 #ms
TCPMAN_TIMEOUT = 500 #ms
UDPMAN_TIMEOUT = 500 #ms

THREADS = 32 # worker threads are mostly blocked inside poll(), so more threads than CPUs is fine
MAX_RANGE_HOSTS = 4096
MAX_PORTS = 1024
BROADCAST_MAC = "ff:ff:ff:ff:ff:ff"

# (title, key) of every column in the host table
TABLE_COLUMNS = [
	("No.", "no"),
	("IPv4 Address", "ip"),
	("MAC Address", "mac"),
	("Status", "status"),
	("Ping", "ping"),
	("TCP Open", "tcp"),
	("UDP Open", "udp"),
	("Time", "time")
]
