# The Python package ships libnodeprobe, so building it means running `make` first.
# Everything else (name, dependencies, entry point...) lives in pyproject.toml.
import os
import shutil
import subprocess
import sys
from pathlib import Path

from setuptools import Distribution, setup
from setuptools.command.build_py import build_py

ROOT = Path(__file__).resolve().parent
LIBRARY = "libnodeprobe-glibc.so" # the file the Makefile produces


def pick_compiler() -> str :
	if (os.environ.get("CC")) :
		return os.environ["CC"]
	for compiler in ("clang", "gcc", "cc") : # the Makefile asks for clang, but gcc is fine too
		if (shutil.which(compiler)) :
			return compiler
	sys.exit("error: no C compiler found (install clang or gcc)")


class build_py_with_library(build_py) :
	def run(self) :
		if (shutil.which("make") is None) :
			sys.exit("error: `make` is required to build libnodeprobe")
		subprocess.run(["make", "-B", f"CC={pick_compiler()}"], cwd = ROOT, check = True)
		super().run()
		target = Path(self.build_lib) / "nodeprobe_tui" / "lib"
		target.mkdir(parents = True, exist_ok = True)
		shutil.copy2(ROOT / LIBRARY, target / LIBRARY)


class PlatformDistribution(Distribution) :
	def has_ext_modules(self) : # the wheel contains a compiled library, so it must not be tagged "any"
		return True


setup(
	distclass = PlatformDistribution,
	cmdclass = {"build_py" : build_py_with_library},
)
