from _structs import *
from _constants import *
from _tools import *
from datetime import datetime
from rich.style import Style
from rich.text import Text
from textual.app import ComposeResult
from textual.containers import Horizontal
from textual.widgets import DataTable, Label, ProgressBar, RichLog, SelectionList, Tree


def theme_color(widget, name : str, fallback : str = "") -> str :
	return widget.app.get_css_variables().get(name, fallback)


def format_ms(seconds : float) -> str :
	return "" if (seconds is None) else f"{seconds * 1000:.0f} ms"


def format_ports(ports : list, trans : str, limit : int = 6) -> str :
	shown = ", ".join([str(port) for port in ports[:limit]])
	return f"{shown}, +{len(ports) - limit}" if (len(ports) > limit) else shown


def status_name(error : NodeProbeError) -> str :
	try :
		return status_t(error.stat).name
	except ValueError :
		return "ERROR"


class HostTable(DataTable) :
	def on_mount(self) -> None :
		for title, key in TABLE_COLUMNS :
			self.add_column(title, key = key)
		self.cursor_type = "row"
		self.zebra_stripes = True

	def make_row(self, number : int, result : NodeProbeResult, error : NodeProbeError) -> list[Text] :
		ok = theme_color(self, "success")
		bad = theme_color(self, "error")
		if (result.alive) :
			status = Text("Up", style = f"bold {ok}")
		elif (error) :
			status = Text("Error", style = f"bold {bad}")
		else :
			status = Text("Down", style = "dim")
		if (result.pingable is None) :
			ping = Text("")
		elif (result.pingable) :
			ping = Text("Yes", style = ok)
		else :
			ping = Text("No", style = "dim")
		tcp = Text(format_ports(result.open_tcp_ports, "TCP"), style = f"bold {ok}" if (result.open_tcp_ports) else "")
		udp = Text(format_ports(result.open_udp_ports, "UDP"), style = f"bold {ok}" if (result.open_udp_ports) else "")
		dim = "" if (result.alive) else "dim"
		return [
			Text(str(number), justify = "right", style = "dim"),
			Text(result.host_ip, style = dim),
			Text(result.mac_text, style = dim),
			status,
			ping,
			tcp,
			udp,
			Text(format_ms(result.total_time) if (result.alive) else "", justify = "right", style = "dim")
		]

	def upsert(self, number : int, result : NodeProbeResult, error : NodeProbeError) -> None :
		row = self.make_row(number, result, error)
		if (result.host_ip in self.rows) :
			for (title, key), cell in zip(TABLE_COLUMNS, row) :
				self.update_cell(result.host_ip, key, cell, update_width = True)
		else :
			self.add_row(*row, key = result.host_ip)


class HostDetails(Tree) :
	def __init__(self, **kwargs) :
		super().__init__("Host", **kwargs)
		self.show_root = False
		self.guide_depth = 3

	def on_mount(self) -> None :
		self.placeholder()

	def placeholder(self, message : str = "Select a host to inspect it") -> None :
		self.clear()
		self.root.add_leaf(Text(message, style = "dim"))
		self.root.expand()

	@staticmethod
	def field(name : str, value : str, style : str = "") -> Text :
		return Text.assemble((f"{name}: ", "dim"), (value, style))

	def ports_label(self, ports : list, trans : str) -> str :
		return ", ".join([f"{port} {get_service_name(port, trans)}".strip() for port in ports])

	def show(self, result : NodeProbeResult, error : NodeProbeError, config : ScanConfig) -> None :
		ok = theme_color(self, "success")
		warn = theme_color(self, "warning")
		bad = theme_color(self, "error")
		self.clear()
		if (not result.alive) :
			host = self.root.add(Text(f"{result.host_ip}  (no ARP reply)", style = "bold"), expand = True)
			host.add_leaf(self.field("Status", "Down", "dim"))
			host.add_leaf(self.field("ARP time", format_ms(result.arpman_time)))
			if (error) :
				host.add_leaf(self.field("Error", result.error, bad))
			return
		host = self.root.add(Text(result.host_ip, style = "bold"), expand = True)
		link = host.add("Link layer — ARP", expand = True)
		link.add_leaf(self.field("MAC address", result.mac_text))
		link.add_leaf(self.field("Resolved in", format_ms(result.arpman_time)))
		if (config.ping) :
			net = host.add("Network layer — ICMP", expand = True)
			if (result.pingable is None) :
				net.add_leaf(self.field("Echo reply", "not tested", "dim"))
			else :
				net.add_leaf(self.field("Echo reply", "received" if (result.pingable) else "no response", ok if (result.pingable) else "dim"))
				net.add_leaf(self.field("Time", format_ms(result.icmpman_time)))
		if (config.tcp_ports) :
			tcp = host.add("Transport layer — TCP", expand = True)
			tcp.add_leaf(self.field("Open", self.ports_label(result.open_tcp_ports, "TCP") or "none", ok if (result.open_tcp_ports) else "dim"))
			if (result.filtered_tcp_ports) :
				tcp.add_leaf(self.field("Filtered (no answer)", self.ports_label(result.filtered_tcp_ports, "TCP"), warn))
			tcp.add_leaf(self.field("Probed", f"{len(config.tcp_ports)} ports in {format_ms(result.tcpman_time)}"))
		if (config.udp_ports) :
			udp = host.add("Transport layer — UDP", expand = True)
			udp.add_leaf(self.field("Open | filtered", self.ports_label(result.open_udp_ports, "UDP") or "none", ok if (result.open_udp_ports) else "dim"))
			udp.add_leaf(self.field("Probed", f"{len(config.udp_ports)} ports in {format_ms(result.udpman_time)}"))
		if (error) :
			host.add_leaf(self.field("Error", result.error, bad))


class EventLog(RichLog) :
	LEVELS = {"INFO" : "primary", "OK" : "success", "WARN" : "warning", "ERROR" : "error"}

	def __init__(self, **kwargs) -> None :
		super().__init__(min_width = 20, wrap = True, markup = False, **kwargs)

	def event(self, level : str, message : str) -> None :
		color = theme_color(self, self.LEVELS.get(level, "primary"))
		self.write(Text.assemble(
			(datetime.now().strftime("%H:%M:%S"), "dim"),
			"  ",
			(f"{level:<5}", Style(color = color, bold = True)),
			"  ",
			message
		))


class ScanStatus(Horizontal) :
	def compose(self) -> ComposeResult :
		yield Label("Ready", id = "scan-state")
		yield ProgressBar(total = 1, show_eta = False, show_percentage = True, id = "scan-progress")
		yield Label("", id = "scan-counts")
		yield Label("", id = "scan-time")

	def update_status(self, state : str, kind : str, done : int, total : int, alive : int, elapsed : float) -> None :
		label = self.query_one("#scan-state", Label)
		label.update(Text.assemble(("● ", theme_color(self, kind)), (state, "bold")))
		bar = self.query_one("#scan-progress", ProgressBar)
		bar.update(total = max(total, 1), progress = done)
		self.query_one("#scan-counts", Label).update(f"{done} / {total} scanned   ·   {alive} up")
		self.query_one("#scan-time", Label).update(f"{elapsed:.1f} s")


class PortList(SelectionList) :
	pass
