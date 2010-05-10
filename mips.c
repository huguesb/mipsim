/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "mips.h"

#include "mipsarch.h"

extern void mips_init_hardware(MIPS_Hardware *hw);
extern void mips_init_memory(MIPS_Memory *mem);

extern void mips_universal_decode(MIPS *m);

static const char *mips_isa_names[] = {
    "MIPS 1",
    "MIPS 2",
    "MIPS 3",
    "MIPS 4",
    "MIPS 5",
    "MIPS 32",
    "MIPS 32 R2",
    "MIPS 64"
};

const char* mips_isa_name(int isa)
{
    return isa >= MIPS_ARCH_FIRST && isa < MIPS_ARCH_LAST ? mips_isa_names[isa - MIPS_ARCH_FIRST] : NULL;
}

MIPS* mips_create(int arch)
{
    if ( arch <= MIPS_ARCH_NONE || arch >= MIPS_ARCH_LAST )
    {
        printf("MIPS: Unknown architecture");
        return NULL;
    }
    
    MIPS *m = (MIPS*)malloc(sizeof(MIPS));
    
    m->architecture = arch;
    m->decode = mips_universal_decode;
    
    mips_init_hardware(&m->hw);
    mips_init_memory(&m->mem);
    
    return m;
}

void mips_destroy(MIPS *m)
{
    
    free(m);
}

int mips_exec(MIPS *m, uint32_t n)
{
    if ( m == NULL || m->decode == NULL )
    {
        printf("MIPS: Cannot run NULL machine");
        return MIPS_ERROR;
    }
    
    m->stop_reason = MIPS_OK;
    
    while ( m->stop_reason == MIPS_OK && n-- )
    {
        m->stop_reason = m->decode(m);
    }
    
    return m->stop_reason;
}
