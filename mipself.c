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
    for ( ELF32_Word i = 0; i < f->nsegment; ++i )
    {
        ELF_Segment *s = f->segments[i];
        
        if ( s == NULL )
            continue;
        
        // Load sections to fine tune RO/RW status...
        mipsim_printf(IO_DEBUG, "Loading segment %i : [%08x-%08x]\n", i, s->p_vaddr, s->p_vaddr + s->p_memsz);
        
        m->mem.map_rw(&m->mem, s->p_vaddr, s->p_memsz, s->p_data);
        
        // give it some slack as newlib apparently use data beyond .bss as if it was normal...
        m->mem.map_alloc(&m->mem, s->p_vaddr + s->p_memsz, 0x1000);
    }
    
    if ( f->header )
        m->hw.set_pc(&m->hw, f->header->e_entry);
    
    return 0;
}

