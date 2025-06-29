#!/usr/bin/env python3

##
##  Copyright(c) 2019-2023 Qualcomm Innovation Center, Inc. All Rights Reserved.
##
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program; if not, see <http://www.gnu.org/licenses/>.
##

import sys
import re
import string
import hex_common


def gen_shortcode(f, tag):
    f.write(f"DEF_SHORTCODE({tag}, {hex_common.semdict[tag]})\n")


def main():
    hex_common.read_semantics_file(sys.argv[1])
    hex_common.read_attribs_file(sys.argv[2])
    hex_common.calculate_attribs()
    tagregs = hex_common.get_tagregs()
    tagimms = hex_common.get_tagimms()

    with open(sys.argv[3], "w") as f:
        f.write("#ifndef DEF_SHORTCODE\n")
        f.write("#define DEF_SHORTCODE(TAG,SHORTCODE)    /* Nothing */\n")
        f.write("#endif\n")

        for tag in hex_common.get_user_tags():
            gen_shortcode(f, tag)

        f.write("#if !defined(CONFIG_USER_ONLY)\n")
        for tag in hex_common.get_sys_tags():
            gen_shortcode(f, tag)
        f.write("#endif\n")

        f.write("#undef DEF_SHORTCODE\n")


if __name__ == "__main__":
    main()
