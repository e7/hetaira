#! /usr/bin/python


import json
import struct


def pack_sjsonb(ent_type, cargo):
    package = None
    if 3 == ent_type:
        str_cargo = json.dumps(cargo, separators=(",", ":"))
        package = struct.pack(
            "!2I2H2I{}s".format(len(str_cargo)), 0xe78f8a9d, 1000, ent_type, 20, len(str_cargo), 0, str_cargo
        )
    elif 1 == ent_type:
        package = struct.pack(
            "!2I2H2I{}s".format(len(cargo)), 0xe78f8a9d, 1000, ent_type, 20, len(cargo), 0, cargo
        )
    else:
        pass
    return package


def unpack_sjsonb(package):
    magic_no, _, ent_type, ent_offset, ent_sz, _ = struct.unpack("!2I2H2I", package[0:20])
    str_cargo = package[ent_offset : ent_offset + ent_sz]
    cargo = json.loads(str_cargo)
    return ent_type, cargo


if __name__ == "__main__":
    pass
