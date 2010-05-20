/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "mipself.h"

#include "io.h"
#include "config.h"

static section_addr_end[] = {
    0, 0
};

/*!
    \brief Callback passed to elf_file_relocate
    \param name section name
    \param size section size
    \return address at which the section should be placed
    
    This function uses *ugly* static variables. It is neither thread-safe
    nor reentrant.
*/
ELF32_Addr section_addr(const char *name, ELF32_Word size)
{
    ELF32_Addr a = 0;
    int n = -1;
    
    if ( !strcmp(name, ".text") )
    {
        n = 0;
    } else if ( !strcmp(name, ".data")
                || !strcmp(name, ".rodata")
                || !strcmp(name, ".bss") ) {
        n = section_addr_end[1] == 0xFFFFFFFF ? 0 : 1;
    }
    
    if ( n != -1 )
    {
        a = section_addr_end[n];
        
        if ( a & 0xFFF )
            section_addr_end[n] = a = (a & ~0xFFF) + 0x1000;
        
        section_addr_end[n] += size;
    } else {
        mipsim_printf(IO_WARNING, "Asked to place section %s. No idea what to do...\n", name);
    }
    
    return a;
}

int mips_load_elf(MIPS *m, ELF_File *f)
{
    ELF32_Addr last = 0;
    
    if ( f->header->e_type == ET_EXEC )
    {
        for ( ELF32_Word i = 0; i < f->nsegment; ++i )
        {
            ELF_Segment *s = f->segments[i];
            
            if ( s == NULL )
                continue;
            
            // Load sections to fine tune RO/RW status...
            mipsim_printf(IO_DEBUG, "Loading segment %i : [%08x-%08x]\n",
                          i, s->p_vaddr, s->p_vaddr + s->p_memsz);
            
            m->mem.map_rw(&m->mem, s->p_vaddr, s->p_memsz, s->p_data);
            
            if ( last < s->p_vaddr + s->p_memsz )
                last = s->p_vaddr + s->p_memsz;
        }
        
        if ( f->header )
            m->hw.set_pc(&m->hw, f->header->e_entry);
    } else if ( f->header->e_type == ET_REL ) {
        MIPSIM_Config *cfg = mipsim_config();
        section_addr_end[0] = cfg->reloc_text;
        section_addr_end[1] = cfg->reloc_data;
        
        if ( elf_file_relocate(f, section_addr) )
        {
            mipsim_printf(IO_WARNING, "Relocation failed\n");
            return 1;
        }
        
        for ( ELF32_Word i = 0; i < f->nsection; ++i )
        {
            ELF_Section *s = f->sections[i];
            
            if ( s == NULL || !(s->s_flags & SHF_ALLOC) )
                continue;
            
            // Load sections to fine tune RO/RW status...
            mipsim_printf(IO_DEBUG, "Loading section %i : [%08x-%08x]\n",
                          i, s->s_addr, s->s_addr + s->s_size);
            
            m->mem.map_rw(&m->mem, s->s_addr, s->s_size, s->s_data);
            
            if ( last < s->s_addr + s->s_size)
                last = s->s_addr + s->s_size;
        }
        
        m->hw.set_pc(&m->hw, cfg->reloc_text);
    }
    
    // give it some slack as newlib apparently use data beyond .bss as if it was normal...
    if ( last )
        m->mem.map_alloc(&m->mem, last, 0x1000);
    
    return 0;
}

