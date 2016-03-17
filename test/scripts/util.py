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
Utility functions that do not fit in any other modules.
"""

def decorate_hook(hook, begin, tag, end):
    """
    Surround a hook with a description.  We refer to this as decorating the
    hook.

    @param hook Decorate this hook.
    @param begin The start of the decoration.
    @param tag A description about the source file from which the hook was
           generated.
    @param end The end of the decoration.
    @return The hook with descriptive comments.
    """
    return "".join([begin, tag, hook, end])
