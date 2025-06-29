/*
 *  Copyright(c) 2019-2021 Qualcomm Innovation Center, Inc. All Rights Reserved.
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

#ifndef HEXAGON_CPU_PARAM_H
#define HEXAGON_CPU_PARAM_H

//// --- Begin LibAFL code ---

/* Binaries that assume 4k page size were observed.
   Unless TARGET_PAGE_BITS is reduced, Qemu elf loader
   will error out for such binaries. */
//#define TARGET_PAGE_BITS 16     /* 64K pages */
#ifdef CONFIG_USER_ONLY
#define TARGET_PAGE_BITS 16     /* 64K pages */
#else
#define TARGET_PAGE_BITS 12     /* 4K pages */
#endif
//// --- End LibAFL code ---

#define TARGET_LONG_BITS 32

#define TARGET_PHYS_ADDR_SPACE_BITS 36
#define TARGET_VIRT_ADDR_SPACE_BITS 32

#endif
