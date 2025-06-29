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


##
## Helpers for gen_helper_function
##
def gen_decl_ea(f):
    f.write("    uint32_t EA __attribute__((unused));\n")


def gen_decl_insn(tag, f, slot):
    str = f"Insn tmp_insn = {{ .slot = {slot} }};\n"
    f.write(str)
    f.write("Insn *insn __attribute__((unused)) = &tmp_insn;\n")


def gen_helper_return_type(f, regtype, regid, regno):
    if regno > 1:
        f.write(", ")
    f.write("int32_t")


def gen_helper_return_type_pair(f, regtype, regid, regno):
    if regno > 1:
        f.write(", ")
    f.write("int64_t")


def gen_helper_arg(f, regtype, regid, regno):
    if regno > 0:
        f.write(", ")
    f.write(f"int32_t {regtype}{regid}V")


def gen_helper_arg_new(f, regtype, regid, regno):
    if regno >= 0:
        f.write(", ")
    f.write(f"int32_t {regtype}{regid}N")


def gen_helper_arg_pair(f, regtype, regid, regno):
    if regno >= 0:
        f.write(", ")
    f.write(f"int64_t {regtype}{regid}V")


def gen_helper_arg_ext(f, regtype, regid, regno):
    if regno > 0:
        f.write(", ")
    f.write(f"void *{regtype}{regid}V_void")


def gen_helper_arg_ext_pair(f, regtype, regid, regno):
    if regno > 0:
        f.write(", ")
    f.write(f"void *{regtype}{regid}V_void")


def gen_helper_arg_opn(f, regtype, regid, i, tag):
    if hex_common.is_pair(regid):
        if hex_common.is_hvx_reg(regtype):
            gen_helper_arg_ext_pair(f, regtype, regid, i)
        else:
            gen_helper_arg_pair(f, regtype, regid, i)
    elif hex_common.is_single(regid):
        if hex_common.is_old_val(regtype, regid, tag):
            if hex_common.is_hvx_reg(regtype):
                gen_helper_arg_ext(f, regtype, regid, i)
            else:
                gen_helper_arg(f, regtype, regid, i)
        elif hex_common.is_new_val(regtype, regid, tag):
            gen_helper_arg_new(f, regtype, regid, i)
        else:
            hex_common.bad_register(regtype, regid)
    else:
        hex_common.bad_register(regtype, regid)


def gen_helper_arg_imm(f, immlett):
    f.write(f", int32_t {hex_common.imm_name(immlett)}")


def gen_helper_dest_decl(f, regtype, regid, regno, subfield=""):
    f.write(f"    int32_t {regtype}{regid}V{subfield} = 0;\n")


def gen_helper_dest_decl_pair(f, regtype, regid, regno, subfield=""):
    f.write(f"    int64_t {regtype}{regid}V{subfield} = 0;\n")


def gen_helper_dest_decl_ext(f, regtype, regid):
    if regtype == "Q":
        f.write(
            f"    /* {regtype}{regid}V is *(MMQReg *)" f"({regtype}{regid}V_void) */\n"
        )
    else:
        f.write(
            f"    /* {regtype}{regid}V is *(MMVector *)"
            f"({regtype}{regid}V_void) */\n"
        )


def gen_helper_dest_decl_ext_pair(f, regtype, regid, regno):
    f.write(
        f"    /* {regtype}{regid}V is *(MMVectorPair *))"
        f"{regtype}{regid}V_void) */\n"
    )


def gen_helper_dest_decl_opn(f, regtype, regid, i):
    if hex_common.is_pair(regid):
        if hex_common.is_hvx_reg(regtype):
            gen_helper_dest_decl_ext_pair(f, regtype, regid, i)
        else:
            gen_helper_dest_decl_pair(f, regtype, regid, i)
    elif hex_common.is_single(regid):
        if hex_common.is_hvx_reg(regtype):
            gen_helper_dest_decl_ext(f, regtype, regid)
        else:
            gen_helper_dest_decl(f, regtype, regid, i)
    else:
        hex_common.bad_register(regtype, regid)


def gen_helper_src_var_ext(f, regtype, regid):
    if regtype == "Q":
        f.write(
            f"    /* {regtype}{regid}V is *(MMQReg *)" f"({regtype}{regid}V_void) */\n"
        )
    else:
        f.write(
            f"    /* {regtype}{regid}V is *(MMVector *)"
            f"({regtype}{regid}V_void) */\n"
        )


def gen_helper_src_var_ext_pair(f, regtype, regid, regno):
    f.write(
        f"    /* {regtype}{regid}V{regno} is *(MMVectorPair *)"
        f"({regtype}{regid}V{regno}_void) */\n"
    )


def gen_helper_return(f, regtype, regid, regno):
    f.write(f"    return {regtype}{regid}V;\n")


def gen_helper_return_pair(f, regtype, regid, regno):
    f.write(f"    return {regtype}{regid}V;\n")


def gen_helper_dst_write_ext(f, regtype, regid):
    return


def gen_helper_dst_write_ext_pair(f, regtype, regid):
    return


def gen_helper_return_opn(f, regtype, regid, i):
    if hex_common.is_pair(regid):
        if hex_common.is_hvx_reg(regtype):
            gen_helper_dst_write_ext_pair(f, regtype, regid)
        else:
            gen_helper_return_pair(f, regtype, regid, i)
    elif hex_common.is_single(regid):
        if hex_common.is_hvx_reg(regtype):
            gen_helper_dst_write_ext(f, regtype, regid)
        else:
            gen_helper_return(f, regtype, regid, i)
    else:
        hex_common.bad_register(regtype, regid)


mem_init_attrs = [
    'A_LOAD',
    'A_STORE',
]
def needs_page_size(tag):
    return any(A in hex_common.attribdict[tag] for A in mem_init_attrs)

def needs_cpu_memop_pc(tag):
    return any(A in hex_common.attribdict[tag] for A in mem_init_attrs + ['A_DMA'])

##
## Generate the TCG code to call the helper
##     For A2_add: Rd32=add(Rs32,Rt32), { RdV=RsV+RtV;}
##     We produce:
##       int32_t HELPER(A2_add)(CPUHexagonState *env, int32_t RsV, int32_t RtV)
##       {
##           uint32_t slot __attribute__(unused)) = 4;
##           int32_t RdV = 0;
##           { RdV=RsV+RtV;}
##           COUNT_HELPER(A2_add);
##           return RdV;
##       }
##
def gen_helper_function(f, tag, tagregs, tagimms):
    regs = tagregs[tag]
    imms = tagimms[tag]

    numresults = 0
    numscalarresults = 0
    numscalarreadwrite = 0
    for regtype, regid in regs:
        if hex_common.is_written(regid):
            numresults += 1
            if hex_common.is_scalar_reg(regtype):
                numscalarresults += 1
        if hex_common.is_readwrite(regid):
            if hex_common.is_scalar_reg(regtype):
                numscalarreadwrite += 1

    if numscalarresults > 1:
        ## The helper is bogus when there is more than one result
        f.write(
            f"void HELPER({tag})(CPUHexagonState *env) " f"{{ BOGUS_HELPER({tag}); }}\n"
        )
    else:
        ## The return type of the function is the type of the destination
        ## register (if scalar)
        i = 0
        for regtype, regid in regs:
            if hex_common.is_written(regid):
                if hex_common.is_pair(regid):
                    if hex_common.is_hvx_reg(regtype):
                        continue
                    else:
                        gen_helper_return_type_pair(f, regtype, regid, i)
                elif hex_common.is_single(regid):
                    if hex_common.is_hvx_reg(regtype):
                        continue
                    else:
                        gen_helper_return_type(f, regtype, regid, i)
                else:
                    hex_common.bad_register(regtype, regid)
            i += 1

        if numscalarresults == 0:
            f.write("void")
        f.write(f" HELPER({tag})(CPUHexagonState *env")

        ## Arguments include the vector destination operands
        i = 1
        for regtype, regid in regs:
            if hex_common.is_written(regid):
                if hex_common.is_pair(regid):
                    if hex_common.is_hvx_reg(regtype):
                        gen_helper_arg_ext_pair(f, regtype, regid, i)
                    else:
                        continue
                elif hex_common.is_single(regid):
                    if hex_common.is_hvx_reg(regtype):
                        gen_helper_arg_ext(f, regtype, regid, i)
                    else:
                        continue
                else:
                    hex_common.bad_register(regtype, regid)
                i += 1

        ## For conditional instructions, we pass in the destination register
        if "A_CONDEXEC" in hex_common.attribdict[tag]:
            for regtype, regid in regs:
                if hex_common.is_writeonly(regid) and not hex_common.is_hvx_reg(
                    regtype
                ):
                    gen_helper_arg_opn(f, regtype, regid, i, tag)
                    i += 1

        ## Arguments to the helper function are the source regs and immediates
        for regtype, regid in regs:
            if hex_common.is_read(regid):
                if hex_common.is_hvx_reg(regtype) and hex_common.is_readwrite(regid):
                    continue
                gen_helper_arg_opn(f, regtype, regid, i, tag)
                i += 1
        for immlett, bits, immshift in imms:
            gen_helper_arg_imm(f, immlett)
            i += 1

        if hex_common.need_pkt_has_multi_cof(tag):
            f.write(", uint32_t pkt_has_multi_cof")
        if (hex_common.need_pkt_need_commit(tag)):
            f.write(", uint32_t pkt_need_commit")

        if hex_common.need_PC(tag):
            if i > 0:
                f.write(", ")
            f.write("target_ulong PC")
            i += 1
        if hex_common.helper_needs_next_PC(tag):
            if i > 0:
                f.write(", ")
            f.write("target_ulong next_PC")
            i += 1
        if hex_common.need_slot(tag):
            if i > 0:
                f.write(", ")
            f.write("uint32_t slotval")
            i += 1
        if hex_common.need_part1(tag):
            if i > 0:
                f.write(", ")
            f.write("uint32_t part1")
        f.write(")\n{\n")

        if needs_cpu_memop_pc(tag):
            f.write("    CPU_MEMOP_PC_SET(env);\n")

        if hex_common.need_slot(tag):
            f.write("    uint32_t slot = slotval >> 1;\n")

        if "A_COPROC" in hex_common.attribdict[tag]:
            f.write('    qemu_log_mask(LOG_UNIMP, "coproc unimp\\n");\n')
        else:
            if hex_common.need_ea(tag):
                gen_decl_ea(f)

            if hex_common.is_scatter_gather(tag):
                if hex_common.is_gather(tag):
                    gen_decl_insn(tag, f, 1)
                else:
                    gen_decl_insn(tag,f,0)
            if hex_common.is_coproc(tag):
                if hex_common.is_coproc_act(tag):
                    gen_decl_insn(tag,f,1)
                else:
                    gen_decl_insn(tag, f, 0)

            ## Declare the return variable
            i = 0
            if "A_CONDEXEC" not in hex_common.attribdict[tag]:
                for regtype, regid in regs:
                    if hex_common.is_writeonly(regid):
                        gen_helper_dest_decl_opn(f, regtype, regid, i)
                    i += 1

            for regtype, regid in regs:
                if hex_common.is_read(regid):
                    if hex_common.is_pair(regid):
                        if hex_common.is_hvx_reg(regtype):
                            gen_helper_src_var_ext_pair(f, regtype, regid, i)
                    elif hex_common.is_single(regid):
                        if hex_common.is_hvx_reg(regtype):
                            gen_helper_src_var_ext(f, regtype, regid)
                    else:
                        hex_common.bad_register(regtype, regid)

            if "A_FPOP" in hex_common.attribdict[tag]:
                f.write("    arch_fpop_start(env);\n")

            f.write(f"    {hex_common.semdict[tag]}\n")
            f.write(f"    COUNT_HELPER({tag});\n")

            if "A_FPOP" in hex_common.attribdict[tag]:
                f.write("    arch_fpop_end(env);\n")

            ## Save/return the return variable
            for regtype, regid in regs:
                if hex_common.is_written(regid):
                    gen_helper_return_opn(f, regtype, regid, i)
        f.write("}\n\n")
        ## End of the helper definition


def main():
    hex_common.read_semantics_file(sys.argv[1])
    hex_common.read_attribs_file(sys.argv[2])
    hex_common.read_overrides_file(sys.argv[3])
    hex_common.read_overrides_file(sys.argv[4])
    hex_common.read_overrides_file(sys.argv[5])
    ## Whether or not idef-parser is enabled is
    ## determined by the number of arguments to
    ## this script:
    ##
    ##   6 args. -> not enabled,
    ##   7 args. -> idef-parser enabled.
    ##
    ## The 7:th arg. then holds a list of the successfully
    ## parsed instructions.
    is_idef_parser_enabled = len(sys.argv) > 7
    if is_idef_parser_enabled:
        hex_common.read_idef_parser_enabled_file(sys.argv[6])
    hex_common.calculate_attribs()
    tagregs = hex_common.get_tagregs()
    tagimms = hex_common.get_tagimms()

    output_file = sys.argv[-1]
    with open(output_file, "w") as f:
        for tag in hex_common.get_user_tags():
            if hex_common.tag_ignore(tag):
                continue
            if hex_common.skip_qemu_helper(tag):
                continue
            if hex_common.is_idef_parser_enabled(tag):
                continue
            gen_helper_function(f, tag, tagregs, tagimms)

        f.write("#if !defined(CONFIG_USER_ONLY)\n")
        for tag in hex_common.get_sys_tags():
            if hex_common.skip_qemu_helper(tag):
                continue
            if hex_common.is_idef_parser_enabled(tag):
                continue
            gen_helper_function(f, tag, tagregs, tagimms)
        f.write("#endif\n")


if __name__ == "__main__":
    main()
