/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#ifndef _ELF_FILE_H_
#define _ELF_FILE_H_

#include <stdint.h>

/*
    Most structures taken from ELF spec, with some slight
    adaptations to the purpose of this loader.
*/

typedef uint32_t ELF32_Addr;
typedef uint16_t ELF32_Half;
typedef uint32_t ELF32_Off;
typedef int32_t  ELF32_Sword;
typedef uint32_t ELF32_Word;
typedef uint8_t  ELF32_Char;

enum {
    EI_MAG0     = 0,
    EI_MAG1     = 1,
    EI_MAG2     = 2,
    EI_MAG3     = 3,
    EI_CLASS    = 4,
    EI_DATA     = 5,
    EI_VERSION  = 6,
    EI_PAD      = 7,
    EI_NIDENT   = 16
};

enum {
    ELFMAG0 = 0x7F,
    ELFMAG1 = 'E',
    ELFMAG2 = 'L',
    ELFMAG3 = 'F'
};

enum {
    ELFCLASSNONE = 0,
    ELFCLASS32   = 1,
    ELFCLASS64   = 2
};

enum {
    ELFDATANONE = 0,
    ELFDATA2LSB = 1,
    ELFDATA2MSB = 2
};

enum {
    ET_NONE   = 0,
    ET_REL    = 1,
    ET_EXEC   = 2,
    ET_DYN    = 3,
    ET_CORE   = 4,
    ET_LOPROC = 0xff00,
    ET_HIPROC = 0xffff
};

enum {
    EM_NONE  = 0,
    EM_M32   = 1,
    EM_SPARC = 2,
    EM_386   = 3,
    EM_68K   = 4,
    EM_88K   = 5,
    EM_860   = 7,
    EM_MIPS  = 8
};

enum {
    EV_NONE    = 0,
    EV_CURRENT = 1
};

enum {
    PT_NONE    = 0,
    PT_LOAD    = 1,
    PT_DYNAMIC = 2,
    PT_INTERP  = 3,
    PT_NOTE    = 4,
    PT_SHLIB   = 5,
    PT_PHDR    = 6,
    PT_LOPROC  = 0x70000000,
    PT_HIPROC  = 0x7FFFFFFF
};

enum {
    SHN_UNDEF     = 0,
    SHN_LORESERVE = 0xff00,
    SHN_LOPROC    = 0xff00,
    SHN_HIPROC    = 0xff1f,
    SHN_ABS       = 0xfff1,
    SHN_COMMON    = 0xfff2,
    SHN_HIRESERVE = 0xffff
};

enum {
    SHT_NULL     = 0,
    SHT_PROGBITS = 1,
    SHT_SYMTAB   = 2,
    SHT_STRTAB   = 3,
    SHT_RELA     = 4,
    SHT_HASH     = 5,
    SHT_DYNAMIC  = 6,
    SHT_NOTE     = 7,
    SHT_NOBITS   = 8,
    SHT_REL      = 9,
    SHT_SHLIB    = 10,
    SHT_DYNSYM   = 11,
    SHT_LOPROC   = 0x70000000,
    SHT_HIPROC   = 0x7FFFFFFF,
    SHT_LOUSER   = 0x80000000,
    SHT_HIUSER   = 0xFFFFFFFF
};

enum {
    SHF_WRITE     = 1,
    SHF_ALLOC     = 2,
    SHF_EXECINSTR = 4,
    SHF_MASKPROC  = 0xf0000000
};

typedef struct {
    ELF32_Char e_ident[EI_NIDENT];
    ELF32_Half e_type;
    ELF32_Half e_machine;
    ELF32_Word e_version;
    ELF32_Addr e_entry;
    ELF32_Off  e_phoff;
    ELF32_Off  e_shoff;
    ELF32_Word e_flags;
    ELF32_Half e_ehsize;
    ELF32_Half e_phentsize;
    ELF32_Half e_phnum;
    ELF32_Half e_shentsize;
    ELF32_Half e_shnum;
    ELF32_Half e_shstrndx;
} ELF_Header;

typedef struct {
    /* Section header */
    ELF32_Word s_name;
    ELF32_Word s_type;
    ELF32_Word s_flags;
    ELF32_Addr s_addr;
    ELF32_Off  s_offset;
    ELF32_Word s_size;
    ELF32_Word s_link;
    ELF32_Word s_info;
    ELF32_Word s_addralign;
    ELF32_Word s_entsize;
    
    /* data (might be null or widely different from ELF file representation) */
    ELF32_Char *s_data;
    ELF32_Word s_reloc;
} ELF_Section;

typedef struct {
    /* Program header */
    ELF32_Word p_type;
    ELF32_Off  p_offset;
    ELF32_Addr p_vaddr;
    ELF32_Addr p_paddr;
    ELF32_Word p_filesz;
    ELF32_Word p_memsz;
    ELF32_Word p_flags;
    ELF32_Word p_align;
    
    /* data (copied from file to memory) */
    ELF32_Char *p_data;
} ELF_Segment;

enum {
    STB_LOCAL   = 0,
    STB_GLOBAL  = 1,
    STB_WEAK    = 2,
    STB_LOPROC  = 13,
    STB_HIPROC  = 15
};

enum {
    STT_NOTYPE  = 0,
    STT_OBJECT  = 1,
    STT_FUNC    = 2,
    STT_SECTION = 3,
    STT_FILE    = 4,
    STT_LOPROC  = 13,
    STT_HIPROC  = 15
};

typedef struct {
    ELF32_Word s_name;
    ELF32_Addr s_value;
    ELF32_Word s_size;
    ELF32_Char s_info;
    ELF32_Char s_other;
    ELF32_Half s_shndx;
} ELF_Sym;

#define ELF32_ST_BIND(i)   ((i)>>4)
#define ELF32_ST_TYPE(i)   ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

enum {
    R_MIPS_NONE     = 0,
    R_MIPS_16       = 1,
    R_MIPS_32       = 2,
    R_MIPS_REL32    = 3,
    R_MIPS_26       = 4,
    R_MIPS_HI16     = 5,
    R_MIPS_LO16     = 6,
    
    R_MIPS_GPREL16  = 7,
    R_MIPS_LITERAL  = 8,
    R_MIPS_GOT16    = 9,
    R_MIPS_PC16     = 10,
    R_MIPS_CALL16   = 11,
    R_MIPS_GPREL32  = 12,
    R_MIPS_GOTHI16  = 21,
    R_MIPS_GOTLO16  = 22,
    R_MIPS_CALLHI16 = 30,
    R_MIPS_CALLLO16 = 31,
};

typedef struct {
    ELF32_Addr r_offset;
    ELF32_Word r_info;
} ELF_Rel;

typedef struct {
    ELF32_Addr  r_offset;
    ELF32_Word  r_info;
    ELF32_Sword r_addend;
} ELF_Rela;

#define ELF32_R_SYM(i)    ((i)>>8)
#define ELF32_R_TYPE(i)   ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

/*
    Simple ELF file manipulation
*/
typedef struct {
    ELF_Header *header;
    
    ELF32_Word nsection;
    ELF_Section **sections;
    
    ELF32_Word nsegment;
    ELF_Segment **segments;
    
    ELF_Section *shstrtab;
} ELF_File;

ELF_File* elf_file_create();
void elf_file_destroy(ELF_File *elf);

const char* elf_string(ELF_File *elf, ELF32_Word strtab, ELF32_Word stridx);
const char* elf_section_name(ELF_File *elf, ELF32_Word n, ELF32_Word *size);

int elf_file_load(ELF_File *elf, const char *filename);

int elf_file_relocate(ELF_File *elf);

ELF_Section* elf_section(ELF_File *elf, const char *name);

const char* elf_symbol_name(ELF_File *elf, ELF32_Addr value, int *stat);

#endif
