/* X86-64 specific support for ELF
   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
   2010  Free Software Foundation, Inc.
   Contributed by Jan Hubicka <jh@suse.cz>.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "bfd_stdint.h"
#include "objalloc.h"
#include "hashtab.h"

#include "elf/x86-64.h"

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value.  */
#define MINUS_ONE (~ (bfd_vma) 0)

/* Since both 32-bit and 64-bit x86-64 encode relocation type in the
   identical manner, we use ELF32_R_TYPE instead of ELF64_R_TYPE to get
   relocation type.  We also use ELF_ST_TYPE instead of ELF64_ST_TYPE
   since they are the same.  */

#define ABI_64_P(abfd) \
  (get_elf_backend_data (abfd)->s->elfclass == ELFCLASS64)

/* The relocation "howto" table.  Order of fields:
   type, rightshift, size, bitsize, pc_relative, bitpos, complain_on_overflow,
   special_function, name, partial_inplace, src_mask, dst_mask, pcrel_offset.  */
static reloc_howto_type x86_64_elf_howto_table[] =
{
  HOWTO(R_X86_64_NONE, 0, 0, 0, FALSE, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_X86_64_NONE",	FALSE, 0x00000000, 0x00000000,
	FALSE),
  HOWTO(R_X86_64_64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_64", FALSE, MINUS_ONE, MINUS_ONE,
	FALSE),
  HOWTO(R_X86_64_PC32, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_PC32", FALSE, 0xffffffff, 0xffffffff,
	TRUE),
  HOWTO(R_X86_64_GOT32, 0, 2, 32, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOT32", FALSE, 0xffffffff, 0xffffffff,
	FALSE),
  HOWTO(R_X86_64_PLT32, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_PLT32", FALSE, 0xffffffff, 0xffffffff,
	TRUE),
  HOWTO(R_X86_64_COPY, 0, 2, 32, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_COPY", FALSE, 0xffffffff, 0xffffffff,
	FALSE),
  HOWTO(R_X86_64_GLOB_DAT, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_GLOB_DAT", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_JUMP_SLOT, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_JUMP_SLOT", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_RELATIVE, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_RELATIVE", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_GOTPCREL, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPCREL", FALSE, 0xffffffff,
	0xffffffff, TRUE),
  HOWTO(R_X86_64_32, 0, 2, 32, FALSE, 0, complain_overflow_unsigned,
	bfd_elf_generic_reloc, "R_X86_64_32", FALSE, 0xffffffff, 0xffffffff,
	FALSE),
  HOWTO(R_X86_64_32S, 0, 2, 32, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_32S", FALSE, 0xffffffff, 0xffffffff,
	FALSE),
  HOWTO(R_X86_64_16, 0, 1, 16, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_16", FALSE, 0xffff, 0xffff, FALSE),
  HOWTO(R_X86_64_PC16,0, 1, 16, TRUE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_PC16", FALSE, 0xffff, 0xffff, TRUE),
  HOWTO(R_X86_64_8, 0, 0, 8, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_8", FALSE, 0xff, 0xff, FALSE),
  HOWTO(R_X86_64_PC8, 0, 0, 8, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_PC8", FALSE, 0xff, 0xff, TRUE),
  HOWTO(R_X86_64_DTPMOD64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_DTPMOD64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_DTPOFF64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_DTPOFF64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_TPOFF64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_TPOFF64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_TLSGD, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_TLSGD", FALSE, 0xffffffff,
	0xffffffff, TRUE),
  HOWTO(R_X86_64_TLSLD, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_TLSLD", FALSE, 0xffffffff,
	0xffffffff, TRUE),
  HOWTO(R_X86_64_DTPOFF32, 0, 2, 32, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_DTPOFF32", FALSE, 0xffffffff,
	0xffffffff, FALSE),
  HOWTO(R_X86_64_GOTTPOFF, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTTPOFF", FALSE, 0xffffffff,
	0xffffffff, TRUE),
  HOWTO(R_X86_64_TPOFF32, 0, 2, 32, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_TPOFF32", FALSE, 0xffffffff,
	0xffffffff, FALSE),
  HOWTO(R_X86_64_PC64, 0, 4, 64, TRUE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_PC64", FALSE, MINUS_ONE, MINUS_ONE,
	TRUE),
  HOWTO(R_X86_64_GOTOFF64, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_GOTOFF64",
	FALSE, MINUS_ONE, MINUS_ONE, FALSE),
  HOWTO(R_X86_64_GOTPC32, 0, 2, 32, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPC32",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  HOWTO(R_X86_64_GOT64, 0, 4, 64, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOT64", FALSE, MINUS_ONE, MINUS_ONE,
	FALSE),
  HOWTO(R_X86_64_GOTPCREL64, 0, 4, 64, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPCREL64", FALSE, MINUS_ONE,
	MINUS_ONE, TRUE),
  HOWTO(R_X86_64_GOTPC64, 0, 4, 64, TRUE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPC64",
	FALSE, MINUS_ONE, MINUS_ONE, TRUE),
  HOWTO(R_X86_64_GOTPLT64, 0, 4, 64, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_GOTPLT64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  HOWTO(R_X86_64_PLTOFF64, 0, 4, 64, FALSE, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_X86_64_PLTOFF64", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),
  EMPTY_HOWTO (32),
  EMPTY_HOWTO (33),
  HOWTO(R_X86_64_GOTPC32_TLSDESC, 0, 2, 32, TRUE, 0,
	complain_overflow_bitfield, bfd_elf_generic_reloc,
	"R_X86_64_GOTPC32_TLSDESC",
	FALSE, 0xffffffff, 0xffffffff, TRUE),
  HOWTO(R_X86_64_TLSDESC_CALL, 0, 0, 0, FALSE, 0,
	complain_overflow_dont, bfd_elf_generic_reloc,
	"R_X86_64_TLSDESC_CALL",
	FALSE, 0, 0, FALSE),
  HOWTO(R_X86_64_TLSDESC, 0, 4, 64, FALSE, 0,
	complain_overflow_bitfield, bfd_elf_generic_reloc,
	"R_X86_64_TLSDESC",
	FALSE, MINUS_ONE, MINUS_ONE, FALSE),
  HOWTO(R_X86_64_IRELATIVE, 0, 4, 64, FALSE, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_X86_64_IRELATIVE", FALSE, MINUS_ONE,
	MINUS_ONE, FALSE),

  /* We have a gap in the reloc numbers here.
     R_X86_64_standard counts the number up to this point, and
     R_X86_64_vt_offset is the value to subtract from a reloc type of
     R_X86_64_GNU_VT* to form an index into this table.  */
#define R_X86_64_standard (R_X86_64_IRELATIVE + 1)
#define R_X86_64_vt_offset (R_X86_64_GNU_VTINHERIT - R_X86_64_standard)

/* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_X86_64_GNU_VTINHERIT, 0, 4, 0, FALSE, 0, complain_overflow_dont,
	 NULL, "R_X86_64_GNU_VTINHERIT", FALSE, 0, 0, FALSE),

/* GNU extension to record C++ vtable member usage.  */
  HOWTO (R_X86_64_GNU_VTENTRY, 0, 4, 0, FALSE, 0, complain_overflow_dont,
	 _bfd_elf_rel_vtable_reloc_fn, "R_X86_64_GNU_VTENTRY", FALSE, 0, 0,
	 FALSE)
};

#define IS_X86_64_PCREL_TYPE(TYPE)	\
  (   ((TYPE) == R_X86_64_PC8)		\
   || ((TYPE) == R_X86_64_PC16)		\
   || ((TYPE) == R_X86_64_PC32)		\
   || ((TYPE) == R_X86_64_PC64))

/* Map BFD relocs to the x86_64 elf relocs.  */
struct elf_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct elf_reloc_map x86_64_reloc_map[] =
{
  { BFD_RELOC_NONE,		R_X86_64_NONE, },
  { BFD_RELOC_64,		R_X86_64_64,   },
  { BFD_RELOC_32_PCREL,		R_X86_64_PC32, },
  { BFD_RELOC_X86_64_GOT32,	R_X86_64_GOT32,},
  { BFD_RELOC_X86_64_PLT32,	R_X86_64_PLT32,},
  { BFD_RELOC_X86_64_COPY,	R_X86_64_COPY, },
  { BFD_RELOC_X86_64_GLOB_DAT,	R_X86_64_GLOB_DAT, },
  { BFD_RELOC_X86_64_JUMP_SLOT, R_X86_64_JUMP_SLOT, },
  { BFD_RELOC_X86_64_RELATIVE,	R_X86_64_RELATIVE, },
  { BFD_RELOC_X86_64_GOTPCREL,	R_X86_64_GOTPCREL, },
  { BFD_RELOC_32,		R_X86_64_32, },
  { BFD_RELOC_X86_64_32S,	R_X86_64_32S, },
  { BFD_RELOC_16,		R_X86_64_16, },
  { BFD_RELOC_16_PCREL,		R_X86_64_PC16, },
  { BFD_RELOC_8,		R_X86_64_8, },
  { BFD_RELOC_8_PCREL,		R_X86_64_PC8, },
  { BFD_RELOC_X86_64_DTPMOD64,	R_X86_64_DTPMOD64, },
  { BFD_RELOC_X86_64_DTPOFF64,	R_X86_64_DTPOFF64, },
  { BFD_RELOC_X86_64_TPOFF64,	R_X86_64_TPOFF64, },
  { BFD_RELOC_X86_64_TLSGD,	R_X86_64_TLSGD, },
  { BFD_RELOC_X86_64_TLSLD,	R_X86_64_TLSLD, },
  { BFD_RELOC_X86_64_DTPOFF32,	R_X86_64_DTPOFF32, },
  { BFD_RELOC_X86_64_GOTTPOFF,	R_X86_64_GOTTPOFF, },
  { BFD_RELOC_X86_64_TPOFF32,	R_X86_64_TPOFF32, },
  { BFD_RELOC_64_PCREL,		R_X86_64_PC64, },
  { BFD_RELOC_X86_64_GOTOFF64,	R_X86_64_GOTOFF64, },
  { BFD_RELOC_X86_64_GOTPC32,	R_X86_64_GOTPC32, },
  { BFD_RELOC_X86_64_GOT64,	R_X86_64_GOT64, },
  { BFD_RELOC_X86_64_GOTPCREL64,R_X86_64_GOTPCREL64, },
  { BFD_RELOC_X86_64_GOTPC64,	R_X86_64_GOTPC64, },
  { BFD_RELOC_X86_64_GOTPLT64,	R_X86_64_GOTPLT64, },
  { BFD_RELOC_X86_64_PLTOFF64,	R_X86_64_PLTOFF64, },
  { BFD_RELOC_X86_64_GOTPC32_TLSDESC, R_X86_64_GOTPC32_TLSDESC, },
  { BFD_RELOC_X86_64_TLSDESC_CALL, R_X86_64_TLSDESC_CALL, },
  { BFD_RELOC_X86_64_TLSDESC,	R_X86_64_TLSDESC, },
  { BFD_RELOC_X86_64_IRELATIVE,	R_X86_64_IRELATIVE, },
  { BFD_RELOC_VTABLE_INHERIT,	R_X86_64_GNU_VTINHERIT, },
  { BFD_RELOC_VTABLE_ENTRY,	R_X86_64_GNU_VTENTRY, },
};

static reloc_howto_type *
elf_x86_64_rtype_to_howto (bfd *abfd, unsigned r_type)
{
  unsigned i;

  if (r_type < (unsigned int) R_X86_64_GNU_VTINHERIT
      || r_type >= (unsigned int) R_X86_64_max)
    {
      if (r_type >= (unsigned int) R_X86_64_standard)
	{
	  (*_bfd_error_handler) (_("%B: invalid relocation type %d"),
				 abfd, (int) r_type);
	  r_type = R_X86_64_NONE;
	}
      i = r_type;
    }
  else
    i = r_type - (unsigned int) R_X86_64_vt_offset;
  BFD_ASSERT (x86_64_elf_howto_table[i].type == r_type);
  return &x86_64_elf_howto_table[i];
}

/* Given a BFD reloc type, return a HOWTO structure.  */
static reloc_howto_type *
elf_x86_64_reloc_type_lookup (bfd *abfd,
			      bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < sizeof (x86_64_reloc_map) / sizeof (struct elf_reloc_map);
       i++)
    {
      if (x86_64_reloc_map[i].bfd_reloc_val == code)
	return elf_x86_64_rtype_to_howto (abfd,
					  x86_64_reloc_map[i].elf_reloc_val);
    }
  return 0;
}

static reloc_howto_type *
elf_x86_64_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			      const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (x86_64_elf_howto_table)
	    / sizeof (x86_64_elf_howto_table[0]));
       i++)
    if (x86_64_elf_howto_table[i].name != NULL
	&& strcasecmp (x86_64_elf_howto_table[i].name, r_name) == 0)
      return &x86_64_elf_howto_table[i];

  return NULL;
}

/* Given an x86_64 ELF reloc type, fill in an arelent structure.  */

static void
elf_x86_64_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED, arelent *cache_ptr,
			  Elf_Internal_Rela *dst)
{
  unsigned r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  cache_ptr->howto = elf_x86_64_rtype_to_howto (abfd, r_type);
  BFD_ASSERT (r_type == cache_ptr->howto->type);
}

/* Support for core dump NOTE sections.  */
static bfd_boolean
elf_x86_64_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  switch (note->descsz)
    {
      default:
	return FALSE;

      case 336:		/* sizeof(istruct elf_prstatus) on Linux/x86_64 */
	/* pr_cursig */
	elf_tdata (abfd)->core_signal
	  = bfd_get_16 (abfd, note->descdata + 12);

	/* pr_pid */
	elf_tdata (abfd)->core_lwpid
	  = bfd_get_32 (abfd, note->descdata + 32);

	/* pr_reg */
	offset = 112;
	size = 216;

	break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bfd_boolean
elf_x86_64_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
      default:
	return FALSE;

      case 136:		/* sizeof(struct elf_prpsinfo) on Linux/x86_64 */
	elf_tdata (abfd)->core_pid
	  = bfd_get_32 (abfd, note->descdata + 24);
	elf_tdata (abfd)->core_program
	 = _bfd_elfcore_strndup (abfd, note->descdata + 40, 16);
	elf_tdata (abfd)->core_command
	 = _bfd_elfcore_strndup (abfd, note->descdata + 56, 80);
    }

  /* Note that for some reason, a spurious space is tacked
     onto the end of the args in some (at least one anyway)
     implementations, so strip it off if it exists.  */

  {
    char *command = elf_tdata (abfd)->core_command;
    int n = strlen (command);

    if (0 < n && command[n - 1] == ' ')
      command[n - 1] = '\0';
  }

  return TRUE;
}

/* Functions for the x86-64 ELF linker.	 */

/* The name of the dynamic interpreter.	 This is put in the .interp
   section.  */

#define ELF64_DYNAMIC_INTERPRETER "/lib/ld64.so.1"
#define ELF32_DYNAMIC_INTERPRETER "/lib/ld32.so.1"

/* If ELIMINATE_COPY_RELOCS is non-zero, the linker will try to avoid
   copying dynamic variables from a shared lib into an app's dynbss
   section, and instead use a dynamic relocation to point into the
   shared lib.  */
#define ELIMINATE_COPY_RELOCS 1

/* The size in bytes of an entry in the global offset table.  */

#define GOT_ENTRY_SIZE 8

/* The size in bytes of an entry in the procedure linkage table.  */

#define PLT_ENTRY_SIZE 16

/* The first entry in a procedure linkage table looks like this.  See the
   SVR4 ABI i386 supplement and the x86-64 ABI to see how this works.  */

static const bfd_byte elf_x86_64_plt0_entry[PLT_ENTRY_SIZE] =
{
  0xff, 0x35, 8, 0, 0, 0,	/* pushq GOT+8(%rip)  */
  0xff, 0x25, 16, 0, 0, 0,	/* jmpq *GOT+16(%rip) */
  0x0f, 0x1f, 0x40, 0x00	/* nopl 0(%rax)       */
};

/* Subsequent entries in a procedure linkage table look like this.  */

static const bfd_byte elf_x86_64_plt_entry[PLT_ENTRY_SIZE] =
{
  0xff, 0x25,	/* jmpq *name@GOTPC(%rip) */
  0, 0, 0, 0,	/* replaced with offset to this symbol in .got.	 */
  0x68,		/* pushq immediate */
  0, 0, 0, 0,	/* replaced with index into relocation table.  */
  0xe9,		/* jmp relative */
  0, 0, 0, 0	/* replaced with offset to start of .plt0.  */
};

/* x86-64 ELF linker hash entry.  */

struct elf_x86_64_link_hash_entry
{
  struct elf_link_hash_entry elf;

  /* Track dynamic relocs copied for this symbol.  */
  struct elf_dyn_relocs *dyn_relocs;

#define GOT_UNKNOWN	0
#define GOT_NORMAL	1
#define GOT_TLS_GD	2
#define GOT_TLS_IE	3
#define GOT_TLS_GDESC	4
#define GOT_TLS_GD_BOTH_P(type) \
  ((type) == (GOT_TLS_GD | GOT_TLS_GDESC))
#define GOT_TLS_GD_P(type) \
  ((type) == GOT_TLS_GD || GOT_TLS_GD_BOTH_P (type))
#define GOT_TLS_GDESC_P(type) \
  ((type) == GOT_TLS_GDESC || GOT_TLS_GD_BOTH_P (type))
#define GOT_TLS_GD_ANY_P(type) \
  (GOT_TLS_GD_P (type) || GOT_TLS_GDESC_P (type))
  unsigned char tls_type;

  /* Offset of the GOTPLT entry reserved for the TLS descriptor,
     starting at the end of the jump table.  */
  bfd_vma tlsdesc_got;
};

#define elf_x86_64_hash_entry(ent) \
  ((struct elf_x86_64_link_hash_entry *)(ent))

struct elf_x86_64_obj_tdata
{
  struct elf_obj_tdata root;

  /* tls_type for each local got entry.  */
  char *local_got_tls_type;

  /* GOTPLT entries for TLS descriptors.  */
  bfd_vma *local_tlsdesc_gotent;
};

#define elf_x86_64_tdata(abfd) \
  ((struct elf_x86_64_obj_tdata *) (abfd)->tdata.any)

#define elf_x86_64_local_got_tls_type(abfd) \
  (elf_x86_64_tdata (abfd)->local_got_tls_type)

#define elf_x86_64_local_tlsdesc_gotent(abfd) \
  (elf_x86_64_tdata (abfd)->local_tlsdesc_gotent)

#define is_x86_64_elf(bfd)				\
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour	\
   && elf_tdata (bfd) != NULL				\
   && elf_object_id (bfd) == X86_64_ELF_DATA)

static bfd_boolean
elf_x86_64_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_x86_64_obj_tdata),
				  X86_64_ELF_DATA);
}

/* x86-64 ELF linker hash table.  */

struct elf_x86_64_link_hash_table
{
  struct elf_link_hash_table elf;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *sdynbss;
  asection *srelbss;

  union
  {
    bfd_signed_vma refcount;
    bfd_vma offset;
  } tls_ld_got;

  /* The amount of space used by the jump slots in the GOT.  */
  bfd_vma sgotplt_jump_table_size;

  /* Small local sym cache.  */
  struct sym_cache sym_cache;

  bfd_vma (*r_info) (bfd_vma, bfd_vma);
  bfd_vma (*r_sym) (bfd_vma);
  unsigned int pointer_r_type;
  const char *dynamic_interpreter;
  int dynamic_interpreter_size;

  /* _TLS_MODULE_BASE_ symbol.  */
  struct bfd_link_hash_entry *tls_module_base;

  /* Used by local STT_GNU_IFUNC symbols.  */
  htab_t loc_hash_table;
  void * loc_hash_memory;

  /* The offset into splt of the PLT entry for the TLS descriptor
     resolver.  Special values are 0, if not necessary (or not found
     to be necessary yet), and -1 if needed but not determined
     yet.  */
  bfd_vma tlsdesc_plt;
  /* The offset into sgot of the GOT entry used by the PLT entry
     above.  */
  bfd_vma tlsdesc_got;
};

/* Get the x86-64 ELF linker hash table from a link_info structure.  */

#define elf_x86_64_hash_table(p) \
  (elf_hash_table_id ((struct elf_link_hash_table *) ((p)->hash)) \
  == X86_64_ELF_DATA ? ((struct elf_x86_64_link_hash_table *) ((p)->hash)) : NULL)

#define elf_x86_64_compute_jump_table_size(htab) \
  ((htab)->elf.srelplt->reloc_count * GOT_ENTRY_SIZE)

/* Create an entry in an x86-64 ELF linker hash table.	*/

static struct bfd_hash_entry *
elf_x86_64_link_hash_newfunc (struct bfd_hash_entry *entry,
			      struct bfd_hash_table *table,
			      const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = (struct bfd_hash_entry *)
          bfd_hash_allocate (table,
                             sizeof (struct elf_x86_64_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf_x86_64_link_hash_entry *eh;

      eh = (struct elf_x86_64_link_hash_entry *) entry;
      eh->dyn_relocs = NULL;
      eh->tls_type = GOT_UNKNOWN;
      eh->tlsdesc_got = (bfd_vma) -1;
    }

  return entry;
}

/* Compute a hash of a local hash entry.  We use elf_link_hash_entry
  for local symbol so that we can handle local STT_GNU_IFUNC symbols
  as global symbol.  We reuse indx and dynstr_index for local symbol
  hash since they aren't used by global symbols in this backend.  */

static hashval_t
elf_x86_64_local_htab_hash (const void *ptr)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) ptr;
  return ELF_LOCAL_SYMBOL_HASH (h->indx, h->dynstr_index);
}

/* Compare local hash entries.  */

static int
elf_x86_64_local_htab_eq (const void *ptr1, const void *ptr2)
{
  struct elf_link_hash_entry *h1
     = (struct elf_link_hash_entry *) ptr1;
  struct elf_link_hash_entry *h2
    = (struct elf_link_hash_entry *) ptr2;

  return h1->indx == h2->indx && h1->dynstr_index == h2->dynstr_index;
}

/* Find and/or create a hash entry for local symbol.  */

static struct elf_link_hash_entry *
elf_x86_64_get_local_sym_hash (struct elf_x86_64_link_hash_table *htab,
			       bfd *abfd, const Elf_Internal_Rela *rel,
			       bfd_boolean create)
{
  struct elf_x86_64_link_hash_entry e, *ret;
  asection *sec = abfd->sections;
  hashval_t h = ELF_LOCAL_SYMBOL_HASH (sec->id,
				       htab->r_sym (rel->r_info));
  void **slot;

  e.elf.indx = sec->id;
  e.elf.dynstr_index = htab->r_sym (rel->r_info);
  slot = htab_find_slot_with_hash (htab->loc_hash_table, &e, h,
				   create ? INSERT : NO_INSERT);

  if (!slot)
    return NULL;

  if (*slot)
    {
      ret = (struct elf_x86_64_link_hash_entry *) *slot;
      return &ret->elf;
    }

  ret = (struct elf_x86_64_link_hash_entry *)
	objalloc_alloc ((struct objalloc *) htab->loc_hash_memory,
			sizeof (struct elf_x86_64_link_hash_entry));
  if (ret)
    {
      memset (ret, 0, sizeof (*ret));
      ret->elf.indx = sec->id;
      ret->elf.dynstr_index = htab->r_sym (rel->r_info);
      ret->elf.dynindx = -1;
      *slot = ret;
    }
  return &ret->elf;
}

/* Create an X86-64 ELF linker hash table.  */

static struct bfd_link_hash_table *
elf_x86_64_link_hash_table_create (bfd *abfd)
{
  struct elf_x86_64_link_hash_table *ret;
  bfd_size_type amt = sizeof (struct elf_x86_64_link_hash_table);

  ret = (struct elf_x86_64_link_hash_table *) bfd_malloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd,
				      elf_x86_64_link_hash_newfunc,
				      sizeof (struct elf_x86_64_link_hash_entry),
				      X86_64_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->sdynbss = NULL;
  ret->srelbss = NULL;
  ret->sym_cache.abfd = NULL;
  ret->tlsdesc_plt = 0;
  ret->tlsdesc_got = 0;
  ret->tls_ld_got.refcount = 0;
  ret->sgotplt_jump_table_size = 0;
  ret->tls_module_base = NULL;

  if (ABI_64_P (abfd))
    {
      ret->r_info = elf64_r_info;
      ret->r_sym = elf64_r_sym;
      ret->pointer_r_type = R_X86_64_64;
      ret->dynamic_interpreter = ELF64_DYNAMIC_INTERPRETER;
      ret->dynamic_interpreter_size = sizeof ELF64_DYNAMIC_INTERPRETER;
    }
  else
    {
      ret->r_info = elf32_r_info;
      ret->r_sym = elf32_r_sym;
      ret->pointer_r_type = R_X86_64_32;
      ret->dynamic_interpreter = ELF32_DYNAMIC_INTERPRETER;
      ret->dynamic_interpreter_size = sizeof ELF32_DYNAMIC_INTERPRETER;
    }

  ret->loc_hash_table = htab_try_create (1024,
					 elf_x86_64_local_htab_hash,
					 elf_x86_64_local_htab_eq,
					 NULL);
  ret->loc_hash_memory = objalloc_create ();
  if (!ret->loc_hash_table || !ret->loc_hash_memory)
    {
      free (ret);
      return NULL;
    }

  return &ret->elf.root;
}

/* Destroy an X86-64 ELF linker hash table.  */

static void
elf_x86_64_link_hash_table_free (struct bfd_link_hash_table *hash)
{
  struct elf_x86_64_link_hash_table *htab
    = (struct elf_x86_64_link_hash_table *) hash;

  if (htab->loc_hash_table)
    htab_delete (htab->loc_hash_table);
  if (htab->loc_hash_memory)
    objalloc_free ((struct objalloc *) htab->loc_hash_memory);
  _bfd_generic_link_hash_table_free (hash);
}

/* Create .plt, .rela.plt, .got, .got.plt, .rela.got, .dynbss, and
   .rela.bss sections in DYNOBJ, and set up shortcuts to them in our
   hash table.  */

static bfd_boolean
elf_x86_64_create_dynamic_sections (bfd *dynobj,
				    struct bfd_link_info *info)
{
  struct elf_x86_64_link_hash_table *htab;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return FALSE;

  htab = elf_x86_64_hash_table (info);
  if (htab == NULL)
    return FALSE;

  htab->sdynbss = bfd_get_section_by_name (dynobj, ".dynbss");
  if (!info->shared)
    htab->srelbss = bfd_get_section_by_name (dynobj, ".rela.bss");

  if (!htab->sdynbss
      || (!info->shared && !htab->srelbss))
    abort ();

  return TRUE;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
elf_x86_64_copy_indirect_symbol (struct bfd_link_info *info,
				 struct elf_link_hash_entry *dir,
				 struct elf_link_hash_entry *ind)
{
  struct elf_x86_64_link_hash_entry *edir, *eind;

  edir = (struct elf_x86_64_link_hash_entry *) dir;
  eind = (struct elf_x86_64_link_hash_entry *) ind;

  if (eind->dyn_relocs != NULL)
    {
      if (edir->dyn_relocs != NULL)
	{
	  struct elf_dyn_relocs **pp;
	  struct elf_dyn_relocs *p;

	  /* Add reloc counts against the indirect sym to the direct sym
	     list.  Merge any entries against the same section.  */
	  for (pp = &eind->dyn_relocs; (p = *pp) != NULL; )
	    {
	      struct elf_dyn_relocs *q;

	      for (q = edir->dyn_relocs; q != NULL; q = q->next)
		if (q->sec == p->sec)
		  {
		    q->pc_count += p->pc_count;
		    q->count += p->count;
		    *pp = p->next;
		    break;
		  }
	      if (q == NULL)
		pp = &p->next;
	    }
	  *pp = edir->dyn_relocs;
	}

      edir->dyn_relocs = eind->dyn_relocs;
      eind->dyn_relocs = NULL;
    }

  if (ind->root.type == bfd_link_hash_indirect
      && dir->got.refcount <= 0)
    {
      edir->tls_type = eind->tls_type;
      eind->tls_type = GOT_UNKNOWN;
    }

  if (ELIMINATE_COPY_RELOCS
      && ind->root.type != bfd_link_hash_indirect
      && dir->dynamic_adjusted)
    {
      /* If called to transfer flags for a weakdef during processing
	 of elf_adjust_dynamic_symbol, don't copy non_got_ref.
	 We clear it ourselves for ELIMINATE_COPY_RELOCS.  */
      dir->ref_dynamic |= ind->ref_dynamic;
      dir->ref_regular |= ind->ref_regular;
      dir->ref_regular_nonweak |= ind->ref_regular_nonweak;
      dir->needs_plt |= ind->needs_plt;
      dir->pointer_equality_needed |= ind->pointer_equality_needed;
    }
  else
    _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

static bfd_boolean
elf64_x86_64_elf_object_p (bfd *abfd)
{
  /* Set the right machine number for an x86-64 elf64 file.  */
  bfd_default_set_arch_mach (abfd, bfd_arch_i386, bfd_mach_x86_64);
  return TRUE;
}

typedef union
  {
    unsigned char c[2];
    uint16_t i;
  }
x86_64_opcode16;

typedef union
  {
    unsigned char c[4];
    uint32_t i;
  }
x86_64_opcode32;

/* Return TRUE if the TLS access code sequence support transition
   from R_TYPE.  */

static bfd_boolean
elf_x86_64_check_tls_transition (bfd *abfd,
				 struct bfd_link_info *info,
				 asection *sec,
				 bfd_byte *contents,
				 Elf_Internal_Shdr *symtab_hdr,
				 struct elf_link_hash_entry **sym_hashes,
				 unsigned int r_type,
				 const Elf_Internal_Rela *rel,
				 const Elf_Internal_Rela *relend)
{
  unsigned int val;
  unsigned long r_symndx;
  struct elf_link_hash_entry *h;
  bfd_vma offset;
  struct elf_x86_64_link_hash_table *htab;

  /* Get the section contents.  */
  if (contents == NULL)
    {
      if (elf_section_data (sec)->this_hdr.contents != NULL)
	contents = elf_section_data (sec)->this_hdr.contents;
      else
	{
	  /* FIXME: How to better handle error condition?  */
	  if (!bfd_malloc_and_get_section (abfd, sec, &contents))
	    return FALSE;

	  /* Cache the section contents for elf_link_input_bfd.  */
	  elf_section_data (sec)->this_hdr.contents = contents;
	}
    }

  htab = elf_x86_64_hash_table (info);
  offset = rel->r_offset;
  switch (r_type)
    {
    case R_X86_64_TLSGD:
    case R_X86_64_TLSLD:
      if ((rel + 1) >= relend)
	return FALSE;

      if (r_type == R_X86_64_TLSGD)
	{
	  /* Check transition from GD access model.  Only
		.byte 0x66; leaq foo@tlsgd(%rip), %rdi
		.word 0x6666; rex64; call __tls_get_addr
	     can transit to different access model.  */

	  static x86_64_opcode32 leaq = { { 0x66, 0x48, 0x8d, 0x3d } },
				 call = { { 0x66, 0x66, 0x48, 0xe8 } };
	  if (offset < 4
	      || (offset + 12) > sec->size
	      || bfd_get_32 (abfd, contents + offset - 4) != leaq.i
	      || bfd_get_32 (abfd, contents + offset + 4) != call.i)
	    return FALSE;
	}
      else
	{
	  /* Check transition from LD access model.  Only
		leaq foo@tlsld(%rip), %rdi;
		call __tls_get_addr
	     can transit to different access model.  */

	  static x86_64_opcode32 ld = { { 0x48, 0x8d, 0x3d, 0xe8 } };
	  x86_64_opcode32 op;

	  if (offset < 3 || (offset + 9) > sec->size)
	    return FALSE;

	  op.i = bfd_get_32 (abfd, contents + offset - 3);
	  op.c[3] = bfd_get_8 (abfd, contents + offset + 4);
	  if (op.i != ld.i)
	    return FALSE;
	}

      r_symndx = htab->r_sym (rel[1].r_info);
      if (r_symndx < symtab_hdr->sh_info)
	return FALSE;

      h = sym_hashes[r_symndx - symtab_hdr->sh_info];
      /* Use strncmp to check __tls_get_addr since __tls_get_addr
	 may be versioned.  */ 
      return (h != NULL
	      && h->root.root.string != NULL
	      && (ELF32_R_TYPE (rel[1].r_info) == R_X86_64_PC32
		  || ELF32_R_TYPE (rel[1].r_info) == R_X86_64_PLT32)
	      && (strncmp (h->root.root.string,
			   "__tls_get_addr", 14) == 0));

    case R_X86_64_GOTTPOFF:
      /* Check transition from IE access model:
		movq foo@gottpoff(%rip), %reg
		addq foo@gottpoff(%rip), %reg
       */

      if (offset < 3 || (offset + 4) > sec->size)
	return FALSE;

      val = bfd_get_8 (abfd, contents + offset - 3);
      if (val != 0x48 && val != 0x4c)
	return FALSE;

      val = bfd_get_8 (abfd, contents + offset - 2);
      if (val != 0x8b && val != 0x03)
	return FALSE;

      val = bfd_get_8 (abfd, contents + offset - 1);
      return (val & 0xc7) == 5;

    case R_X86_64_GOTPC32_TLSDESC:
      /* Check transition from GDesc access model:
		leaq x@tlsdesc(%rip), %rax

	 Make sure it's a leaq adding rip to a 32-bit offset
	 into any register, although it's probably almost always
	 going to be rax.  */

      if (offset < 3 || (offset + 4) > sec->size)
	return FALSE;

      val = bfd_get_8 (abfd, contents + offset - 3);
      if ((val & 0xfb) != 0x48)
	return FALSE;

      if (bfd_get_8 (abfd, contents + offset - 2) != 0x8d)
	return FALSE;

      val = bfd_get_8 (abfd, contents + offset - 1);
      return (val & 0xc7) == 0x05;

    case R_X86_64_TLSDESC_CALL:
      /* Check transition from GDesc access model:
		call *x@tlsdesc(%rax)
       */
      if (offset + 2 <= sec->size)
	{
	  /* Make sure that it's a call *x@tlsdesc(%rax).  */
	  static x86_64_opcode16 call = { { 0xff, 0x10 } };
	  return bfd_get_16 (abfd, contents + offset) == call.i;
	}

      return FALSE;

    default:
      abort ();
    }
}

/* Return TRUE if the TLS access transition is OK or no transition
   will be performed.  Update R_TYPE if there is a transition.  */

static bfd_boolean
elf_x86_64_tls_transition (struct bfd_link_info *info, bfd *abfd,
			   asection *sec, bfd_byte *contents,
			   Elf_Internal_Shdr *symtab_hdr,
			   struct elf_link_hash_entry **sym_hashes,
			   unsigned int *r_type, int tls_type,
			   const Elf_Internal_Rela *rel,
			   const Elf_Internal_Rela *relend,
			   struct elf_link_hash_entry *h,
			   unsigned long r_symndx)
{
  unsigned int from_type = *r_type;
  unsigned int to_type = from_type;
  bfd_boolean check = TRUE;

  /* Skip TLS transition for functions.  */
  if (h != NULL
      && (h->type == STT_FUNC
	  || h->type == STT_GNU_IFUNC))
    return TRUE;

  switch (from_type)
    {
    case R_X86_64_TLSGD:
    case R_X86_64_GOTPC32_TLSDESC:
    case R_X86_64_TLSDESC_CALL:
    case R_X86_64_GOTTPOFF:
      if (info->executable)
	{
	  if (h == NULL)
	    to_type = R_X86_64_TPOFF32;
	  else
	    to_type = R_X86_64_GOTTPOFF;
	}

      /* When we are called from elf_x86_64_relocate_section,
	 CONTENTS isn't NULL and there may be additional transitions
	 based on TLS_TYPE.  */
      if (contents != NULL)
	{
	  unsigned int new_to_type = to_type;

	  if (info->executable
	      && h != NULL
	      && h->dynindx == -1
	      && tls_type == GOT_TLS_IE)
	    new_to_type = R_X86_64_TPOFF32;

	  if (to_type == R_X86_64_TLSGD
	      || to_type == R_X86_64_GOTPC32_TLSDESC
	      || to_type == R_X86_64_TLSDESC_CALL)
	    {
	      if (tls_type == GOT_TLS_IE)
		new_to_type = R_X86_64_GOTTPOFF;
	    }

	  /* We checked the transition before when we were called from
	     elf_x86_64_check_relocs.  We only want to check the new
	     transition which hasn't been checked before.  */
	  check = new_to_type != to_type && from_type == to_type;
	  to_type = new_to_type;
	}

      break;

    case R_X86_64_TLSLD:
      if (info->executable)
	to_type = R_X86_64_TPOFF32;
      break;

    default:
      return TRUE;
    }

  /* Return TRUE if there is no transition.  */
  if (from_type == to_type)
    return TRUE;

  /* Check if the transition can be performed.  */
  if (check
      && ! elf_x86_64_check_tls_transition (abfd, info, sec, contents,
					    symtab_hdr, sym_hashes,
					    from_type, rel, relend))
    {
      reloc_howto_type *from, *to;
      const char *name;

      from = elf_x86_64_rtype_to_howto (abfd, from_type);
      to = elf_x86_64_rtype_to_howto (abfd, to_type);

      if (h)
	name = h->root.root.string;
      else
	{
	  struct elf_x86_64_link_hash_table *htab;

	  htab = elf_x86_64_hash_table (info);
	  if (htab == NULL)
	    name = "*unknown*";
	  else
	    {
	      Elf_Internal_Sym *isym;

	      isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					    abfd, r_symndx);
	      name = bfd_elf_sym_name (abfd, symtab_hdr, isym, NULL);
	    }
	}

      (*_bfd_error_handler)
	(_("%B: TLS transition from %s to %s against `%s' at 0x%lx "
	   "in section `%A' failed"),
	 abfd, sec, from->name, to->name, name,
	 (unsigned long) rel->r_offset);
      bfd_set_error (bfd_error_bad_value);
      return FALSE;
    }

  *r_type = to_type;
  return TRUE;
}

/* Look through the relocs for a section during the first phase, and
   calculate needed space in the global offset table, procedure
   linkage table, and dynamic reloc sections.  */

static bfd_boolean
elf_x86_64_check_relocs (bfd *abfd, struct bfd_link_info *info,
			 asection *sec,
			 const Elf_Internal_Rela *relocs)
{
  struct elf_x86_64_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;

  if (info->relocatable)
    return TRUE;

  BFD_ASSERT (is_x86_64_elf (abfd));

  htab = elf_x86_64_hash_table (info);
  if (htab == NULL)
    return FALSE;

  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);

  sreloc = NULL;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned int r_type;
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *isym;
      const char *name;

      r_symndx = htab->r_sym (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_symndx >= NUM_SHDR_ENTRIES (symtab_hdr))
	{
	  (*_bfd_error_handler) (_("%B: bad symbol index: %d"),
				 abfd, r_symndx);
	  return FALSE;
	}

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					abfd, r_symndx);
	  if (isym == NULL)
	    return FALSE;

	  /* Check relocation against local STT_GNU_IFUNC symbol.  */
	  if (ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
	    {
	      h = elf_x86_64_get_local_sym_hash (htab, abfd, rel,
						 TRUE);
	      if (h == NULL)
		return FALSE;

	      /* Fake a STT_GNU_IFUNC symbol.  */
	      h->type = STT_GNU_IFUNC;
	      h->def_regular = 1;
	      h->ref_regular = 1;
	      h->forced_local = 1;
	      h->root.type = bfd_link_hash_defined;
	    }
	  else
	    h = NULL;
	}
      else
	{
	  isym = NULL;
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      /* Check invalid x32 relocations.  */
      if (!ABI_64_P (abfd))
	switch (r_type)
	  {
	  default:
	    break;

	  case R_X86_64_64:
	  case R_X86_64_DTPOFF64:
	  case R_X86_64_TPOFF64:
	  case R_X86_64_PC64:
	  case R_X86_64_GOTOFF64:
	  case R_X86_64_GOT64:
	  case R_X86_64_GOTPCREL64:
	  case R_X86_64_GOTPC64:
	  case R_X86_64_GOTPLT64:
	  case R_X86_64_PLTOFF64:
	      {
		if (h)
		  name = h->root.root.string;
		else
		  name = bfd_elf_sym_name (abfd, symtab_hdr, isym,
					   NULL);
		(*_bfd_error_handler)
		  (_("%B: relocation %s against symbol `%s' isn't "
		     "supported in x32 mode"), abfd,
		   x86_64_elf_howto_table[r_type].name, name);
		bfd_set_error (bfd_error_bad_value);
		return FALSE;
	      }
	    break;
	  }

      if (h != NULL)
	{
	  /* Create the ifunc sections for static executables.  If we
	     never see an indirect function symbol nor we are building
	     a static executable, those sections will be empty and
	     won't appear in output.  */
	  switch (r_type)
	    {
	    default:
	      break;

	    case R_X86_64_32S:
	    case R_X86_64_32:
	    case R_X86_64_64:
	    case R_X86_64_PC32:
	    case R_X86_64_PC64:
	    case R_X86_64_PLT32:
	    case R_X86_64_GOTPCREL:
	    case R_X86_64_GOTPCREL64:
	      if (!_bfd_elf_create_ifunc_sections (abfd, info))
		return FALSE;
	      break;
	    }

	  /* Since STT_GNU_IFUNC symbol must go through PLT, we handle
	     it here if it is defined in a non-shared object.  */
	  if (h->type == STT_GNU_IFUNC
	      && h->def_regular)
	    {
	      /* It is referenced by a non-shared object. */
	      h->ref_regular = 1;
	      h->needs_plt = 1;

	      /* STT_GNU_IFUNC symbol must go through PLT.  */
	      h->plt.refcount += 1;

	      /* STT_GNU_IFUNC needs dynamic sections.  */
	      if (htab->elf.dynobj == NULL)
		htab->elf.dynobj = abfd;

	      switch (r_type)
		{
		default:
		  if (h->root.root.string)
		    name = h->root.root.string;
		  else
		    name = bfd_elf_sym_name (abfd, symtab_hdr, isym,
					     NULL);
		  (*_bfd_error_handler)
		    (_("%B: relocation %s against STT_GNU_IFUNC "
		       "symbol `%s' isn't handled by %s"), abfd,
		     x86_64_elf_howto_table[r_type].name,
		     name, __FUNCTION__);
		  bfd_set_error (bfd_error_bad_value);
		  return FALSE;

		case R_X86_64_32:
		  if (ABI_64_P (abfd))
		    goto not_pointer;
		case R_X86_64_64:
		  h->non_got_ref = 1;
		  h->pointer_equality_needed = 1;
		  if (info->shared)
		    {
		      /* We must copy these reloc types into the output
			 file.  Create a reloc section in dynobj and
			 make room for this reloc.  */
		      sreloc = _bfd_elf_create_ifunc_dyn_reloc
			(abfd, info, sec, sreloc,
			 &((struct elf_x86_64_link_hash_entry *) h)->dyn_relocs);
		      if (sreloc == NULL)
			return FALSE;
		    }
		  break;

		case R_X86_64_32S:
		case R_X86_64_PC32:
		case R_X86_64_PC64:
not_pointer:
		  h->non_got_ref = 1;
		  if (r_type != R_X86_64_PC32
		      && r_type != R_X86_64_PC64)
		    h->pointer_equality_needed = 1;
		  break;

		case R_X86_64_PLT32:
		  break;

		case R_X86_64_GOTPCREL:
		case R_X86_64_GOTPCREL64:
		  h->got.refcount += 1;
		  if (htab->elf.sgot == NULL
		      && !_bfd_elf_create_got_section (htab->elf.dynobj,
						       info))
		    return FALSE;
		  break;
		}

	      continue;
	    }
	}

      if (! elf_x86_64_tls_transition (info, abfd, sec, NULL,
				       symtab_hdr, sym_hashes,
				       &r_type, GOT_UNKNOWN,
				       rel, rel_end, h, r_symndx))
	return FALSE;

      switch (r_type)
	{
	case R_X86_64_TLSLD:
	  htab->tls_ld_got.refcount += 1;
	  goto create_got;

	case R_X86_64_TPOFF32:
	  if (!info->executable && ABI_64_P (abfd))
	    {
	      if (h)
		name = h->root.root.string;
	      else
		name = bfd_elf_sym_name (abfd, symtab_hdr, isym,
					 NULL);
	      (*_bfd_error_handler)
		(_("%B: relocation %s against `%s' can not be used when making a shared object; recompile with -fPIC"),
		 abfd,
		 x86_64_elf_howto_table[r_type].name, name);
	      bfd_set_error (bfd_error_bad_value);
	      return FALSE;
	    }
	  break;

	case R_X86_64_GOTTPOFF:
	  if (!info->executable)
	    info->flags |= DF_STATIC_TLS;
	  /* Fall through */

	case R_X86_64_GOT32:
	case R_X86_64_GOTPCREL:
	case R_X86_64_TLSGD:
	case R_X86_64_GOT64:
	case R_X86_64_GOTPCREL64:
	case R_X86_64_GOTPLT64:
	case R_X86_64_GOTPC32_TLSDESC:
	case R_X86_64_TLSDESC_CALL:
	  /* This symbol requires a global offset table entry.	*/
	  {
	    int tls_type, old_tls_type;

	    switch (r_type)
	      {
	      default: tls_type = GOT_NORMAL; break;
	      case R_X86_64_TLSGD: tls_type = GOT_TLS_GD; break;
	      case R_X86_64_GOTTPOFF: tls_type = GOT_TLS_IE; break;
	      case R_X86_64_GOTPC32_TLSDESC:
	      case R_X86_64_TLSDESC_CALL:
		tls_type = GOT_TLS_GDESC; break;
	      }

	    if (h != NULL)
	      {
		if (r_type == R_X86_64_GOTPLT64)
		  {
		    /* This relocation indicates that we also need
		       a PLT entry, as this is a function.  We don't need
		       a PLT entry for local symbols.  */
		    h->needs_plt = 1;
		    h->plt.refcount += 1;
		  }
		h->got.refcount += 1;
		old_tls_type = elf_x86_64_hash_entry (h)->tls_type;
	      }
	    else
	      {
		bfd_signed_vma *local_got_refcounts;

		/* This is a global offset table entry for a local symbol.  */
		local_got_refcounts = elf_local_got_refcounts (abfd);
		if (local_got_refcounts == NULL)
		  {
		    bfd_size_type size;

		    size = symtab_hdr->sh_info;
		    size *= sizeof (bfd_signed_vma)
		      + sizeof (bfd_vma) + sizeof (char);
		    local_got_refcounts = ((bfd_signed_vma *)
					   bfd_zalloc (abfd, size));
		    if (local_got_refcounts == NULL)
		      return FALSE;
		    elf_local_got_refcounts (abfd) = local_got_refcounts;
		    elf_x86_64_local_tlsdesc_gotent (abfd)
		      = (bfd_vma *) (local_got_refcounts + symtab_hdr->sh_info);
		    elf_x86_64_local_got_tls_type (abfd)
		      = (char *) (local_got_refcounts + 2 * symtab_hdr->sh_info);
		  }
		local_got_refcounts[r_symndx] += 1;
		old_tls_type
		  = elf_x86_64_local_got_tls_type (abfd) [r_symndx];
	      }

	    /* If a TLS symbol is accessed using IE at least once,
	       there is no point to use dynamic model for it.  */
	    if (old_tls_type != tls_type && old_tls_type != GOT_UNKNOWN
		&& (! GOT_TLS_GD_ANY_P (old_tls_type)
		    || tls_type != GOT_TLS_IE))
	      {
		if (old_tls_type == GOT_TLS_IE && GOT_TLS_GD_ANY_P (tls_type))
		  tls_type = old_tls_type;
		else if (GOT_TLS_GD_ANY_P (old_tls_type)
			 && GOT_TLS_GD_ANY_P (tls_type))
		  tls_type |= old_tls_type;
		else
		  {
		    if (h)
		      name = h->root.root.string;
		    else
		      name = bfd_elf_sym_name (abfd, symtab_hdr,
					       isym, NULL);
		    (*_bfd_error_handler)
		      (_("%B: '%s' accessed both as normal and thread local symbol"),
		       abfd, name);
		    return FALSE;
		  }
	      }

	    if (old_tls_type != tls_type)
	      {
		if (h != NULL)
		  elf_x86_64_hash_entry (h)->tls_type = tls_type;
		else
		  elf_x86_64_local_got_tls_type (abfd) [r_symndx] = tls_type;
	      }
	  }
	  /* Fall through */

	case R_X86_64_GOTOFF64:
	case R_X86_64_GOTPC32:
	case R_X86_64_GOTPC64:
	create_got:
	  if (htab->elf.sgot == NULL)
	    {
	      if (htab->elf.dynobj == NULL)
		htab->elf.dynobj = abfd;
	      if (!_bfd_elf_create_got_section (htab->elf.dynobj,
						info))
		return FALSE;
	    }
	  break;

	case R_X86_64_PLT32:
	  /* This symbol requires a procedure linkage table entry.  We
	     actually build the entry in adjust_dynamic_symbol,
	     because this might be a case of linking PIC code which is
	     never referenced by a dynamic object, in which case we
	     don't need to generate a procedure linkage table entry
	     after all.	 */

	  /* If this is a local symbol, we resolve it directly without
	     creating a procedure linkage table entry.	*/
	  if (h == NULL)
	    continue;

	  h->needs_plt = 1;
	  h->plt.refcount += 1;
	  break;

	case R_X86_64_PLTOFF64:
	  /* This tries to form the 'address' of a function relative
	     to GOT.  For global symbols we need a PLT entry.  */
	  if (h != NULL)
	    {
	      h->needs_plt = 1;
	      h->plt.refcount += 1;
	    }
	  goto create_got;

	case R_X86_64_32:
	  if (!ABI_64_P (abfd))
	    goto pointer;
	case R_X86_64_8:
	case R_X86_64_16:
	case R_X86_64_32S:
	  /* Let's help debug shared library creation.  These relocs
	     cannot be used in shared libs.  Don't error out for
	     sections we don't care about, such as debug sections or
	     non-constant sections.  */
	  if (info->shared
	      && (sec->flags & SEC_ALLOC) != 0
	      && (sec->flags & SEC_READONLY) != 0)
	    {
	      if (h)
		name = h->root.root.string;
	      else
		name = bfd_elf_sym_name (abfd, symtab_hdr, isym, NULL);
	      (*_bfd_error_handler)
		(_("%B: relocation %s against `%s' can not be used when making a shared object; recompile with -fPIC"),
		 abfd, x86_64_elf_howto_table[r_type].name, name);
	      bfd_set_error (bfd_error_bad_value);
	      return FALSE;
	    }
	  /* Fall through.  */

	case R_X86_64_PC8:
	case R_X86_64_PC16:
	case R_X86_64_PC32:
	case R_X86_64_PC64:
	case R_X86_64_64:
pointer:
	  if (h != NULL && info->executable)
	    {
	      /* If this reloc is in a read-only section, we might
		 need a copy reloc.  We can't check reliably at this
		 stage whether the section is read-only, as input
		 sections have not yet been mapped to output sections.
		 Tentatively set the flag for now, and correct in
		 adjust_dynamic_symbol.  */
	      h->non_got_ref = 1;

	      /* We may need a .plt entry if the function this reloc
		 refers to is in a shared lib.  */
	      h->plt.refcount += 1;
	      if (r_type != R_X86_64_PC32 && r_type != R_X86_64_PC64)
		h->pointer_equality_needed = 1;
	    }

	  /* If we are creating a shared library, and this is a reloc
	     against a global symbol, or a non PC relative reloc
	     against a local symbol, then we need to copy the reloc
	     into the shared library.  However, if we are linking with
	     -Bsymbolic, we do not need to copy a reloc against a
	     global symbol which is defined in an object we are
	     including in the link (i.e., DEF_REGULAR is set).	At
	     this point we have not seen all the input files, so it is
	     possible that DEF_REGULAR is not set now but will be set
	     later (it is never cleared).  In case of a weak definition,
	     DEF_REGULAR may be cleared later by a strong definition in
	     a shared library.  We account for that possibility below by
	     storing information in the relocs_copied field of the hash
	     table entry.  A similar situation occurs when creating
	     shared libraries and symbol visibility changes render the
	     symbol local.

	     If on the other hand, we are creating an executable, we
	     may need to keep relocations for symbols satisfied by a
	     dynamic library if we manage to avoid copy relocs for the
	     symbol.  */
	  if ((info->shared
	       && (sec->flags & SEC_ALLOC) != 0
	       && (! IS_X86_64_PCREL_TYPE (r_type)
		   || (h != NULL
		       && (! SYMBOLIC_BIND (info, h)
			   || h->root.type == bfd_link_hash_defweak
			   || !h->def_regular))))
	      || (ELIMINATE_COPY_RELOCS
		  && !info->shared
		  && (sec->flags & SEC_ALLOC) != 0
		  && h != NULL
		  && (h->root.type == bfd_link_hash_defweak
		      || !h->def_regular)))
	    {
	      struct elf_dyn_relocs *p;
	      struct elf_dyn_relocs **head;

	      /* We must copy these reloc types into the output file.
		 Create a reloc section in dynobj and make room for
		 this reloc.  */
	      if (sreloc == NULL)
		{
		  if (htab->elf.dynobj == NULL)
		    htab->elf.dynobj = abfd;

		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, htab->elf.dynobj, ABI_64_P (abfd) ? 3 : 2,
		     abfd, /*rela?*/ TRUE);

		  if (sreloc == NULL)
		    return FALSE;
		}

	      /* If this is a global symbol, we count the number of
		 relocations we need for this symbol.  */
	      if (h != NULL)
		{
		  head = &((struct elf_x86_64_link_hash_entry *) h)->dyn_relocs;
		}
	      else
		{
		  /* Track dynamic relocs needed for local syms too.
		     We really need local syms available to do this
		     easily.  Oh well.  */
		  asection *s;
		  void **vpp;

		  isym = bfd_sym_from_r_symndx (&htab->sym_cache,
						abfd, r_symndx);
		  if (isym == NULL)
		    return FALSE;

		  s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		  if (s == NULL)
		    s = sec;

		  /* Beware of type punned pointers vs strict aliasing
		     rules.  */
		  vpp = &(elf_section_data (s)->local_dynrel);
		  head = (struct elf_dyn_relocs **)vpp;
		}

	      p = *head;
	      if (p == NULL || p->sec != sec)
		{
		  bfd_size_type amt = sizeof *p;

		  p = ((struct elf_dyn_relocs *)
		       bfd_alloc (htab->elf.dynobj, amt));
		  if (p == NULL)
		    return FALSE;
		  p->next = *head;
		  *head = p;
		  p->sec = sec;
		  p->count = 0;
		  p->pc_count = 0;
		}

	      p->count += 1;
	      if (IS_X86_64_PCREL_TYPE (r_type))
		p->pc_count += 1;
	    }
	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_X86_64_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return FALSE;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_X86_64_GNU_VTENTRY:
	  BFD_ASSERT (h != NULL);
	  if (h != NULL
	      && !bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return FALSE;
	  break;

	default:
	  break;
	}
    }

  return TRUE;
}

/* Return the section that should be marked against GC for a given
   relocation.	*/

static asection *
elf_x86_64_gc_mark_hook (asection *sec,
			 struct bfd_link_info *info,
			 Elf_Internal_Rela *rel,
			 struct elf_link_hash_entry *h,
			 Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_X86_64_GNU_VTINHERIT:
      case R_X86_64_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Update the got entry reference counts for the section being removed.	 */

static bfd_boolean
elf_x86_64_gc_sweep_hook (bfd *abfd, struct bfd_link_info *info,
			  asection *sec,
			  const Elf_Internal_Rela *relocs)
{
  struct elf_x86_64_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_signed_vma *local_got_refcounts;
  const Elf_Internal_Rela *rel, *relend;

  if (info->relocatable)
    return TRUE;

  htab = elf_x86_64_hash_table (info);
  if (htab == NULL)
    return FALSE;

  elf_section_data (sec)->local_dynrel = NULL;

  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);
  local_got_refcounts = elf_local_got_refcounts (abfd);

  htab = elf_x86_64_hash_table (info);
  relend = relocs + sec->reloc_count;
  for (rel = relocs; rel < relend; rel++)
    {
      unsigned long r_symndx;
      unsigned int r_type;
      struct elf_link_hash_entry *h = NULL;

      r_symndx = htab->r_sym (rel->r_info);
      if (r_symndx >= symtab_hdr->sh_info)
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}
      else
	{
	  /* A local symbol.  */
	  Elf_Internal_Sym *isym;

	  isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					abfd, r_symndx);

	  /* Check relocation against local STT_GNU_IFUNC symbol.  */
	  if (isym != NULL
	      && ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
	    {
	      h = elf_x86_64_get_local_sym_hash (htab, abfd, rel, FALSE);
	      if (h == NULL)
		abort ();
	    }
	}

      if (h)
	{
	  struct elf_x86_64_link_hash_entry *eh;
	  struct elf_dyn_relocs **pp;
	  struct elf_dyn_relocs *p;

	  eh = (struct elf_x86_64_link_hash_entry *) h;

	  for (pp = &eh->dyn_relocs; (p = *pp) != NULL; pp = &p->next)
	    if (p->sec == sec)
	      {
		/* Everything must go for SEC.  */
		*pp = p->next;
		break;
	      }
	}

      r_type = ELF32_R_TYPE (rel->r_info);
      if (! elf_x86_64_tls_transition (info, abfd, sec, NULL,
				       symtab_hdr, sym_hashes,
				       &r_type, GOT_UNKNOWN,
				       rel, relend, h, r_symndx))
	return FALSE;

      switch (r_type)
	{
	case R_X86_64_TLSLD:
	  if (htab->tls_ld_got.refcount > 0)
	    htab->tls_ld_got.refcount -= 1;
	  break;

	case R_X86_64_TLSGD:
	case R_X86_64_GOTPC32_TLSDESC:
	case R_X86_64_TLSDESC_CALL:
	case R_X86_64_GOTTPOFF:
	case R_X86_64_GOT32:
	case R_X86_64_GOTPCREL:
	case R_X86_64_GOT64:
	case R_X86_64_GOTPCREL64:
	case R_X86_64_GOTPLT64:
	  if (h != NULL)
	    {
	      if (r_type == R_X86_64_GOTPLT64 && h->plt.refcount > 0)
	        h->plt.refcount -= 1;
	      if (h->got.refcount > 0)
		h->got.refcount -= 1;
	      if (h->type == STT_GNU_IFUNC)
		{
		  if (h->plt.refcount > 0)
		    h->plt.refcount -= 1;
		}
	    }
	  else if (local_got_refcounts != NULL)
	    {
	      if (local_got_refcounts[r_symndx] > 0)
		local_got_refcounts[r_symndx] -= 1;
	    }
	  break;

	case R_X86_64_8:
	case R_X86_64_16:
	case R_X86_64_32:
	case R_X86_64_64:
	case R_X86_64_32S:
	case R_X86_64_PC8:
	case R_X86_64_PC16:
	case R_X86_64_PC32:
	case R_X86_64_PC64:
	  if (info->shared
	      && (h == NULL || h->type != STT_GNU_IFUNC))
	    break;
	  /* Fall thru */

	case R_X86_64_PLT32:
	case R_X86_64_PLTOFF64:
	  if (h != NULL)
	    {
	      if (h->plt.refcount > 0)
		h->plt.refcount -= 1;
	    }
	  break;

	default:
	  break;
	}
    }

  return TRUE;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.	*/

static bfd_boolean
elf_x86_64_adjust_dynamic_symbol (struct bfd_link_info *info,
				  struct elf_link_hash_entry *h)
{
  struct elf_x86_64_link_hash_table *htab;
  asection *s;

  /* STT_GNU_IFUNC symbol must go through PLT. */
  if (h->type == STT_GNU_IFUNC)
    {
      if (h->plt.refcount <= 0)
	{
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}
      return TRUE;
    }

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC
      || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || SYMBOL_CALLS_LOCAL (info, h)
	  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      && h->root.type == bfd_link_hash_undefweak))
	{
	  /* This case can occur if we saw a PLT32 reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a procedure
	     linkage table, and we can just do a PC32 reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}

      return TRUE;
    }
  else
    /* It's possible that we incorrectly decided a .plt reloc was
       needed for an R_X86_64_PC32 reloc to a non-function sym in
       check_relocs.  We can't decide accurately between function and
       non-function syms in check-relocs;  Objects loaded later in
       the link may change h->type.  So fix it now.  */
    h->plt.offset = (bfd_vma) -1;

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.	 */
  if (h->u.weakdef != NULL)
    {
      BFD_ASSERT (h->u.weakdef->root.type == bfd_link_hash_defined
		  || h->u.weakdef->root.type == bfd_link_hash_defweak);
      h->root.u.def.section = h->u.weakdef->root.u.def.section;
      h->root.u.def.value = h->u.weakdef->root.u.def.value;
      if (ELIMINATE_COPY_RELOCS || info->nocopyreloc)
	h->non_got_ref = h->u.weakdef->non_got_ref;
      return TRUE;
    }

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.	 */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.	*/
  if (info->shared)
    return TRUE;

  /* If there are no references to this symbol that do not use the
     GOT, we don't need to generate a copy reloc.  */
  if (!h->non_got_ref)
    return TRUE;

  /* If -z nocopyreloc was given, we won't generate them either.  */
  if (info->nocopyreloc)
    {
      h->non_got_ref = 0;
      return TRUE;
    }

  if (ELIMINATE_COPY_RELOCS)
    {
      struct elf_x86_64_link_hash_entry * eh;
      struct elf_dyn_relocs *p;

      eh = (struct elf_x86_64_link_hash_entry *) h;
      for (p = eh->dyn_relocs; p != NULL; p = p->next)
	{
	  s = p->sec->output_section;
	  if (s != NULL && (s->flags & SEC_READONLY) != 0)
	    break;
	}

      /* If we didn't find any dynamic relocs in read-only sections, then
	 we'll be keeping the dynamic relocs and avoiding the copy reloc.  */
      if (p == NULL)
	{
	  h->non_got_ref = 0;
	  return TRUE;
	}
    }

  if (h->size == 0)
    {
      (*_bfd_error_handler) (_("dynamic variable `%s' is zero size"),
			     h->root.root.string);
      return TRUE;
    }

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.	 There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */

  htab = elf_x86_64_hash_table (info);
  if (htab == NULL)
    return FALSE;

  /* We must generate a R_X86_64_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
     runtime process image.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      const struct elf_backend_data *bed;
      bed = get_elf_backend_data (info->output_bfd);
      htab->srelbss->size += bed->s->sizeof_rela;
      h->needs_copy = 1;
    }

  s = htab->sdynbss;

  return _bfd_elf_adjust_dynamic_copy (h, s);
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bfd_boolean
elf_x86_64_allocate_dynrelocs (struct elf_link_hash_entry *h, void * inf)
{
  struct bfd_link_info *info;
  struct elf_x86_64_link_hash_table *htab;
  struct elf_x86_64_link_hash_entry *eh;
  struct elf_dyn_relocs *p;
  const struct elf_backend_data *bed;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;
  eh = (struct elf_x86_64_link_hash_entry *) h;

  info = (struct bfd_link_info *) inf;
  htab = elf_x86_64_hash_table (info);
  if (htab == NULL)
    return FALSE;
  bed = get_elf_backend_data (info->output_bfd);

  /* Since STT_GNU_IFUNC symbol must go through PLT, we handle it
     here if it is defined and referenced in a non-shared object.  */
  if (h->type == STT_GNU_IFUNC
      && h->def_regular)
    return _bfd_elf_allocate_ifunc_dyn_relocs (info, h,
					       &eh->dyn_relocs,
					       PLT_ENTRY_SIZE,
					       GOT_ENTRY_SIZE);
  else if (htab->elf.dynamic_sections_created
	   && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

      if (info->shared
	  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, 0, h))
	{
	  asection *s = htab->elf.splt;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (s->size == 0)
	    s->size += PLT_ENTRY_SIZE;

	  h->plt.offset = s->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (! info->shared
	      && !h->def_regular)
	    {
	      h->root.u.def.section = s;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry.  */
	  s->size += PLT_ENTRY_SIZE;

	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  htab->elf.sgotplt->size += GOT_ENTRY_SIZE;

	  /* We also need to make an entry in the .rela.plt section.  */
	  htab->elf.srelplt->size += bed->s->sizeof_rela;
	  htab->elf.srelplt->reloc_count++;
	}
      else
	{
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}
    }
  else
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
    }

  eh->tlsdesc_got = (bfd_vma) -1;

  /* If R_X86_64_GOTTPOFF symbol is now local to the binary,
     make it a R_X86_64_TPOFF32 requiring no GOT entry.  */
  if (h->got.refcount > 0
      && info->executable
      && h->dynindx == -1
      && elf_x86_64_hash_entry (h)->tls_type == GOT_TLS_IE)
    {
      h->got.offset = (bfd_vma) -1;
    }
  else if (h->got.refcount > 0)
    {
      asection *s;
      bfd_boolean dyn;
      int tls_type = elf_x86_64_hash_entry (h)->tls_type;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

      if (GOT_TLS_GDESC_P (tls_type))
	{
	  eh->tlsdesc_got = htab->elf.sgotplt->size
	    - elf_x86_64_compute_jump_table_size (htab);
	  htab->elf.sgotplt->size += 2 * GOT_ENTRY_SIZE;
	  h->got.offset = (bfd_vma) -2;
	}
      if (! GOT_TLS_GDESC_P (tls_type)
	  || GOT_TLS_GD_P (tls_type))
	{
	  s = htab->elf.sgot;
	  h->got.offset = s->size;
	  s->size += GOT_ENTRY_SIZE;
	  if (GOT_TLS_GD_P (tls_type))
	    s->size += GOT_ENTRY_SIZE;
	}
      dyn = htab->elf.dynamic_sections_created;
      /* R_X86_64_TLSGD needs one dynamic relocation if local symbol
	 and two if global.
	 R_X86_64_GOTTPOFF needs one dynamic relocation.  */
      if ((GOT_TLS_GD_P (tls_type) && h->dynindx == -1)
	  || tls_type == GOT_TLS_IE)
	htab->elf.srelgot->size += bed->s->sizeof_rela;
      else if (GOT_TLS_GD_P (tls_type))
	htab->elf.srelgot->size += 2 * bed->s->sizeof_rela;
      else if (! GOT_TLS_GDESC_P (tls_type)
	       && (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		   || h->root.type != bfd_link_hash_undefweak)
	       && (info->shared
		   || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h)))
	htab->elf.srelgot->size += bed->s->sizeof_rela;
      if (GOT_TLS_GDESC_P (tls_type))
	{
	  htab->elf.srelplt->size += bed->s->sizeof_rela;
	  htab->tlsdesc_plt = (bfd_vma) -1;
	}
    }
  else
    h->got.offset = (bfd_vma) -1;

  if (eh->dyn_relocs == NULL)
    return TRUE;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for pc-relative relocs that have become local due to symbol
     visibility changes.  */

  if (info->shared)
    {
      /* Relocs that use pc_count are those that appear on a call
	 insn, or certain REL relocs that can generated via assembly.
	 We want calls to protected symbols to resolve directly to the
	 function rather than going via the plt.  If people want
	 function pointer comparisons to work as expected then they
	 should avoid writing weird assembly.  */
      if (SYMBOL_CALLS_LOCAL (info, h))
	{
	  struct elf_dyn_relocs **pp;

	  for (pp = &eh->dyn_relocs; (p = *pp) != NULL; )
	    {
	      p->count -= p->pc_count;
	      p->pc_count = 0;
	      if (p->count == 0)
		*pp = p->next;
	      else
		pp = &p->next;
	    }
	}

      /* Also discard relocs on undefined weak syms with non-default
	 visibility.  */
      if (eh->dyn_relocs != NULL
	  && h->root.type == bfd_link_hash_undefweak)
	{
	  if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
	    eh->dyn_relocs = NULL;

	  /* Make sure undefined weak symbols are output as a dynamic
	     symbol in PIEs.  */
	  else if (h->dynindx == -1
		   && ! h->forced_local
		   && ! bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

    }
  else if (ELIMINATE_COPY_RELOCS)
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */

      if (!h->non_got_ref
	  && ((h->def_dynamic
	       && !h->def_regular)
	      || (htab->elf.dynamic_sections_created
		  && (h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined))))
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (h->dynindx == -1
	      && ! h->forced_local
	      && ! bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;

	  /* If that succeeded, we know we'll be keeping all the
	     relocs.  */
	  if (h->dynindx != -1)
	    goto keep;
	}

      eh->dyn_relocs = NULL;

    keep: ;
    }

  /* Finally, allocate space.  */
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      asection * sreloc;

      sreloc = elf_section_data (p->sec)->sreloc;

      BFD_ASSERT (sreloc != NULL);

      sreloc->size += p->count * bed->s->sizeof_rela;
    }

  return TRUE;
}

/* Allocate space in .plt, .got and associated reloc sections for
   local dynamic relocs.  */

static bfd_boolean
elf_x86_64_allocate_local_dynrelocs (void **slot, void *inf)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) *slot;

  if (h->type != STT_GNU_IFUNC
      || !h->def_regular
      || !h->ref_regular
      || !h->forced_local
      || h->root.type != bfd_link_hash_defined)
    abort ();

  return elf_x86_64_allocate_dynrelocs (h, inf);
}

/* Find any dynamic relocs that apply to read-only sections.  */

static bfd_boolean
elf_x86_64_readonly_dynrelocs (struct elf_link_hash_entry *h,
			       void * inf)
{
  struct elf_x86_64_link_hash_entry *eh;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  eh = (struct elf_x86_64_link_hash_entry *) h;
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      asection *s = p->sec->output_section;

      if (s != NULL && (s->flags & SEC_READONLY) != 0)
	{
	  struct bfd_link_info *info = (struct bfd_link_info *) inf;

	  info->flags |= DF_TEXTREL;

	  /* Not an error, just cut short the traversal.  */
	  return FALSE;
	}
    }
  return TRUE;
}

/* Set the sizes of the dynamic sections.  */

static bfd_boolean
elf_x86_64_size_dynamic_sections (bfd *output_bfd,
				  struct bfd_link_info *info)
{
  struct elf_x86_64_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd_boolean relocs;
  bfd *ibfd;
  const struct elf_backend_data *bed;

  htab = elf_x86_64_hash_table (info);
  if (htab == NULL)
    return FALSE;
  bed = get_elf_backend_data (output_bfd);

  dynobj = htab->elf.dynobj;
  if (dynobj == NULL)
    abort ();

  if (htab->elf.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (info->executable)
	{
	  s = bfd_get_section_by_name (dynobj, ".interp");
	  if (s == NULL)
	    abort ();
	  s->size = htab->dynamic_interpreter_size;
	  s->contents = (unsigned char *) htab->dynamic_interpreter;
	}
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link_next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      char *local_tls_type;
      bfd_vma *local_tlsdesc_gotent;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srel;

      if (! is_x86_64_elf (ibfd))
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = (struct elf_dyn_relocs *)
		    (elf_section_data (s)->local_dynrel);
	       p != NULL;
	       p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (p->count != 0)
		{
		  srel = elf_section_data (p->sec)->sreloc;
		  srel->size += p->count * bed->s->sizeof_rela;
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      local_tls_type = elf_x86_64_local_got_tls_type (ibfd);
      local_tlsdesc_gotent = elf_x86_64_local_tlsdesc_gotent (ibfd);
      s = htab->elf.sgot;
      srel = htab->elf.srelgot;
      for (; local_got < end_local_got;
	   ++local_got, ++local_tls_type, ++local_tlsdesc_gotent)
	{
	  *local_tlsdesc_gotent = (bfd_vma) -1;
	  if (*local_got > 0)
	    {
	      if (GOT_TLS_GDESC_P (*local_tls_type))
		{
		  *local_tlsdesc_gotent = htab->elf.sgotplt->size
		    - elf_x86_64_compute_jump_table_size (htab);
		  htab->elf.sgotplt->size += 2 * GOT_ENTRY_SIZE;
		  *local_got = (bfd_vma) -2;
		}
	      if (! GOT_TLS_GDESC_P (*local_tls_type)
		  || GOT_TLS_GD_P (*local_tls_type))
		{
		  *local_got = s->size;
		  s->size += GOT_ENTRY_SIZE;
		  if (GOT_TLS_GD_P (*local_tls_type))
		    s->size += GOT_ENTRY_SIZE;
		}
	      if (info->shared
		  || GOT_TLS_GD_ANY_P (*local_tls_type)
		  || *local_tls_type == GOT_TLS_IE)
		{
		  if (GOT_TLS_GDESC_P (*local_tls_type))
		    {
		      htab->elf.srelplt->size
			+= bed->s->sizeof_rela;
		      htab->tlsdesc_plt = (bfd_vma) -1;
		    }
		  if (! GOT_TLS_GDESC_P (*local_tls_type)
		      || GOT_TLS_GD_P (*local_tls_type))
		    srel->size += bed->s->sizeof_rela;
		}
	    }
	  else
	    *local_got = (bfd_vma) -1;
	}
    }

  if (htab->tls_ld_got.refcount > 0)
    {
      /* Allocate 2 got entries and 1 dynamic reloc for R_X86_64_TLSLD
	 relocs.  */
      htab->tls_ld_got.offset = htab->elf.sgot->size;
      htab->elf.sgot->size += 2 * GOT_ENTRY_SIZE;
      htab->elf.srelgot->size += bed->s->sizeof_rela;
    }
  else
    htab->tls_ld_got.offset = -1;

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->elf, elf_x86_64_allocate_dynrelocs,
			  info);

  /* Allocate .plt and .got entries, and space for local symbols.  */
  htab_traverse (htab->loc_hash_table,
		 elf_x86_64_allocate_local_dynrelocs,
		 info);

  /* For every jump slot reserved in the sgotplt, reloc_count is
     incremented.  However, when we reserve space for TLS descriptors,
     it's not incremented, so in order to compute the space reserved
     for them, it suffices to multiply the reloc count by the jump
     slot size.  */
  if (htab->elf.srelplt)
    htab->sgotplt_jump_table_size
      = elf_x86_64_compute_jump_table_size (htab);

  if (htab->tlsdesc_plt)
    {
      /* If we're not using lazy TLS relocations, don't generate the
	 PLT and GOT entries they require.  */
      if ((info->flags & DF_BIND_NOW))
	htab->tlsdesc_plt = 0;
      else
	{
	  htab->tlsdesc_got = htab->elf.sgot->size;
	  htab->elf.sgot->size += GOT_ENTRY_SIZE;
	  /* Reserve room for the initial entry.
	     FIXME: we could probably do away with it in this case.  */
	  if (htab->elf.splt->size == 0)
	    htab->elf.splt->size += PLT_ENTRY_SIZE;
	  htab->tlsdesc_plt = htab->elf.splt->size;
	  htab->elf.splt->size += PLT_ENTRY_SIZE;
	}
    }

  if (htab->elf.sgotplt)
    {
      struct elf_link_hash_entry *got;
      got = elf_link_hash_lookup (elf_hash_table (info),
				  "_GLOBAL_OFFSET_TABLE_",
				  FALSE, FALSE, FALSE);

      /* Don't allocate .got.plt section if there are no GOT nor PLT
         entries and there is no refeence to _GLOBAL_OFFSET_TABLE_.  */
      if ((got == NULL
	   || !got->ref_regular_nonweak)
	  && (htab->elf.sgotplt->size
	      == get_elf_backend_data (output_bfd)->got_header_size)
	  && (htab->elf.splt == NULL
	      || htab->elf.splt->size == 0)
	  && (htab->elf.sgot == NULL
	      || htab->elf.sgot->size == 0)
	  && (htab->elf.iplt == NULL
	      || htab->elf.iplt->size == 0)
	  && (htab->elf.igotplt == NULL
	      || htab->elf.igotplt->size == 0))
	htab->elf.sgotplt->size = 0;
    }

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = FALSE;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->elf.splt
	  || s == htab->elf.sgot
	  || s == htab->elf.sgotplt
	  || s == htab->elf.iplt
	  || s == htab->elf.igotplt
	  || s == htab->sdynbss)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (CONST_STRNEQ (bfd_get_section_name (dynobj, s), ".rela"))
	{
	  if (s->size != 0 && s != htab->elf.srelplt)
	    relocs = TRUE;

	  /* We use the reloc_count field as a counter if we need
	     to copy relocs into the output file.  */
	  if (s != htab->elf.srelplt)
	    s->reloc_count = 0;
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0)
	{
	  /* If we don't need this section, strip it from the
	     output file.  This is mostly to handle .rela.bss and
	     .rela.plt.  We must create both sections in
	     create_dynamic_sections, because they must be created
	     before the linker maps input sections to output
	     sections.  The linker does that before
	     adjust_dynamic_symbol is called, and it is that
	     function which decides whether anything needs to go
	     into these sections.  */

	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  We use bfd_zalloc
	 here in case unused entries are not reclaimed before the
	 section's contents are written out.  This should not happen,
	 but this way if it does, we get a R_X86_64_NONE reloc instead
	 of garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return FALSE;
    }

  if (htab->elf.dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in elf_x86_64_finish_dynamic_sections, but we
	 must add the entries now so that we get the correct size for
	 the .dynamic section.	The DT_DEBUG entry is filled in by the
	 dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL) \
  _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (info->executable)
	{
	  if (!add_dynamic_entry (DT_DEBUG, 0))
	    return FALSE;
	}

      if (htab->elf.splt->size != 0)
	{
	  if (!add_dynamic_entry (DT_PLTGOT, 0)
	      || !add_dynamic_entry (DT_PLTRELSZ, 0)
	      || !add_dynamic_entry (DT_PLTREL, DT_RELA)
	      || !add_dynamic_entry (DT_JMPREL, 0))
	    return FALSE;

	  if (htab->tlsdesc_plt
	      && (!add_dynamic_entry (DT_TLSDESC_PLT, 0)
		  || !add_dynamic_entry (DT_TLSDESC_GOT, 0)))
	    return FALSE;
	}

      if (relocs)
	{
	  if (!add_dynamic_entry (DT_RELA, 0)
	      || !add_dynamic_entry (DT_RELASZ, 0)
	      || !add_dynamic_entry (DT_RELAENT, bed->s->sizeof_rela))
	    return FALSE;

	  /* If any dynamic relocs apply to a read-only section,
	     then we need a DT_TEXTREL entry.  */
	  if ((info->flags & DF_TEXTREL) == 0)
	    elf_link_hash_traverse (&htab->elf, 
				    elf_x86_64_readonly_dynrelocs,
				    info);

	  if ((info->flags & DF_TEXTREL) != 0)
	    {
	      if (!add_dynamic_entry (DT_TEXTREL, 0))
		return FALSE;
	    }
	}
    }
#undef add_dynamic_entry

  return TRUE;
}

static bfd_boolean
elf_x86_64_always_size_sections (bfd *output_bfd,
				 struct bfd_link_info *info)
{
  asection *tls_sec = elf_hash_table (info)->tls_sec;

  if (tls_sec)
    {
      struct elf_link_hash_entry *tlsbase;

      tlsbase = elf_link_hash_lookup (elf_hash_table (info),
				      "_TLS_MODULE_BASE_",
				      FALSE, FALSE, FALSE);

      if (tlsbase && tlsbase->type == STT_TLS)
	{
	  struct elf_x86_64_link_hash_table *htab;
	  struct bfd_link_hash_entry *bh = NULL;
	  const struct elf_backend_data *bed
	    = get_elf_backend_data (output_bfd);

	  htab = elf_x86_64_hash_table (info);
	  if (htab == NULL)
	    return FALSE;

	  if (!(_bfd_generic_link_add_one_symbol
		(info, output_bfd, "_TLS_MODULE_BASE_", BSF_LOCAL,
		 tls_sec, 0, NULL, FALSE,
		 bed->collect, &bh)))
	    return FALSE;

	  htab->tls_module_base = bh;

	  tlsbase = (struct elf_link_hash_entry *)bh;
	  tlsbase->def_regular = 1;
	  tlsbase->other = STV_HIDDEN;
	  (*bed->elf_backend_hide_symbol) (info, tlsbase, TRUE);
	}
    }

  return TRUE;
}

/* _TLS_MODULE_BASE_ needs to be treated especially when linking
   executables.  Rather than setting it to the beginning of the TLS
   section, we have to set it to the end.  This function may be called
   multiple times, it is idempotent.  */

static void
elf_x86_64_set_tls_module_base (struct bfd_link_info *info)
{
  struct elf_x86_64_link_hash_table *htab;
  struct bfd_link_hash_entry *base;

  if (!info->executable)
    return;

  htab = elf_x86_64_hash_table (info);
  if (htab == NULL)
    return;

  base = htab->tls_module_base;
  if (base == NULL)
    return;

  base->u.def.value = htab->elf.tls_size;
}

/* Return the base VMA address which should be subtracted from real addresses
   when resolving @dtpoff relocation.
   This is PT_TLS segment p_vaddr.  */

static bfd_vma
elf_x86_64_dtpoff_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  return elf_hash_table (info)->tls_sec->vma;
}

/* Return the relocation value for @tpoff relocation
   if STT_TLS virtual address is ADDRESS.  */

static bfd_vma
elf_x86_64_tpoff (struct bfd_link_info *info, bfd_vma address)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);
  const struct elf_backend_data *bed = get_elf_backend_data (info->output_bfd);
  bfd_vma static_tls_size;

  /* If tls_segment is NULL, we should have signalled an error already.  */
  if (htab->tls_sec == NULL)
    return 0;

  /* Consider special static TLS alignment requirements.  */
  static_tls_size = BFD_ALIGN (htab->tls_size, bed->static_tls_alignment);
  return address - static_tls_size - htab->tls_sec->vma;
}

/* Is the instruction before OFFSET in CONTENTS a 32bit relative
   branch?  */

static bfd_boolean
is_32bit_relative_branch (bfd_byte *contents, bfd_vma offset)
{
  /* Opcode		Instruction
     0xe8		call
     0xe9		jump
     0x0f 0x8x		conditional jump */
  return ((offset > 0
	   && (contents [offset - 1] == 0xe8
	       || contents [offset - 1] == 0xe9))
	  || (offset > 1
	      && contents [offset - 2] == 0x0f
	      && (contents [offset - 1] & 0xf0) == 0x80));
}

/* Relocate an x86_64 ELF section.  */

static bfd_boolean
elf_x86_64_relocate_section (bfd *output_bfd,
			     struct bfd_link_info *info,
			     bfd *input_bfd,
			     asection *input_section,
			     bfd_byte *contents,
			     Elf_Internal_Rela *relocs,
			     Elf_Internal_Sym *local_syms,
			     asection **local_sections)
{
  struct elf_x86_64_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_vma *local_got_offsets;
  bfd_vma *local_tlsdesc_gotents;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;

  BFD_ASSERT (is_x86_64_elf (input_bfd));

  htab = elf_x86_64_hash_table (info);
  if (htab == NULL)
    return FALSE;
  symtab_hdr = &elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);
  local_got_offsets = elf_local_got_offsets (input_bfd);
  local_tlsdesc_gotents = elf_x86_64_local_tlsdesc_gotent (input_bfd);

  elf_x86_64_set_tls_module_base (info);

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      unsigned int r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *sym;
      asection *sec;
      bfd_vma off, offplt;
      bfd_vma relocation;
      bfd_boolean unresolved_reloc;
      bfd_reloc_status_type r;
      int tls_type;
      asection *base_got;

      r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type == (int) R_X86_64_GNU_VTINHERIT
	  || r_type == (int) R_X86_64_GNU_VTENTRY)
	continue;

      if (r_type >= R_X86_64_max)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}

      howto = x86_64_elf_howto_table + r_type;
      r_symndx = htab->r_sym (rel->r_info);
      h = NULL;
      sym = NULL;
      sec = NULL;
      unresolved_reloc = FALSE;
      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];

	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym,
						&sec, rel);

	  /* Relocate against local STT_GNU_IFUNC symbol.  */
	  if (!info->relocatable
	      && ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
	    {
	      h = elf_x86_64_get_local_sym_hash (htab, input_bfd,
						 rel, FALSE);
	      if (h == NULL)
		abort ();

	      /* Set STT_GNU_IFUNC symbol value.  */ 
	      h->root.u.def.value = sym->st_value;
	      h->root.u.def.section = sec;
	    }
	}
      else
	{
	  bfd_boolean warned ATTRIBUTE_UNUSED;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned);
	}

      if (sec != NULL && elf_discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, relend, howto, contents);

      if (info->relocatable)
	continue;

      /* Since STT_GNU_IFUNC symbol must go through PLT, we handle
	 it here if it is defined in a non-shared object.  */
      if (h != NULL
	  && h->type == STT_GNU_IFUNC
	  && h->def_regular)
	{
	  asection *plt;
	  bfd_vma plt_index;
	  const char *name;

	  if ((input_section->flags & SEC_ALLOC) == 0
	      || h->plt.offset == (bfd_vma) -1)
	    abort ();

	  /* STT_GNU_IFUNC symbol must go through PLT.  */
	  plt = htab->elf.splt ? htab->elf.splt : htab->elf.iplt;
	  relocation = (plt->output_section->vma
			+ plt->output_offset + h->plt.offset);

	  switch (r_type)
	    {
	    default:
	      if (h->root.root.string)
		name = h->root.root.string;
	      else
		name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym,
					 NULL);
	      (*_bfd_error_handler)
		(_("%B: relocation %s against STT_GNU_IFUNC "
		   "symbol `%s' isn't handled by %s"), input_bfd,
		 x86_64_elf_howto_table[r_type].name,
		 name, __FUNCTION__);
	      bfd_set_error (bfd_error_bad_value);
	      return FALSE;

	    case R_X86_64_32S:
	      if (info->shared)
		abort ();
	      goto do_relocation;

	    case R_X86_64_32:
	      if (ABI_64_P (output_bfd))
		goto do_relocation;
	    case R_X86_64_64: 
	      if (rel->r_addend != 0)
		{
		  if (h->root.root.string)
		    name = h->root.root.string;
		  else
		    name = bfd_elf_sym_name (input_bfd, symtab_hdr,
					     sym, NULL);
		  (*_bfd_error_handler)
		    (_("%B: relocation %s against STT_GNU_IFUNC "
		       "symbol `%s' has non-zero addend: %d"),
		     input_bfd, x86_64_elf_howto_table[r_type].name,
		     name, rel->r_addend);
		  bfd_set_error (bfd_error_bad_value);
		  return FALSE;
		}

	      /* Generate dynamic relcoation only when there is a
		 non-GOF reference in a shared object.  */
	      if (info->shared && h->non_got_ref)
		{
		  Elf_Internal_Rela outrel;
		  asection *sreloc;

		  /* Need a dynamic relocation to get the real function
		     address.  */
		  outrel.r_offset = _bfd_elf_section_offset (output_bfd,
							     info,
							     input_section,
							     rel->r_offset);
		  if (outrel.r_offset == (bfd_vma) -1
		      || outrel.r_offset == (bfd_vma) -2)
		    abort ();

		  outrel.r_offset += (input_section->output_section->vma
				      + input_section->output_offset);

		  if (h->dynindx == -1
		      || h->forced_local
		      || info->executable)
		    {
		      /* This symbol is resolved locally.  */
		      outrel.r_info = htab->r_info (0, R_X86_64_IRELATIVE);
		      outrel.r_addend = (h->root.u.def.value
					 + h->root.u.def.section->output_section->vma
					 + h->root.u.def.section->output_offset);
		    }
		  else
		    {
		      outrel.r_info = htab->r_info (h->dynindx, r_type);
		      outrel.r_addend = 0;
		    }

		  sreloc = htab->elf.irelifunc;
		  elf_append_rela (output_bfd, sreloc, &outrel);

		  /* If this reloc is against an external symbol, we
		     do not want to fiddle with the addend.  Otherwise,
		     we need to include the symbol value so that it
		     becomes an addend for the dynamic reloc.  For an
		     internal symbol, we have updated addend.  */
		  continue;
		}

	    case R_X86_64_PC32:
	    case R_X86_64_PC64:
	    case R_X86_64_PLT32:
	      goto do_relocation;

	    case R_X86_64_GOTPCREL:
	    case R_X86_64_GOTPCREL64:
	      base_got = htab->elf.sgot;
	      off = h->got.offset;

	      if (base_got == NULL)
		abort ();

	      if (off == (bfd_vma) -1)
		{
		  /* We can't use h->got.offset here to save state, or
		     even just remember the offset, as finish_dynamic_symbol
		     would use that as offset into .got.  */

		  if (htab->elf.splt != NULL)
		    {
		      plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;
		      off = (plt_index + 3) * GOT_ENTRY_SIZE;
		      base_got = htab->elf.sgotplt;
		    }
		  else
		    {
		      plt_index = h->plt.offset / PLT_ENTRY_SIZE;
		      off = plt_index * GOT_ENTRY_SIZE;
		      base_got = htab->elf.igotplt;
		    }

		  if (h->dynindx == -1
		      || h->forced_local
		      || info->symbolic)
		    {
		      /* This references the local defitionion.  We must 
			 initialize this entry in the global offset table.
			 Since the offset must always be a multiple of 8, 
			 we use the least significant bit to record
			 whether we have initialized it already.

			 When doing a dynamic link, we create a .rela.got
			 relocation entry to initialize the value.  This
			 is done in the finish_dynamic_symbol routine.	 */
		      if ((off & 1) != 0)
			off &= ~1;
		      else
			{
			  bfd_put_64 (output_bfd, relocation,
				      base_got->contents + off);
			  /* Note that this is harmless for the GOTPLT64
			     case, as -1 | 1 still is -1.  */
			  h->got.offset |= 1;
			}
		    }
		}

	      relocation = (base_got->output_section->vma
			    + base_got->output_offset + off);

	      if (r_type != R_X86_64_GOTPCREL
		  && r_type != R_X86_64_GOTPCREL64)
		{
		  asection *gotplt;
		  if (htab->elf.splt != NULL)
		    gotplt = htab->elf.sgotplt;
		  else
		    gotplt = htab->elf.igotplt;
		  relocation -= (gotplt->output_section->vma
				 - gotplt->output_offset);
		}

	      goto do_relocation;
	    }
	}

      /* When generating a shared object, the relocations handled here are
	 copied into the output file to be resolved at run time.  */
      switch (r_type)
	{
	case R_X86_64_GOT32:
	case R_X86_64_GOT64:
	  /* Relocation is to the entry for this symbol in the global
	     offset table.  */
	case R_X86_64_GOTPCREL:
	case R_X86_64_GOTPCREL64:
	  /* Use global offset table entry as symbol value.  */
	case R_X86_64_GOTPLT64:
	  /* This is the same as GOT64 for relocation purposes, but
	     indicates the existence of a PLT entry.  The difficulty is,
	     that we must calculate the GOT slot offset from the PLT
	     offset, if this symbol got a PLT entry (it was global).
	     Additionally if it's computed from the PLT entry, then that
	     GOT offset is relative to .got.plt, not to .got.  */
	  base_got = htab->elf.sgot;

	  if (htab->elf.sgot == NULL)
	    abort ();

	  if (h != NULL)
	    {
	      bfd_boolean dyn;

	      off = h->got.offset;
	      if (h->needs_plt
	          && h->plt.offset != (bfd_vma)-1
		  && off == (bfd_vma)-1)
		{
		  /* We can't use h->got.offset here to save
		     state, or even just remember the offset, as
		     finish_dynamic_symbol would use that as offset into
		     .got.  */
		  bfd_vma plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;
		  off = (plt_index + 3) * GOT_ENTRY_SIZE;
		  base_got = htab->elf.sgotplt;
		}

	      dyn = htab->elf.dynamic_sections_created;

	      if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, info->shared, h)
		  || (info->shared
		      && SYMBOL_REFERENCES_LOCAL (info, h))
		  || (ELF_ST_VISIBILITY (h->other)
		      && h->root.type == bfd_link_hash_undefweak))
		{
		  /* This is actually a static link, or it is a -Bsymbolic
		     link and the symbol is defined locally, or the symbol
		     was forced to be local because of a version file.	We
		     must initialize this entry in the global offset table.
		     Since the offset must always be a multiple of 8, we
		     use the least significant bit to record whether we
		     have initialized it already.

		     When doing a dynamic link, we create a .rela.got
		     relocation entry to initialize the value.	This is
		     done in the finish_dynamic_symbol routine.	 */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      bfd_put_64 (output_bfd, relocation,
				  base_got->contents + off);
		      /* Note that this is harmless for the GOTPLT64 case,
		         as -1 | 1 still is -1.  */
		      h->got.offset |= 1;
		    }
		}
	      else
		unresolved_reloc = FALSE;
	    }
	  else
	    {
	      if (local_got_offsets == NULL)
		abort ();

	      off = local_got_offsets[r_symndx];

	      /* The offset must always be a multiple of 8.  We use
		 the least significant bit to record whether we have
		 already generated the necessary reloc.	 */
	      if ((off & 1) != 0)
		off &= ~1;
	      else
		{
		  bfd_put_64 (output_bfd, relocation,
			      base_got->contents + off);

		  if (info->shared)
		    {
		      asection *s;
		      Elf_Internal_Rela outrel;

		      /* We need to generate a R_X86_64_RELATIVE reloc
			 for the dynamic linker.  */
		      s = htab->elf.srelgot;
		      if (s == NULL)
			abort ();

		      outrel.r_offset = (base_got->output_section->vma
					 + base_got->output_offset
					 + off);
		      outrel.r_info = htab->r_info (0, R_X86_64_RELATIVE);
		      outrel.r_addend = relocation;
		      elf_append_rela (output_bfd, s, &outrel);
		    }

		  local_got_offsets[r_symndx] |= 1;
		}
	    }

	  if (off >= (bfd_vma) -2)
	    abort ();

	  relocation = base_got->output_section->vma
		       + base_got->output_offset + off;
	  if (r_type != R_X86_64_GOTPCREL && r_type != R_X86_64_GOTPCREL64)
	    relocation -= htab->elf.sgotplt->output_section->vma
			  - htab->elf.sgotplt->output_offset;

	  break;

	case R_X86_64_GOTOFF64:
	  /* Relocation is relative to the start of the global offset
	     table.  */

	  /* Check to make sure it isn't a protected function symbol
	     for shared library since it may not be local when used
	     as function address.  */
	  if (info->shared
	      && h
	      && h->def_regular
	      && h->type == STT_FUNC
	      && ELF_ST_VISIBILITY (h->other) == STV_PROTECTED)
	    {
	      (*_bfd_error_handler)
		(_("%B: relocation R_X86_64_GOTOFF64 against protected function `%s' can not be used when making a shared object"),
		 input_bfd, h->root.root.string);
	      bfd_set_error (bfd_error_bad_value);
	      return FALSE;
	    }

	  /* Note that sgot is not involved in this
	     calculation.  We always want the start of .got.plt.  If we
	     defined _GLOBAL_OFFSET_TABLE_ in a different way, as is
	     permitted by the ABI, we might have to change this
	     calculation.  */
	  relocation -= htab->elf.sgotplt->output_section->vma
			+ htab->elf.sgotplt->output_offset;
	  break;

	case R_X86_64_GOTPC32:
	case R_X86_64_GOTPC64:
	  /* Use global offset table as symbol value.  */
	  relocation = htab->elf.sgotplt->output_section->vma
		       + htab->elf.sgotplt->output_offset;
	  unresolved_reloc = FALSE;
	  break;

	case R_X86_64_PLTOFF64:
	  /* Relocation is PLT entry relative to GOT.  For local
	     symbols it's the symbol itself relative to GOT.  */
          if (h != NULL
	      /* See PLT32 handling.  */
	      && h->plt.offset != (bfd_vma) -1
	      && htab->elf.splt != NULL)
	    {
	      relocation = (htab->elf.splt->output_section->vma
			    + htab->elf.splt->output_offset
			    + h->plt.offset);
	      unresolved_reloc = FALSE;
	    }

	  relocation -= htab->elf.sgotplt->output_section->vma
			+ htab->elf.sgotplt->output_offset;
	  break;

	case R_X86_64_PLT32:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  /* Resolve a PLT32 reloc against a local symbol directly,
	     without using the procedure linkage table.	 */
	  if (h == NULL)
	    break;

	  if (h->plt.offset == (bfd_vma) -1
	      || htab->elf.splt == NULL)
	    {
	      /* We didn't make a PLT entry for this symbol.  This
		 happens when statically linking PIC code, or when
		 using -Bsymbolic.  */
	      break;
	    }

	  relocation = (htab->elf.splt->output_section->vma
			+ htab->elf.splt->output_offset
			+ h->plt.offset);
	  unresolved_reloc = FALSE;
	  break;

	case R_X86_64_PC8:
	case R_X86_64_PC16:
	case R_X86_64_PC32:
	  if (info->shared
	      && ABI_64_P (output_bfd)
	      && (input_section->flags & SEC_ALLOC) != 0
	      && (input_section->flags & SEC_READONLY) != 0
	      && h != NULL)
	    {
	      bfd_boolean fail = FALSE;
	      bfd_boolean branch
		= (r_type == R_X86_64_PC32
		   && is_32bit_relative_branch (contents, rel->r_offset));

	      if (SYMBOL_REFERENCES_LOCAL (info, h))
		{
		  /* Symbol is referenced locally.  Make sure it is
		     defined locally or for a branch.  */
		  fail = !h->def_regular && !branch;
		}
	      else
		{
		  /* Symbol isn't referenced locally.  We only allow
		     branch to symbol with non-default visibility. */
		  fail = (!branch
			  || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT);
		}

	      if (fail)
		{
		  const char *fmt;
		  const char *v;
		  const char *pic = "";

		  switch (ELF_ST_VISIBILITY (h->other))
		    {
		    case STV_HIDDEN:
		      v = _("hidden symbol");
		      break;
		    case STV_INTERNAL:
		      v = _("internal symbol");
		      break;
		    case STV_PROTECTED:
		      v = _("protected symbol");
		      break;
		    default:
		      v = _("symbol");
		      pic = _("; recompile with -fPIC");
		      break;
		    }

		  if (h->def_regular)
		    fmt = _("%B: relocation %s against %s `%s' can not be used when making a shared object%s");
		  else
		    fmt = _("%B: relocation %s against undefined %s `%s' can not be used when making a shared object%s");

		  (*_bfd_error_handler) (fmt, input_bfd,
					 x86_64_elf_howto_table[r_type].name,
					 v,  h->root.root.string, pic);
		  bfd_set_error (bfd_error_bad_value);
		  return FALSE;
		}
	    }
	  /* Fall through.  */

	case R_X86_64_8:
	case R_X86_64_16:
	case R_X86_64_32:
	case R_X86_64_PC64:
	case R_X86_64_64:
	  /* FIXME: The ABI says the linker should make sure the value is
	     the same when it's zeroextended to 64 bit.	 */

	  if ((input_section->flags & SEC_ALLOC) == 0)
	    break;

	  if ((info->shared
	       && (h == NULL
		   || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		   || h->root.type != bfd_link_hash_undefweak)
	       && (! IS_X86_64_PCREL_TYPE (r_type)
		   || ! SYMBOL_CALLS_LOCAL (info, h)))
	      || (ELIMINATE_COPY_RELOCS
		  && !info->shared
		  && h != NULL
		  && h->dynindx != -1
		  && !h->non_got_ref
		  && ((h->def_dynamic
		       && !h->def_regular)
		      || h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined)))
	    {
	      Elf_Internal_Rela outrel;
	      bfd_boolean skip, relocate;
	      asection *sreloc;

	      /* When generating a shared object, these relocations
		 are copied into the output file to be resolved at run
		 time.	*/
	      skip = FALSE;
	      relocate = FALSE;

	      outrel.r_offset =
		_bfd_elf_section_offset (output_bfd, info, input_section,
					 rel->r_offset);
	      if (outrel.r_offset == (bfd_vma) -1)
		skip = TRUE;
	      else if (outrel.r_offset == (bfd_vma) -2)
		skip = TRUE, relocate = TRUE;

	      outrel.r_offset += (input_section->output_section->vma
				  + input_section->output_offset);

	      if (skip)
		memset (&outrel, 0, sizeof outrel);

	      /* h->dynindx may be -1 if this symbol was marked to
		 become local.  */
	      else if (h != NULL
		       && h->dynindx != -1
		       && (IS_X86_64_PCREL_TYPE (r_type)
			   || ! info->shared
			   || ! SYMBOLIC_BIND (info, h)
			   || ! h->def_regular))
		{
		  outrel.r_info = htab->r_info (h->dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  /* This symbol is local, or marked to become local.  */
		  if (r_type == htab->pointer_r_type)
		    {
		      relocate = TRUE;
		      outrel.r_info = htab->r_info (0, R_X86_64_RELATIVE);
		      outrel.r_addend = relocation + rel->r_addend;
		    }
		  else
		    {
		      long sindx;

		      if (bfd_is_abs_section (sec))
			sindx = 0;
		      else if (sec == NULL || sec->owner == NULL)
			{
			  bfd_set_error (bfd_error_bad_value);
			  return FALSE;
			}
		      else
			{
			  asection *osec;

			  /* We are turning this relocation into one
			     against a section symbol.  It would be
			     proper to subtract the symbol's value,
			     osec->vma, from the emitted reloc addend,
			     but ld.so expects buggy relocs.  */
			  osec = sec->output_section;
			  sindx = elf_section_data (osec)->dynindx;
			  if (sindx == 0)
			    {
			      asection *oi = htab->elf.text_index_section;
			      sindx = elf_section_data (oi)->dynindx;
			    }
			  BFD_ASSERT (sindx != 0);
			}

		      outrel.r_info = htab->r_info (sindx, r_type);
		      outrel.r_addend = relocation + rel->r_addend;
		    }
		}

	      sreloc = elf_section_data (input_section)->sreloc;

	      BFD_ASSERT (sreloc != NULL && sreloc->contents != NULL);

	      elf_append_rela (output_bfd, sreloc, &outrel);

	      /* If this reloc is against an external symbol, we do
		 not want to fiddle with the addend.  Otherwise, we
		 need to include the symbol value so that it becomes
		 an addend for the dynamic reloc.  */
	      if (! relocate)
		continue;
	    }

	  break;

	case R_X86_64_TLSGD:
	case R_X86_64_GOTPC32_TLSDESC:
	case R_X86_64_TLSDESC_CALL:
	case R_X86_64_GOTTPOFF:
	  tls_type = GOT_UNKNOWN;
	  if (h == NULL && local_got_offsets)
	    tls_type = elf_x86_64_local_got_tls_type (input_bfd) [r_symndx];
	  else if (h != NULL)
	    tls_type = elf_x86_64_hash_entry (h)->tls_type;

	  if (! elf_x86_64_tls_transition (info, input_bfd,
					   input_section, contents,
					   symtab_hdr, sym_hashes,
					   &r_type, tls_type, rel,
					   relend, h, r_symndx))
	    return FALSE;

	  if (r_type == R_X86_64_TPOFF32)
	    {
	      bfd_vma roff = rel->r_offset;

	      BFD_ASSERT (! unresolved_reloc);

	      if (ELF32_R_TYPE (rel->r_info) == R_X86_64_TLSGD)
		{
		  /* GD->LE transition.
		     .byte 0x66; leaq foo@tlsgd(%rip), %rdi
		     .word 0x6666; rex64; call __tls_get_addr
		     Change it into:
		     movq %fs:0, %rax
		     leaq foo@tpoff(%rax), %rax */
		  memcpy (contents + roff - 4,
			  "\x64\x48\x8b\x04\x25\0\0\0\0\x48\x8d\x80\0\0\0",
			  16);
		  bfd_put_32 (output_bfd,
			      elf_x86_64_tpoff (info, relocation),
			      contents + roff + 8);
		  /* Skip R_X86_64_PC32/R_X86_64_PLT32.  */
		  rel++;
		  continue;
		}
	      else if (ELF32_R_TYPE (rel->r_info) == R_X86_64_GOTPC32_TLSDESC)
		{
		  /* GDesc -> LE transition.
		     It's originally something like:
		     leaq x@tlsdesc(%rip), %rax

		     Change it to:
		     movl $x@tpoff, %rax.  */

		  unsigned int val, type;

		  type = bfd_get_8 (input_bfd, contents + roff - 3);
		  val = bfd_get_8 (input_bfd, contents + roff - 1);
		  bfd_put_8 (output_bfd, 0x48 | ((type >> 2) & 1),
			     contents + roff - 3);
		  bfd_put_8 (output_bfd, 0xc7, contents + roff - 2);
		  bfd_put_8 (output_bfd, 0xc0 | ((val >> 3) & 7),
			     contents + roff - 1);
		  bfd_put_32 (output_bfd,
			      elf_x86_64_tpoff (info, relocation),
			      contents + roff);
		  continue;
		}
	      else if (ELF32_R_TYPE (rel->r_info) == R_X86_64_TLSDESC_CALL)
		{
		  /* GDesc -> LE transition.
		     It's originally:
		     call *(%rax)
		     Turn it into:
		     xchg %ax,%ax.  */
		  bfd_put_8 (output_bfd, 0x66, contents + roff);
		  bfd_put_8 (output_bfd, 0x90, contents + roff + 1);
		  continue;
		}
	      else if (ELF32_R_TYPE (rel->r_info) == R_X86_64_GOTTPOFF)
		{
		  /* IE->LE transition:
		     Originally it can be one of:
		     movq foo@gottpoff(%rip), %reg
		     addq foo@gottpoff(%rip), %reg
		     We change it into:
		     movq $foo, %reg
		     leaq foo(%reg), %reg
		     addq $foo, %reg.  */

		  unsigned int val, type, reg;

		  val = bfd_get_8 (input_bfd, contents + roff - 3);
		  type = bfd_get_8 (input_bfd, contents + roff - 2);
		  reg = bfd_get_8 (input_bfd, contents + roff - 1);
		  reg >>= 3;
		  if (type == 0x8b)
		    {
		      /* movq */
		      if (val == 0x4c)
			bfd_put_8 (output_bfd, 0x49,
				   contents + roff - 3);
		      bfd_put_8 (output_bfd, 0xc7,
				 contents + roff - 2);
		      bfd_put_8 (output_bfd, 0xc0 | reg,
				 contents + roff - 1);
		    }
		  else if (reg == 4)
		    {
		      /* addq -> addq - addressing with %rsp/%r12 is
			 special  */
		      if (val == 0x4c)
			bfd_put_8 (output_bfd, 0x49,
				   contents + roff - 3);
		      bfd_put_8 (output_bfd, 0x81,
				 contents + roff - 2);
		      bfd_put_8 (output_bfd, 0xc0 | reg,
				 contents + roff - 1);
		    }
		  else
		    {
		      /* addq -> leaq */
		      if (val == 0x4c)
			bfd_put_8 (output_bfd, 0x4d,
				   contents + roff - 3);
		      bfd_put_8 (output_bfd, 0x8d,
				 contents + roff - 2);
		      bfd_put_8 (output_bfd, 0x80 | reg | (reg << 3),
				 contents + roff - 1);
		    }
		  bfd_put_32 (output_bfd,
			      elf_x86_64_tpoff (info, relocation),
			      contents + roff);
		  continue;
		}
	      else
		BFD_ASSERT (FALSE);
	    }

	  if (htab->elf.sgot == NULL)
	    abort ();

	  if (h != NULL)
	    {
	      off = h->got.offset;
	      offplt = elf_x86_64_hash_entry (h)->tlsdesc_got;
	    }
	  else
	    {
	      if (local_got_offsets == NULL)
		abort ();

	      off = local_got_offsets[r_symndx];
	      offplt = local_tlsdesc_gotents[r_symndx];
	    }

	  if ((off & 1) != 0)
	    off &= ~1;
	  else
	    {
	      Elf_Internal_Rela outrel;
	      int dr_type, indx;
	      asection *sreloc;

	      if (htab->elf.srelgot == NULL)
		abort ();

	      indx = h && h->dynindx != -1 ? h->dynindx : 0;

	      if (GOT_TLS_GDESC_P (tls_type))
		{
		  outrel.r_info = htab->r_info (indx, R_X86_64_TLSDESC);
		  BFD_ASSERT (htab->sgotplt_jump_table_size + offplt
			      + 2 * GOT_ENTRY_SIZE <= htab->elf.sgotplt->size);
		  outrel.r_offset = (htab->elf.sgotplt->output_section->vma
				     + htab->elf.sgotplt->output_offset
				     + offplt
				     + htab->sgotplt_jump_table_size);
		  sreloc = htab->elf.srelplt;
		  if (indx == 0)
		    outrel.r_addend = relocation - elf_x86_64_dtpoff_base (info);
		  else
		    outrel.r_addend = 0;
		  elf_append_rela (output_bfd, sreloc, &outrel);
		}

	      sreloc = htab->elf.srelgot;

	      outrel.r_offset = (htab->elf.sgot->output_section->vma
				 + htab->elf.sgot->output_offset + off);

	      if (GOT_TLS_GD_P (tls_type))
		dr_type = R_X86_64_DTPMOD64;
	      else if (GOT_TLS_GDESC_P (tls_type))
		goto dr_done;
	      else
		dr_type = R_X86_64_TPOFF64;

	      bfd_put_64 (output_bfd, 0, htab->elf.sgot->contents + off);
	      outrel.r_addend = 0;
	      if ((dr_type == R_X86_64_TPOFF64
		   || dr_type == R_X86_64_TLSDESC) && indx == 0)
		outrel.r_addend = relocation - elf_x86_64_dtpoff_base (info);
	      outrel.r_info = htab->r_info (indx, dr_type);

	      elf_append_rela (output_bfd, sreloc, &outrel);

	      if (GOT_TLS_GD_P (tls_type))
		{
		  if (indx == 0)
		    {
		      BFD_ASSERT (! unresolved_reloc);
		      bfd_put_64 (output_bfd,
				  relocation - elf_x86_64_dtpoff_base (info),
				  htab->elf.sgot->contents + off + GOT_ENTRY_SIZE);
		    }
		  else
		    {
		      bfd_put_64 (output_bfd, 0,
				  htab->elf.sgot->contents + off + GOT_ENTRY_SIZE);
		      outrel.r_info = htab->r_info (indx,
						    R_X86_64_DTPOFF64);
		      outrel.r_offset += GOT_ENTRY_SIZE;
		      elf_append_rela (output_bfd, sreloc,
						&outrel);
		    }
		}

	    dr_done:
	      if (h != NULL)
		h->got.offset |= 1;
	      else
		local_got_offsets[r_symndx] |= 1;
	    }

	  if (off >= (bfd_vma) -2
	      && ! GOT_TLS_GDESC_P (tls_type))
	    abort ();
	  if (r_type == ELF32_R_TYPE (rel->r_info))
	    {
	      if (r_type == R_X86_64_GOTPC32_TLSDESC
		  || r_type == R_X86_64_TLSDESC_CALL)
		relocation = htab->elf.sgotplt->output_section->vma
		  + htab->elf.sgotplt->output_offset
		  + offplt + htab->sgotplt_jump_table_size;
	      else
		relocation = htab->elf.sgot->output_section->vma
		  + htab->elf.sgot->output_offset + off;
	      unresolved_reloc = FALSE;
	    }
	  else
	    {
	      bfd_vma roff = rel->r_offset;

	      if (ELF32_R_TYPE (rel->r_info) == R_X86_64_TLSGD)
		{
		  /* GD->IE transition.
		     .byte 0x66; leaq foo@tlsgd(%rip), %rdi
		     .word 0x6666; rex64; call __tls_get_addr@plt
		     Change it into:
		     movq %fs:0, %rax
		     addq foo@gottpoff(%rip), %rax */
		  memcpy (contents + roff - 4,
			  "\x64\x48\x8b\x04\x25\0\0\0\0\x48\x03\x05\0\0\0",
			  16);

		  relocation = (htab->elf.sgot->output_section->vma
				+ htab->elf.sgot->output_offset + off
				- roff
				- input_section->output_section->vma
				- input_section->output_offset
				- 12);
		  bfd_put_32 (output_bfd, relocation,
			      contents + roff + 8);
		  /* Skip R_X86_64_PLT32.  */
		  rel++;
		  continue;
		}
	      else if (ELF32_R_TYPE (rel->r_info) == R_X86_64_GOTPC32_TLSDESC)
		{
		  /* GDesc -> IE transition.
		     It's originally something like:
		     leaq x@tlsdesc(%rip), %rax

		     Change it to:
		     movq x@gottpoff(%rip), %rax # before xchg %ax,%ax.  */

		  /* Now modify the instruction as appropriate. To
		     turn a leaq into a movq in the form we use it, it
		     suffices to change the second byte from 0x8d to
		     0x8b.  */
		  bfd_put_8 (output_bfd, 0x8b, contents + roff - 2);

		  bfd_put_32 (output_bfd,
			      htab->elf.sgot->output_section->vma
			      + htab->elf.sgot->output_offset + off
			      - rel->r_offset
			      - input_section->output_section->vma
			      - input_section->output_offset
			      - 4,
			      contents + roff);
		  continue;
		}
	      else if (ELF32_R_TYPE (rel->r_info) == R_X86_64_TLSDESC_CALL)
		{
		  /* GDesc -> IE transition.
		     It's originally:
		     call *(%rax)

		     Change it to:
		     xchg %ax, %ax.  */

		  bfd_put_8 (output_bfd, 0x66, contents + roff);
		  bfd_put_8 (output_bfd, 0x90, contents + roff + 1);
		  continue;
		}
	      else
		BFD_ASSERT (FALSE);
	    }
	  break;

	case R_X86_64_TLSLD:
	  if (! elf_x86_64_tls_transition (info, input_bfd,
					   input_section, contents,
					   symtab_hdr, sym_hashes,
					   &r_type, GOT_UNKNOWN,
					   rel, relend, h, r_symndx))
	    return FALSE;

	  if (r_type != R_X86_64_TLSLD)
	    {
	      /* LD->LE transition:
		 leaq foo@tlsld(%rip), %rdi; call __tls_get_addr.
		 We change it into:
		 .word 0x6666; .byte 0x66; movl %fs:0, %rax.  */

	      BFD_ASSERT (r_type == R_X86_64_TPOFF32);
	      memcpy (contents + rel->r_offset - 3,
		      "\x66\x66\x66\x64\x48\x8b\x04\x25\0\0\0", 12);
	      /* Skip R_X86_64_PC32/R_X86_64_PLT32.  */
	      rel++;
	      continue;
	    }

	  if (htab->elf.sgot == NULL)
	    abort ();

	  off = htab->tls_ld_got.offset;
	  if (off & 1)
	    off &= ~1;
	  else
	    {
	      Elf_Internal_Rela outrel;

	      if (htab->elf.srelgot == NULL)
		abort ();

	      outrel.r_offset = (htab->elf.sgot->output_section->vma
				 + htab->elf.sgot->output_offset + off);

	      bfd_put_64 (output_bfd, 0,
			  htab->elf.sgot->contents + off);
	      bfd_put_64 (output_bfd, 0,
			  htab->elf.sgot->contents + off + GOT_ENTRY_SIZE);
	      outrel.r_info = htab->r_info (0, R_X86_64_DTPMOD64);
	      outrel.r_addend = 0;
	      elf_append_rela (output_bfd, htab->elf.srelgot,
					&outrel);
	      htab->tls_ld_got.offset |= 1;
	    }
	  relocation = htab->elf.sgot->output_section->vma
		       + htab->elf.sgot->output_offset + off;
	  unresolved_reloc = FALSE;
	  break;

	case R_X86_64_DTPOFF32:
	  if (!info->executable|| (input_section->flags & SEC_CODE) == 0)
	    relocation -= elf_x86_64_dtpoff_base (info);
	  else
	    relocation = elf_x86_64_tpoff (info, relocation);
	  break;

	case R_X86_64_TPOFF32:
	  BFD_ASSERT (info->executable);
	  relocation = elf_x86_64_tpoff (info, relocation);
	  break;

	default:
	  break;
	}

      /* Dynamic relocs are not propagated for SEC_DEBUGGING sections
	 because such sections are not SEC_ALLOC and thus ld.so will
	 not process them.  */
      if (unresolved_reloc
	  && !((input_section->flags & SEC_DEBUGGING) != 0
	       && h->def_dynamic))
	(*_bfd_error_handler)
	  (_("%B(%A+0x%lx): unresolvable %s relocation against symbol `%s'"),
	   input_bfd,
	   input_section,
	   (long) rel->r_offset,
	   howto->name,
	   h->root.root.string);

do_relocation:
      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);

      if (r != bfd_reloc_ok)
	{
	  const char *name;

	  if (h != NULL)
	    name = h->root.root.string;
	  else
	    {
	      name = bfd_elf_string_from_elf_section (input_bfd,
						      symtab_hdr->sh_link,
						      sym->st_name);
	      if (name == NULL)
		return FALSE;
	      if (*name == '\0')
		name = bfd_section_name (input_bfd, sec);
	    }

	  if (r == bfd_reloc_overflow)
	    {
	      if (! ((*info->callbacks->reloc_overflow)
		     (info, (h ? &h->root : NULL), name, howto->name,
		      (bfd_vma) 0, input_bfd, input_section,
		      rel->r_offset)))
		return FALSE;
	    }
	  else
	    {
	      (*_bfd_error_handler)
		(_("%B(%A+0x%lx): reloc against `%s': error %d"),
		 input_bfd, input_section,
		 (long) rel->r_offset, name, (int) r);
	      return FALSE;
	    }
	}
    }

  return TRUE;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bfd_boolean
elf_x86_64_finish_dynamic_symbol (bfd *output_bfd,
				  struct bfd_link_info *info,
				  struct elf_link_hash_entry *h,
				  Elf_Internal_Sym *sym)
{
  struct elf_x86_64_link_hash_table *htab;

  htab = elf_x86_64_hash_table (info);
  if (htab == NULL)
    return FALSE;

  if (h->plt.offset != (bfd_vma) -1)
    {
      bfd_vma plt_index;
      bfd_vma got_offset;
      Elf_Internal_Rela rela;
      bfd_byte *loc;
      asection *plt, *gotplt, *relplt;
      const struct elf_backend_data *bed;

      /* When building a static executable, use .iplt, .igot.plt and
	 .rela.iplt sections for STT_GNU_IFUNC symbols.  */
      if (htab->elf.splt != NULL)
	{
	  plt = htab->elf.splt;
	  gotplt = htab->elf.sgotplt;
	  relplt = htab->elf.srelplt;
	}
      else
	{
	  plt = htab->elf.iplt;
	  gotplt = htab->elf.igotplt;
	  relplt = htab->elf.irelplt;
	}

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.	 */
      if ((h->dynindx == -1
	   && !((h->forced_local || info->executable)
		&& h->def_regular
		&& h->type == STT_GNU_IFUNC))
	  || plt == NULL
	  || gotplt == NULL
	  || relplt == NULL)
	abort ();

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.

	 Get the offset into the .got table of the entry that
	 corresponds to this function.	Each .got entry is GOT_ENTRY_SIZE
	 bytes. The first three are reserved for the dynamic linker.

	 For static executables, we don't reserve anything.  */

      if (plt == htab->elf.splt)
	{
	  plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;
	  got_offset = (plt_index + 3) * GOT_ENTRY_SIZE;
	}
      else
	{
	  plt_index = h->plt.offset / PLT_ENTRY_SIZE;
	  got_offset = plt_index * GOT_ENTRY_SIZE;
	}

      /* Fill in the entry in the procedure linkage table.  */
      memcpy (plt->contents + h->plt.offset, elf_x86_64_plt_entry,
	      PLT_ENTRY_SIZE);

      /* Insert the relocation positions of the plt section.  The magic
	 numbers at the end of the statements are the positions of the
	 relocations in the plt section.  */
      /* Put offset for jmp *name@GOTPCREL(%rip), since the
	 instruction uses 6 bytes, subtract this value.  */
      bfd_put_32 (output_bfd,
		      (gotplt->output_section->vma
		       + gotplt->output_offset
		       + got_offset
		       - plt->output_section->vma
		       - plt->output_offset
		       - h->plt.offset
		       - 6),
		  plt->contents + h->plt.offset + 2);

      /* Don't fill PLT entry for static executables.  */
      if (plt == htab->elf.splt)
	{
	  /* Put relocation index.  */
	  bfd_put_32 (output_bfd, plt_index,
		      plt->contents + h->plt.offset + 7);
	  /* Put offset for jmp .PLT0.  */
	  bfd_put_32 (output_bfd, - (h->plt.offset + PLT_ENTRY_SIZE),
		      plt->contents + h->plt.offset + 12);
	}

      /* Fill in the entry in the global offset table, initially this
	 points to the pushq instruction in the PLT which is at offset 6.  */
      bfd_put_64 (output_bfd, (plt->output_section->vma
			       + plt->output_offset
			       + h->plt.offset + 6),
		  gotplt->contents + got_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = (gotplt->output_section->vma
		       + gotplt->output_offset
		       + got_offset);
      if (h->dynindx == -1
	  || ((info->executable
	       || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
	      && h->def_regular
	      && h->type == STT_GNU_IFUNC))
	{
	  /* If an STT_GNU_IFUNC symbol is locally defined, generate
	     R_X86_64_IRELATIVE instead of R_X86_64_JUMP_SLOT.  */
	  rela.r_info = htab->r_info (0, R_X86_64_IRELATIVE);
	  rela.r_addend = (h->root.u.def.value
			   + h->root.u.def.section->output_section->vma
			   + h->root.u.def.section->output_offset);
	}
      else
	{
	  rela.r_info = htab->r_info (h->dynindx, R_X86_64_JUMP_SLOT);
	  rela.r_addend = 0;
	}

      bed = get_elf_backend_data (output_bfd);
      loc = relplt->contents + plt_index * bed->s->sizeof_rela;
      bed->s->swap_reloca_out (output_bfd, &rela, loc);

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value if there were any
	     relocations where pointer equality matters (this is a clue
	     for the dynamic linker, to make function pointer
	     comparisons work between an application and shared
	     library), otherwise set it to zero.  If a function is only
	     called from a binary, there is no need to slow down
	     shared libraries because of that.  */
	  sym->st_shndx = SHN_UNDEF;
	  if (!h->pointer_equality_needed)
	    sym->st_value = 0;
	}
    }

  if (h->got.offset != (bfd_vma) -1
      && ! GOT_TLS_GD_ANY_P (elf_x86_64_hash_entry (h)->tls_type)
      && elf_x86_64_hash_entry (h)->tls_type != GOT_TLS_IE)
    {
      Elf_Internal_Rela rela;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */
      if (htab->elf.sgot == NULL || htab->elf.srelgot == NULL)
	abort ();

      rela.r_offset = (htab->elf.sgot->output_section->vma
		       + htab->elf.sgot->output_offset
		       + (h->got.offset &~ (bfd_vma) 1));

      /* If this is a static link, or it is a -Bsymbolic link and the
	 symbol is defined locally or was forced to be local because
	 of a version file, we just want to emit a RELATIVE reloc.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (h->def_regular
	  && h->type == STT_GNU_IFUNC)
	{
	  if (info->shared)
	    {
	      /* Generate R_X86_64_GLOB_DAT.  */
	      goto do_glob_dat;
	    }
	  else
	    {
	      asection *plt;

	      if (!h->pointer_equality_needed)
		abort ();

	      /* For non-shared object, we can't use .got.plt, which
		 contains the real function addres if we need pointer
		 equality.  We load the GOT entry with the PLT entry.  */
	      plt = htab->elf.splt ? htab->elf.splt : htab->elf.iplt;
	      bfd_put_64 (output_bfd, (plt->output_section->vma
				       + plt->output_offset
				       + h->plt.offset),
			  htab->elf.sgot->contents + h->got.offset);
	      return TRUE;
	    }
	}
      else if (info->shared
	       && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  if (!h->def_regular)
	    return FALSE;
	  BFD_ASSERT((h->got.offset & 1) != 0);
	  rela.r_info = htab->r_info (0, R_X86_64_RELATIVE);
	  rela.r_addend = (h->root.u.def.value
			   + h->root.u.def.section->output_section->vma
			   + h->root.u.def.section->output_offset);
	}
      else
	{
	  BFD_ASSERT((h->got.offset & 1) == 0);
do_glob_dat:
	  bfd_put_64 (output_bfd, (bfd_vma) 0,
		      htab->elf.sgot->contents + h->got.offset);
	  rela.r_info = htab->r_info (h->dynindx, R_X86_64_GLOB_DAT);
	  rela.r_addend = 0;
	}

      elf_append_rela (output_bfd, htab->elf.srelgot, &rela);
    }

  if (h->needs_copy)
    {
      Elf_Internal_Rela rela;

      /* This symbol needs a copy reloc.  Set it up.  */

      if (h->dynindx == -1
	  || (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	  || htab->srelbss == NULL)
	abort ();

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = htab->r_info (h->dynindx, R_X86_64_COPY);
      rela.r_addend = 0;
      elf_append_rela (output_bfd, htab->srelbss, &rela);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  SYM may
     be NULL for local symbols.  */
  if (sym != NULL
      && (strcmp (h->root.root.string, "_DYNAMIC") == 0
	  || h == htab->elf.hgot))
    sym->st_shndx = SHN_ABS;

  return TRUE;
}

/* Finish up local dynamic symbol handling.  We set the contents of
   various dynamic sections here.  */

static bfd_boolean
elf_x86_64_finish_local_dynamic_symbol (void **slot, void *inf)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) *slot;
  struct bfd_link_info *info
    = (struct bfd_link_info *) inf; 

  return elf_x86_64_finish_dynamic_symbol (info->output_bfd,
					     info, h, NULL);
}

/* Used to decide how to sort relocs in an optimal manner for the
   dynamic linker, before writing them out.  */

static enum elf_reloc_type_class
elf_x86_64_reloc_type_class (const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_X86_64_RELATIVE:
      return reloc_class_relative;
    case R_X86_64_JUMP_SLOT:
      return reloc_class_plt;
    case R_X86_64_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Finish up the dynamic sections.  */

static bfd_boolean
elf_x86_64_finish_dynamic_sections (bfd *output_bfd,
				    struct bfd_link_info *info)
{
  struct elf_x86_64_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn;

  htab = elf_x86_64_hash_table (info);
  if (htab == NULL)
    return FALSE;

  dynobj = htab->elf.dynobj;
  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");

  if (htab->elf.dynamic_sections_created)
    {
      bfd_byte *dyncon, *dynconend;
      const struct elf_backend_data *bed;
      bfd_size_type sizeof_dyn;

      if (sdyn == NULL || htab->elf.sgot == NULL)
	abort ();

      bed = get_elf_backend_data (dynobj);
      sizeof_dyn = bed->s->sizeof_dyn;
      dyncon = sdyn->contents;
      dynconend = sdyn->contents + sdyn->size;
      for (; dyncon < dynconend; dyncon += sizeof_dyn)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  (*bed->s->swap_dyn_in) (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      continue;

	    case DT_PLTGOT:
	      s = htab->elf.sgotplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_JMPREL:
	      dyn.d_un.d_ptr = htab->elf.srelplt->output_section->vma;
	      break;

	    case DT_PLTRELSZ:
	      s = htab->elf.srelplt->output_section;
	      dyn.d_un.d_val = s->size;
	      break;

	    case DT_RELASZ:
	      /* The procedure linkage table relocs (DT_JMPREL) should
		 not be included in the overall relocs (DT_RELA).
		 Therefore, we override the DT_RELASZ entry here to
		 make it not include the JMPREL relocs.  Since the
		 linker script arranges for .rela.plt to follow all
		 other relocation sections, we don't have to worry
		 about changing the DT_RELA entry.  */
	      if (htab->elf.srelplt != NULL)
		{
		  s = htab->elf.srelplt->output_section;
		  dyn.d_un.d_val -= s->size;
		}
	      break;

	    case DT_TLSDESC_PLT:
	      s = htab->elf.splt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset
		+ htab->tlsdesc_plt;
	      break;

	    case DT_TLSDESC_GOT:
	      s = htab->elf.sgot;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset
		+ htab->tlsdesc_got;
	      break;
	    }

	  (*bed->s->swap_dyn_out) (output_bfd, &dyn, dyncon);
	}

      /* Fill in the special first entry in the procedure linkage table.  */
      if (htab->elf.splt && htab->elf.splt->size > 0)
	{
	  /* Fill in the first entry in the procedure linkage table.  */
	  memcpy (htab->elf.splt->contents, elf_x86_64_plt0_entry,
		  PLT_ENTRY_SIZE);
	  /* Add offset for pushq GOT+8(%rip), since the instruction
	     uses 6 bytes subtract this value.  */
	  bfd_put_32 (output_bfd,
		      (htab->elf.sgotplt->output_section->vma
		       + htab->elf.sgotplt->output_offset
		       + 8
		       - htab->elf.splt->output_section->vma
		       - htab->elf.splt->output_offset
		       - 6),
		      htab->elf.splt->contents + 2);
	  /* Add offset for jmp *GOT+16(%rip). The 12 is the offset to
	     the end of the instruction.  */
	  bfd_put_32 (output_bfd,
		      (htab->elf.sgotplt->output_section->vma
		       + htab->elf.sgotplt->output_offset
		       + 16
		       - htab->elf.splt->output_section->vma
		       - htab->elf.splt->output_offset
		       - 12),
		      htab->elf.splt->contents + 8);

	  elf_section_data (htab->elf.splt->output_section)->this_hdr.sh_entsize =
	    PLT_ENTRY_SIZE;

	  if (htab->tlsdesc_plt)
	    {
	      bfd_put_64 (output_bfd, (bfd_vma) 0,
			  htab->elf.sgot->contents + htab->tlsdesc_got);

	      memcpy (htab->elf.splt->contents + htab->tlsdesc_plt,
		      elf_x86_64_plt0_entry,
		      PLT_ENTRY_SIZE);

	      /* Add offset for pushq GOT+8(%rip), since the
		 instruction uses 6 bytes subtract this value.  */
	      bfd_put_32 (output_bfd,
			  (htab->elf.sgotplt->output_section->vma
			   + htab->elf.sgotplt->output_offset
			   + 8
			   - htab->elf.splt->output_section->vma
			   - htab->elf.splt->output_offset
			   - htab->tlsdesc_plt
			   - 6),
			  htab->elf.splt->contents + htab->tlsdesc_plt + 2);
	      /* Add offset for jmp *GOT+TDG(%rip), where TGD stands for
		 htab->tlsdesc_got. The 12 is the offset to the end of
		 the instruction.  */
	      bfd_put_32 (output_bfd,
			  (htab->elf.sgot->output_section->vma
			   + htab->elf.sgot->output_offset
			   + htab->tlsdesc_got
			   - htab->elf.splt->output_section->vma
			   - htab->elf.splt->output_offset
			   - htab->tlsdesc_plt
			   - 12),
			  htab->elf.splt->contents + htab->tlsdesc_plt + 8);
	    }
	}
    }

  if (htab->elf.sgotplt)
    {
      if (bfd_is_abs_section (htab->elf.sgotplt->output_section))
	{
	  (*_bfd_error_handler)
	    (_("discarded output section: `%A'"), htab->elf.sgotplt);
	  return FALSE;
	}

      /* Fill in the first three entries in the global offset table.  */
      if (htab->elf.sgotplt->size > 0)
	{
	  /* Set the first entry in the global offset table to the address of
	     the dynamic section.  */
	  if (sdyn == NULL)
	    bfd_put_64 (output_bfd, (bfd_vma) 0, htab->elf.sgotplt->contents);
	  else
	    bfd_put_64 (output_bfd,
			sdyn->output_section->vma + sdyn->output_offset,
			htab->elf.sgotplt->contents);
	  /* Write GOT[1] and GOT[2], needed for the dynamic linker.  */
	  bfd_put_64 (output_bfd, (bfd_vma) 0, htab->elf.sgotplt->contents + GOT_ENTRY_SIZE);
	  bfd_put_64 (output_bfd, (bfd_vma) 0, htab->elf.sgotplt->contents + GOT_ENTRY_SIZE*2);
	}

      elf_section_data (htab->elf.sgotplt->output_section)->this_hdr.sh_entsize =
	GOT_ENTRY_SIZE;
    }

  if (htab->elf.sgot && htab->elf.sgot->size > 0)
    elf_section_data (htab->elf.sgot->output_section)->this_hdr.sh_entsize
      = GOT_ENTRY_SIZE;

  /* Fill PLT and GOT entries for local STT_GNU_IFUNC symbols.  */
  htab_traverse (htab->loc_hash_table,
		 elf_x86_64_finish_local_dynamic_symbol,
		 info);

  return TRUE;
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
elf_x86_64_plt_sym_val (bfd_vma i, const asection *plt,
			const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + (i + 1) * PLT_ENTRY_SIZE;
}

/* Handle an x86-64 specific section when reading an object file.  This
   is called when elfcode.h finds a section with an unknown type.  */

static bfd_boolean
elf_x86_64_section_from_shdr (bfd *abfd,
				Elf_Internal_Shdr *hdr,
				const char *name,
				int shindex)
{
  if (hdr->sh_type != SHT_X86_64_UNWIND)
    return FALSE;

  if (! _bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return FALSE;

  return TRUE;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it to put SHN_X86_64_LCOMMON items in .lbss, instead
   of .bss.  */

static bfd_boolean
elf_x86_64_add_symbol_hook (bfd *abfd,
			    struct bfd_link_info *info,
			    Elf_Internal_Sym *sym,
			    const char **namep ATTRIBUTE_UNUSED,
			    flagword *flagsp ATTRIBUTE_UNUSED,
			    asection **secp,
			    bfd_vma *valp)
{
  asection *lcomm;

  switch (sym->st_shndx)
    {
    case SHN_X86_64_LCOMMON:
      lcomm = bfd_get_section_by_name (abfd, "LARGE_COMMON");
      if (lcomm == NULL)
	{
	  lcomm = bfd_make_section_with_flags (abfd,
					       "LARGE_COMMON",
					       (SEC_ALLOC
						| SEC_IS_COMMON
						| SEC_LINKER_CREATED));
	  if (lcomm == NULL)
	    return FALSE;
	  elf_section_flags (lcomm) |= SHF_X86_64_LARGE;
	}
      *secp = lcomm;
      *valp = sym->st_size;
      return TRUE;
    }

  if ((abfd->flags & DYNAMIC) == 0
      && ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
    elf_tdata (info->output_bfd)->has_ifunc_symbols = TRUE;

  return TRUE;
}


/* Given a BFD section, try to locate the corresponding ELF section
   index.  */

static bfd_boolean
elf_x86_64_elf_section_from_bfd_section (bfd *abfd ATTRIBUTE_UNUSED,
					 asection *sec, int *index_return)
{
  if (sec == &_bfd_elf_large_com_section)
    {
      *index_return = SHN_X86_64_LCOMMON;
      return TRUE;
    }
  return FALSE;
}

/* Process a symbol.  */

static void
elf_x86_64_symbol_processing (bfd *abfd ATTRIBUTE_UNUSED,
			      asymbol *asym)
{
  elf_symbol_type *elfsym = (elf_symbol_type *) asym;

  switch (elfsym->internal_elf_sym.st_shndx)
    {
    case SHN_X86_64_LCOMMON:
      asym->section = &_bfd_elf_large_com_section;
      asym->value = elfsym->internal_elf_sym.st_size;
      /* Common symbol doesn't set BSF_GLOBAL.  */
      asym->flags &= ~BSF_GLOBAL;
      break;
    }
}

static bfd_boolean
elf_x86_64_common_definition (Elf_Internal_Sym *sym)
{
  return (sym->st_shndx == SHN_COMMON
	  || sym->st_shndx == SHN_X86_64_LCOMMON);
}

static unsigned int
elf_x86_64_common_section_index (asection *sec)
{
  if ((elf_section_flags (sec) & SHF_X86_64_LARGE) == 0)
    return SHN_COMMON;
  else
    return SHN_X86_64_LCOMMON;
}

static asection *
elf_x86_64_common_section (asection *sec)
{
  if ((elf_section_flags (sec) & SHF_X86_64_LARGE) == 0)
    return bfd_com_section_ptr;
  else
    return &_bfd_elf_large_com_section;
}

static bfd_boolean
elf_x86_64_merge_symbol (struct bfd_link_info *info ATTRIBUTE_UNUSED,
			 struct elf_link_hash_entry **sym_hash ATTRIBUTE_UNUSED,
			 struct elf_link_hash_entry *h,
			 Elf_Internal_Sym *sym,
			 asection **psec,
			 bfd_vma *pvalue ATTRIBUTE_UNUSED,
			 unsigned int *pold_alignment ATTRIBUTE_UNUSED,
			 bfd_boolean *skip ATTRIBUTE_UNUSED,
			 bfd_boolean *override ATTRIBUTE_UNUSED,
			 bfd_boolean *type_change_ok ATTRIBUTE_UNUSED,
			 bfd_boolean *size_change_ok ATTRIBUTE_UNUSED,
			 bfd_boolean *newdef ATTRIBUTE_UNUSED,
			 bfd_boolean *newdyn,
			 bfd_boolean *newdyncommon ATTRIBUTE_UNUSED,
			 bfd_boolean *newweak ATTRIBUTE_UNUSED,
			 bfd *abfd ATTRIBUTE_UNUSED,
			 asection **sec,
			 bfd_boolean *olddef ATTRIBUTE_UNUSED,
			 bfd_boolean *olddyn,
			 bfd_boolean *olddyncommon ATTRIBUTE_UNUSED,
			 bfd_boolean *oldweak ATTRIBUTE_UNUSED,
			 bfd *oldbfd,
			 asection **oldsec)
{
  /* A normal common symbol and a large common symbol result in a
     normal common symbol.  We turn the large common symbol into a
     normal one.  */
  if (!*olddyn
      && h->root.type == bfd_link_hash_common
      && !*newdyn
      && bfd_is_com_section (*sec)
      && *oldsec != *sec)
    {
      if (sym->st_shndx == SHN_COMMON
	  && (elf_section_flags (*oldsec) & SHF_X86_64_LARGE) != 0)
	{
	  h->root.u.c.p->section
	    = bfd_make_section_old_way (oldbfd, "COMMON");
	  h->root.u.c.p->section->flags = SEC_ALLOC;
	}
      else if (sym->st_shndx == SHN_X86_64_LCOMMON
	       && (elf_section_flags (*oldsec) & SHF_X86_64_LARGE) == 0)
	*psec = *sec = bfd_com_section_ptr;
    }

  return TRUE;
}

static int
elf_x86_64_additional_program_headers (bfd *abfd,
				       struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  asection *s;
  int count = 0;

  /* Check to see if we need a large readonly segment.  */
  s = bfd_get_section_by_name (abfd, ".lrodata");
  if (s && (s->flags & SEC_LOAD))
    count++;

  /* Check to see if we need a large data segment.  Since .lbss sections
     is placed right after the .bss section, there should be no need for
     a large data segment just because of .lbss.  */
  s = bfd_get_section_by_name (abfd, ".ldata");
  if (s && (s->flags & SEC_LOAD))
    count++;

  return count;
}

/* Return TRUE if symbol should be hashed in the `.gnu.hash' section.  */

static bfd_boolean
elf_x86_64_hash_symbol (struct elf_link_hash_entry *h)
{
  if (h->plt.offset != (bfd_vma) -1
      && !h->def_regular
      && !h->pointer_equality_needed)
    return FALSE;

  return _bfd_elf_hash_symbol (h);
}

/* Return TRUE iff relocations for INPUT are compatible with OUTPUT. */

static bfd_boolean
elf_x86_64_relocs_compatible (const bfd_target *input,
			      const bfd_target *output)
{
  return ((xvec_get_elf_backend_data (input)->s->elfclass
	   == xvec_get_elf_backend_data (output)->s->elfclass)
	  && _bfd_elf_relocs_compatible (input, output));
}

static const struct bfd_elf_special_section
  elf_x86_64_special_sections[]=
{
  { STRING_COMMA_LEN (".gnu.linkonce.lb"), -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE + SHF_X86_64_LARGE},
  { STRING_COMMA_LEN (".gnu.linkonce.lr"), -2, SHT_PROGBITS, SHF_ALLOC + SHF_X86_64_LARGE},
  { STRING_COMMA_LEN (".gnu.linkonce.lt"), -2, SHT_PROGBITS, SHF_ALLOC + SHF_EXECINSTR + SHF_X86_64_LARGE},
  { STRING_COMMA_LEN (".lbss"),	           -2, SHT_NOBITS,   SHF_ALLOC + SHF_WRITE + SHF_X86_64_LARGE},
  { STRING_COMMA_LEN (".ldata"),	   -2, SHT_PROGBITS, SHF_ALLOC + SHF_WRITE + SHF_X86_64_LARGE},
  { STRING_COMMA_LEN (".lrodata"),	   -2, SHT_PROGBITS, SHF_ALLOC + SHF_X86_64_LARGE},
  { NULL,	                0,          0, 0,            0 }
};

#define TARGET_LITTLE_SYM		    bfd_elf64_x86_64_vec
#define TARGET_LITTLE_NAME		    "elf64-x86-64"
#define ELF_ARCH			    bfd_arch_i386
#define ELF_TARGET_ID			    X86_64_ELF_DATA
#define ELF_MACHINE_CODE		    EM_X86_64
#define ELF_MAXPAGESIZE			    0x200000
#define ELF_MINPAGESIZE			    0x1000
#define ELF_COMMONPAGESIZE		    0x1000

#define elf_backend_can_gc_sections	    1
#define elf_backend_can_refcount	    1
#define elf_backend_want_got_plt	    1
#define elf_backend_plt_readonly	    1
#define elf_backend_want_plt_sym	    0
#define elf_backend_got_header_size	    (GOT_ENTRY_SIZE*3)
#define elf_backend_rela_normal		    1

#define elf_info_to_howto		    elf_x86_64_info_to_howto

#define bfd_elf64_bfd_link_hash_table_create \
  elf_x86_64_link_hash_table_create
#define bfd_elf64_bfd_link_hash_table_free \
  elf_x86_64_link_hash_table_free
#define bfd_elf64_bfd_reloc_type_lookup	    elf_x86_64_reloc_type_lookup
#define bfd_elf64_bfd_reloc_name_lookup \
  elf_x86_64_reloc_name_lookup

#define elf_backend_adjust_dynamic_symbol   elf_x86_64_adjust_dynamic_symbol
#define elf_backend_relocs_compatible	    elf_x86_64_relocs_compatible
#define elf_backend_check_relocs	    elf_x86_64_check_relocs
#define elf_backend_copy_indirect_symbol    elf_x86_64_copy_indirect_symbol
#define elf_backend_create_dynamic_sections elf_x86_64_create_dynamic_sections
#define elf_backend_finish_dynamic_sections elf_x86_64_finish_dynamic_sections
#define elf_backend_finish_dynamic_symbol   elf_x86_64_finish_dynamic_symbol
#define elf_backend_gc_mark_hook	    elf_x86_64_gc_mark_hook
#define elf_backend_gc_sweep_hook	    elf_x86_64_gc_sweep_hook
#define elf_backend_grok_prstatus	    elf_x86_64_grok_prstatus
#define elf_backend_grok_psinfo		    elf_x86_64_grok_psinfo
#define elf_backend_reloc_type_class	    elf_x86_64_reloc_type_class
#define elf_backend_relocate_section	    elf_x86_64_relocate_section
#define elf_backend_size_dynamic_sections   elf_x86_64_size_dynamic_sections
#define elf_backend_always_size_sections    elf_x86_64_always_size_sections
#define elf_backend_init_index_section	    _bfd_elf_init_1_index_section
#define elf_backend_plt_sym_val		    elf_x86_64_plt_sym_val
#define elf_backend_object_p		    elf64_x86_64_elf_object_p
#define bfd_elf64_mkobject		    elf_x86_64_mkobject

#define elf_backend_section_from_shdr \
	elf_x86_64_section_from_shdr

#define elf_backend_section_from_bfd_section \
  elf_x86_64_elf_section_from_bfd_section
#define elf_backend_add_symbol_hook \
  elf_x86_64_add_symbol_hook
#define elf_backend_symbol_processing \
  elf_x86_64_symbol_processing
#define elf_backend_common_section_index \
  elf_x86_64_common_section_index
#define elf_backend_common_section \
  elf_x86_64_common_section
#define elf_backend_common_definition \
  elf_x86_64_common_definition
#define elf_backend_merge_symbol \
  elf_x86_64_merge_symbol
#define elf_backend_special_sections \
  elf_x86_64_special_sections
#define elf_backend_additional_program_headers \
  elf_x86_64_additional_program_headers
#define elf_backend_hash_symbol \
  elf_x86_64_hash_symbol

#undef  elf_backend_post_process_headers
#define elf_backend_post_process_headers  _bfd_elf_set_osabi

#include "elf64-target.h"

/* FreeBSD support.  */

#undef  TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		    bfd_elf64_x86_64_freebsd_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		    "elf64-x86-64-freebsd"

#undef	ELF_OSABI
#define	ELF_OSABI			    ELFOSABI_FREEBSD

#undef  elf64_bed
#define elf64_bed elf64_x86_64_fbsd_bed

#include "elf64-target.h"

/* Solaris 2 support.  */

#undef  TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		    bfd_elf64_x86_64_sol2_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		    "elf64-x86-64-sol2"

/* Restore default: we cannot use ELFOSABI_SOLARIS, otherwise ELFOSABI_NONE
   objects won't be recognized.  */
#undef ELF_OSABI

#undef  elf64_bed
#define elf64_bed			    elf64_x86_64_sol2_bed

/* The 64-bit static TLS arena size is rounded to the nearest 16-byte
   boundary.  */
#undef elf_backend_static_tls_alignment
#define elf_backend_static_tls_alignment    16

/* The Solaris 2 ABI requires a plt symbol on all platforms.

   Cf. Linker and Libraries Guide, Ch. 2, Link-Editor, Generating the Output
   File, p.63.  */
#undef elf_backend_want_plt_sym
#define elf_backend_want_plt_sym	    1

#include "elf64-target.h"

/* Intel L1OM support.  */

static bfd_boolean
elf64_l1om_elf_object_p (bfd *abfd)
{
  /* Set the right machine number for an L1OM elf64 file.  */
  bfd_default_set_arch_mach (abfd, bfd_arch_l1om, bfd_mach_l1om);
  return TRUE;
}

#undef  TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		    bfd_elf64_l1om_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		    "elf64-l1om"
#undef ELF_ARCH
#define ELF_ARCH			    bfd_arch_l1om

#undef	ELF_MACHINE_CODE
#define ELF_MACHINE_CODE		    EM_L1OM

#undef	ELF_OSABI

#undef  elf64_bed
#define elf64_bed elf64_l1om_bed

#undef elf_backend_object_p
#define elf_backend_object_p		    elf64_l1om_elf_object_p

#undef  elf_backend_post_process_headers
#undef  elf_backend_static_tls_alignment

#undef elf_backend_want_plt_sym
#define elf_backend_want_plt_sym	    0

#include "elf64-target.h"

/* FreeBSD L1OM support.  */

#undef  TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		    bfd_elf64_l1om_freebsd_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		    "elf64-l1om-freebsd"

#undef	ELF_OSABI
#define	ELF_OSABI			    ELFOSABI_FREEBSD

#undef  elf64_bed
#define elf64_bed elf64_l1om_fbsd_bed

#undef  elf_backend_post_process_headers
#define elf_backend_post_process_headers  _bfd_elf_set_osabi

#include "elf64-target.h"

/* 32bit x86-64 support.  */

static bfd_boolean
elf32_x86_64_elf_object_p (bfd *abfd)
{
  /* Set the right machine number for an x86-64 elf32 file.  */
  bfd_default_set_arch_mach (abfd, bfd_arch_i386, bfd_mach_x64_32);
  return TRUE;
}

#undef  TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		    bfd_elf32_x86_64_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		    "elf32-x86-64"

#undef ELF_ARCH
#define ELF_ARCH			    bfd_arch_i386

#undef	ELF_MACHINE_CODE
#define ELF_MACHINE_CODE		    EM_X86_64

#define bfd_elf32_bfd_link_hash_table_create \
  elf_x86_64_link_hash_table_create
#define bfd_elf32_bfd_link_hash_table_free \
  elf_x86_64_link_hash_table_free
#define bfd_elf32_bfd_reloc_type_lookup	\
  elf_x86_64_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup \
  elf_x86_64_reloc_name_lookup
#define bfd_elf32_mkobject \
  elf_x86_64_mkobject

#undef	ELF_OSABI

#undef elf_backend_post_process_headers

#undef elf_backend_object_p
#define elf_backend_object_p \
  elf32_x86_64_elf_object_p

#undef elf_backend_bfd_from_remote_memory
#define elf_backend_bfd_from_remote_memory \
  _bfd_elf32_bfd_from_remote_memory

#undef elf_backend_size_info
#define elf_backend_size_info \
  _bfd_elf32_size_info

#include "elf32-target.h"
