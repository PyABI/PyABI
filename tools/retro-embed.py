#!/usr/bin/env python3

# retro-encode takes an ngaImage and produces either .py or .cpp source
#
# Copyright (c) 2020-2020, Scott McCallum (github.com scott91e1)
#
# Usage:
#
#     retro-encode.py

import os
import sys
from struct import pack, unpack

memory = []

def load_image():
    global memory
    cells = int(os.path.getsize("ngaImage") / 4)
    f = open("ngaImage", "rb")
    memory = list(unpack(cells * "i", f.read()))
    f.close()

if __name__ == "__main__":
    load_image()

    print(len(memory))
    print('[')
    line = []
    for iter in range(0, len(memory)):
        if iter > 0:
            line.append(',')
        line.append(str(memory[iter]))
        if len(''.join(line)) > 65:
            print(''.join(line))
            line = []
    print(''.join(line))
    print(']')
