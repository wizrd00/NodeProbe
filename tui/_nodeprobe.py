from ctypes import *
from _structs import *
from _constants import *
from _tools import *

# Creating this interface according to NodeProbe Documentation at link https://github.com/wizrd00/NodeProbe/blob/main/assets/doc.md

class arpman :
	def __init__(self, iface : str, timeout : int, src_addr : tuple[str, str], dst_addr : tuple[str, str]) :
		self.context = arpman_context_t(
			ifindex = get_ifindex(iface),
			timeout = get_timeout(timeout),
			src_mac = get_mac(src_addr[0]),
			out_mac = get_mac(dst_addr[0]), # it must be the broadcast MAC address or gateway MAC address
			src_ip = get_ip(src_addr[1]),
			dst_ip = get_ip(dst_addr[1])
		)
		stat = lib.arpman_create_context(pointer(self.context))
		if (stat != status_t.SUCCESS.value) :
			raise NodeProbeError("lib.arpman_create_context()", stat)
		self.created = True

	def arpman_mac_request(self) -> Mac | None :
		mac = Mac()
		stat = lib.arpman_mac_request(pointer(self.context), pointer(mac))
		match (stat) :
			case status_t.SUCCESS.value :
				return mac
			case status_t.TIMEOUT.value :
				return None
			case _ :
				raise NodeProbeError("lib.arpman_mac_request() failed", stat)

	def __str__(self) :
		text = "self.context = arpman_context_t(" + "\n" \
		+ "\t" + f"ifindex = {self.context.ifindex}," + "\n" \
		+ "\t" + f"timeout = {self.context.timeout}," + "\n" \
		+ "\t" + f"src_mac = {":".join([f"{i:02X}" for i in self.context.src_mac])}," + "\n" \
		+ "\t" + f"out_mac = {":".join([f"{i:02X}" for i in self.context.out_mac])}," + "\n" \
		+ "\t" + f"src_ip = {".".join([str(i) for i in self.context.src_ip])}," + "\n" \
		+ "\t" + f"dst_ip = {".".join([str(i) for i in self.context.dst_ip])}," + "\n" \
		+ ")" + "\n"
		return text

	def __del__(self) :
		# a failed *_create_context() must not be followed by *_delete_context() (sockfd is still 0)
		if (getattr(self, "created", False)) :
			lib.arpman_delete_context(pointer(self.context))


class icmpman :
	def __init__(self, iface : str, timeout : int, id : int, src_addr : tuple[str, str], dst_addr : tuple[str, str]) :
		self.context = icmpman_context_t(
			ifindex = get_ifindex(iface),
			timeout = get_timeout(timeout),
			id = get_id(id),
			mtu_size = get_mtu(iface),
			src_mac = get_mac(src_addr[0]),
			dst_mac = get_mac(dst_addr[0]),
			src_ip = get_ip(src_addr[1]),
			dst_ip = get_ip(dst_addr[1])
		)
		stat = lib.icmpman_create_context(pointer(self.context))
		if (stat != status_t.SUCCESS.value) :
			raise NodeProbeError("lib.icmpman_create_context()", stat)
		self.created = True

	def icmpman_echo_request(self) -> bool | None :
		stat = lib.icmpman_echo_request(pointer(self.context))
		match (stat) :
			case status_t.SUCCESS.value :
				return True
			case status_t.TIMEOUT.value :
				return False
			case _ :
				raise NodeProbeError("lib.icmpman_echo_request() failed", stat)

	def __str__(self) :
		text = "self.context = icmpman_context_t(" + "\n" \
		+ "\t" + f"ifindex = {self.context.ifindex}," + "\n" \
		+ "\t" + f"timeout = {self.context.timeout}," + "\n" \
		+ "\t" + f"id = {self.context.id}," + "\n" \
		+ "\t" + f"mtu_size = {self.context.mtu_size}," + "\n" \
		+ "\t" + f"src_mac = {":".join([f"{i:02X}" for i in self.context.src_mac])}," + "\n" \
		+ "\t" + f"dst_mac = {":".join([f"{i:02X}" for i in self.context.dst_mac])}," + "\n" \
		+ "\t" + f"src_ip = {".".join([str(i) for i in self.context.src_ip])}," + "\n" \
		+ "\t" + f"dst_ip = {".".join([str(i) for i in self.context.dst_ip])}," + "\n" \
		+ ")" + "\n"
		return text

	def __del__(self) :
		# a failed *_create_context() must not be followed by *_delete_context() (sockfd is still 0)
		if (getattr(self, "created", False)) :
			lib.icmpman_delete_context(pointer(self.context))


class tcpman :
	def __init__(self, iface : str, timeout : int, src_addr : tuple[str, str, int], dst_addr : tuple[str, str, int]) :
		self.context = tcpman_context_t(
			ifindex = get_ifindex(iface),
			timeout = get_timeout(timeout),
			mtu_size = get_mtu(iface),
			src_mac = get_mac(src_addr[0]),
			dst_mac = get_mac(dst_addr[0]),
			src_ip = get_ip(src_addr[1]),
			dst_ip = get_ip(dst_addr[1]),
			src_port = get_port(src_addr[2]),
			dst_port = get_port(dst_addr[2])
		)
		stat = lib.tcpman_create_context(pointer(self.context))
		if (stat != status_t.SUCCESS.value) :
			raise NodeProbeError("lib.tcpman_create_context()", stat)
		self.created = True

	def set_src_port(self, port : int) -> None :
		self.context.src_port = get_port(port)

	def set_dst_port(self, port : int) -> None :
		self.context.dst_port = get_port(port)

	def tcpman_sync_request(self) -> bool | None : # True = open, False = closed, None = filtered
		stat = lib.tcpman_sync_request(pointer(self.context))
		match (stat) :
			case status_t.SUCCESS.value :
				return True
			case status_t.FAILURE.value :
				return False
			case status_t.TIMEOUT.value :
				return None # no answer at all, a firewall has probably dropped the SYN
			case _ :
				raise NodeProbeError("lib.tcpman_sync_request() failed", stat)

	def __str__(self) :
		text = "self.context = tcpman_context_t(" + "\n" \
		+ "\t" + f"ifindex = {self.context.ifindex}," + "\n" \
		+ "\t" + f"timeout = {self.context.timeout}," + "\n" \
		+ "\t" + f"mtu_size = {self.context.mtu_size}," + "\n" \
		+ "\t" + f"src_mac = {":".join([f"{i:02X}" for i in self.context.src_mac])}," + "\n" \
		+ "\t" + f"dst_mac = {":".join([f"{i:02X}" for i in self.context.dst_mac])}," + "\n" \
		+ "\t" + f"src_ip = {".".join([str(i) for i in self.context.src_ip])}," + "\n" \
		+ "\t" + f"dst_ip = {".".join([str(i) for i in self.context.dst_ip])}," + "\n" \
		+ "\t" + f"src_port = {self.context.src_port}," + "\n" \
		+ "\t" + f"dst_port = {self.context.dst_port}," + "\n" \
		+ ")" + "\n"
		return text

	def __del__(self) :
		# a failed *_create_context() must not be followed by *_delete_context() (sockfd is still 0)
		if (getattr(self, "created", False)) :
			lib.tcpman_delete_context(pointer(self.context))


class udpman :
	def __init__(self, iface : str, timeout : int, src_addr : tuple[str, str, int], dst_addr : tuple[str, str, int]) :
		self.context = udpman_context_t(
			ifindex = get_ifindex(iface),
			timeout = get_timeout(timeout),
			mtu_size = get_mtu(iface),
			src_mac = get_mac(src_addr[0]),
			dst_mac = get_mac(dst_addr[0]),
			src_ip = get_ip(src_addr[1]),
			dst_ip = get_ip(dst_addr[1]),
			src_port = get_port(src_addr[2]),
			dst_port = get_port(dst_addr[2])
		)
		stat = lib.udpman_create_context(pointer(self.context))
		if (stat != status_t.SUCCESS.value) :
			raise NodeProbeError("lib.udpman_create_context()", stat)
		self.created = True

	def set_src_port(self, port : int) -> None :
		self.context.src_port = get_port(port)

	def set_dst_port(self, port : int) -> None :
		self.context.dst_port = get_port(port)

	def udpman_udp_request(self) -> bool | None :
		stat = lib.udpman_udp_request(pointer(self.context))
		match (stat) :
			case status_t.SUCCESS.value :
				return True
			case status_t.FAILURE.value :
				return False
			case _ :
				raise NodeProbeError("lib.udpman_udp_request() failed", stat)

	def __str__(self) :
		text = "self.context = udpman_context_t(" + "\n" \
		+ "\t" + f"ifindex = {self.context.ifindex}," + "\n" \
		+ "\t" + f"timeout = {self.context.timeout}," + "\n" \
		+ "\t" + f"mtu_size = {self.context.mtu_size}," + "\n" \
		+ "\t" + f"src_mac = {":".join([f"{i:02X}" for i in self.context.src_mac])}," + "\n" \
		+ "\t" + f"dst_mac = {":".join([f"{i:02X}" for i in self.context.dst_mac])}," + "\n" \
		+ "\t" + f"src_ip = {".".join([str(i) for i in self.context.src_ip])}," + "\n" \
		+ "\t" + f"dst_ip = {".".join([str(i) for i in self.context.dst_ip])}," + "\n" \
		+ "\t" + f"src_port = {self.context.src_port}," + "\n" \
		+ "\t" + f"dst_port = {self.context.dst_port}," + "\n" \
		+ ")" + "\n"
		return text

	def __del__(self) :
		# a failed *_create_context() must not be followed by *_delete_context() (sockfd is still 0)
		if (getattr(self, "created", False)) :
			lib.udpman_delete_context(pointer(self.context))
