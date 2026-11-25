from ._structs import *
from ._constants import *
from ._tools import *
from ._menus import *
from itertools import cycle
import sys
from textual.app import App
from textual.binding import Binding
from textual.theme import Theme

NODEPROBE_DARK = Theme(
	name = "nodeprobe-dark",
	primary = "#4C9BE8",
	secondary = "#7A8CA5",
	accent = "#E5A84B",
	warning = "#E5A84B",
	error = "#E5534B",
	success = "#4CAF7A",
	foreground = "#D5DBE3",
	background = "#14181F",
	surface = "#1A2029",
	panel = "#232B37",
	dark = True
)

NODEPROBE_LIGHT = Theme(
	name = "nodeprobe-light",
	primary = "#1F6FEB",
	secondary = "#57606A",
	accent = "#BF8700",
	warning = "#BF8700",
	error = "#CF222E",
	success = "#1A7F37",
	foreground = "#1F2328",
	background = "#EEF1F5",
	surface = "#FFFFFF",
	panel = "#DDE3EA",
	dark = False
)

class NodeProbeTUI(App) :
	BINDINGS = [
		Binding("t", "switch_theme", "Theme"),
		Binding("q", "quit", "Quit")
	]
	CSS_PATH = "style.css"

	def on_mount(self) -> None :
		self.register_theme(NODEPROBE_DARK)
		self.register_theme(NODEPROBE_LIGHT)
		self.themes = cycle([
			"nodeprobe-dark",
			"nodeprobe-light",
			"nord",
			"gruvbox",
			"tokyo-night",
			"solarized-light",
			"atom-one-dark",
			"atom-one-light"
		])
		self.theme = next(self.themes)
		self.push_screen(SetupScreen())

	def action_switch_theme(self) -> None :
		self.theme = next(self.themes)


def main() -> None :
	if (len(sys.argv) > 1 and sys.argv[1] in ("-V", "--version")) :
		from . import __version__
		print(f"nodeprobe {__version__}")
		return
	NodeProbeTUI().run()
