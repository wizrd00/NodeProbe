from _structs import *
from _constants import *
from _elements import *
from _core import *
from _tools import *
from collections import Counter
from datetime import datetime
from ipaddress import ip_address
from json import dump
from rich.text import Text
from textual import on
from textual.app import ComposeResult
from textual.binding import Binding
from textual.containers import Horizontal, Vertical
from textual.screen import Screen
from textual.widgets import Button, Checkbox, DataTable, Footer, Header, Input, Label, Static, TabbedContent, TabPane
from textual.widgets.selection_list import Selection
import time


class SetupScreen(Screen) :
	BINDINGS = [
		Binding("ctrl+s", "start", "Start scan")
	]

	def __init__(self) -> None :
		super().__init__()
		self.infos : list[IfaceInfo] = []
		self.iface : IfaceInfo = None
		self.raw_access = has_raw_access()

	def compose(self) -> ComposeResult :
		yield Header(icon = " ")
		with Vertical(id = "setup") :
			with Vertical(classes = "panel", id = "iface-panel") :
				yield DataTable(id = "ifaces", cursor_type = "row", zebra_stripes = True)
			with Vertical(classes = "panel", id = "target-panel") :
				yield Input(placeholder = "192.168.1.0/24", id = "range")
				yield Static(id = "range-hint")
			with Vertical(classes = "panel", id = "probe-panel") :
				yield Checkbox("ICMP echo (ping) — host discovery via ARP always runs first", True, id = "ping")
				with TabbedContent(id = "ports-tabs") :
					for trans in ("TCP", "UDP") :
						with TabPane(f"{trans} ports", id = f"{trans.lower()}-tab") :
							with Horizontal(classes = "port-row") :
								yield PortList(
									*[Selection(f"{port:<6}{service}", port, True) for port, t, service in get_suggestion_ports() if (t == trans)],
									id = f"{trans.lower()}-ports"
								)
								with Vertical(classes = "port-side") :
									yield Button("Select all", id = f"{trans.lower()}-all", compact = True)
									yield Button("Clear", id = f"{trans.lower()}-none", compact = True)
									yield Label("Additional ports", classes = "muted")
									yield Input(placeholder = "8080, 8000-8010", id = f"{trans.lower()}-extra")
			with Horizontal(id = "setup-actions") :
				yield Static(id = "setup-warning")
				yield Button("Start Scan", variant = "primary", id = "start", disabled = True)
		yield Footer()

	def on_mount(self) -> None :
		self.title = TITLE
		self.sub_title = "New scan"
		self.query_one("#iface-panel").border_title = "Network interface"
		self.query_one("#target-panel").border_title = "Target range  (a.b.c.d/x  ·  a.b.c.x-y  ·  ip ip ip)"
		self.query_one("#probe-panel").border_title = "Probes"
		table = self.query_one("#ifaces", DataTable)
		table.add_columns("Interface", "IPv4 Address", "MAC Address", "MTU", "State")
		self.infos = get_iface_infos()
		for info in self.infos :
			table.add_row(info.name, info.ipv4, info.mac, str(info.mtu), "Up" if (info.is_up) else Text("Down", style = "dim"), key = info.name)
		if (not self.raw_access) :
			self.query_one("#setup-warning", Static).update(Text("⚠ Raw sockets need root or CAP_NET_RAW — run this program with sudo", style = f"bold {theme_color(self, 'warning')}"))
		if (self.infos) :
			table.focus()
			self.select_iface(self.infos[0])
		else :
			self.query_one("#setup-warning", Static).update(Text("⚠ No usable network interface found", style = f"bold {theme_color(self, 'warning')}"))

	def select_iface(self, info : IfaceInfo) -> None :
		self.iface = info
		self.query_one("#range", Input).value = get_default_range(info)
		self.validate()

	@on(DataTable.RowHighlighted, "#ifaces")
	def iface_highlighted(self, event : DataTable.RowHighlighted) -> None :
		if (event.row_key is None or event.row_key.value is None) :
			return
		info = next((i for i in self.infos if (i.name == event.row_key.value)), None)
		if (info and info is not self.iface) :
			self.select_iface(info)

	@on(Input.Changed)
	def input_changed(self, event : Input.Changed) -> None :
		self.validate()

	@on(Input.Submitted)
	def input_submitted(self, event : Input.Submitted) -> None :
		self.action_start()

	@on(Button.Pressed)
	def button_pressed(self, event : Button.Pressed) -> None :
		button = event.button.id
		if (button == "start") :
			self.action_start()
		elif (button.endswith("-all")) :
			self.query_one(f"#{button[:3]}-ports", PortList).select_all()
		elif (button.endswith("-none")) :
			self.query_one(f"#{button[:3]}-ports", PortList).deselect_all()

	def collect(self) -> ScanConfig :
		# raises ValueError with a message that is fit to show to the user
		if (self.iface is None) :
			raise ValueError("select a network interface")
		range_text = self.query_one("#range", Input).value.strip()
		ips = get_ip_range(range_text)
		if (not ips) :
			raise ValueError("the range contains no hosts")
		tcp = sorted(set(self.query_one("#tcp-ports", PortList).selected) | set(parse_ports(self.query_one("#tcp-extra", Input).value)))
		udp = sorted(set(self.query_one("#udp-ports", PortList).selected) | set(parse_ports(self.query_one("#udp-extra", Input).value)))
		return ScanConfig(
			iface = self.iface.name,
			range_text = range_text,
			ips = ips,
			tcp_ports = tcp,
			udp_ports = udp,
			ping = self.query_one("#ping", Checkbox).value
		)

	def validate(self) -> bool :
		hint = self.query_one("#range-hint", Static)
		button = self.query_one("#start", Button)
		try :
			config = self.collect()
		except ValueError as error :
			hint.update(Text(f"✗ {error}", style = theme_color(self, "error")))
			button.disabled = True
			return False
		count = len(config.ips)
		hint.update(Text(f"✓ {count} host{'s' if (count != 1) else ''}  ·  {config.ips[0]} – {config.ips[-1]}", style = theme_color(self, "success")))
		button.disabled = not self.raw_access
		return self.raw_access

	@on(SelectionList.SelectedChanged)
	@on(Checkbox.Changed)
	def options_changed(self) -> None :
		self.validate()

	def action_start(self) -> None :
		if (not self.validate()) :
			self.notify("Fix the highlighted problem before starting", severity = "warning")
			return
		self.app.push_screen(ScanScreen(self.collect()))


class ScanScreen(Screen) :
	BINDINGS = [
		Binding("f5", "rescan_all", "Rescan all"),
		Binding("r", "rescan_selected", "Rescan host"),
		Binding("x", "stop", "Stop"),
		Binding("u", "toggle_down", "Unreachable"),
		Binding("e", "export", "Export"),
		Binding("c", "clear_log", "Clear log"),
		Binding("slash", "focus_filter", "Filter"),
		Binding("escape", "back", "Back")
	]

	def __init__(self, config : ScanConfig) -> None :
		super().__init__()
		self.config = config
		self.probe : NodeProbe = None
		self.running = False
		self.stopping = False
		self.started = 0.0
		self.elapsed = 0.0
		self.hosts : dict[str, tuple[NodeProbeResult, NodeProbeError]] = {}
		self.numbers : dict[str, int] = {}
		self.errors : Counter = Counter()
		self.run_alive = 0
		self.selected_ip : str = None
		self.show_down = False
		self.sort_key : str = None
		self.sort_reverse = False

	def compose(self) -> ComposeResult :
		yield Header(icon = " ")
		with Vertical(id = "scan") :
			with Horizontal(id = "filter-bar") :
				yield Input(placeholder = "Filter hosts by IP, MAC, port or service    ( / )", id = "filter")
				yield Label(id = "filter-info")
			yield HostTable(id = "hosts")
			with Horizontal(id = "lower") :
				yield HostDetails(id = "details")
				yield EventLog(id = "events")
			yield ScanStatus(id = "status")
		yield Footer()

	def on_mount(self) -> None :
		self.title = TITLE
		self.sub_title = f"{self.config.iface}  ·  {self.config.range_text}"
		self.query_one("#hosts").border_title = "Hosts"
		self.query_one("#details").border_title = "Details"
		self.query_one("#events").border_title = "Event log"
		self.query_one("#hosts", HostTable).focus()
		self.set_interval(0.1, self.poll)
		self.app.theme_changed_signal.subscribe(self, lambda _ : self.refresh_table())
		self.start_probe(self.config.ips, reset = True)

	# ---------------------------------------------------------------- scanning

	@property
	def events(self) -> EventLog :
		return self.query_one("#events", EventLog)

	def start_probe(self, ips : list[str], reset : bool) -> None :
		if (reset) :
			self.hosts.clear()
			self.numbers.clear()
			self.errors.clear()
			self.selected_ip = None
			self.refresh_table()
			self.query_one("#details", HostDetails).placeholder()
		try :
			self.probe = NodeProbe(
				iface = self.config.iface,
				out_mac = self.config.out_mac,
				ips = ips,
				tcp_ports = self.config.tcp_ports,
				udp_ports = self.config.udp_ports,
				ping = self.config.ping
			)
		except Exception as error :
			self.events.event("ERROR", f"unable to start the scan: {error}")
			self.update_status()
			return
		self.running = True
		self.stopping = False
		self.run_alive = 0
		self.started = time.perf_counter()
		self.elapsed = 0.0
		protocols = ["ARP"] + (["ICMP"] if (self.config.ping) else []) + (["TCP"] if (self.config.tcp_ports) else []) + (["UDP"] if (self.config.udp_ports) else [])
		self.events.event("INFO", f"Scanning {len(ips)} host{'s' if (len(ips) != 1) else ''} on {self.config.iface} ({' + '.join(protocols)})")
		self.probe.run()

	def poll(self) -> None :
		if (self.probe is None) :
			return
		finished = self.running and self.probe.is_done() # must be checked before draining, or late results could be missed
		batch = self.probe.results.drain()
		if (batch) :
			self.handle_batch(batch)
		if (self.running) :
			self.elapsed = time.perf_counter() - self.started
		if (finished) :
			self.finish()
		self.update_status()

	def handle_batch(self, batch : list[tuple[NodeProbeResult, NodeProbeError]]) -> None :
		table = self.query_one("#hosts", HostTable)
		details = self.query_one("#details", HostDetails)
		for result, error in batch :
			ip = result.host_ip
			if (ip not in self.numbers) :
				self.numbers[ip] = len(self.numbers) + 1
			self.hosts[ip] = (result, error)
			if (error) :
				status = status_name(error)
				if (self.errors[status] == 0) :
					self.events.event("ERROR", f"{ip}: {error.message} [{status}]")
				self.errors[status] += 1
			elif (result.alive) :
				self.run_alive += 1
				ports = [f"{port}/tcp" for port in result.open_tcp_ports] + [f"{port}/udp" for port in result.open_udp_ports]
				self.events.event("OK", f"{ip:<15}  {result.mac_text}" + (f"  open: {', '.join(ports)}" if (ports) else ""))
			if (ip == self.selected_ip) :
				details.show(result, error, self.config)
		if (self.sort_key) :
			self.refresh_table()
		else :
			for result, error in batch :
				if (self.is_visible(result)) :
					table.upsert(self.numbers[result.host_ip], result, error)
			self.update_filter_info()

	def finish(self) -> None :
		self.running = False
		alive = sum([1 for result, error in self.hosts.values() if (result.alive)])
		if (self.stopping) :
			self.events.event("WARN", f"Scan stopped after {self.probe.completed} of {self.probe.total} hosts")
		else :
			self.events.event("INFO", f"Scan complete — {self.run_alive} of {self.probe.total} host{'s' if (self.probe.total != 1) else ''} up in {self.elapsed:.1f} s")
		for status, count in self.errors.items() :
			if (count > 1) :
				self.events.event("WARN", f"{count} hosts failed with {status}")
		self.errors.clear()
		self.stopping = False

	def update_status(self) -> None :
		if (self.probe is None) :
			return
		if (self.stopping and self.running) :
			state, kind = "Stopping…", "warning"
		elif (self.running) :
			state, kind = "Scanning", "primary"
		elif (self.probe.cancel.is_set()) :
			state, kind = "Stopped", "warning"
		else :
			state, kind = "Complete", "success"
		alive = sum([1 for result, error in self.hosts.values() if (result.alive)])
		self.query_one("#status", ScanStatus).update_status(state, kind, self.probe.completed, self.probe.total, alive, self.elapsed)

	# ------------------------------------------------------------------- table

	def search_text(self, result : NodeProbeResult) -> str :
		parts = [result.host_ip, result.mac_text]
		parts += [f"{port} {get_service_name(port, 'TCP')}" for port in result.open_tcp_ports + result.filtered_tcp_ports]
		parts += [f"{port} {get_service_name(port, 'UDP')}" for port in result.open_udp_ports]
		return " ".join(parts).lower()

	def is_visible(self, result : NodeProbeResult) -> bool :
		if (not result.alive and not self.show_down) :
			return False
		needle = self.query_one("#filter", Input).value.strip().lower()
		return all([word in self.search_text(result) for word in needle.split()]) if (needle) else True

	def sort_value(self, item : tuple[NodeProbeResult, NodeProbeError]) :
		result, error = item
		return {
			"no" : lambda : self.numbers[result.host_ip],
			"ip" : lambda : int(ip_address(result.host_ip)),
			"mac" : lambda : result.mac_text,
			"status" : lambda : 2 if (result.alive) else 1 if (error) else 0,
			"ping" : lambda : -1 if (result.pingable is None) else int(result.pingable),
			"tcp" : lambda : len(result.open_tcp_ports),
			"udp" : lambda : len(result.open_udp_ports),
			"time" : lambda : result.total_time
		}[self.sort_key]()

	def refresh_table(self) -> None :
		table = self.query_one("#hosts", HostTable)
		items = list(self.hosts.values())
		if (self.sort_key) :
			items.sort(key = self.sort_value, reverse = self.sort_reverse)
		table.clear()
		for result, error in items :
			if (self.is_visible(result)) :
				table.upsert(self.numbers[result.host_ip], result, error)
		if (self.selected_ip in table.rows) :
			table.move_cursor(row = table.get_row_index(self.selected_ip))
		self.update_filter_info()

	def update_filter_info(self) -> None :
		shown = self.query_one("#hosts", HostTable).row_count
		note = "" if (self.show_down) else "   ·   unreachable hidden"
		self.query_one("#filter-info", Label).update(f"{shown} of {len(self.hosts)} hosts{note}")

	@on(DataTable.RowHighlighted, "#hosts")
	def host_highlighted(self, event : DataTable.RowHighlighted) -> None :
		if (event.row_key is None or event.row_key.value is None) :
			return
		self.selected_ip = event.row_key.value
		if (self.selected_ip in self.hosts) :
			result, error = self.hosts[self.selected_ip]
			self.query_one("#details", HostDetails).show(result, error, self.config)

	@on(DataTable.HeaderSelected, "#hosts")
	def header_selected(self, event : DataTable.HeaderSelected) -> None :
		key = event.column_key.value
		self.sort_reverse = (not self.sort_reverse) if (key == self.sort_key) else False
		self.sort_key = key
		self.refresh_table()

	@on(Input.Changed, "#filter")
	def filter_changed(self, event : Input.Changed) -> None :
		self.refresh_table()

	@on(Input.Submitted, "#filter")
	def filter_submitted(self, event : Input.Submitted) -> None :
		self.query_one("#hosts", HostTable).focus()

	# ----------------------------------------------------------------- actions

	def action_rescan_all(self) -> None :
		if (self.running) :
			self.notify("A scan is already running — press x to stop it first", severity = "warning")
			return
		self.start_probe(self.config.ips, reset = True)

	def action_rescan_selected(self) -> None :
		if (self.running) :
			self.notify("A scan is already running — press x to stop it first", severity = "warning")
		elif (self.selected_ip is None) :
			self.notify("Select a host first", severity = "warning")
		else :
			self.start_probe([self.selected_ip], reset = False)

	def action_stop(self) -> None :
		if (self.running and not self.stopping) :
			self.stopping = True
			self.probe.stop()

	def action_toggle_down(self) -> None :
		self.show_down = not self.show_down
		self.refresh_table()

	def action_clear_log(self) -> None :
		self.events.clear()

	def action_focus_filter(self) -> None :
		self.query_one("#filter", Input).focus()

	def action_export(self) -> None :
		if (not self.hosts) :
			self.notify("Nothing to export yet", severity = "warning")
			return
		name = f"nodeprobe-{datetime.now().strftime('%Y%m%d-%H%M%S')}.json"
		data = {
			"interface" : self.config.iface,
			"range" : self.config.range_text,
			"ping" : self.config.ping,
			"tcp_ports" : self.config.tcp_ports,
			"udp_ports" : self.config.udp_ports,
			"hosts" : [result.to_dict() for result, error in self.hosts.values()]
		}
		try :
			with open(name, "w") as file :
				dump(data, file, indent = 2)
		except OSError as error :
			self.notify(f"Export failed: {error}", severity = "error")
			return
		self.notify(f"Saved {name}")

	def action_back(self) -> None :
		if (isinstance(self.focused, Input) and self.focused.id == "filter") :
			self.query_one("#hosts", HostTable).focus()
			return
		if (self.probe and self.running) :
			self.probe.stop()
		self.app.pop_screen()
