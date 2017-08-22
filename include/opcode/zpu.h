/* zpu.h.  RISC-V opcode list for GDB, the GNU debugger.
   Copyright (C) 2011-2017 Free Software Foundation, Inc.
   Contributed by Andrew Waterman

   This file is part of GDB, GAS, and the GNU binutils.

   GDB, GAS, and the GNU binutils are free software; you can redistribute
   them and/or modify them under the terms of the GNU General Public
   License as published by the Free Software Foundation; either version
   3, or (at your option) any later version.

   GDB, GAS, and the GNU binutils are distributed in the hope that they
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef _ZPU_H_
#define _ZPU_H_

#include "zpu-opc.h"
#include <stdlib.h>
#include <stdint.h>

typedef uint64_t insn_t;

static inline unsigned int zpu_insn_length (insn_t insn)
{
  return 4;
}

#define RV_X(x, s, n)  (((x) >> (s)) & ((1 << (n)) - 1))

#define ENCODE_ITYPE_IMM(x)			\
  (RV_X(x, 0, 12) << 20)
#define ENCODE_STYPE_IMM(x)				\
  ((RV_X(x, 0, 5) << 7) | (RV_X(x, 5, 7) << 25))

#define ENCODE_SBTYPE_IMM(x)						\
  ((RV_X(x, 1, 4) << 8) | (RV_X(x, 5, 6) << 25) | (RV_X(x, 11, 1) << 7) | (RV_X(x, 12, 1) << 31))

/* BIS1 Register Fields */
#define OP_MASK_R0		0x1f
#define OP_SH_R0			0
#define OP_MASK_R5		0x1f
#define OP_SH_R5			5
#define OP_MASK_R10		0x1f
#define OP_SH_R10			10
#define OP_MASK_R16		0x1f
#define OP_SH_R16			16
#define OP_MASK_R21		0x1f
#define OP_SH_R21			21
#define OP_MASK_IMMS16        0xffff
#define OP_SH_IMMS16            0
#define OP_MASK_IMMU16        0xffff
#define OP_SH_IMMU16            0
#define OP_MASK_IMMS21        0x1fffff
#define OP_SH_IMMS21            0
#define OP_MASK_IMMS26        0x3ffffff
#define OP_SH_IMMS26            0
#define OP_MASK_OP		0x3f
#define OP_SH_OP		26

#define NGPR 32
#define NFPR 32

/* Replace bits MASK << SHIFT of STRUCT with the equivalent bits in
   VALUE << SHIFT.  VALUE is evaluated exactly once.  */
#define INSERT_BITS(STRUCT, VALUE, MASK, SHIFT) \
  (STRUCT) = (((STRUCT) & ~((insn_t)(MASK) << (SHIFT))) \
	      | ((insn_t)((VALUE) & (MASK)) << (SHIFT)))

/* Extract bits MASK << SHIFT from STRUCT and shift them right
   SHIFT places.  */
#define EXTRACT_BITS(STRUCT, MASK, SHIFT) \
  (((STRUCT) >> (SHIFT)) & (MASK))

/* Extract the operand given by FIELD from integer INSN.  */
#define EXTRACT_OPERAND(FIELD, INSN) \
  EXTRACT_BITS ((INSN), OP_MASK_##FIELD, OP_SH_##FIELD)

/* Argument types */
enum zpu_inst_type {
  ZPU_NOP,
  ZPU_ARITH,
  ZPU_ARITHI,
  ZPU_LDST,
  ZPU_MOV,
  ZPU_MOVP,
  ZPU_JMP,
  ZPU_CALL, 
  ZPU_RET
};


/* This structure holds information for a particular instruction.  */

struct zpu_opcode
{
  /* The name of the instruction.  */
  const char *name;
  /* The ISA subset.  Unused for now */
  const char *subset;
  /* Enum describing the instruction type */
  enum zpu_inst_type type;
  /* The basic opcode for the instruction.  When assembling, this
     opcode is modified by the arguments to produce the actual opcode
     that is used.  If pinfo is INSN_MACRO, then this is 0.  */
  insn_t match;
  /* If pinfo is not INSN_MACRO, then this is a bit mask for the
     relevant portions of the opcode when disassembling.  If the
     actual opcode anded with the match field equals the opcode field,
     then we have found the correct instruction.  If pinfo is
     INSN_MACRO, then this field is the macro identifier.  */
  insn_t mask;
  /* A function to determine if a word corresponds to this instruction.
     Usually, this computes ((word & mask) == match).  */
  int (*match_func) (const struct zpu_opcode *op, insn_t word);
  /* For a macro, this is INSN_MACRO.  Otherwise, it is a collection
     of bits describing the instruction, notably any relevant hazard
     information.  */
  unsigned long pinfo;
};

extern const char * const zpu_gpr_names_numeric[NGPR];
extern const struct zpu_opcode zpu_opcodes[];

#endif /* _ZPU_H_ */
