#!/usr/bin/env python3

# retro-unu is a tool for extracting code from literate sources. It
# will write output to stdout.
#
# it has been enhaced to convert literate sources to valid reStructedText
# with the source defined by code blocks.  It can also reverse the process.
#
# PS. The conversion to .rst is lossly and reformats comments
#
# A code block starts with ~~~ on a line by itself and ends with
# a # second ~~~.
#
# Copyright (c) 2020-2020, Scott McCallum (github.com scott91e1)
#
# Copyright (c) 2020-2020, Charles Childers
#
# Usage:
#
#    retro-unu.py filename
#    retro-unu.py filename --to-rst
#    retro-unu.py filename --to-unu

import sys

if __name__ == "__main__":
    if len(sys.argv) == 1:
        sys.exit(0)

    f = sys.argv[1]
    in_block = False
    with open(f, "r") as source:
        if len(sys.argv) == 1:
            for line in source.readlines():
                if line.rstrip() == "~~~":
                    in_block = not in_block
                elif in_block:
                    print(line.rstrip())

        elif sys.argv[2] == "--to-rst":
            need_guard = True
            for line in source.readlines():
                if line.rstrip() == "~~~":
                    in_block = not in_block
                elif in_block:
                    print("    " + line.rstrip())
                else:
                    if line[:4] == "    ":
                        if need_guard:
                            print("    ---")
                        else:
                            need_guard = False
                    else:
                        need_guard = True

        elif sys.argv[2] == "--to-unu":
            for line in source.readlines():
                if line.rstrip() == "~~~":
                    in_block = not in_block
                elif in_block:
                    print(line.rstrip())
                else:
                    pass
