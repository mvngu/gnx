# gnx -- algorithms for graphs and networks
# Copyright (C) 2016 Minh Van Nguyen <mvngu.name AT gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, see <http://www.gnu.org/licenses/>.

"""
Insert hooks into the master gnx.h header file of the gnx library.  Hooks are
required for gnx to build with our custom wrappers to library functions.
"""

from os import path
import argparse
import os
import sys

sys.path.append(path.abspath(path.join(path.dirname(__file__), "..")))
from util import decorate_hook

###########################################################################
# helper functions
###########################################################################

def insert_hooks(target, installoc, prefix):
    """
    Insert hooks into a C source file.

    @param target Insert hooks into this file.
    @param installoc Whether to have src/alloc.h as part of the installed
           public interface.
    @param prefix Insert a hook at the line that has this prefix string.
    """
    assert installoc in ("yes", "no")
    install_alloc = False
    if installoc == "yes":
        install_alloc = True

    header_hook = prefix + "ALLOC_H */"
    begin = "/* start of generated code */\n"
    tag = "/* code generated from %s */\n" % __file__
    end = "/* end of generated code */\n"
    code = ""

    with open(target, "r") as f:
        for line in f:
            if line.strip() == header_hook:
                code += line
                if install_alloc:
                    hook = '#include "alloc.h"\n'
                    code += decorate_hook(hook, begin, tag, end)
                continue

            code += line

    with open(target, "w") as f:
        f.write(code)

###########################################################################
# script starts here
###########################################################################

if __name__ == "__main__":
    s = "Insert hooks into the master gnx.h header file.\n"
    parser = argparse.ArgumentParser(description=s)
    parser.add_argument("--target", metavar="file", required=True,
                        help="insert hooks into this file")
    parser.add_argument("--installoc", metavar="boolean", required=True,
                        help="install src/alloc.h")
    args = parser.parse_args()
    target = args.target
    installoc = args.installoc

    if not path.exists(target):
        raise IOError("File not found: %s" % target)

    prefix = "/* GNX_HOOK:"
    insert_hooks(target, installoc, prefix)
