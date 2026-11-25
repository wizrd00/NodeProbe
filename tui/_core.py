from ctypes import *
from _structs import *
from _constants import *
from _tools import *
from _nodeprobe import *
from collections import deque
import threading # dispite of GIL existanse, most of libnodeprobe operations are I/O-bound. So threading library will be a good choice
import random
import time


def task_runner(tasks : "NodeProbeTaskList", results : "NodeProbeResultList", cancel : threading.Event) :
	while (not cancel.is_set()) :
		task = tasks.get_task()
		if (not task) :
			break
		try :
			output = task.run()
		except Exception as error : # a dead worker would make the scan never finish
			output = (NodeProbeResult(task.ip), NodeProbeError(str(error), -1))
			output[0].error = str(error)
		results.add_result(output)


class NodeProbeTask :
	def __init__(self, iface : str, out_mac : str, ip : str, tcp_ports : Iterator[int], udp_ports : Iterator[int], ping : bool, cancel : threading.Event = None, src_mac : str = None, src_ip : str = None) :
		self.iface = iface
		self.out_mac = out_mac
		self.ip = ip
		self.tcp_ports = tcp_ports
		self.udp_ports = udp_ports
		self.ping = ping
		self.cancel = cancel if (cancel) else threading.Event()
		self.src_mac = src_mac
		self.src_ip = src_ip

	def run(self) -> tuple[NodeProbeResult, NodeProbeError] :
		result = NodeProbeResult(self.ip)
		src_mac = self.src_mac if (self.src_mac) else get_src_mac(self.iface)
		dst_mac = self.out_mac
		src_ip = self.src_ip if (self.src_ip) else get_src_ip(self.iface)
		dst_ip = self.ip
		src_port = 0
		dst_port = 0
		try :
			mod = arpman(self.iface, ARPMAN_TIMEOUT, (src_mac, src_ip), (dst_mac, dst_ip))
			start = time.perf_counter()
			result.host_mac = mod.arpman_mac_request()
			result.arpman_time = time.perf_counter() - start
			mod = None
			if (not result.host_mac) :
				return (result, None)
			dst_mac = result.mac_text
			if (self.ping and not self.cancel.is_set()) :
				mod = icmpman(self.iface, ICMPMAN_TIMEOUT, random.randint(0, 65535), (src_mac, src_ip), (dst_mac, dst_ip))
				start = time.perf_counter()
				result.pingable = mod.icmpman_echo_request()
				result.icmpman_time = time.perf_counter() - start
				mod = None
			if (self.tcp_ports and not self.cancel.is_set()) :
				mod = tcpman(self.iface, TCPMAN_TIMEOUT, (src_mac, src_ip, src_port), (dst_mac, dst_ip, dst_port))
				start = time.perf_counter()
				for port in self.tcp_ports :
					if (self.cancel.is_set()) :
						break
					mod.set_src_port(random.randint(1, 65535))
					mod.set_dst_port(port)
					state = mod.tcpman_sync_request()
					if (state) :
						result.open_tcp_ports.append(port)
					elif (state is None) :
						result.filtered_tcp_ports.append(port)
				result.tcpman_time = time.perf_counter() - start
				mod = None
			if (self.udp_ports and not self.cancel.is_set()) :
				mod = udpman(self.iface, UDPMAN_TIMEOUT, (src_mac, src_ip, src_port), (dst_mac, dst_ip, dst_port))
				start = time.perf_counter()
				for port in self.udp_ports :
					if (self.cancel.is_set()) :
						break
					mod.set_src_port(random.randint(1, 65535))
					mod.set_dst_port(port)
					if (mod.udpman_udp_request()) :
						result.open_udp_ports.append(port)
				result.udpman_time = time.perf_counter() - start
				mod = None
		except NodeProbeError as error :
			result.error = str(error)
			return (result, error)
		return (result, None)


class NodeProbeResultList :
	def __init__(self) :
		self.results = deque()
		self.added = 0
		self.mutex = threading.Semaphore(value = 1)

	def add_result(self, result : tuple[NodeProbeResult, NodeProbeError]) -> None :
		self.mutex.acquire()
		self.results.append(result)
		self.added += 1
		self.mutex.release()

	def get_result(self) -> tuple[NodeProbeResult, NodeProbeError] | None :
		self.mutex.acquire()
		result = self.results.popleft() if (len(self.results) > 0) else None
		self.mutex.release()
		return result

	def drain(self) -> list[tuple[NodeProbeResult, NodeProbeError]] :
		self.mutex.acquire()
		results = list(self.results)
		self.results.clear()
		self.mutex.release()
		return results


class NodeProbeTaskList :
	def __init__(self) :
		self.tasks = deque()
		self.mutex = threading.Semaphore(value = 1)

	def add_task(self, task : NodeProbeTask) -> None :
		self.mutex.acquire()
		self.tasks.append(task)
		self.mutex.release()

	def get_task(self) -> NodeProbeTask | None :
		self.mutex.acquire()
		task = self.tasks.popleft() if (len(self.tasks) > 0) else None
		self.mutex.release()
		return task

	def clear(self) -> None :
		self.mutex.acquire()
		self.tasks.clear()
		self.mutex.release()


class NodeProbe :
	def __init__(self, iface : str, out_mac : str, ips : Iterator[str], tcp_ports : Iterator[int], udp_ports : Iterator[int], ping : bool) :
		self.iface = iface
		self.out_mac = out_mac
		self.ips = list(ips) # the same lists are shared between all tasks, so they must not be one-shot iterators
		self.tcp_ports = list(tcp_ports)
		self.udp_ports = list(udp_ports)
		self.ping = ping
		self.total = len(self.ips)
		self.src_mac = get_src_mac(iface)
		self.src_ip = get_src_ip(iface)
		self.tasks = NodeProbeTaskList()
		self.results = NodeProbeResultList()
		self.cancel = threading.Event()
		self.threads = []

	@property
	def completed(self) -> int :
		return self.results.added

	def is_done(self) :
		return all([not thread.is_alive() for thread in self.threads])

	def stop(self) -> None :
		self.cancel.set()
		self.tasks.clear()

	def run(self) :
		for ip in self.ips :
			self.tasks.add_task(
				NodeProbeTask(
					iface = self.iface,
					out_mac = self.out_mac,
					ip = ip,
					tcp_ports = self.tcp_ports,
					udp_ports = self.udp_ports,
					ping = self.ping,
					cancel = self.cancel,
					src_mac = self.src_mac,
					src_ip = self.src_ip
				)
			)
		for _ in range(max(1, min(THREADS, self.total))) :
			self.threads.append(
				threading.Thread(
					target = task_runner,
					args = (self.tasks, self.results, self.cancel),
					daemon = True
				)
			)
		for thread in self.threads :
			thread.start()
