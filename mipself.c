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

int mips_load_elf(MIPS *m, ELF_File *f)
{
    for ( ELF32_Word i = 0; i < f->nsegment; ++i )
    {
        ELF_Segment *s = f->segments[i];
        
        if ( s == NULL )
            continue;
        
        printf("Loading segment %i : [%08x-%08x]\n", i, s->p_vaddr, s->p_vaddr + s->p_memsz);
        
        m->mem.map_ro(&m->mem, s->p_vaddr, s->p_memsz, s->p_data);
    }
    
    m->hw.pc = f->header->e_entry;
    
    return 0;
}

