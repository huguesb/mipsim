/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "elffile.h"

#include <stdio.h>
#include <stdlib.h>

enum {
    ELFMAG0 = 0x7F,
    ELFMAG1 = 'E',
    ELFMAG2 = 'L',
    ELFMAG3 = 'F'
};

enum {
    ELFCLASSNONE = 0,
    ELFCLASS32 = 1,
    ELFCLASS64 = 2
};

enum {
    ELFDATANONE = 0,
    ELFDATA2LSB = 1,
    ELFDATA2MSB = 2
};

enum {
    ET_NONE = 0,
    ET_REL = 1,
    ET_EXEC = 2,
    ET_DYN = 3,
    ET_CORE = 4,
    ET_LOPROC = 0xff00,
    ET_HIPROC = 0xffff
};

enum {
    EM_NONE = 0,
    EM_M32 = 1,
    EM_SPARC = 2,
    EM_386 = 3,
    EM_68K = 4,
    EM_88K = 5,
    EM_860 = 7,
    EM_MIPS = 8
};

enum {
    EV_NONE = 0,
    EV_CURRENT = 1
};

enum {
    PT_NONE = 0,
    PT_LOAD = 1,
    PT_DYNAMIC = 2,
    PT_INTERP = 3,
    PT_NOTE = 4,
    PT_SHLIB = 5,
    PT_PHDR = 6,
    PT_LOPROC = 0x70000000,
    PT_HIPROC = 0x7FFFFFFF
};

ELF32_Half elf_fget_half(FILE *f, ELF32_Char endian)
{
    ELF32_Char c0 = fgetc(f);
    ELF32_Char c1 = fgetc(f);
    
    return endian == ELFDATA2MSB ?
            (c0 << 8) | c1 :
            (c1 << 8) | c0;
}


ELF32_Word elf_fget_word(FILE *f, ELF32_Char endian)
{
    ELF32_Char c0 = fgetc(f);
    ELF32_Char c1 = fgetc(f);
    ELF32_Char c2 = fgetc(f);
    ELF32_Char c3 = fgetc(f);
    
    return endian == ELFDATA2MSB ?
            (c0 << 24) | (c1 << 16) | (c2 << 8) | c3 :
            (c3 << 24) | (c2 << 16) | (c1 << 8) | c0;
}

ELF_File* elf_file_create()
{
    ELF_File *f = (ELF_File*)malloc(sizeof(ELF_File));
    
    if ( f )
    {
        f->header = NULL;
        f->nsection = 0;
        f->nsegment = 0;
        f->sections = NULL;
        f->segments = NULL;
    }
    
    return f;
}

void elf_file_cleanup(ELF_File *f)
{
    /* free section data */
    for ( ELF32_Word i = 0; i < f->nsection; ++i )
    {
        
        free(f->sections[i]);
    }
    
    /* free segment data */
    for ( ELF32_Word i = 0; i < f->nsegment; ++i )
    {
        if ( f->segments[i] )
        {
            free(f->segments[i]->p_data);
            free(f->segments[i]);
        }
    }
    
    /* free top level */
    free(f->segments);
    free(f->sections);
    free(f->header);
    
    /* NULL-ify */
    f->header = NULL;
    f->nsection = 0;
    f->nsegment = 0;
    f->sections = NULL;
    f->segments = NULL;
}

int elf_file_load_header(ELF_Header *hdr, FILE *handle, const char *filename)
{
    if ( fread(hdr->e_ident, sizeof(ELF32_Char), EI_NIDENT, handle) != (sizeof(ELF32_Char) * EI_NIDENT) )
    {
        printf("ELF:%s: Invalid header : failed to read identifier\n", filename);
        return 1;
    }
    
    if ( hdr->e_ident[EI_MAG0] != ELFMAG0 ||
         hdr->e_ident[EI_MAG1] != ELFMAG1 ||
         hdr->e_ident[EI_MAG2] != ELFMAG2 ||
         hdr->e_ident[EI_MAG3] != ELFMAG3 )
    {
        printf("ELF:%s: Invalid header : wrong magic number\n", filename);
        return 1;
    }
    
    if ( hdr->e_ident[EI_CLASS] != ELFCLASS32 )
    {
        printf("ELF:%s: Invalid header : expected class %i, found %i\n", filename, ELFCLASS32, hdr->e_ident[EI_CLASS]);
        return 1;;
    }
    
    if ( hdr->e_ident[EI_VERSION] != EV_CURRENT )
    {
        printf("ELF:%s: Invalid header : expected version %i, found %i\n", filename, EV_CURRENT, hdr->e_ident[EI_VERSION]);
        return 1;
    }
    
    const ELF32_Char endian = hdr->e_ident[EI_DATA];
    
    if ( endian != ELFDATA2LSB && endian != ELFDATA2MSB )
    {
        printf("ELF:%s: Invalid header : unknown data format %2x", filename, endian);
        return 1;
    }
    
    hdr->e_type      = elf_fget_half(handle, endian);
    hdr->e_machine   = elf_fget_half(handle, endian);
    hdr->e_version   = elf_fget_word(handle, endian);
    hdr->e_entry     = elf_fget_word(handle, endian);
    hdr->e_phoff     = elf_fget_word(handle, endian);
    hdr->e_shoff     = elf_fget_word(handle, endian);
    hdr->e_flags     = elf_fget_word(handle, endian);
    hdr->e_ehsize    = elf_fget_half(handle, endian);
    hdr->e_phentsize = elf_fget_half(handle, endian);
    hdr->e_phnum     = elf_fget_half(handle, endian);
    hdr->e_shentsize = elf_fget_half(handle, endian);
    hdr->e_shnum     = elf_fget_half(handle, endian);
    hdr->e_shstrndx  = elf_fget_half(handle, endian);
    
    if ( hdr->e_version != EV_CURRENT )
    {
        printf("ELF:%s: Invalid header : expected version %i, found %i\n", filename, EV_CURRENT, hdr->e_version);
        return 1;
    }
    
    #define ELF_CHECK_EQU(s, val, expect) \
        if ( val != expect ) \
        { printf("ELF:%s: Invalid " s " expected %i, found %i\n", filename, expect, val); return 1; }
    #define ELF_CHECK_SUP(s, val, expect) \
        if ( val < expect ) \
        { printf("ELF:%s: Invalid " s " expected at least %i, found %i\n", filename, expect, val); return 1; }
    
    ELF_CHECK_EQU("type"   , hdr->e_type, ET_EXEC)
    ELF_CHECK_EQU("machine", hdr->e_machine, EM_MIPS)
    
    ELF_CHECK_SUP("header size", hdr->e_ehsize, sizeof(ELF_Header))
    
    if ( hdr->e_shoff )
        ELF_CHECK_SUP("section table offset", hdr->e_shoff, hdr->e_ehsize)
    if ( hdr->e_phoff )
        ELF_CHECK_SUP("program table offset", hdr->e_phoff, hdr->e_ehsize)
    
    ELF_CHECK_SUP("program header size", hdr->e_phentsize, 8 * sizeof(ELF32_Word))
    
    return 0;
}

int elf_file_load_segment(ELF_Segment *s, FILE *handle, int endian, ELF32_Off min_offset, const char *filename)
{
    s->p_type   = elf_fget_word(handle, endian);
    s->p_offset = elf_fget_word(handle, endian);
    s->p_vaddr  = elf_fget_word(handle, endian);
    s->p_paddr  = elf_fget_word(handle, endian);
    s->p_filesz = elf_fget_word(handle, endian);
    s->p_memsz  = elf_fget_word(handle, endian);
    s->p_flags  = elf_fget_word(handle, endian);
    s->p_align  = elf_fget_word(handle, endian);
    
    s->p_data   = NULL;
    
    if ( s->p_type == PT_LOAD )
    {
        if ( s->p_memsz < s->p_filesz )
        {
            printf("ELF:%s: Invalid file : suspicious segment size\n", filename);
            return 1;
        }
        
        if ( s->p_offset < min_offset )
        {
            printf("ELF:%s: Invalid file : suspicious segment offset\n", filename);
            return 1;
        }
        
        if ( fseek(handle, s->p_offset, SEEK_SET) )
        {
            printf("ELF:%s: Invalid file : unable to reach segment\n", filename);
            return 1;
        }
        
        s->p_data = (ELF32_Char*)calloc(s->p_memsz, sizeof(ELF32_Char));
        size_t sz = fread(s->p_data, sizeof(ELF32_Char), s->p_filesz, handle);
        
        if ( sz != s->p_filesz * sizeof(ELF32_Char) )
        {
            printf("ELF:%s: Invalid file : unable to read data of segment [%d vs %u]\n", filename, sz, s->p_filesz);
            free(s->p_data);
            s->p_data = NULL;
            return 1;
        }
        
        printf("ELF:%s: Succesfully loaded segment (%d bytes)\n", filename, s->p_filesz);
    } else {
        printf("ELF:%s: Skipped segment (type %8x)\n", filename, s->p_type);
        return 2;
    }
    
    return 0;
}

int elf_file_load_segments(ELF_File *elf, FILE *handle, const char *filename)
{
    if ( fseek(handle, elf->header->e_phoff, SEEK_SET) )
    {
        printf("ELF:%s: Invalid file : unable to locate program header table\n", filename);
        return 1;
    }
    
    if ( elf->header->e_phnum )
        elf->segments = (ELF_Segment**)malloc(elf->header->e_phnum * sizeof(ELF_Segment*));
    
    const ELF32_Char endian = elf->header->e_ident[EI_DATA];
    const ELF32_Off min_offset = elf->header->e_phoff + elf->header->e_phentsize * (elf->header->e_phnum + 1);
    
    if ( elf->segments )
    {
        elf->nsegment = 0;
        
        for ( ELF32_Word i = 0; i < elf->header->e_phnum; ++i )
        {
            ELF32_Word soff = elf->header->e_phoff + elf->header->e_phentsize * i;
            
            if ( soff & 3 )
                printf("ELF:%s: Suspicious (un)alignement\n", filename);
            
            if ( fseek(handle, soff, SEEK_SET) )
            {
                printf("ELF:%s: Invalid file : broken program table\n", filename);
            } else {
                ELF_Segment *s = (ELF_Segment*)malloc(sizeof(ELF_Segment));
                
                if ( elf_file_load_segment(s, handle, endian, min_offset, filename) )
                {
                    
                } else {
                    elf->segments[elf->nsegment] = s;
                    ++elf->nsegment;
                }
            }
        }
    }
    
    return 0;
}

int elf_file_load(ELF_File *elf, const char *filename)
{
    if ( elf == NULL )
    {
        printf("cannot load file %s into NULL struct.\n", filename);
        return -1;
    } else {
        elf_file_cleanup(elf);
    }
    
    FILE *handle = fopen(filename, "rb");
    
    if ( !handle )
    {
        printf("ELF:%s: Unable to open file\n", filename);
        return -1;
    }
    
    elf->header = (ELF_Header*)malloc(sizeof(ELF_Header));
    
    int exit_code = elf_file_load_header(elf->header, handle, filename);
    
    if ( exit_code )
    {
        free(elf->header);
        elf->header = NULL;
    } else {
        exit_code = elf_file_load_segments(elf, handle, filename);
        
        if ( exit_code )
        {
            
            
        } else {
            
        }
    }
    
    fclose(handle);
    return exit_code;
}

void elf_file_destroy(ELF_File *f)
{
    elf_file_cleanup(f);
    
    free(f);
}

