/* RISC-V disassembler
   Copyright (C) 2011-2017 Free Software Foundation, Inc.

   Contributed by Andrew Waterman (andrew@sifive.com).
   Based on MIPS target.

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
#include "dis-asm.h"
#include "libiberty.h"
#include "opcode/zpu.h"
#include "opintl.h"
#include "elf-bfd.h"
#include "elf/zpu.h"

#include <stdint.h>
#include <ctype.h>

struct zpu_private_data
{
  bfd_vma gp;
  bfd_vma print_addr;
  bfd_vma hi_addr[OP_MASK_RD + 1];
};

static const char * const *zpu_gpr_names;
static const char * const *zpu_fpr_names;

/* Other options.  */
static int no_aliases;	/* If set disassemble as most general inst.  */

static void
set_default_zpu_dis_options (void)
{
#if JOEV
  zpu_gpr_names = zpu_gpr_names_abi;
  zpu_fpr_names = zpu_fpr_names_abi;
#endif /* JOEV */

  no_aliases = 0;
}

static void
parse_zpu_dis_option (const char *option)
{
  if (strcmp (option, "no-aliases") == 0)
    no_aliases = 1;
  else if (strcmp (option, "numeric") == 0)
    {
      zpu_gpr_names = zpu_gpr_names_numeric;
      zpu_fpr_names = zpu_fpr_names_numeric;
    }
  else
    {
      /* Invalid option.  */
      fprintf (stderr, _("Unrecognized disassembler option: %s\n"), option);
    }
}

static void
parse_zpu_dis_options (const char *opts_in)
{
  char *opts = xstrdup (opts_in), *opt = opts, *opt_end = opts;

  set_default_zpu_dis_options ();

  for ( ; opt_end != NULL; opt = opt_end + 1)
    {
      if ((opt_end = strchr (opt, ',')) != NULL)
	*opt_end = 0;
      parse_zpu_dis_option (opt);
    }

  free (opts);
}

/* Print one argument from an array.  */

static void
arg_print (struct disassemble_info *info, unsigned long val,
	   const char* const* array, size_t size)
{
  const char *s = val >= size || array[val] == NULL ? "unknown" : array[val];
  (*info->fprintf_func) (info->stream, "%s", s);
}

static void
maybe_print_address (struct zpu_private_data *pd, int base_reg, int offset)
{
#if JOEV
  if (pd->hi_addr[base_reg] != (bfd_vma)-1)
    {
      pd->print_addr = pd->hi_addr[base_reg] + offset;
      pd->hi_addr[base_reg] = -1;
    }
  else if (base_reg == X_GP && pd->gp != (bfd_vma)-1)
    pd->print_addr = pd->gp + offset;
  else if (base_reg == X_TP || base_reg == 0)
    pd->print_addr = offset;
#endif /* JOEV */
}

static void 
extract_immediate(insn_t inst, unsigned int offset_size, unsigned int signed_p, int* extracted_sign, unsigned int* extracted_offset)
{
  unsigned int offset_mask = signed_p? ((1 << (offset_size - 1)) - 1): ((1 << offset_size) - 1);
  unsigned int sign_mask = (1 << (offset_size - 1));

  *extracted_sign = signed_p?(inst & sign_mask):0;
  *extracted_offset = inst & offset_mask;
  if (*extracted_sign && !(*extracted_offset)) {
    *extracted_offset = offset_mask + 1;
  } else if (*extracted_sign) {
    *extracted_offset = ((~(*extracted_offset)) & offset_mask) + 1;
  }
}

/* Print insn arguments for 32/64-bit code.  */

static void
print_insn_args (enum zpu_inst_type type, insn_t l, bfd_vma pc, disassemble_info *info)
{
  fprintf_ftype print = info->fprintf_func;
  unsigned int offset_sign, offset;
  switch (type) {
  case ZPU_NOP:
    break;
  case ZPU_ARITH:
    {
      print (info->stream, "\t%s,", zpu_gpr_names_numeric[EXTRACT_OPERAND(R10, l)]);
      print (info->stream, " %s,", zpu_gpr_names_numeric[EXTRACT_OPERAND(R5, l)]);
      print (info->stream, " %s", zpu_gpr_names_numeric[EXTRACT_OPERAND(R0, l)]);
    }
    break;
  case ZPU_ARITHI:
      print (info->stream, "\t%s,", zpu_gpr_names_numeric[EXTRACT_OPERAND(R21, l)]);
      print (info->stream, " %s,", zpu_gpr_names_numeric[EXTRACT_OPERAND(R16, l)]);
      extract_immediate(l, 16, 1, &offset_sign, &offset);
      print(info->stream, " %s%d", offset_sign?"-":"", offset);
    break;
  case ZPU_LDST:
      print (info->stream, "\t%s,", zpu_gpr_names_numeric[EXTRACT_OPERAND(R21, l)]);
      print (info->stream, " %s,", zpu_gpr_names_numeric[EXTRACT_OPERAND(R16, l)]);
      extract_immediate(l, 16, 1, &offset_sign, &offset);
      print(info->stream, " %s%d", offset_sign?"-":"", offset);
    break;
  case ZPU_MOV:
      print (info->stream, "\t%s,", zpu_gpr_names_numeric[EXTRACT_OPERAND(R21, l)]);
      print (info->stream, " %s", zpu_gpr_names_numeric[EXTRACT_OPERAND(R16, l)]);
    break;
  case ZPU_MOVP:
      print (info->stream, "\t%s,", zpu_gpr_names_numeric[EXTRACT_OPERAND(R21, l)]);
      extract_immediate(l, 16, 0, &offset_sign, &offset);
      print(info->stream, " %d", offset);
    break;
  case ZPU_JMP:
    extract_immediate(l, 26, 1, &offset_sign, &offset);
    print(info->stream, "\t%s%d", offset_sign?"-":"", offset);
    break;
  case ZPU_CALL :
      print (info->stream, "\t%s,", zpu_gpr_names_numeric[EXTRACT_OPERAND(R21, l)]);
      extract_immediate(l, 21, 1, &offset_sign, &offset);
    print(info->stream, " %s%d", offset_sign?"-":"", offset);
    break;
  case ZPU_RET: 
      print (info->stream, "\t%s", zpu_gpr_names_numeric[EXTRACT_OPERAND(R21, l)]);
    break;
  default: 
    break;
  }


#if JOEV
  struct zpu_private_data *pd = info->private_data;
  int rs1 = (l >> OP_SH_RS1) & OP_MASK_RS1;
  int rd = (l >> OP_SH_RD) & OP_MASK_RD;
  fprintf_ftype print = info->fprintf_func;

  if (*d != '\0')
    print (info->stream, "\t");

  for (; *d != '\0'; d++)
    {
      switch (*d)
	{
	case 'C': /* RVC */
	  switch (*++d)
	    {
	    case 's': /* RS1 x8-x15 */
	    case 'w': /* RS1 x8-x15 */
	      print (info->stream, "%s",
		     zpu_gpr_names[EXTRACT_OPERAND (CRS1S, l) + 8]);
	      break;
	    case 't': /* RS2 x8-x15 */
	    case 'x': /* RS2 x8-x15 */
	      print (info->stream, "%s",
		     zpu_gpr_names[EXTRACT_OPERAND (CRS2S, l) + 8]);
	      break;
	    case 'U': /* RS1, constrained to equal RD */
	      print (info->stream, "%s", zpu_gpr_names[rd]);
	      break;
	    case 'c': /* RS1, constrained to equal sp */
	      print (info->stream, "%s", zpu_gpr_names[X_SP]);
	      break;
	    case 'V': /* RS2 */
	      print (info->stream, "%s",
		     zpu_gpr_names[EXTRACT_OPERAND (CRS2, l)]);
	      break;
	    case 'i':
	      print (info->stream, "%d", (int)EXTRACT_RVC_SIMM3 (l));
	      break;
	    case 'o':
	    case 'j':
	      print (info->stream, "%d", (int)EXTRACT_RVC_IMM (l));
	      break;
	    case 'k':
	      print (info->stream, "%d", (int)EXTRACT_RVC_LW_IMM (l));
	      break;
	    case 'l':
	      print (info->stream, "%d", (int)EXTRACT_RVC_LD_IMM (l));
	      break;
	    case 'm':
	      print (info->stream, "%d", (int)EXTRACT_RVC_LWSP_IMM (l));
	      break;
	    case 'n':
	      print (info->stream, "%d", (int)EXTRACT_RVC_LDSP_IMM (l));
	      break;
	    case 'K':
	      print (info->stream, "%d", (int)EXTRACT_RVC_ADDI4SPN_IMM (l));
	      break;
	    case 'L':
	      print (info->stream, "%d", (int)EXTRACT_RVC_ADDI16SP_IMM (l));
	      break;
	    case 'M':
	      print (info->stream, "%d", (int)EXTRACT_RVC_SWSP_IMM (l));
	      break;
	    case 'N':
	      print (info->stream, "%d", (int)EXTRACT_RVC_SDSP_IMM (l));
	      break;
	    case 'p':
	      info->target = EXTRACT_RVC_B_IMM (l) + pc;
	      (*info->print_address_func) (info->target, info);
	      break;
	    case 'a':
	      info->target = EXTRACT_RVC_J_IMM (l) + pc;
	      (*info->print_address_func) (info->target, info);
	      break;
	    case 'u':
	      print (info->stream, "0x%x",
		     (int)(EXTRACT_RVC_IMM (l) & (ZPU_BIGIMM_REACH-1)));
	      break;
	    case '>':
	      print (info->stream, "0x%x", (int)EXTRACT_RVC_IMM (l) & 0x3f);
	      break;
	    case '<':
	      print (info->stream, "0x%x", (int)EXTRACT_RVC_IMM (l) & 0x1f);
	      break;
	    case 'T': /* floating-point RS2 */
	      print (info->stream, "%s",
		     zpu_fpr_names[EXTRACT_OPERAND (CRS2, l)]);
	      break;
	    case 'D': /* floating-point RS2 x8-x15 */
	      print (info->stream, "%s",
		     zpu_fpr_names[EXTRACT_OPERAND (CRS2S, l) + 8]);
	      break;
	    }
	  break;

	case ',':
	case '(':
	case ')':
	case '[':
	case ']':
	  print (info->stream, "%c", *d);
	  break;

	case '0':
	  /* Only print constant 0 if it is the last argument */
	  if (!d[1])
	    print (info->stream, "0");
	  break;

	case 'b':
	case 's':
	  print (info->stream, "%s", zpu_gpr_names[rs1]);
	  break;

	case 't':
	  print (info->stream, "%s",
		 zpu_gpr_names[EXTRACT_OPERAND (RS2, l)]);
	  break;

	case 'u':
	  print (info->stream, "0x%x",
		 (unsigned)EXTRACT_UTYPE_IMM (l) >> ZPU_IMM_BITS);
	  break;

	case 'm':
	  arg_print (info, EXTRACT_OPERAND (RM, l),
		     zpu_rm, ARRAY_SIZE (zpu_rm));
	  break;

	case 'P':
	  arg_print (info, EXTRACT_OPERAND (PRED, l),
		     zpu_pred_succ, ARRAY_SIZE (zpu_pred_succ));
	  break;

	case 'Q':
	  arg_print (info, EXTRACT_OPERAND (SUCC, l),
		     zpu_pred_succ, ARRAY_SIZE (zpu_pred_succ));
	  break;

	case 'o':
	  maybe_print_address (pd, rs1, EXTRACT_ITYPE_IMM (l));
	  /* Fall through.  */
	case 'j':
	  if (((l & MASK_ADDI) == MATCH_ADDI && rs1 != 0)
	      || (l & MASK_JALR) == MATCH_JALR)
	    maybe_print_address (pd, rs1, EXTRACT_ITYPE_IMM (l));
	  print (info->stream, "%d", (int)EXTRACT_ITYPE_IMM (l));
	  break;

	case 'q':
	  maybe_print_address (pd, rs1, EXTRACT_STYPE_IMM (l));
	  print (info->stream, "%d", (int)EXTRACT_STYPE_IMM (l));
	  break;

	case 'a':
	  info->target = EXTRACT_UJTYPE_IMM (l) + pc;
	  (*info->print_address_func) (info->target, info);
	  break;

	case 'p':
	  info->target = EXTRACT_SBTYPE_IMM (l) + pc;
	  (*info->print_address_func) (info->target, info);
	  break;

	case 'd':
	  if ((l & MASK_AUIPC) == MATCH_AUIPC)
	    pd->hi_addr[rd] = pc + EXTRACT_UTYPE_IMM (l);
	  else if ((l & MASK_LUI) == MATCH_LUI)
	    pd->hi_addr[rd] = EXTRACT_UTYPE_IMM (l);
	  else if ((l & MASK_C_LUI) == MATCH_C_LUI)
	    pd->hi_addr[rd] = EXTRACT_RVC_LUI_IMM (l);
	  print (info->stream, "%s", zpu_gpr_names[rd]);
	  break;

	case 'z':
	  print (info->stream, "%s", zpu_gpr_names[0]);
	  break;

	case '>':
	  print (info->stream, "0x%x", (int)EXTRACT_OPERAND (SHAMT, l));
	  break;

	case '<':
	  print (info->stream, "0x%x", (int)EXTRACT_OPERAND (SHAMTW, l));
	  break;

	case 'S':
	case 'U':
	  print (info->stream, "%s", zpu_fpr_names[rs1]);
	  break;

	case 'T':
	  print (info->stream, "%s", zpu_fpr_names[EXTRACT_OPERAND (RS2, l)]);
	  break;

	case 'D':
	  print (info->stream, "%s", zpu_fpr_names[rd]);
	  break;

	case 'R':
	  print (info->stream, "%s", zpu_fpr_names[EXTRACT_OPERAND (RS3, l)]);
	  break;

	case 'E':
	  {
	    const char* csr_name = NULL;
	    unsigned int csr = EXTRACT_OPERAND (CSR, l);
	    switch (csr)
	      {
#define DECLARE_CSR(name, num) case num: csr_name = #name; break;
#include "opcode/zpu-opc.h"
#undef DECLARE_CSR
	      }
	    if (csr_name)
	      print (info->stream, "%s", csr_name);
	    else
	      print (info->stream, "0x%x", csr);
	    break;
	  }

	case 'Z':
	  print (info->stream, "%d", rs1);
	  break;

	default:
	  /* xgettext:c-format */
	  print (info->stream, _("# internal error, undefined modifier (%c)"),
		 *d);
	  return;
	}
    }
#endif /* JOEV */
}

/* Print the RISC-V instruction at address MEMADDR in debugged memory,
   on using INFO.  Returns length of the instruction, in bytes.
   BIGENDIAN must be 1 if this is big-endian code, 0 if
   this is little-endian code.  */

static int
zpu_disassemble_insn (bfd_vma memaddr, insn_t word, disassemble_info *info)
{
  const struct zpu_opcode *op;
  static bfd_boolean init = 0;
  static const struct zpu_opcode *zpu_hash[OP_MASK_OP + 1];
  struct zpu_private_data *pd;
  int insnlen;

#define OP_HASH_IDX(i) ((((i) & (OP_MASK_OP << OP_SH_OP)) >> OP_SH_OP) & OP_MASK_OP)

  /* Build a hash table to shorten the search time.  */
  if (! init)
    {
      for (op = zpu_opcodes; op->name; op++)
	if (!zpu_hash[OP_HASH_IDX (op->match)]) {
	  zpu_hash[OP_HASH_IDX (op->match)] = op;
	}

      init = 1;
    }

  if (info->private_data == NULL)
    {
      int i;

      pd = info->private_data = xcalloc (1, sizeof (struct zpu_private_data));
      pd->gp = -1;
      pd->print_addr = -1;
      for (i = 0; i < (int)ARRAY_SIZE (pd->hi_addr); i++)
	pd->hi_addr[i] = -1;

      for (i = 0; i < info->symtab_size; i++)
	if (strcmp (bfd_asymbol_name (info->symtab[i]), ZPU_GP_SYMBOL) == 0)
	  pd->gp = bfd_asymbol_value (info->symtab[i]);
    }
  else
    pd = info->private_data;

  insnlen = zpu_insn_length (word);

  info->bytes_per_chunk = insnlen % 4 == 0 ? 4 : 2;
  info->bytes_per_line = 8;
  info->display_endian = info->endian;
  info->insn_info_valid = 1;
  info->branch_delay_insns = 0;
  info->data_size = 0;
  info->insn_type = dis_nonbranch;
  info->target = 0;
  info->target2 = 0;

  op = zpu_hash[OP_HASH_IDX (word)];
  if (op != NULL)
    {
      int xlen = 0;

      /* If XLEN is not known, get its value from the ELF class.  */
      if (info->mach == bfd_mach_zpu)
	xlen = 32;
      if (info->mach == bfd_mach_zpu2)
	xlen = 32;
      else if (info->section != NULL)
	{
	  Elf_Internal_Ehdr *ehdr = elf_elfheader (info->section->owner);
	  xlen = ehdr->e_ident[EI_CLASS] == ELFCLASS64 ? 64 : 32;
	}

      for (; op->name; op++)
	{
	  /* Does the opcode match?  */
	  if (! (op->match_func) (op, word))
	    continue;
#if 0
	  /* Is this a pseudo-instruction and may we print it as such?  */
	  if (no_aliases && (op->pinfo & INSN_ALIAS))
	    continue;
	  /* Is this instruction restricted to a certain value of XLEN?  */
	  if (isdigit (op->subset[0]) && atoi (op->subset) != xlen)
	    continue;
#endif
	  /* It's a match.  */
	  (*info->fprintf_func) (info->stream, "%s", op->name);
	  print_insn_args (op->type, word, memaddr, info);

	  /* Try to disassemble multi-instruction addressing sequences.  */
	  if (pd->print_addr != (bfd_vma)-1)
	    {
	      info->target = pd->print_addr;
	      (*info->fprintf_func) (info->stream, " # ");
	      (*info->print_address_func) (info->target, info);
	      pd->print_addr = -1;
	    }

	  return insnlen;
	}
    }

  /* We did not find a match, so just print the instruction bits.  */
  info->insn_type = dis_noninsn;
  (*info->fprintf_func) (info->stream, "0x%llx", (unsigned long long)word);
  return insnlen;
}

int
print_insn_zpu (bfd_vma memaddr, struct disassemble_info *info)
{
  bfd_byte packet[2];
  insn_t insn = 0;
  bfd_vma n;
  int status;

  if (info->disassembler_options != NULL)
    {
      parse_zpu_dis_options (info->disassembler_options);
      /* Avoid repeatedly parsing the options.  */
      info->disassembler_options = NULL;
    }
  else if (zpu_gpr_names == NULL)
    set_default_zpu_dis_options ();

  /* Instructions are a sequence of 2-byte packets in little-endian order.  */
  for (n = 0; n < sizeof (insn) && n < zpu_insn_length (insn); n += 2)
    {
      status = (*info->read_memory_func) (memaddr + n, packet, 2, info);
      if (status != 0)
	{
	  /* Don't fail just because we fell off the end.  */
	  if (n > 0)
	    break;
	  (*info->memory_error_func) (status, memaddr, info);
	  return status;
	}

      insn |= ((insn_t) bfd_getl16 (packet)) << (8 * n);
    }

  return zpu_disassemble_insn (memaddr, insn, info);
}

void
print_zpu_disassembler_options (FILE *stream)
{
  fprintf (stream, _("\n\
The following RISC-V-specific disassembler options are supported for use\n\
with the -M switch (multiple options should be separated by commas):\n"));

  fprintf (stream, _("\n\
  numeric       Print numeric reigster names, rather than ABI names.\n"));

  fprintf (stream, _("\n\
  no-aliases    Disassemble only into canonical instructions, rather\n\
                than into pseudoinstructions.\n"));

  fprintf (stream, _("\n"));
}
