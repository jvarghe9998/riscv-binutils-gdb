/* This file defines the interface between the RISC-V simulator and GDB.

   Copyright (C) 2005-2015 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Order has to match gdb zpu-tdep list.  */
enum sim_zpu_regnum {
  SIM_ZPU_ZERO_REGNUM = 0,
  SIM_ZPU_RA_REGNUM,
  SIM_ZPU_SP_REGNUM,
  SIM_ZPU_GP_REGNUM,
  SIM_ZPU_TP_REGNUM,
  SIM_ZPU_T0_REGNUM,
  SIM_ZPU_T1_REGNUM,
  SIM_ZPU_T2_REGNUM,
  SIM_ZPU_S0_REGNUM,
#define SIM_ZPU_FP_REGNUM SIM_ZPU_S0_REGNUM
  SIM_ZPU_S1_REGNUM,
  SIM_ZPU_A0_REGNUM,
  SIM_ZPU_A1_REGNUM,
  SIM_ZPU_A2_REGNUM,
  SIM_ZPU_A3_REGNUM,
  SIM_ZPU_A4_REGNUM,
  SIM_ZPU_A5_REGNUM,
  SIM_ZPU_A6_REGNUM,
  SIM_ZPU_A7_REGNUM,
  SIM_ZPU_S2_REGNUM,
  SIM_ZPU_S3_REGNUM,
  SIM_ZPU_S4_REGNUM,
  SIM_ZPU_S5_REGNUM,
  SIM_ZPU_S6_REGNUM,
  SIM_ZPU_S7_REGNUM,
  SIM_ZPU_S8_REGNUM,
  SIM_ZPU_S9_REGNUM,
  SIM_ZPU_S10_REGNUM,
  SIM_ZPU_S11_REGNUM,
  SIM_ZPU_T3_REGNUM,
  SIM_ZPU_T4_REGNUM,
  SIM_ZPU_T5_REGNUM,
  SIM_ZPU_T6_REGNUM,
  SIM_ZPU_PC_REGNUM,
  SIM_ZPU_FT0_REGNUM,
#define SIM_ZPU_FIRST_FP_REGNUM SIM_ZPU_FT0_REGNUM
  SIM_ZPU_FT1_REGNUM,
  SIM_ZPU_FT2_REGNUM,
  SIM_ZPU_FT3_REGNUM,
  SIM_ZPU_FT4_REGNUM,
  SIM_ZPU_FT5_REGNUM,
  SIM_ZPU_FT6_REGNUM,
  SIM_ZPU_FT7_REGNUM,
  SIM_ZPU_FS0_REGNUM,
  SIM_ZPU_FS1_REGNUM,
  SIM_ZPU_FA0_REGNUM,
  SIM_ZPU_FA1_REGNUM,
  SIM_ZPU_FA2_REGNUM,
  SIM_ZPU_FA3_REGNUM,
  SIM_ZPU_FA4_REGNUM,
  SIM_ZPU_FA5_REGNUM,
  SIM_ZPU_FA6_REGNUM,
  SIM_ZPU_FA7_REGNUM,
  SIM_ZPU_FS2_REGNUM,
  SIM_ZPU_FS3_REGNUM,
  SIM_ZPU_FS4_REGNUM,
  SIM_ZPU_FS5_REGNUM,
  SIM_ZPU_FS6_REGNUM,
  SIM_ZPU_FS7_REGNUM,
  SIM_ZPU_FS8_REGNUM,
  SIM_ZPU_FS9_REGNUM,
  SIM_ZPU_FS10_REGNUM,
  SIM_ZPU_FS11_REGNUM,
  SIM_ZPU_FT8_REGNUM,
  SIM_ZPU_FT9_REGNUM,
  SIM_ZPU_FT10_REGNUM,
  SIM_ZPU_FT11_REGNUM,
#define SIM_ZPU_LAST_FP_REGNUM SIM_ZPU_FT11_REGNUM

#define SIM_ZPU_FIRST_CSR_REGNUM SIM_ZPU_LAST_FP_REGNUM + 1
#define DECLARE_CSR(name, num) SIM_ZPU_ ## num ## _REGNUM,
#include "opcode/zpu-opc.h"
#undef DECLARE_CSR
#define SIM_ZPU_LAST_CSR_REGNUM SIM_ZPU_LAST_REGNUM - 1

  SIM_ZPU_LAST_REGNUM
};
