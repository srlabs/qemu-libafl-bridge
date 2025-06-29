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

#include "internal.h"
#include "helper_protos_generated.h.inc"

DEF_HELPER_FLAGS_3(raise_exception, TCG_CALL_NO_RETURN, noreturn, env, i32, i32)
DEF_HELPER_1(debug_start_packet, void, env)
DEF_HELPER_FLAGS_3(debug_check_store_width, TCG_CALL_NO_WG, void, env, int, int)
DEF_HELPER_FLAGS_5(debug_commit_end, TCG_CALL_NO_WG, void, env, i32, int, int, int)
DEF_HELPER_2(commit_store, void, env, int)
DEF_HELPER_3(gather_store, void, env, i32, int)
DEF_HELPER_1(commit_hvx_stores, void, env)
DEF_HELPER_FLAGS_4(fcircadd, TCG_CALL_NO_RWG_SE, s32, s32, s32, s32, s32)
DEF_HELPER_FLAGS_1(fbrev, TCG_CALL_NO_RWG_SE, i32, i32)
DEF_HELPER_3(sfrecipa, i64, env, f32, f32)
DEF_HELPER_2(sfinvsqrta, i64, env, f32)
DEF_HELPER_5(vacsh_val, s64, env, s64, s64, s64, i32)
DEF_HELPER_FLAGS_4(vacsh_pred, TCG_CALL_NO_RWG_SE, s32, env, s64, s64, s64)
DEF_HELPER_FLAGS_2(cabacdecbin_val, TCG_CALL_NO_RWG_SE, s64, s64, s64)
DEF_HELPER_FLAGS_2(cabacdecbin_pred, TCG_CALL_NO_RWG_SE, s32, s64, s64)

#ifndef CONFIG_USER_ONLY
DEF_HELPER_5(data_cache_op, void, env, i32, int, int, i32)
DEF_HELPER_5(insn_cache_op, void, env, i32, int, int, i32)
DEF_HELPER_2(swi, void, env, i32)
DEF_HELPER_2(cswi, void, env, i32)
DEF_HELPER_2(ciad, void, env, i32)
DEF_HELPER_2(siad, void, env, i32)
DEF_HELPER_2(wait, void, env, i32)
DEF_HELPER_2(resume, void, env, i32)
DEF_HELPER_2(getimask, i32, env, i32)
DEF_HELPER_2(iassignw, void, env, i32)
DEF_HELPER_2(iassignr, i32, env, i32)
DEF_HELPER_3(setimask, void, env, i32, i32)
DEF_HELPER_2(nmi, void, env, i32)
DEF_HELPER_3(setprio, void, env, i32, i32)
DEF_HELPER_2(start, void, env, i32)
DEF_HELPER_1(stop, void, env)
#endif

DEF_HELPER_5(check_vtcm_memcpy, void, env, i32, i32, i32, i32)

/* Floating point */
DEF_HELPER_2(conv_sf2df, f64, env, f32)
DEF_HELPER_2(conv_df2sf, f32, env, f64)
DEF_HELPER_2(conv_uw2sf, f32, env, s32)
DEF_HELPER_2(conv_uw2df, f64, env, s32)
DEF_HELPER_2(conv_w2sf, f32, env, s32)
DEF_HELPER_2(conv_w2df, f64, env, s32)
DEF_HELPER_2(conv_ud2sf, f32, env, s64)
DEF_HELPER_2(conv_ud2df, f64, env, s64)
DEF_HELPER_2(conv_d2sf, f32, env, s64)
DEF_HELPER_2(conv_d2df, f64, env, s64)
DEF_HELPER_2(conv_sf2uw, i32, env, f32)
DEF_HELPER_2(conv_sf2w, s32, env, f32)
DEF_HELPER_2(conv_sf2ud, i64, env, f32)
DEF_HELPER_2(conv_sf2d, s64, env, f32)
DEF_HELPER_2(conv_df2uw, i32, env, f64)
DEF_HELPER_2(conv_df2w, s32, env, f64)
DEF_HELPER_2(conv_df2ud, i64, env, f64)
DEF_HELPER_2(conv_df2d, s64, env, f64)
DEF_HELPER_2(conv_sf2uw_chop, i32, env, f32)
DEF_HELPER_2(conv_sf2w_chop, s32, env, f32)
DEF_HELPER_2(conv_sf2ud_chop, i64, env, f32)
DEF_HELPER_2(conv_sf2d_chop, s64, env, f32)
DEF_HELPER_2(conv_df2uw_chop, i32, env, f64)
DEF_HELPER_2(conv_df2w_chop, s32, env, f64)
DEF_HELPER_2(conv_df2ud_chop, i64, env, f64)
DEF_HELPER_2(conv_df2d_chop, s64, env, f64)
DEF_HELPER_3(sfadd, f32, env, f32, f32)
DEF_HELPER_3(sfsub, f32, env, f32, f32)
DEF_HELPER_3(sfcmpeq, s32, env, f32, f32)
DEF_HELPER_3(sfcmpgt, s32, env, f32, f32)
DEF_HELPER_3(sfcmpge, s32, env, f32, f32)
DEF_HELPER_3(sfcmpuo, s32, env, f32, f32)
DEF_HELPER_3(sfmax, f32, env, f32, f32)
DEF_HELPER_3(sfmin, f32, env, f32, f32)
DEF_HELPER_3(sfclass, s32, env, f32, s32)
DEF_HELPER_3(sffixupn, f32, env, f32, f32)
DEF_HELPER_3(sffixupd, f32, env, f32, f32)
DEF_HELPER_2(sffixupr, f32, env, f32)

DEF_HELPER_3(dfadd, f64, env, f64, f64)
DEF_HELPER_3(dfsub, f64, env, f64, f64)
DEF_HELPER_3(dfmax, f64, env, f64, f64)
DEF_HELPER_3(dfmin, f64, env, f64, f64)
DEF_HELPER_3(dfcmpeq, s32, env, f64, f64)
DEF_HELPER_3(dfcmpgt, s32, env, f64, f64)
DEF_HELPER_3(dfcmpge, s32, env, f64, f64)
DEF_HELPER_3(dfcmpuo, s32, env, f64, f64)
DEF_HELPER_3(dfclass, s32, env, f64, s32)

DEF_HELPER_3(sfmpy, f32, env, f32, f32)
DEF_HELPER_4(sffma, f32, env, f32, f32, f32)
DEF_HELPER_5(sffma_sc, f32, env, f32, f32, f32, f32)
DEF_HELPER_4(sffms, f32, env, f32, f32, f32)
DEF_HELPER_4(sffma_lib, f32, env, f32, f32, f32)
DEF_HELPER_4(sffms_lib, f32, env, f32, f32, f32)

DEF_HELPER_3(dfmpyfix, f64, env, f64, f64)
DEF_HELPER_4(dfmpyhh, f64, env, f64, f64, f64)

DEF_HELPER_2(creg_read, i32, env, i32)
DEF_HELPER_2(creg_read_pair, i64, env, i32)
DEF_HELPER_2(debug_value, void, env, s32)
DEF_HELPER_2(debug_value_i64, void, env, s64)
DEF_HELPER_3(debug_print_vec, void, env, s32, ptr)

/* Histogram instructions */
DEF_HELPER_1(vhist, void, env)
DEF_HELPER_1(vhistq, void, env)
DEF_HELPER_1(vwhist256, void, env)
DEF_HELPER_1(vwhist256q, void, env)
DEF_HELPER_1(vwhist256_sat, void, env)
DEF_HELPER_1(vwhist256q_sat, void, env)
DEF_HELPER_1(vwhist128, void, env)
DEF_HELPER_1(vwhist128q, void, env)
DEF_HELPER_2(vwhist128m, void, env, s32)
DEF_HELPER_2(vwhist128qm, void, env, s32)

DEF_HELPER_4(probe_noshuf_load, void, env, i32, int, int)
DEF_HELPER_2(probe_pkt_scalar_store_s0, void, env, int)
DEF_HELPER_2(probe_hvx_stores, void, env, int)
DEF_HELPER_2(probe_pkt_scalar_hvx_stores, void, env, int)
DEF_HELPER_2(assert_store_valid, void, env, int)

DEF_HELPER_1(read_pcyclelo, i32, env)
DEF_HELPER_1(read_pcyclehi, i32, env)

#if !defined(CONFIG_USER_ONLY)
DEF_HELPER_3(cpu_limit, void, env, i32, i32)
DEF_HELPER_2(greg_read, i32, env, i32)
DEF_HELPER_2(greg_read_pair, i64, env, i32)
DEF_HELPER_1(inc_gcycle_xt, void, env)
DEF_HELPER_3(modify_ssr, void, env, i32, i32)
DEF_HELPER_1(pending_interrupt, void, env)
DEF_HELPER_3(raise_stack_overflow, void, env, i32, i32)
DEF_HELPER_1(resched, void, env)
DEF_HELPER_2(sreg_read, i32, env, i32)
DEF_HELPER_2(sreg_read_pair, i64, env, i32)
DEF_HELPER_3(sreg_write, void, env, i32, i32)
DEF_HELPER_3(sreg_write_pair, void, env, i32, i64)
DEF_HELPER_3(check_ccr_write, void, env, i32, i32)
#endif
