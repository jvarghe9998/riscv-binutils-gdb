/* ZPU opcode list
   Copyright (C) 2011-2017 Free Software Foundation, Inc.

   Contributed by Joe Varghese (varghese@zyzyxtech.com)
   Based on RISC-V target.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "opcode/zpu.h"
#include <stdio.h>

/* Register names used by gas and objdump.  */

const char * const zpu_gpr_names_numeric[NGPR] =
{
  "r0",   "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",
  "r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "r15",
  "r16",  "r17",  "r18",  "r19",  "r20",  "r21",  "r22",  "r23",
  "r24",  "r25",  "r26",  "r27",  "r28",  "r29",  "r30",  "r31"
};

static int
match_opcode (const struct zpu_opcode *op, insn_t insn)
{
  return ((unsigned int)((insn ^ op->match) & op->mask) == 0);
}

static int
match_never (const struct zpu_opcode *op ATTRIBUTE_UNUSED,
	     insn_t insn ATTRIBUTE_UNUSED)
{
  return 0;
}

#define OPCODE_MASK (0x3f << 26)

const struct zpu_opcode zpu_opcodes[] =
{
/* name,      isa,   operands, match, mask, match_func, pinfo.  */
{"nop", "B", ZPU_NOP, 0x0, OPCODE_MASK, match_opcode, 0},
{"halt", "B", ZPU_NOP, (0x3f << 26), OPCODE_MASK, match_opcode, 0},

/* Three operand arithmetic instructions */
{"add", "B", ZPU_ARITH, (0x01 << 26), OPCODE_MASK, match_opcode, 0},
{"sub", "B", ZPU_ARITH,  (0x02 << 26), OPCODE_MASK, match_opcode, 0},
{"mul", "B", ZPU_ARITH,  (0x03 << 26), OPCODE_MASK, match_opcode, 0},
{"and", "B", ZPU_ARITH,  (0x04 << 26), OPCODE_MASK, match_opcode, 0},
{"or", "B", ZPU_ARITH,     (0x05 << 26), OPCODE_MASK, match_opcode, 0},
{"xor", "B", ZPU_ARITH,   (0x06 << 26), OPCODE_MASK, match_opcode, 0},
{"shl", "B", ZPU_ARITH,    (0x07 << 26), OPCODE_MASK, match_opcode, 0},
{"shr", "B", ZPU_ARITH,    (0x08 << 26), OPCODE_MASK, match_opcode, 0},

/* Three operand arithmetic with two register and one immediate operands */
{"addi", "B", ZPU_ARITHI,   (0x11 << 26), OPCODE_MASK, match_opcode, 0},
{"subi", "B", ZPU_ARITHI,   (0x12 << 26), OPCODE_MASK, match_opcode, 0},
{"muli", "B", ZPU_ARITHI,   (0x13 << 26), OPCODE_MASK, match_opcode, 0},
{"andi", "B", ZPU_ARITHI,   (0x14 << 26), OPCODE_MASK, match_opcode, 0},
{"ori", "B", ZPU_ARITHI,     (0x15 << 26), OPCODE_MASK, match_opcode, 0},
{"xori", "B", ZPU_ARITHI,    (0x16 << 26), OPCODE_MASK, match_opcode, 0},
{"shli", "B", ZPU_ARITHI,    (0x17 << 26), OPCODE_MASK, match_opcode, 0},
{"shri", "B", ZPU_ARITHI,    (0x18 << 26), OPCODE_MASK, match_opcode, 0},

/* Load Store */
{"ldb", "B", ZPU_LDST,    (0x20 << 26), OPCODE_MASK, match_opcode, 0},
{"ldw", "B", ZPU_LDST,    (0x21 << 26), OPCODE_MASK, match_opcode, 0},
{"ldd", "B", ZPU_LDST,    (0x22 << 26), OPCODE_MASK, match_opcode, 0},
{"ldq", "B", ZPU_LDST,    (0x23 << 26), OPCODE_MASK, match_opcode, 0},
{"stb", "B", ZPU_LDST,    (0x24 << 26), OPCODE_MASK, match_opcode, 0},
{"stw", "B", ZPU_LDST,    (0x25 << 26), OPCODE_MASK, match_opcode, 0},
{"std", "B", ZPU_LDST,    (0x26 << 26), OPCODE_MASK, match_opcode, 0},
{"stq", "B", ZPU_LDST,    (0x27 << 26), OPCODE_MASK, match_opcode, 0},

/* Move */
{"mov", "B", ZPU_MOV,    (0x28 << 26), OPCODE_MASK, match_opcode, 0},

/* Move Partial */
{"movlo", "B", ZPU_MOVP,    (0x29 << 26), OPCODE_MASK, match_opcode, 0},
{"movhi", "B", ZPU_MOVP,    (0x2a << 26), OPCODE_MASK, match_opcode, 0},
{"movsel", "B", ZPU_MOVP,    (0x2b << 26), OPCODE_MASK, match_opcode, 0},

/* Jump */
{"jmp", "B", ZPU_JMP,    (0x30 << 26), OPCODE_MASK, match_opcode, 0},
{"jmpeq", "B", ZPU_JMP,    (0x31 << 26), OPCODE_MASK, match_opcode, 0},
{"jmpneq", "B", ZPU_JMP,    (0x32 << 26), OPCODE_MASK, match_opcode, 0},
{"jmpgt", "B", ZPU_JMP,    (0x33 << 26), OPCODE_MASK, match_opcode, 0},
{"jmplt", "B", ZPU_JMP,    (0x34 << 26), OPCODE_MASK, match_opcode, 0},

/* Call */
{"call", "B", ZPU_CALL,    (0x35 << 26), OPCODE_MASK, match_opcode, 0},

/* Return */
{"ret", "B", ZPU_RET,    (0x36 << 26), OPCODE_MASK, match_opcode, 0},


/* Terminate the list.  */
{0, 0, 0, 0, 0, 0, 0}
};

