/*
 *  Copyright(c) 2019-2023 Qualcomm Innovation Center, Inc. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HEXAGON_MACROS_H
#define HEXAGON_MACROS_H

#include "cpu.h"
#include "hex_regs.h"
#include "reg_fields.h"
#include "attribs.h"

#define PCALIGN 4
#define PCALIGN_MASK (PCALIGN - 1)

#ifdef QEMU_GENERATE
#define GET_FIELD(RES, FIELD, REGIN) \
    tcg_gen_extract_tl(RES, REGIN, reg_field_info[FIELD].offset, \
                                   reg_field_info[FIELD].width)
#else
#define GET_FIELD(FIELD, REGIN) \
    fEXTRACTU_BITS(REGIN, reg_field_info[FIELD].width, \
                   reg_field_info[FIELD].offset)
#endif

#ifdef QEMU_GENERATE
#define GET_USR_FIELD(FIELD, DST) \
    tcg_gen_extract_tl(DST, hex_gpr[HEX_REG_USR], \
                       reg_field_info[FIELD].offset, \
                       reg_field_info[FIELD].width)

#define TYPE_INT(X)          __builtin_types_compatible_p(typeof(X), int)
#define TYPE_TCGV(X)         __builtin_types_compatible_p(typeof(X), TCGv)
#define TYPE_TCGV_I64(X)     __builtin_types_compatible_p(typeof(X), TCGv_i64)
#else
#define GET_USR_FIELD(FIELD) \
    fEXTRACTU_BITS(env->gpr[HEX_REG_USR], reg_field_info[FIELD].width, \
                   reg_field_info[FIELD].offset)

#define SET_USR_FIELD(FIELD, VAL) \
    do { \
        if (pkt_need_commit) { \
            fINSERT_BITS(env->new_value_usr, \
                        reg_field_info[FIELD].width, \
                        reg_field_info[FIELD].offset, (VAL)); \
        } else { \
            fINSERT_BITS(env->gpr[HEX_REG_USR], \
                        reg_field_info[FIELD].width, \
                        reg_field_info[FIELD].offset, (VAL)); \
        } \
    } while (0)
#endif

#ifdef QEMU_GENERATE
/*
 * Section 5.5 of the Hexagon V67 Programmer's Reference Manual
 *
 * Slot 1 store with slot 0 load
 * A slot 1 store operation with a slot 0 load operation can appear in a packet.
 * The packet attribute :mem_noshuf inhibits the instruction reordering that
 * would otherwise be done by the assembler. For example:
 *     {
 *         memw(R5) = R2 // slot 1 store
 *         R3 = memh(R6) // slot 0 load
 *     }:mem_noshuf
 * Unlike most packetized operations, these memory operations are not executed
 * in parallel (Section 3.3.1). Instead, the store instruction in Slot 1
 * effectively executes first, followed by the load instruction in Slot 0. If
 * the addresses of the two operations are overlapping, the load will receive
 * the newly stored data. This feature is supported in processor versions
 * V65 or greater.
 *
 *
 * For qemu, we look for a load in slot 0 when there is  a store in slot 1
 * in the same packet.  When we see this, we call a helper that probes the
 * load to make sure it doesn't fault.  Then, we process the store ahead of
 * the actual load.
 */
#define CHECK_NOSHUF(VA, SIZE) \
    do { \
        if (insn->slot == 0 && ctx->pkt->pkt_has_scalar_store_s1) { \
            probe_noshuf_load(VA, SIZE, ctx->mem_idx); \
            process_store(ctx, 1); \
        } \
    } while (0)

#define CHECK_NOSHUF_PRED(GET_EA, SIZE, PRED) \
    do { \
        TCGLabel *label = gen_new_label(); \
        tcg_gen_brcondi_tl(TCG_COND_EQ, PRED, 0, label); \
        GET_EA; \
        if (insn->slot == 0 && ctx->pkt->pkt_has_scalar_store_s1) { \
            probe_noshuf_load(EA, SIZE, ctx->mem_idx); \
        } \
        gen_set_label(label); \
        if (insn->slot == 0 && ctx->pkt->pkt_has_scalar_store_s1) { \
            process_store(ctx, 1); \
        } \
    } while (0)

#define MEM_LOAD1s(DST, VA) \
    do { \
        CHECK_NOSHUF(VA, 1); \
        tcg_gen_qemu_ld_tl(DST, VA, ctx->mem_idx, MO_SB | MO_ALIGN); \
    } while (0)
#define MEM_LOAD1u(DST, VA) \
    do { \
        CHECK_NOSHUF(VA, 1); \
        tcg_gen_qemu_ld_tl(DST, VA, ctx->mem_idx, MO_UB | MO_ALIGN); \
    } while (0)
#define MEM_LOAD2s(DST, VA) \
    do { \
        CHECK_NOSHUF(VA, 2); \
        tcg_gen_qemu_ld_tl(DST, VA, ctx->mem_idx, MO_TESW | MO_ALIGN); \
    } while (0)
#define MEM_LOAD2u(DST, VA) \
    do { \
        CHECK_NOSHUF(VA, 2); \
        tcg_gen_qemu_ld_tl(DST, VA, ctx->mem_idx, MO_TEUW | MO_ALIGN); \
    } while (0)
#define MEM_LOAD4s(DST, VA) \
    do { \
        CHECK_NOSHUF(VA, 4); \
        tcg_gen_qemu_ld_tl(DST, VA, ctx->mem_idx, MO_TESL | MO_ALIGN); \
    } while (0)
#define MEM_LOAD4u(DST, VA) \
    do { \
        CHECK_NOSHUF(VA, 4); \
        tcg_gen_qemu_ld_tl(DST, VA, ctx->mem_idx, MO_TEUL | MO_ALIGN); \
    } while (0)
#define MEM_LOAD8u(DST, VA) \
    do { \
        CHECK_NOSHUF(VA, 8); \
        tcg_gen_qemu_ld_i64(DST, VA, ctx->mem_idx, MO_TEUQ | MO_ALIGN); \
    } while (0)

#define MEM_STORE1_FUNC(X) \
    __builtin_choose_expr(TYPE_INT(X), \
        gen_store1i, \
        __builtin_choose_expr(TYPE_TCGV(X), \
            gen_store1, (void)0))
#define MEM_STORE1(VA, DATA, SLOT) \
    MEM_STORE1_FUNC(DATA)(tcg_env, VA, DATA, SLOT)

#define MEM_STORE2_FUNC(X) \
    __builtin_choose_expr(TYPE_INT(X), \
        gen_store2i, \
        __builtin_choose_expr(TYPE_TCGV(X), \
            gen_store2, (void)0))
#define MEM_STORE2(VA, DATA, SLOT) \
    MEM_STORE2_FUNC(DATA)(tcg_env, VA, DATA, SLOT)

#define MEM_STORE4_FUNC(X) \
    __builtin_choose_expr(TYPE_INT(X), \
        gen_store4i, \
        __builtin_choose_expr(TYPE_TCGV(X), \
            gen_store4, (void)0))
#define MEM_STORE4(VA, DATA, SLOT) \
    MEM_STORE4_FUNC(DATA)(tcg_env, VA, DATA, SLOT)

#define MEM_STORE8_FUNC(X) \
    __builtin_choose_expr(TYPE_INT(X), \
        gen_store8i, \
        __builtin_choose_expr(TYPE_TCGV_I64(X), \
            gen_store8, (void)0))
#define MEM_STORE8(VA, DATA, SLOT) \
    MEM_STORE8_FUNC(DATA)(tcg_env, VA, DATA, SLOT)
#else
#define MEM_STORE1(VA, DATA, SLOT) log_store32(env, VA, DATA, 1, SLOT)
#define MEM_STORE2(VA, DATA, SLOT) log_store32(env, VA, DATA, 2, SLOT)
#define MEM_STORE4(VA, DATA, SLOT) log_store32(env, VA, DATA, 4, SLOT)
#define MEM_STORE8(VA, DATA, SLOT) log_store64(env, VA, DATA, 8, SLOT)
#endif

#ifdef QEMU_GENERATE
static inline void gen_cancel(DisasContext *ctx)
{
    Insn *insn = ctx->insn;
    if (!bitmap_empty(ctx->wreg_mult_gprs, NUM_GPREGS) ||
        GET_ATTRIB(insn->opcode, A_STORE)) {
        uint32_t slot = insn->is_endloop ? 4 : insn->slot;
        tcg_gen_ori_tl(hex_slot_cancelled, hex_slot_cancelled, 1 << slot);
    }
}

#define CANCEL gen_cancel(slot);
#else
#define CANCEL cancel_slot(env, slot)
#endif

#define LOAD_CANCEL(EA) do { CANCEL; } while (0)

#define fMAX(A, B) (((A) > (B)) ? (A) : (B))

#define fMIN(A, B) (((A) < (B)) ? (A) : (B))

#define fABS(A) (((A) < 0) ? (-(A)) : (A))
#define fINSERT_BITS(REG, WIDTH, OFFSET, INVAL) \
    REG = ((WIDTH) ? deposit64(REG, (OFFSET), (WIDTH), (INVAL)) : REG)
#define fEXTRACTU_BITS(INREG, WIDTH, OFFSET) \
    ((WIDTH) ? extract64((INREG), (OFFSET), (WIDTH)) : 0LL)
#define fEXTRACTU_BIDIR(INREG, WIDTH, OFFSET) \
    (fZXTN(WIDTH, 32, fBIDIR_LSHIFTR((INREG), (OFFSET), 4_8)))
#define fEXTRACTU_RANGE(INREG, HIBIT, LOWBIT) \
    (((HIBIT) - (LOWBIT) + 1) ? \
        extract64((INREG), (LOWBIT), ((HIBIT) - (LOWBIT) + 1)) : \
        0LL)
#define fINSERT_RANGE(INREG, HIBIT, LOWBIT, INVAL) \
    do { \
        int width = ((HIBIT) - (LOWBIT) + 1); \
        INREG = (width >= 0 ? \
            deposit64((INREG), (LOWBIT), width, (INVAL)) : \
            INREG); \
    } while (0)

#define f8BITSOF(VAL) ((VAL) ? 0xff : 0x00)

#ifdef QEMU_GENERATE
#define fLSBOLD(VAL) tcg_gen_andi_tl(LSB, (VAL), 1)
#else
#define fLSBOLD(VAL)  ((VAL) & 1)
#endif

#ifdef QEMU_GENERATE
#define fLSBNEW(PVAL)   tcg_gen_andi_tl(LSB, (PVAL), 1)
#else
#define fLSBNEW(PVAL)   ((PVAL) & 1)
#endif

#ifdef QEMU_GENERATE
#define fLSBOLDNOT(VAL) \
    do { \
        tcg_gen_andi_tl(LSB, (VAL), 1); \
        tcg_gen_xori_tl(LSB, LSB, 1); \
    } while (0)
#define fLSBNEWNOT(PNUM) \
    do { \
        tcg_gen_andi_tl(LSB, (PNUM), 1); \
        tcg_gen_xori_tl(LSB, LSB, 1); \
    } while (0)
#else
#define fLSBNEWNOT(PNUM) (!fLSBNEW(PNUM))
#define fLSBOLDNOT(VAL) (!fLSBOLD(VAL))
#define fLSBNEW0NOT (!fLSBNEW0)
#define fLSBNEW1NOT (!fLSBNEW1)
#endif

#define fNEWREG(VAL) ((int32_t)(VAL))

#define fNEWREG_ST(VAL) (VAL)

#define fVSATUVALN(N, VAL) \
    ({ \
        (((int64_t)(VAL)) < 0) ? 0 : ((1LL << (N)) - 1); \
    })
#define fSATUVALN(N, VAL) \
    ({ \
        fSET_OVERFLOW(); \
        ((VAL) < 0) ? 0 : ((1LL << (N)) - 1); \
    })
#define fSATVALN(N, VAL) \
    ({ \
        fSET_OVERFLOW(); \
        ((VAL) < 0) ? (-(1LL << ((N) - 1))) : ((1LL << ((N) - 1)) - 1); \
    })
#define fVSATVALN(N, VAL) \
    ({ \
        ((VAL) < 0) ? (-(1LL << ((N) - 1))) : ((1LL << ((N) - 1)) - 1); \
    })
#define fZXTN(N, M, VAL) (((N) != 0) ? extract64((VAL), 0, (N)) : 0LL)
#define fSXTN(N, M, VAL) (((N) != 0) ? sextract64((VAL), 0, (N)) : 0LL)

#define fSATN(N, VAL) \
    ((fSXTN(N, 64, VAL) == (VAL)) ? (VAL) : fSATVALN(N, VAL))
#define fVSATN(N, VAL) \
    ((fSXTN(N, 64, VAL) == (VAL)) ? (VAL) : fVSATVALN(N, VAL))
#define fADDSAT64(DST, A, B) \
    do { \
        uint64_t __a = fCAST8u(A); \
        uint64_t __b = fCAST8u(B); \
        uint64_t __sum = __a + __b; \
        uint64_t __xor = __a ^ __b; \
        const uint64_t __mask = 0x8000000000000000ULL; \
        if (__xor & __mask) { \
            DST = __sum; \
        } \
        else if ((__a ^ __sum) & __mask) { \
            if (__sum & __mask) { \
                DST = 0x7FFFFFFFFFFFFFFFLL; \
                fSET_OVERFLOW(); \
            } else { \
                DST = 0x8000000000000000LL; \
                fSET_OVERFLOW(); \
            } \
        } else { \
            DST = __sum; \
        } \
    } while (0)
#define fVSATUN(N, VAL) \
    ((fZXTN(N, 64, VAL) == (VAL)) ? (VAL) : fVSATUVALN(N, VAL))
#define fSATUN(N, VAL) \
    ((fZXTN(N, 64, VAL) == (VAL)) ? (VAL) : fSATUVALN(N, VAL))
#define fSATH(VAL) (fSATN(16, VAL))
#define fSATUH(VAL) (fSATUN(16, VAL))
#define fVSATH(VAL) (fVSATN(16, VAL))
#define fVSATUH(VAL) (fVSATUN(16, VAL))
#define fSATUB(VAL) (fSATUN(8, VAL))
#define fSATB(VAL) (fSATN(8, VAL))
#define fVSATUB(VAL) (fVSATUN(8, VAL))
#define fVSATB(VAL) (fVSATN(8, VAL))
#define fIMMEXT(IMM) (IMM = IMM)
#define fMUST_IMMEXT(IMM) fIMMEXT(IMM)

#define fPCALIGN(IMM) IMM = (IMM & ~PCALIGN_MASK)

#ifdef QEMU_GENERATE
static inline TCGv gen_read_ireg(TCGv result, TCGv val, int shift)
{
    /*
     * Section 2.2.4 of the Hexagon V67 Programmer's Reference Manual
     *
     *  The "I" value from a modifier register is divided into two pieces
     *      LSB         bits 23:17
     *      MSB         bits 31:28
     * The value is signed
     *
     * At the end we shift the result according to the shift argument
     */
    TCGv msb = tcg_temp_new();
    TCGv lsb = tcg_temp_new();

    tcg_gen_extract_tl(lsb, val, 17, 7);
    tcg_gen_sari_tl(msb, val, 21);
    tcg_gen_deposit_tl(result, msb, lsb, 0, 7);

    tcg_gen_shli_tl(result, result, shift);
    return result;
}
#endif

#define fREAD_LR() (env->gpr[HEX_REG_LR])

#ifdef QEMU_GENERATE
#else
#define fREAD_FP() (env->gpr[HEX_REG_FP])
#endif
#define fREAD_PC() (PC)

#ifdef QEMU_GENERATE
#define fREAD_P0() gen_read_preg(tmp, 0)
#else
#define fREAD_P0() (env->pred[0])
#endif

/*
 * FIXME
 * This is a nop in upstream
 * Should it be defined only in system mode?  (probably not)
 * It's only reference is is in op_helper.c, so the check
 * is not performed for every COF.  We should add the check
 * to COF instructions that have TCG overrides.  See
 * gen_write_new_pc function in genptr.c
 */
#define fCHECK_PCALIGN(A, PC)                                    \
    do {                                                         \
        if (((A) & PCALIGN_MASK) != 0) {                         \
            env->cause_code = HEX_CAUSE_PC_NOT_ALIGNED;          \
            raise_exception(env, HEX_EVENT_PRECISE, PC); \
        }                                                        \
    } while (0)

#define fWRITE_NPC(A) write_new_pc(env, pkt_has_multi_cof != 0, A, PC)

#define MARK_LATE_PRED_WRITE(RNUM) /* Not modelled in qemu */

#define fBRANCH(LOC, TYPE)          fWRITE_NPC(LOC)
#define fJUMPR(REGNO, TARGET, TYPE) fBRANCH(TARGET, COF_TYPE_JUMPR)
#define fHINTJR(TARGET) { /* Not modelled in qemu */}

#define fSET_OVERFLOW() SET_USR_FIELD(USR_OVF, 1)
#define fSET_LPCFG(VAL) SET_USR_FIELD(USR_LPCFG, (VAL))
#define fGET_LPCFG (GET_USR_FIELD(USR_LPCFG))
#define fPART1(WORK) if (part1) { WORK; return; }
#define fCAST4u(A) ((uint32_t)(A))
#define fCAST4s(A) ((int32_t)(A))
#define fCAST8u(A) ((uint64_t)(A))
#define fCAST8s(A) ((int64_t)(A))
#define fCAST2_2s(A) ((int16_t)(A))
#define fCAST2_2u(A) ((uint16_t)(A))
#define fCAST4_4s(A) ((int32_t)(A))
#define fCAST4_4u(A) ((uint32_t)(A))
#define fCAST4_8s(A) ((int64_t)((int32_t)(A)))
#define fCAST4_8u(A) ((uint64_t)((uint32_t)(A)))
#define fCAST8_8s(A) ((int64_t)(A))
#define fCAST8_8u(A) ((uint64_t)(A))
#define fCAST2_8s(A) ((int64_t)((int16_t)(A)))
#define fCAST2_8u(A) ((uint64_t)((uint16_t)(A)))
#define fZE8_16(A) ((int16_t)((uint8_t)(A)))
#define fSE8_16(A) ((int16_t)((int8_t)(A)))
#define fSE16_32(A) ((int32_t)((int16_t)(A)))
#define fZE16_32(A) ((uint32_t)((uint16_t)(A)))
#define fSE32_64(A) ((int64_t)((int32_t)(A)))
#define fZE32_64(A) ((uint64_t)((uint32_t)(A)))
#define fSE8_32(A) ((int32_t)((int8_t)(A)))
#define fZE8_32(A) ((int32_t)((uint8_t)(A)))
#define fMPY8UU(A, B) (int)(fZE8_16(A) * fZE8_16(B))
#define fMPY8US(A, B) (int)(fZE8_16(A) * fSE8_16(B))
#define fMPY8SU(A, B) (int)(fSE8_16(A) * fZE8_16(B))
#define fMPY8SS(A, B) (int)((short)(A) * (short)(B))
#define fMPY16SS(A, B) fSE32_64(fSE16_32(A) * fSE16_32(B))
#define fMPY16UU(A, B) fZE32_64(fZE16_32(A) * fZE16_32(B))
#define fMPY16SU(A, B) fSE32_64(fSE16_32(A) * fZE16_32(B))
#define fMPY16US(A, B) fMPY16SU(B, A)
#define fMPY32SS(A, B) (fSE32_64(A) * fSE32_64(B))
#define fMPY32UU(A, B) (fZE32_64(A) * fZE32_64(B))
#define fMPY32SU(A, B) (fSE32_64(A) * fZE32_64(B))
#define fMPY3216SS(A, B) (fSE32_64(A) * fSXTN(16, 64, B))
#define fMPY3216SU(A, B) (fSE32_64(A) * fZXTN(16, 64, B))
#define fROUND(A) (A + 0x8000)
#define fCLIP(DST, SRC, U) \
    do { \
        int32_t maxv = (1 << U) - 1; \
        int32_t minv = -(1 << U); \
        DST = fMIN(maxv, fMAX(SRC, minv)); \
    } while (0)
#define fCRND(A) ((((A) & 0x3) == 0x3) ? ((A) + 1) : ((A)))
#define fRNDN(A, N) ((((N) == 0) ? (A) : (((fSE32_64(A)) + (1 << ((N) - 1))))))
#define fCRNDN(A, N) (conv_round(A, N))
#define fADD128(A, B) (add128(A, B))
#define fSUB128(A, B) (sub128(A, B))
#define fSHIFTR128(A, B) (shiftr128(A, B))
#define fSHIFTL128(A, B) (shiftl128(A, B))
#define fAND128(A, B) (and128(A, B))
#define fCAST8S_16S(A) (cast8s_to_16s(A))
#define fCAST16S_8S(A) (cast16s_to_8s(A))

#ifdef QEMU_GENERATE
#define fEA_RI(REG, IMM) tcg_gen_addi_tl(EA, REG, IMM)
#define fEA_RRs(REG, REG2, SCALE) \
    do { \
        TCGv tmp = tcg_temp_new(); \
        tcg_gen_shli_tl(tmp, REG2, SCALE); \
        tcg_gen_add_tl(EA, REG, tmp); \
    } while (0)
#define fEA_IRs(IMM, REG, SCALE) \
    do { \
        tcg_gen_shli_tl(EA, REG, SCALE); \
        tcg_gen_addi_tl(EA, EA, IMM); \
    } while (0)
#else
#define fEA_RI(REG, IMM) \
    do { \
        EA = REG + IMM; \
    } while (0)
#define fEA_RRs(REG, REG2, SCALE) \
    do { \
        EA = REG + (REG2 << SCALE); \
    } while (0)
#define fEA_IRs(IMM, REG, SCALE) \
    do { \
        EA = IMM + (REG << SCALE); \
    } while (0)
#endif

#ifdef QEMU_GENERATE
#define fEA_IMM(IMM) tcg_gen_movi_tl(EA, IMM)
#define fEA_REG(REG) tcg_gen_mov_tl(EA, REG)
#define fEA_BREVR(REG)      gen_helper_fbrev(EA, REG)
#define fEA_GPI(IMM) \
    do { \
        if (insn->extension_valid) { \
            tcg_gen_movi_tl(EA, IMM); \
        } else { \
            tcg_gen_addi_tl(EA, hex_gpr[HEX_REG_GP], IMM); \
        } \
    } while (0)
#define fPM_I(REG, IMM)     tcg_gen_addi_tl(REG, REG, IMM)
#define fPM_M(REG, MVAL)    tcg_gen_add_tl(REG, REG, MVAL)
#define fPM_M_BREV(REG, MVAL)    tcg_gen_add_tl(REG, REG, MVAL)
#define fPM_CIRI(REG, IMM, MVAL) \
    do { \
        gen_helper_fcircadd(REG, REG, tcg_constant_tl(siV), MuV, \
                            hex_gpr[HEX_REG_CS0 + MuN]); \
    } while (0)
#else
#define fEA_IMM(IMM)        do { EA = (IMM); } while (0)
#define fEA_REG(REG)        do { EA = (REG); } while (0)
#define fEA_GPI(IMM)        do { EA = (fREAD_GP() + (IMM)); } while (0)
#define fPM_I(REG, IMM)     do { REG = REG + (IMM); } while (0)
#define fPM_M(REG, MVAL)    do { REG = REG + (MVAL); } while (0)
#endif
#define fSCALE(N, A) (((int64_t)(A)) << N)
#define fVSATW(A) fVSATN(32, ((long long)A))
#define fSATW(A) fSATN(32, ((long long)A))
#define fVSAT(A) fVSATN(32, (A))
#define fSAT(A) fSATN(32, (A))
#define fSAT_ORIG_SHL(A, ORIG_REG) \
    ((((int32_t)((fSAT(A)) ^ ((int32_t)(ORIG_REG)))) < 0) \
        ? fSATVALN(32, ((int32_t)(ORIG_REG))) \
        : ((((ORIG_REG) > 0) && ((A) == 0)) ? fSATVALN(32, (ORIG_REG)) \
                                            : fSAT(A)))
#define fPASS(A) A
#define fBIDIR_SHIFTL(SRC, SHAMT, REGSTYPE) \
    (((SHAMT) < 0) ? ((fCAST##REGSTYPE(SRC) >> ((-(SHAMT)) - 1)) >> 1) \
                   : (fCAST##REGSTYPE(SRC) << (SHAMT)))
#define fBIDIR_ASHIFTL(SRC, SHAMT, REGSTYPE) \
    fBIDIR_SHIFTL(SRC, SHAMT, REGSTYPE##s)
#define fBIDIR_LSHIFTL(SRC, SHAMT, REGSTYPE) \
    fBIDIR_SHIFTL(SRC, SHAMT, REGSTYPE##u)
#define fBIDIR_ASHIFTL_SAT(SRC, SHAMT, REGSTYPE) \
    (((SHAMT) < 0) ? ((fCAST##REGSTYPE##s(SRC) >> ((-(SHAMT)) - 1)) >> 1) \
                   : fSAT_ORIG_SHL(fCAST##REGSTYPE##s(SRC) << (SHAMT), (SRC)))
#define fBIDIR_SHIFTR(SRC, SHAMT, REGSTYPE) \
    (((SHAMT) < 0) ? ((fCAST##REGSTYPE(SRC) << ((-(SHAMT)) - 1)) << 1) \
                   : (fCAST##REGSTYPE(SRC) >> (SHAMT)))
#define fBIDIR_ASHIFTR(SRC, SHAMT, REGSTYPE) \
    fBIDIR_SHIFTR(SRC, SHAMT, REGSTYPE##s)
#define fBIDIR_LSHIFTR(SRC, SHAMT, REGSTYPE) \
    fBIDIR_SHIFTR(SRC, SHAMT, REGSTYPE##u)
#define fBIDIR_ASHIFTR_SAT(SRC, SHAMT, REGSTYPE) \
    (((SHAMT) < 0) ? fSAT_ORIG_SHL((fCAST##REGSTYPE##s(SRC) \
                        << ((-(SHAMT)) - 1)) << 1, (SRC)) \
                   : (fCAST##REGSTYPE##s(SRC) >> (SHAMT)))
#ifdef QEMU_GENERATE
#define fASHIFTR(DST, SRC, SHAMT, REGSTYPE) \
    gen_ashiftr_##REGSTYPE##s(DST, SRC, SHAMT)
#define fLSHIFTR(DST, SRC, SHAMT, REGSTYPE) \
    gen_lshiftr_##REGSTYPE##u(DST, SRC, SHAMT)
#else
#define fASHIFTR(SRC, SHAMT, REGSTYPE) (fCAST##REGSTYPE##s(SRC) >> (SHAMT))
#define fLSHIFTR(SRC, SHAMT, REGSTYPE) \
    (((SHAMT) >= (sizeof(SRC) * 8)) ? 0 : (fCAST##REGSTYPE##u(SRC) >> (SHAMT)))
#endif
#define fROTL(SRC, SHAMT, REGSTYPE) \
    (((SHAMT) == 0) ? (SRC) : ((fCAST##REGSTYPE##u(SRC) << (SHAMT)) | \
                              ((fCAST##REGSTYPE##u(SRC) >> \
                                 ((sizeof(SRC) * 8) - (SHAMT))))))
#define fROTR(SRC, SHAMT, REGSTYPE) \
    (((SHAMT) == 0) ? (SRC) : ((fCAST##REGSTYPE##u(SRC) >> (SHAMT)) | \
                              ((fCAST##REGSTYPE##u(SRC) << \
                                 ((sizeof(SRC) * 8) - (SHAMT))))))
#ifdef QEMU_GENERATE
#define fASHIFTL(DST, SRC, SHAMT, REGSTYPE) \
    gen_ashiftl_##REGSTYPE##s(DST, SRC, SHAMT)
#else
#define fASHIFTL(SRC, SHAMT, REGSTYPE) \
    (((SHAMT) >= (sizeof(SRC) * 8)) ? 0 : (fCAST##REGSTYPE##s(SRC) << (SHAMT)))
#endif

#ifdef QEMU_GENERATE
#define fLOAD(NUM, SIZE, SIGN, EA, DST) MEM_LOAD##SIZE##SIGN(DST, EA)
#else
#define MEM_LOAD1 cpu_ldub_data_ra
#define MEM_LOAD2 cpu_lduw_data_ra
#define MEM_LOAD4 cpu_ldl_data_ra
#define MEM_LOAD8 cpu_ldq_data_ra

#define fLOAD(NUM, SIZE, SIGN, EA, DST) \
    do { \
        check_noshuf(env, pkt_has_scalar_store_s1, slot, EA, SIZE, CPU_MEMOP_PC(env)); \
        DST = (size##SIZE##SIGN##_t)MEM_LOAD##SIZE(env, EA, CPU_MEMOP_PC(env)); \
    } while (0)
#endif

#define fMEMOP(NUM, SIZE, SIGN, EA, FNTYPE, VALUE)

#ifdef CONFIG_USER_ONLY
#define fFRAMECHECK(ADDR, EA) do { } while (0) /* Not modelled in linux-user */
#endif

#ifdef QEMU_GENERATE
#define fLOAD_LOCKED(NUM, SIZE, SIGN, EA, DST) \
    gen_load_locked##SIZE##SIGN(DST, EA, ctx->mem_idx);
#endif

#ifdef QEMU_GENERATE
#define fSTORE(NUM, SIZE, EA, SRC) MEM_STORE##SIZE(EA, SRC, insn->slot)
#else
#define fSTORE(NUM, SIZE, EA, SRC) MEM_STORE##SIZE(EA, SRC, slot)
#endif

#ifdef QEMU_GENERATE
#define fSTORE_LOCKED(NUM, SIZE, EA, SRC, PRED) \
    gen_store_conditional##SIZE(ctx, PRED, EA, SRC);
#endif

#ifdef QEMU_GENERATE
#define GETBYTE_FUNC(X) \
    __builtin_choose_expr(TYPE_TCGV(X), \
        gen_get_byte, \
        __builtin_choose_expr(TYPE_TCGV_I64(X), \
            gen_get_byte_i64, (void)0))
#define fGETBYTE(N, SRC) GETBYTE_FUNC(SRC)(BYTE, N, SRC, true)
#define fGETUBYTE(N, SRC) GETBYTE_FUNC(SRC)(BYTE, N, SRC, false)
#else
#define fGETBYTE(N, SRC) ((int8_t)((SRC >> ((N) * 8)) & 0xff))
#define fGETUBYTE(N, SRC) ((uint8_t)((SRC >> ((N) * 8)) & 0xff))
#endif

#define fSETBYTE(N, DST, VAL) \
    do { \
        DST = (DST & ~(0x0ffLL << ((N) * 8))) | \
        (((uint64_t)((VAL) & 0x0ffLL)) << ((N) * 8)); \
    } while (0)

#ifdef QEMU_GENERATE
#define fGETHALF(N, SRC)  gen_get_half(HALF, N, SRC, true)
#define fGETUHALF(N, SRC) gen_get_half(HALF, N, SRC, false)
#else
#define fGETHALF(N, SRC) ((int16_t)((SRC >> ((N) * 16)) & 0xffff))
#define fGETUHALF(N, SRC) ((uint16_t)((SRC >> ((N) * 16)) & 0xffff))
#endif
#define fSETHALF(N, DST, VAL) \
    do { \
        DST = (DST & ~(0x0ffffLL << ((N) * 16))) | \
        (((uint64_t)((VAL) & 0x0ffff)) << ((N) * 16)); \
    } while (0)
#define fSETHALFw fSETHALF
#define fSETHALFd fSETHALF

#define fGETWORD(N, SRC) \
    ((int64_t)((int32_t)((SRC >> ((N) * 32)) & 0x0ffffffffLL)))
#define fGETUWORD(N, SRC) \
    ((uint64_t)((uint32_t)((SRC >> ((N) * 32)) & 0x0ffffffffLL)))

#define fSETWORD(N, DST, VAL) \
    do { \
        DST = (DST & ~(0x0ffffffffLL << ((N) * 32))) | \
              (((VAL) & 0x0ffffffffLL) << ((N) * 32)); \
    } while (0)

#define fVTCM_MEMCPY(DST, SRC, SIZE)
#define fACC()
#define fEXTENSION_AUDIO(A) A

#define fSETBIT(N, DST, VAL) \
    do { \
        DST = (DST & ~(1ULL << (N))) | (((uint64_t)(VAL)) << (N)); \
    } while (0)

#define fGETBIT(N, SRC) (((SRC) >> N) & 1)
#define fSETBITS(HI, LO, DST, VAL) \
    do { \
        int j; \
        for (j = LO; j <= HI; j++) { \
            fSETBIT(j, DST, VAL); \
        } \
    } while (0)
#define fCOUNTONES_2(VAL) ctpop16(VAL)
#define fCOUNTONES_4(VAL) ctpop32(VAL)
#define fCOUNTONES_8(VAL) ctpop64(VAL)
#define fBREV_8(VAL) revbit64(VAL)
#define fBREV_4(VAL) revbit32(VAL)
#define fCL1_8(VAL) clo64(VAL)
#define fCL1_4(VAL) clo32(VAL)
#define fCL1_2(VAL) clo16(VAL)
#define fCL1_1(VAL) clo8(VAL)
#define fINTERLEAVE(ODD, EVEN) interleave(ODD, EVEN)
#define fDEINTERLEAVE(MIXED) deinterleave(MIXED)
#define fHIDE(A) A
#define fCONSTLL(A) A##LL
#define fECHO(A) (A)

#ifdef CONFIG_USER_ONLY
#define fTRAP(TRAPTYPE, IMM) \
    do { \
        raise_exception(env, HEX_EVENT_TRAP0, PC); \
    } while (0)
#endif

#define fDO_TRACE(SREG)
#define fBREAK()

#define fUNPAUSE()

#define fALIGN_REG_FIELD_VALUE(FIELD, VAL) \
    ((VAL) << reg_field_info[FIELD].offset)
#define fGET_REG_FIELD_MASK(FIELD) \
    (((1 << reg_field_info[FIELD].width) - 1) << reg_field_info[FIELD].offset)
#define fREAD_REG_FIELD(REG, FIELD) \
    fEXTRACTU_BITS(env->gpr[HEX_REG_##REG], \
                   reg_field_info[FIELD].width, \
                   reg_field_info[FIELD].offset)

#define fGET_FIELD(VAL, FIELD) \
    fEXTRACTU_BITS(VAL, \
                   reg_field_info[FIELD].width, \
                   reg_field_info[FIELD].offset)
#define fSET_FIELD(VAL, FIELD, NEWVAL) \
    fINSERT_BITS(VAL, \
                 reg_field_info[FIELD].width, \
                 reg_field_info[FIELD].offset, \
                 (NEWVAL))

#ifdef QEMU_GENERATE
#define fDCZEROA(REG) \
    do { \
        ctx->dczero_addr = tcg_temp_new(); \
        tcg_gen_mov_tl(ctx->dczero_addr, (REG)); \
    } while (0)
#endif

#define fBRANCH_SPECULATE_STALL(DOTNEWVAL, JUMP_COND, SPEC_DIR, HINTBITNUM, \
                                STRBITNUM) /* Nothing */

#ifdef CONFIG_USER_ONLY
/*
 * This macro can only be true in guest mode.
 * In user mode, the 4 VIRTINSN's can't be reached
 */
#define fTRAP1_VIRTINSN(IMM)       (false)
#define fVIRTINSN_SPSWAP(IMM, REG) g_assert_not_reached()
#define fVIRTINSN_GETIE(IMM, REG)  g_assert_not_reached()
#define fVIRTINSN_SETIE(IMM, REG)  g_assert_not_reached()
#define fVIRTINSN_RTE(IMM, REG)    g_assert_not_reached()
#endif

#define fPREDUSE_TIMING()

#define fSTORE_DMA(NUM,SIZE,EA,SRC) { mem_dmalink_store(thread, EA, SIZE, SRC, 0); }
#define fDMSTART(NEWPTR) \
    do { \
        dma_adapter_cmd(thread, DMA_CMD_START, NEWPTR, 0); \
        arch_dma_tick_until_stop(thread->processor_ptr, thread->threadId); \
    } while (0)
#define fDMLINK(CURPTR, NEWPTR) \
    do { \
        dma_adapter_cmd(thread, DMA_CMD_LINK, CURPTR, NEWPTR); \
        arch_dma_tick_until_stop(thread->processor_ptr, thread->threadId); \
    } while (0)
#define fDMPOLL(DST) DST=dma_adapter_cmd(thread,DMA_CMD_POLL,0,0)
#define fDMWAIT(DST) DST=dma_adapter_cmd(thread,DMA_CMD_WAIT,0,0)
#define fDMSYNCHT(DST) DST=dma_adapter_cmd(thread,DMA_CMD_SYNCHT,0,0)
#define fDMTLBSYNCH(DST) DST=dma_adapter_cmd(thread,DMA_CMD_TLBSYNCH,0,0)
#define fDMPAUSE(DST) DST=dma_adapter_cmd(thread,DMA_CMD_PAUSE,0,0)
#define fDMRESUME(PTR) dma_adapter_cmd(thread,DMA_CMD_RESUME,PTR,0)
#define fDMWAITDESCRIPTOR(SRC,DST) DST=dma_adapter_cmd(thread,DMA_CMD_WAITDESCRIPTOR,SRC,0)
#define fDMCFGRD(DMANUM,DST) DST=dma_adapter_cmd(thread,DMA_CMD_CFGRD,DMANUM,0)
#define fDMCFGWR(DMANUM,DATA) dma_adapter_cmd(thread,DMA_CMD_CFGWR,DMANUM,DATA)
#define GET_DMA_LDST_ERROR_BADVA(EXTENDED_VA, VA) \
    ((EXTENDED_VA) ? (size4u_t)(((uint64_t)VA >> 32) | (VA & ~0xFFF)) \
                   : (size4u_t)VA)

#define fIN_DEBUG_MODE(TNUM) \
    0    /* FIXME */

#endif
