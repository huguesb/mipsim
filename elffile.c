/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "elffile.h"

#include <stdio.h>
#include <stdlib.h>

#include "io.h"

#define intersect(a, b, c, d) (((c) < (b)) && ((d) > (a)))

/*!
    \struct ELF_File
    \brief In-memory representation of an ELF binary
*/

/*!
    \struct ELF_Header
    \brief In-memory representation of an ELF binary header
*/

/*!
    \struct ELF_Section
    \brief In-memory representation of an ELF section
*/

/*!
    \struct ELF_Segment
    \brief In-memory representation of an ELF program segment
*/

/*!
    \struct ELF_Sym
    \brief Entry of a symbol table
*/

/*!
    \struct ELF_Rel
    \brief Entry of a SHT_REL relocation table
*/

/*!
    \struct ELF_Rela
    \brief Entry of a SHT_RELA relocation table
*/

/*!
    \internal
    \brief Load a halfword from an ELF file
    \param f file being loaded
    \param endian endianness of file
    \return the halfword at the current position
    
    The current position in the input file is incremented twice
*/
ELF32_Half elf_fget_half(FILE *f, ELF32_Char endian)
{
    int c0 = fgetc(f) & 0x00FF;
    int c1 = fgetc(f) & 0x00FF;
    
    return endian == ELFDATA2MSB ?
            (c0 << 8) | c1 :
            (c1 << 8) | c0;
}

/*!
    \internal
    \brief Load a word from an ELF file
    \param f file being loaded
    \param endian endianness of file
    \return the word at the current position
    
    The current position in the input file is incremented four times
*/
ELF32_Word elf_fget_word(FILE *f, ELF32_Char endian)
{
    int c0 = fgetc(f) & 0x00FF;
    int c1 = fgetc(f) & 0x00FF;
    int c2 = fgetc(f) & 0x00FF;
    int c3 = fgetc(f) & 0x00FF;
    
    return endian == ELFDATA2MSB ?
            (c0 << 24) | (c1 << 16) | (c2 << 8) | c3 :
            (c3 << 24) | (c2 << 16) | (c1 << 8) | c0;
}

/*!
    \brief Create a structure suitable for ELF file loading
    \return NULL on failure, allocated structure otherwise
    
    The caller is responsible for destroying the created structure via elf_file_destroy
*/
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
        f->shstrtab = NULL;
    }
    
    return f;
}

/*!
    \internal
    \brief Load a segment from an ELF file
    \param s structure in which to store data
    \param handle file being loaded
    \param endian endianness of file
    \param filename path of file being loaded
    \return 0 on succes
*/
void elf_file_cleanup(ELF_File *f)
{
    if ( f == NULL )
        return;
    
    /* free section data */
    for ( ELF32_Word i = 0; i < f->nsection; ++i )
    {
        if ( f->sections[i] )
        {
            free(f->sections[i]->s_data);
            free(f->sections[i]);
        }
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
    f->shstrtab = NULL;
}

/*!
    \brief Destroy an ELF file
    \param elf ELF file to destroy
    
    Release all resources allocated by elf_file_load and elf_file_create
*/
void elf_file_destroy(ELF_File *elf)
{
    if ( elf != NULL )
    {
        elf_file_cleanup(elf);
        free(elf);
    }
}

/*!
    \brief Find the name of a section
    \return pointer to section name, NULL on failure
    
    The pointer MUST NOT be free'd.
*/
const char* elf_section_name(ELF_File *elf, ELF32_Word n, ELF32_Word *size)
{
    if ( elf == NULL
        || elf->shstrtab == NULL
        || elf->shstrtab->s_data == NULL
        || n >= elf->nsection
        )
        return NULL;
    
    ELF_Section *s = elf->sections[n];
    
    if ( s && size )
        *size = s->s_size;
    else
        return NULL;
    
    return (char*)(elf->shstrtab->s_data) + s->s_name;
}

/*!
    \internal
    \brief Load a segment from an ELF file
    \param s structure in which to store data
    \param handle file being loaded
    \param endian endianness of file
    \param filename path of file being loaded
    \return 0 on succes
*/
int elf_file_load_header(ELF_Header *hdr, FILE *handle, const char *filename)
{
    // identifier (only part which can be read without being endian-aware)
    if ( fread(hdr->e_ident, sizeof(ELF32_Char), EI_NIDENT, handle) != (sizeof(ELF32_Char) * EI_NIDENT) )
    {
        mipsim_printf(IO_WARNING, "ELF:%s: Invalid header : failed to read identifier\n", filename);
        return 1;
    }
    
    /*
        Some macros for error checking
    */
    #define ELF_CHECK_EQU(s, val, expect) \
        if ( val != expect ) \
        { \
            mipsim_printf(IO_WARNING, \
                        "ELF:%s: Invalid header. Expected " s " %i, found %i\n", \
                        filename, \
                        expect, \
                        val); \
            return 1; \
        }
    #define ELF_CHECK_EITHER(s, val, e1, e2) \
        if ( val != e1 && val != e2 ) \
        { \
            mipsim_printf(IO_WARNING, \
                        "ELF:%s: Invalid header. Expected " s " %i or %i, found %i\n", \
                        filename, \
                        e1, \
                        e2, \
                        val); \
            return 1; \
        }
    #define ELF_CHECK_SUP(s, val, expect) \
        if ( val < expect ) \
        { \
            mipsim_printf(IO_WARNING, \
                        "ELF:%s: Invalid header. Expected " s " at least %i, found %i\n", \
                        filename, \
                        expect, \
                        val); \
            return 1; \
        }
    
    
    if ( hdr->e_ident[EI_MAG0] != ELFMAG0 ||
         hdr->e_ident[EI_MAG1] != ELFMAG1 ||
         hdr->e_ident[EI_MAG2] != ELFMAG2 ||
         hdr->e_ident[EI_MAG3] != ELFMAG3 )
    {
        mipsim_printf(IO_WARNING, "ELF:%s: Invalid header : wrong magic number\n", filename);
        return 1;
    }
    
    ELF_CHECK_EQU("class",   hdr->e_ident[EI_CLASS], ELFCLASS32)
    ELF_CHECK_EQU("version", hdr->e_ident[EI_VERSION], EV_CURRENT)
    
    const ELF32_Char endian = hdr->e_ident[EI_DATA];
    
    ELF_CHECK_EQU("format", endian, ELFDATA2MSB)
    
    /*
        read rest of header data, endian-aware
    */
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
    
    ELF_CHECK_EQU("version", hdr->e_version, EV_CURRENT)
    ELF_CHECK_EITHER("type", hdr->e_type, ET_EXEC, ET_REL)
    ELF_CHECK_EQU("machine", hdr->e_machine, EM_MIPS)
    
    ELF_CHECK_SUP("header size", hdr->e_ehsize, sizeof(ELF_Header))
    
    if ( hdr->e_shoff )
    {
        ELF_CHECK_SUP("section table offset", hdr->e_shoff, hdr->e_ehsize)
        ELF_CHECK_SUP("section header size", hdr->e_shentsize, 10 * sizeof(ELF32_Word))
    }
    
    if ( hdr->e_phoff )
    {
        ELF_CHECK_SUP("program table offset", hdr->e_phoff, hdr->e_ehsize)
        ELF_CHECK_SUP("program header size", hdr->e_phentsize, 8 * sizeof(ELF32_Word))
    }
    
    return 0;
}

/*!
    \internal
    \brief Load a segment from an ELF file
    \param s structure in which to store data
    \param handle file being loaded
    \param endian endianness of file
    \param filename path of file being loaded
    \return 0 on succes
*/
int elf_file_load_segment(ELF_Segment *s, FILE *handle, int endian, const char *filename)
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
            mipsim_printf(IO_WARNING, "ELF:%s: Invalid file : suspicious segment size\n", filename);
            return 1;
        }
        
        if ( fseek(handle, s->p_offset, SEEK_SET) )
        {
            mipsim_printf(IO_WARNING, "ELF:%s: Invalid file : unable to reach segment\n", filename);
            return 1;
        }
        
        s->p_data = (ELF32_Char*)calloc(s->p_memsz, sizeof(ELF32_Char));
        size_t sz = fread(s->p_data, sizeof(ELF32_Char), s->p_filesz, handle);
        
        if ( sz != s->p_filesz * sizeof(ELF32_Char) )
        {
            mipsim_printf(IO_WARNING, "ELF:%s: Invalid file : unable to read data of segment [%d vs %u]\n", filename, sz, s->p_filesz);
            free(s->p_data);
            s->p_data = NULL;
            return 1;
        }
        
        mipsim_printf(IO_DEBUG, "ELF:%s: Succesfully loaded segment (%d bytes)\n", filename, s->p_filesz);
    } else {
        mipsim_printf(IO_DEBUG, "ELF:%s: Skipped segment (type %8x)\n", filename, s->p_type);
        return 2;
    }
    
    return 0;
}


/*!
    \internal
    \brief Load all segments from an ELF file
    \param elf structure in which to store data
    \param handle file being loaded
    \param endian endianness of file
    \param filename path of file being loaded
    \return 0 on succes
*/
int elf_file_load_segments(ELF_File *elf, FILE *handle, const char *filename)
{
    if ( fseek(handle, elf->header->e_phoff, SEEK_SET) )
    {
        mipsim_printf(IO_WARNING, "ELF:%s: Invalid file : unable to locate program header table\n", filename);
        return 1;
    }
    
    if ( elf->header->e_phnum )
        elf->segments = (ELF_Segment**)malloc(elf->header->e_phnum * sizeof(ELF_Segment*));
    
    const ELF32_Char endian = elf->header->e_ident[EI_DATA];
    const ELF32_Off min_offset = elf->header->e_phoff + elf->header->e_phentsize * (elf->header->e_phnum + 1);
    
    ELF32_Addr first_addr = -1, last_addr = 0;
    
    if ( elf->segments )
    {
        elf->nsegment = 0;
        
        for ( ELF32_Word i = 0; i < elf->header->e_phnum; ++i )
        {
            ELF32_Word soff = elf->header->e_phoff + elf->header->e_phentsize * i;
            
            if ( soff & 3 )
            {
                mipsim_printf(IO_WARNING,
                              "ELF:%s: Suspicious (un)alignement\n",
                              filename);
                return 1;
            }
            
            if ( fseek(handle, soff, SEEK_SET) )
            {
                mipsim_printf(IO_WARNING,
                              "ELF:%s: Invalid file : broken program table\n",
                              filename);
                return 1;
            } else {
                ELF_Segment *s = (ELF_Segment*)malloc(sizeof(ELF_Segment));
                
                if ( elf_file_load_segment(s, handle, endian, filename) )
                {
                    free(s);
                } else {
                    // cheap overlap check
                    if ( intersect( first_addr, last_addr,
                                    s->p_vaddr, s->p_vaddr + s->p_memsz)
                       )
                    {
                        // accurate test required...
                        for ( ELF32_Word k = 0; k < elf->nsegment; ++k )
                        {
                            ELF_Segment *sk = elf->segments[k];
                            
                            if ( intersect( sk->p_vaddr,
                                            sk->p_vaddr + sk->p_memsz,
                                            s->p_vaddr,
                                            s->p_vaddr + s->p_memsz
                                          )
                                )
                            {
                                mipsim_printf(IO_WARNING,
                                              "ELF:%s: Overlapping segments\n",
                                              filename);
                                return 1;
                            }
                        }
                    } else {
                        if ( s->p_vaddr < first_addr )
                            first_addr = s->p_vaddr;
                        if ( s->p_vaddr + s->p_memsz > last_addr )
                            last_addr  = s->p_vaddr + s->p_memsz;
                    }
                    
                    elf->segments[elf->nsegment] = s;
                    ++elf->nsegment;
                }
            }
        }
    }
    
    return 0;
}

/*!
    \internal
    \brief Load section data from an ELF file
    \param d where to store data
    \param handle file being loaded
    \param off offset of data in file
    \param sz size of data to read
    \param filename path of file being loaded
    \return 0 on succes
    
    Caller is responsible for freeing the allocated data
*/
int elf_fread(ELF32_Char **d, FILE *handle, ELF32_Off off, ELF32_Word sz, const char *filename)
{
    if ( sz <= 0 )
        return 1;
    
    if ( fseek(handle, off, SEEK_SET) )
    {
        mipsim_printf(IO_WARNING, "ELF:%s: Invalid file : unable to reach section data\n", filename);
        *d = NULL;
        return 1;
    }
    
    *d = (ELF32_Char*)malloc(sz * sizeof(ELF32_Char));
    
    if ( *d == NULL )
    {
        mipsim_printf(IO_WARNING, "ELF:%s: Failed to allocate memory for section data\n", filename);
        return 1;
    }
    
    size_t rsz = fread(*d, sizeof(ELF32_Char), sz, handle);
    
    if ( rsz != sz * sizeof(ELF32_Char) )
    {
        mipsim_printf(IO_WARNING, "ELF:%s: Invalid file : unable to read data of section [%d vs %u]\n", filename, rsz, sz);
        free(*d);
        *d = NULL;
        return 1;
    }
    
    return 0;
}

/*!
    \internal
    \brief Load a section from an ELF file
    \param s structure in which to store data
    \param handle file being loaded
    \param endian endianness of file
    \param filename path of file being loaded
    \return 0 on succes
*/
int elf_file_load_section(ELF_Section *s, FILE *handle, int endian, const char *filename)
{
    s->s_name      = elf_fget_word(handle, endian);
    s->s_type      = elf_fget_word(handle, endian);
    s->s_flags     = elf_fget_word(handle, endian);
    s->s_addr      = elf_fget_word(handle, endian);
    s->s_offset    = elf_fget_word(handle, endian);
    s->s_size      = elf_fget_word(handle, endian);
    s->s_link      = elf_fget_word(handle, endian);
    s->s_info      = elf_fget_word(handle, endian);
    s->s_addralign = elf_fget_word(handle, endian);
    s->s_entsize   = elf_fget_word(handle, endian);
    
    s->s_data      = NULL;
    s->s_reloc     = 0;
    
    ELF32_Word mask = s->s_addralign > 1 ? s->s_addralign - 1 : 0;
    
    if ( (mask & ~(s->s_addralign)) != mask )
    {
        mipsim_printf(IO_WARNING,
                      "ELF:%s: Invalid address alignemnt constraint (%d)\n",
                      filename, s->s_addralign);
        return 1;
    }
    
    int ret = 0;
    
    if ( s->s_type == SHT_STRTAB )
    {
        ret = elf_fread(&s->s_data, handle, s->s_offset, s->s_size, filename);
    } else if ( s->s_type == SHT_SYMTAB ) {
        // s_link points to strtab
        
        ELF32_Off off = s->s_offset;
        ELF32_Off end = s->s_offset + s->s_size;
        
        ELF_Sym *sym = (ELF_Sym*)malloc((s->s_size / s->s_entsize) * sizeof(ELF_Sym));
        s->s_data = (ELF32_Char*)((void*)sym);
        
        while ( off + s->s_entsize < end )
        {
            if ( off & mask )
            {
                mipsim_printf(IO_WARNING,
                              "ELF:%s: Improper alignement of symbol table entreis\n",
                              filename
                              );
                free(s->s_data);
                s->s_data = NULL;
                return 1;
            }
            
            sym->s_name  = elf_fget_word(handle, endian);
            sym->s_value = elf_fget_word(handle, endian);
            sym->s_size  = fgetc(handle);
            sym->s_info  = fgetc(handle);
            sym->s_other = elf_fget_word(handle, endian);
            sym->s_shndx = elf_fget_half(handle, endian);
            ++sym;
            
            off += s->s_entsize;
        }
    } else if ( s->s_type == SHT_REL ) {
        // s_link points to symtab
        // s_info points to section to reloc
        
        ELF32_Off off = s->s_offset;
        ELF32_Off end = s->s_offset + s->s_size;
        
        ELF_Rel *rel = (ELF_Rel*)malloc((s->s_size / s->s_entsize) * sizeof(ELF_Rel));
        s->s_data = (ELF32_Char*)((void*)rel);
        
        while ( off + s->s_entsize < end )
        {
            if ( off & mask )
            {
                mipsim_printf(IO_WARNING,
                              "ELF:%s: Improper alignement of symbol table entreis\n",
                              filename
                              );
                free(s->s_data);
                s->s_data = NULL;
                return 1;
            }
            
            rel->r_offset = elf_fget_word(handle, endian);
            rel->r_info   = elf_fget_word(handle, endian);
            ++rel;
            
            off += s->s_entsize;
        }
    } else if ( s->s_type == SHT_RELA ) {
        // s_link points to symtab
        // s_info points to section to reloc
        
        ELF32_Off off = s->s_offset;
        ELF32_Off end = s->s_offset + s->s_size;
        
        ELF_Rela *rela = (ELF_Rela*)malloc((s->s_size / s->s_entsize) * sizeof(ELF_Rela));
        s->s_data = (ELF32_Char*)((void*)rela);
        
        while ( off + s->s_entsize < end )
        {
            if ( off & mask )
            {
                mipsim_printf(IO_WARNING,
                              "ELF:%s: Improper alignement of symbol table entreis\n",
                              filename
                              );
                free(s->s_data);
                s->s_data = NULL;
                return 1;
            }
            
            rela->r_offset = elf_fget_word(handle, endian);
            rela->r_info   = elf_fget_word(handle, endian);
            rela->r_addend = elf_fget_word(handle, endian);
            ++rela;
            
            off += s->s_entsize;
        }
    } else if ( s->s_type == SHT_PROGBITS ) {
        ret = elf_fread(&s->s_data, handle, s->s_offset, s->s_size, filename);
    } else if ( s->s_flags & SHF_ALLOC ) {
        ret = elf_fread(&s->s_data, handle, s->s_offset, s->s_size, filename);
    }
    
    return ret;
}

/*!
    \internal
    \brief Load all sections from an ELF file
    \param elf structure in which to store data
    \param handle file being loaded
    \param filename path of file being loaded
    \return 0 on succes
*/
int elf_file_load_sections(ELF_File *elf, FILE *handle, const char *filename)
{
    if ( fseek(handle, elf->header->e_shoff, SEEK_SET) )
    {
        mipsim_printf(IO_WARNING, "ELF:%s: Invalid file : unable to locate section header table\n", filename);
        return 1;
    }
    
    if ( !elf->header->e_shnum )
        return 0;
    
    mipsim_printf(IO_DEBUG, "ELF:%s:Loading %d sections\n", filename, elf->header->e_shnum);
    
    const ELF32_Char endian = elf->header->e_ident[EI_DATA];
    const ELF32_Off min_offset = elf->header->e_shoff + elf->header->e_shentsize * (elf->header->e_shnum + 1);
    
    elf->sections = (ELF_Section**)malloc(elf->header->e_shnum * sizeof(ELF_Section*));
    elf->nsection = elf->header->e_shnum;
    
    ELF32_Addr first_addr = -1, last_addr = 0;
    
    ELF32_Word soff = elf->header->e_shoff;
    
    if ( (soff & 3) || (elf->header->e_shentsize & 3) )
        mipsim_printf(IO_WARNING, "ELF:%s: Suspicious section (un)alignement\n", filename);
    
    for ( ELF32_Word i = 0; i < elf->nsection; ++i, soff += elf->header->e_shentsize )
    {
        elf->sections[i] = NULL;
        
        if ( fseek(handle, soff, SEEK_SET) )
        {
            mipsim_printf(IO_WARNING, "ELF:%s: Invalid file : broken section table\n", filename);
        } else {
            ELF_Section *s = (ELF_Section*)malloc(sizeof(ELF_Section));
            
            if ( elf_file_load_section(s, handle, endian, filename) )
            {
                free(s);
            } else {
                // overlap check for sections with an absolute address
                if ( s->s_addr )
                {
                    // cheap overlap check
                    if ( intersect( first_addr, last_addr,
                                    s->s_addr, s->s_addr + s->s_size)
                        )
                    {
                        // accurate test required...
                        for ( ELF32_Word k = 0; k < elf->nsection; ++k )
                        {
                            ELF_Section *sk = elf->sections[k];
                            
                            if ( sk != NULL && sk->s_addr &&
                                intersect( sk->s_addr,
                                            sk->s_addr + sk->s_size,
                                            s->s_addr,
                                            s->s_addr + s->s_size
                                            )
                                )
                            {
                                mipsim_printf(IO_WARNING,
                                                "ELF:%s: Overlapping sections %d & %d\n",
                                                filename, k, i);
                                return 1;
                            }
                        }
                    } else {
                        if ( s->s_addr < first_addr )
                            first_addr = s->s_addr;
                        if ( s->s_addr + s->s_size > last_addr )
                            last_addr  = s->s_addr + s->s_size;
                    }
                    
                }
                
                elf->sections[i] = s;
                
                // keep track of SectionHeaderSTRingTABle
                if ( i == elf->header->e_shstrndx && s->s_type == SHT_STRTAB )
                    elf->shstrtab = s;
            }
        }
    }
    
    ELF_Section *sst = elf->shstrtab;
    
    mipsim_printf(IO_DEBUG, "---------------------------------------------------+\n");
    mipsim_printf(IO_DEBUG, "[id]        name         flags   addr       size   |\n");
    mipsim_printf(IO_DEBUG, "---------------------------------------------------+\n");
    
    for ( ELF32_Word i = 0; i < elf->nsection; ++i )
    {
        ELF_Section *s = elf->sections[i];
        
        mipsim_printf(IO_DEBUG, "[%2d] ", i);
        
        if ( s != NULL )
        {
            mipsim_printf(IO_DEBUG, "%20s %c%c%c 0x%08x 0x%08x\n",
                          sst ? sst->s_data + s->s_name : NULL,
                          (s->s_flags & SHF_ALLOC) ? 'a' : '-',
                          (s->s_flags & SHF_WRITE) ? 'w' : '-',
                          (s->s_flags & SHF_EXECINSTR) ? 'x' : '-',
                          s->s_addr,
                          s->s_size
                          );
            
        } else {
            mipsim_printf(IO_DEBUG, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
        }
    }
    
    return 0;
}

/*!
    \brief Load the contents of an ELF file into memory
    \param elf Structure in which data will be stored
    \param filename Path to file to load
    \return 0 on success
    
    Not all sections are loaded, only what makes sense for the simulator...
*/
int elf_file_load(ELF_File *elf, const char *filename)
{
    if ( elf == NULL )
    {
        mipsim_printf(IO_WARNING, "cannot load file %s into NULL struct.\n", filename);
        return -1;
    } else {
        elf_file_cleanup(elf);
    }
    
    FILE *handle = fopen(filename, "rb");
    
    if ( !handle )
    {
        mipsim_printf(IO_WARNING, "ELF:%s: Unable to open file\n", filename);
        return -1;
    }
    
    elf->header = (ELF_Header*)malloc(sizeof(ELF_Header));
    
    if ( !elf->header )
    {
        mipsim_printf(IO_WARNING, "ELF:%s: Unable to allocate memory for ELF header\n", filename);
        return -1;
    }
    
    int exit_code = elf_file_load_header(elf->header, handle, filename);
    
    if ( !exit_code )
       exit_code = elf_file_load_sections(elf, handle, filename);
    else {
        free(elf->header);
        elf->header = NULL;
    }
    
    if ( !exit_code )
        exit_code = elf_file_load_segments(elf, handle, filename);
    
    fclose(handle);
    return exit_code;
}

/*!
    \brief Relocate the contents of an ELF binary prior to execution
    \param elf ELF file to relocate
    \param text_base Base relocation address of text section
    \param data_base Base relocation address of data section(s)
    \return 0 on success
*/
int elf_file_relocate(ELF_File *elf, addr_for_name section_addr)
{
    /*
        ensure that all SHF_ALLOC sections are placed
    */
    for ( ELF32_Word i = 0; i < elf->nsection; ++i )
    {
        ELF_Section *s = elf->sections[i];
        
        if ( s == NULL || s->s_data == NULL )
            continue;
        
        if ( (s->s_flags & SHF_ALLOC) && !s->s_addr )
        {
            ELF32_Word size;
            const char *name = elf_section_name(elf, i, &size);
            s->s_addr = section_addr(name, size);
            
            mipsim_printf(IO_DEBUG, "Placed %s @ 0x%08x\n", name, s->s_addr);
        }
    }
    
    /*
        relocate sections
    */
    for ( ELF32_Word i = 0; i < elf->nsection; ++i )
    {
        ELF_Section *s = elf->sections[i];
        
        if ( s == NULL
            || s->s_data == NULL
            || (s->s_type != SHT_REL && s->s_type != SHT_RELA) )
            continue;
        
        mipsim_printf(IO_DEBUG, "Reloc\n");
        
        const ELF32_Word n = s->s_size / s->s_entsize;
        
        ELF_Section *symtab = s->s_link < elf->nsection ? elf->sections[s->s_link] : NULL;
        ELF_Section *target = s->s_info < elf->nsection ? elf->sections[s->s_info] : NULL;
        
        if ( symtab == NULL || target == NULL )
        {
            mipsim_printf(IO_WARNING, "ELF:Invalid relocation section\n");
            return 1;
        }
        
        ELF32_Char *sd = target->s_data;
        
        ELF_Sym *symbols = (ELF_Sym*)((void*)symtab->s_data);
        const ELF32_Word symcount = symtab->s_size / symtab->s_entsize;
        
        if ( s->s_type == SHT_REL )
        {
            ELF_Rel *r = (ELF_Rel*)((void*)s->s_data);
            
            for ( ELF32_Word i = 0; i < n; ++i, ++r )
            {
                ELF32_Word sidx = ELF32_R_SYM(r->r_info);
                ELF32_Char type = ELF32_R_TYPE(r->r_info);
                
                if ( sidx >= symcount )
                {
                    mipsim_printf(IO_WARNING, "ELF:Invalid relocation section\n");
                    return 1;
                }
                
                if ( type == R_386_NONE )
                {
                    
                } else if ( type == R_386_32 ) {
                    
                } else if ( type == R_386_PC32 ) {
                    
                } else {
                    mipsim_printf(IO_WARNING, "ELF: Unsupported relocation type %d\n", type);
                    return 1;
                }
            }
        } else if ( s->s_type == SHT_RELA ) {
            ELF_Rela *r = (ELF_Rela*)((void*)s->s_data);
            
            for ( ELF32_Word i = 0; i < n; ++i, ++r )
            {
                ELF32_Word sidx = ELF32_R_SYM(r->r_info);
                ELF32_Char type = ELF32_R_TYPE(r->r_info);
                
                if ( sidx >= symcount )
                {
                    mipsim_printf(IO_WARNING, "ELF:Invalid relocation section\n");
                    return 1;
                }
                
                if ( type == R_386_NONE )
                {
                    
                } else if ( type == R_386_32 ) {
                    
                } else if ( type == R_386_PC32 ) {
                    
                } else {
                    mipsim_printf(IO_WARNING, "ELF: Unsupported relocation type %d\n", type);
                    return 1;
                }
            }
        }
    }
    
    return 0;
}
