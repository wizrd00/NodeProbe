from collections.abc import Iterator
from ctypes import *
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path
from subprocess import getoutput
import sys

Mac = c_ubyte * 6
IPv4 = c_ubyte * 4

class status_t(Enum) :
	SUCCESS = 0
	FAILURE = 1
	TIMEOUT = 2
	INVALID = 3
	ERRALOC = 4
	ERRTIME = 5
	ERRSOCK = 6
	ERRBIND = 7
	ERRSEND = 8
	ERRRECV = 9
	ERRCLOS = 10
	ERRPOLL = 11


class NodeProbeResult :
	def __init__(self, ip : str) :
		self.host_ip : str = ip
		self.host_mac : Mac = None
		self.pingable : bool = None
		self.open_tcp_ports : list = []
		self.filtered_tcp_ports : list = [] # no answer at all, probably dropped by a firewall
		self.open_udp_ports : list = []
		self.arpman_time : float = None
		self.icmpman_time : float = None
		self.tcpman_time : float = None
		self.udpman_time : float = None
		self.error : str = None

	@property
	def alive(self) -> bool :
		return self.host_mac is not None

	@property
	def mac_text(self) -> str :
		return ":".join([f"{i:02X}" for i in self.host_mac]) if (self.host_mac) else ""

	@property
	def total_time(self) -> float :
		return sum([t for t in (self.arpman_time, self.icmpman_time, self.tcpman_time, self.udpman_time) if (t is not None)])

	def to_dict(self) -> dict :
		return {
			"ip" : self.host_ip,
			"alive" : self.alive,
			"mac" : self.mac_text or None,
			"pingable" : self.pingable,
			"open_tcp_ports" : self.open_tcp_ports,
			"filtered_tcp_ports" : self.filtered_tcp_ports,
			"open_udp_ports" : self.open_udp_ports,
			"arpman_time" : self.arpman_time,
			"icmpman_time" : self.icmpman_time,
			"tcpman_time" : self.tcpman_time,
			"udpman_time" : self.udpman_time,
			"error" : self.error
		}

	def __str__(self) :
		text = [
			f"host_ip = {self.host_ip}",
			f"host_mac = {self.mac_text}",
			f"the host {"is" if (self.pingable) else "is not"} pingable",
			f"open_tcp_ports = {self.open_tcp_ports}",
			f"open_udp_ports = {self.open_udp_ports}",
			f"arpman_time = {self.arpman_time}",
			f"icmpman_time = {self.icmpman_time}",
			f"tcpman_time = {self.tcpman_time}",
			f"udpman_time = {self.udpman_time}"
		] if (self.host_mac) else [
			f"host_ip = {self.host_ip}",
			f"the host doesn't seem to be alive"
		]
		return "\n".join(text)


@dataclass
class IfaceInfo :
	name : str
	ipv4 : str
	netmask : str
	mac : str
	mtu : int
	is_up : bool


@dataclass
class ScanConfig :
	iface : str
	range_text : str
	ips : list
	tcp_ports : list
	udp_ports : list
	ping : bool
	out_mac : str = "ff:ff:ff:ff:ff:ff"


class arpman_context_t(Structure) :
	_fields_ = [
		("sockfd", c_int),
		("ifindex", c_int),
		("timeout", c_int),
		("src_mac", Mac),
		("out_mac", Mac),
		("src_ip", IPv4),
		("dst_ip", IPv4)
	]

class icmpman_context_t(Structure) :
	_fields_ = [
		("sockfd", c_int),
		("ifindex", c_int),
		("timeout", c_int),
		("id", c_ushort),
		("mtu_size", c_size_t),
		("src_mac", Mac),
		("dst_mac", Mac),
		("src_ip", IPv4),
		("dst_ip", IPv4)
	]

class tcpman_context_t(Structure) :
	_fields_ = [
		("sockfd", c_int),
		("ifindex", c_int),
		("timeout", c_int),
		("mtu_size", c_size_t),
		("src_mac", Mac),
		("dst_mac", Mac),
		("src_ip", IPv4),
		("dst_ip", IPv4),
		("src_port", c_ushort),
		("dst_port", c_ushort)
	]

class udpman_context_t(Structure) :
	_fields_ = [
		("sockfd", c_int),
		("ifindex", c_int),
		("timeout", c_int),
		("mtu_size", c_size_t),
		("src_mac", Mac),
		("dst_mac", Mac),
		("src_ip", IPv4),
		("dst_ip", IPv4),
		("src_port", c_ushort),
		("dst_port", c_ushort)
	]


def load_library() :
	libc = getoutput("ldd --version 2>&1 | grep -qi musl && echo musl || echo glibc")
	path = Path(__file__).resolve().parent / "library" / f"libnodeprobe-{libc}.so"
	try :
		return cdll.LoadLibrary(str(path))
	except OSError as error :
		sys.exit(f"unable to load {path}\n{error}\nbuild NodeProbe ({libc}) and copy the library into the library/ directory")

lib = load_library()
