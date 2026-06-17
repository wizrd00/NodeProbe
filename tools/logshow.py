#!/usr/bin/python
import sys
import os
import struct

def main() :
	fmt = "6c11c9c32c32c512c"
	if (len(sys.argv) != 2) :
		raise Exception("sys.argv[1] not found")
	if (not os.path.exists(sys.argv[1])) :
		raise Exception(f"{sys.argv[1]} not found")
	with open(sys.argv[1], "rb") as log :
		while (log) :
			msg = log.read(struct.calcsize(fmt))
			msg = struct.unpack(fmt, msg)
			level = (b"".join(msg[:6])).split(b"\x00")[0].decode()
			date = (b"".join(msg[6:17])).split(b"\x00")[0].decode()
			clock = (b"".join(msg[17:26])).split(b"\x00")[0].decode()
			mod = (b"".join(msg[26:58])).split(b"\x00")[0].decode()
			pos = (b"".join(msg[58:90])).split(b"\x00")[0].decode()
			logmsg = (b"".join(msg[90:])).split(b"\x00")[0].decode()
			if (msg[0] == b"\x00") :
				break;
			print(f"[{level}][{date}][{clock}][MOD:{mod}][POS:{pos}][MSG:{logmsg}]")

if __name__ == "__main__" :
	main()
