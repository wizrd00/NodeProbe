from _structs import *
from _constants import *
from _exceptions import NodeProbeError
from collections.abc import Iterable
from ctypes import c_int, c_size_t, c_ushort
from functools import cache
from ipaddress import ip_address, ip_network
from json import load
from pathlib import Path
from psutil import net_if_addrs, net_if_stats
from re import match
from socket import AF_INET, AF_PACKET, SOCK_RAW, if_nametoindex, inet_pton, socket

PORTS_FILE = Path(__file__).resolve().parent / "ports.json"

pat = [
	r"^((1\d{2}|2[0-4]\d|25[0-5]|[1-9]?\d)(\.(?=\d)|(?!\d))){4}(/([12]?\d|3[0-2]))?$", # matches formats like 192.168.1.1/24
	r"^((1\d{2}|2[0-4]\d|25[0-5]|[1-9]?\d)(\.(?=\d))){3}((1\d{2}|2[0-4]\d|25[0-5]|[1-9]?\d)-(1\d{2}|2[0-4]\d|25[0-5]|[1-9]?\d))?$", # matches formats like 192.168.1.12-48
	r"^(((1\d{2}|2[0-4]\d|25[0-5]|[1-9]?\d)(\.(?=\d)|(?!\d))){4}(\ |$)){1,}$" # matches formats like 10.10.1.1 10.10.1.2 10.10.1.5 etc.
]

def CHECK_STAT(stat : c_int, msg : str) :
	if (stat != status_t.SUCCESS.value) :
		raise NodeProbeError(msg, stat)

def get_ifindex(iface : str) -> c_int :
		return c_int(if_nametoindex(iface))

def get_timeout(timeout : int) -> c_int :
	return c_int(timeout)

def get_id(id : int) -> c_ushort :
	if (id not in range(0, 65536)) :
		raise ValueError(f"the \"id\" argument with value \"{id}\" must be a 16-bit number")
	return c_ushort(id)

def get_mac(mac : str) -> Mac :
	if (not match(r"^((?:[a-fA-F0-9]{2}[:]){5}[a-fA-F0-9]{2})$", mac)) :
		raise ValueError(f"the \"mac\" argument with value \"{mac}\" is invalid")
	return Mac(*[int(i, base = 16) for i in mac.split(":")])

def get_ip(ip : str) -> IPv4 :
	if (not match(r"^(?:(?:25[0-5]|2[0-4]\d|1?\d{1,2})(?:\.(?!$)|$)){4}$", ip)) :
		raise ValueError(f"the \"ip\" argument with value \"{ip}\" is invalid")
	return IPv4(*[int(i) for i in ip.split(".")])

def get_port(port : int) -> c_ushort :
	if (port not in range(0, 65536)) :
		raise ValueError(f"the \"port\" argument with value \"{port}\" must be in (0, 65535) range")
	return c_ushort(port)

def get_mtu(iface : str) -> c_size_t :
	stats = net_if_stats()
	return c_size_t(stats[iface].mtu)

def get_src_mac(iface : str) -> str :
	addrs = net_if_addrs()[iface]
	for addr in addrs :
		if (addr.family == AF_PACKET) :
			return addr.address
	# there is no MAC address
	raise ValueError(f"there is no MAC address assocaited to \"{iface}\"!")

def get_src_ip(iface : str) -> str :
	addrs = net_if_addrs()[iface]
	for addr in addrs :
		if (addr.family == AF_INET) :
			return addr.address
	# there is no IP address
	raise ValueError(f"there is no IP address assocaited to \"{iface}\"")

def check_ip_format(ip : str) -> bool :
	global pat
	return True if (match(pat[0], ip) or match(pat[1], ip) or match(pat[2], ip)) else False

def get_input_placeholder(iface : str) -> str :
	for addr in net_if_addrs()[iface] :
		if (addr.family != AF_INET or not addr.netmask) :
			continue
		a = inet_pton(AF_INET, addr.address)
		b = inet_pton(AF_INET, addr.netmask)
		c = int.from_bytes(b)
		net_sec = ".".join([str(a[i] & b[i]) for i in range(4)])
		count = 0
		for i in range(32) :
			count += 1 if ((1 << i) & c != 0) else 0
		return f"{net_sec}/{count}"
	return "192.168.1.1/24"

def get_suggestion_ports() -> list[tuple[int, str, str]] :
	ports = []
	with open(PORTS_FILE, "r") as file :
		tmp = load(file)
	for k, v in tmp.items() :
		for i in v :
			ports.append((i["port"], k, i["service"]))
	return ports

@cache
def get_service_map() -> dict[tuple[int, str], str] :
	return {(port, trans) : service for port, trans, service in get_suggestion_ports()}

def get_service_name(port : int, trans : str) -> str :
	return get_service_map().get((port, trans), "")

def get_tcp_ports(ports : Iterable[tuple[int, str]]) -> list[int] :
	return sorted({port for port, trans in ports if (trans == "TCP")})

def get_udp_ports(ports : Iterable[tuple[int, str]]) -> list[int] :
	return sorted({port for port, trans in ports if (trans == "UDP")})

def parse_ports(text : str) -> list[int] :
	# accepts formats like "8080, 9090 8000-8010"
	ports = []
	for item in text.replace(",", " ").split() :
		if (match(r"^\d{1,5}-\d{1,5}$", item)) :
			low, high = [int(i) for i in item.split("-")]
			if (low > high) :
				raise ValueError(f"invalid port range \"{item}\"")
			ports.extend(range(low, high + 1))
		elif (match(r"^\d{1,5}$", item)) :
			ports.append(int(item))
		else :
			raise ValueError(f"invalid port \"{item}\"")
		if (len(ports) > MAX_PORTS) :
			raise ValueError(f"too many ports (max {MAX_PORTS})")
	if (any([port < 1 or port > 65535 for port in ports])) :
		raise ValueError("ports must be in 1-65535 range")
	return list(dict.fromkeys(ports))

def get_ifaces() -> list[str] :
	return list(net_if_addrs().keys())

def get_iface_infos() -> list[IfaceInfo] :
	# only interfaces that are able to send ARP requests are interesting
	infos = []
	stats = net_if_stats()
	for name, addrs in net_if_addrs().items() :
		ipv4 = next((addr for addr in addrs if (addr.family == AF_INET)), None)
		mac = next((addr.address for addr in addrs if (addr.family == AF_PACKET)), None)
		if (ipv4 is None or mac is None or name == "lo") :
			continue
		stat = stats.get(name)
		infos.append(IfaceInfo(name, ipv4.address, ipv4.netmask or "255.255.255.0", mac, stat.mtu if (stat) else 0, stat.isup if (stat) else False))
	return infos

def get_default_range(info : IfaceInfo) -> str :
	return str(ip_network(f"{info.ipv4}/{info.netmask}", strict = False))

def has_raw_access() -> bool :
	try :
		with socket(AF_PACKET, SOCK_RAW) :
			return True
	except OSError :
		return False

def get_ip_range(ip : str) -> list[str] :
	ip = ip.strip()
	if (match(pat[0], ip)) :
		net = ip_network(ip, strict = False)
		if (net.num_addresses > MAX_RANGE_HOSTS + 2) :
			raise ValueError(f"range is too large (max {MAX_RANGE_HOSTS} hosts)")
		return [str(host) for host in net.hosts()]
	elif (match(pat[1], ip)) :
		prefix, span = ip.rsplit(".", 1)
		low, high = [int(i) for i in span.split("-")]
		if (low > high) :
			raise ValueError(f"invalid range \"{span}\"")
		return [f"{prefix}.{i}" for i in range(low, high + 1)]
	elif (match(pat[2], ip)) :
		ips = [str(ip_address(i)) for i in ip.split()]
		if (len(ips) > MAX_RANGE_HOSTS) :
			raise ValueError(f"too many hosts (max {MAX_RANGE_HOSTS})")
		return list(dict.fromkeys(ips))
	raise ValueError("invalid IPv4 range")
