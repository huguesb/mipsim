/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "mipself.h"

#include "io.h"

int mips_load_elf(MIPS *m, ELF_File *f)
{
    ELF32_Addr last = 0;
    
    if ( f->header->e_type == ET_EXEC )
    {
        // load segments whenever possible
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
    }
    
    // give it some slack as newlib apparently use data beyond .bss as if it was normal...
    if ( last )
        m->mem.map_alloc(&m->mem, last, 0x1000);
    
    return 0;
}

