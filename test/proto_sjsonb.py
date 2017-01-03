#! /usr/bin/python


import sys
import json
import struct
import socket
import endecode


if __name__ == "__main__":
    cli = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        cli.connect(("127.0.0.1", 22055))
    except socket.error as e:
        print e
        sys.exit(1)

    cli.send(endecode.pack_sjsonb(3, {"interface":"getusername",}))
    buf = cli.recv(4096)
    print endecode.unpack_sjsonb(buf)
