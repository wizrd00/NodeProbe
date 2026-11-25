from importlib.metadata import PackageNotFoundError, version

try :
	__version__ = version("nodeprobe")
except PackageNotFoundError : # running from a source checkout
	__version__ = "0+unknown"
