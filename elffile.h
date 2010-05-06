/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#ifndef _ELF_FILE_H_
#define _ELF_FILE_H_

#include <stdint.h>

/*
    Taken from ELF spec
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

typedef struct {
    ELF32_Char e_ident[EI_NIDENT];
    ELF32_Half e_type;
    ELF32_Half e_machine;
    ELF32_Word e_version;
    ELF32_Addr e_entry;
    ELF32_Off e_phoff;
    ELF32_Off e_shoff;
    ELF32_Word e_flags;
    ELF32_Half e_ehsize;
    ELF32_Half e_phentsize;
    ELF32_Half e_phnum;
    ELF32_Half e_shentsize;
    ELF32_Half e_shnum;
    ELF32_Half e_shstrndx;
} ELF_Header;

typedef struct {
    
    
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

/*
    Simple ELF file manipulation
*/
typedef struct {
    ELF_Header *header;
    
    ELF32_Word nsection;
    ELF_Section **sections;
    
    ELF32_Word nsegment;
    ELF_Segment **segments;
} ELF_File;

ELF_File* elf_file_create();
void elf_file_destroy(ELF_File *f);

void elf_file_load(ELF_File *f, const char *filename);

#endif
