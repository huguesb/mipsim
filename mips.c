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

#include "io.h"
#include "decode.h"

extern void mips_init_memory(MIPS_Memory *mem);
extern void mips_init_processor(MIPS_Processor *hw);
extern void mips_init_coprocessor(MIPS_Coprocessor *hw);

//extern void mips_cleanup_memory(MIPS_Memory *mem);
extern void mips_cleanup_processor(MIPS_Processor *hw);
extern void mips_cleanup_coprocessor(MIPS_Coprocessor *hw);

extern int mips_universal_decode(MIPS *m);

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

static const char *mips_reg_names[32] = {
    "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0",   "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0",   "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8",   "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
};

const char* mips_reg_name(int reg)
{
    return reg >= 0 && reg < 32 ? mips_reg_names[reg & 31] : NULL;
}

MIPS* mips_create(int arch)
{
    if ( arch <= MIPS_ARCH_NONE || arch >= MIPS_ARCH_LAST )
    {
        mipsim_printf(IO_WARNING, "MIPS: Unknown architecture");
        return NULL;
    }
    
    MIPS *m = (MIPS*)malloc(sizeof(MIPS));
    
    m->architecture = arch;
    m->decode = mips_universal_decode;
    
    mips_init_memory(&m->mem);
    mips_init_processor(&m->hw);
    mips_init_coprocessor(&m->cp0);
    mips_init_coprocessor(&m->cp1);
    mips_init_coprocessor(&m->cp2);
    mips_init_coprocessor(&m->cp3);
    
    return m;
}

void mips_destroy(MIPS *m)
{
    mips_cleanup_coprocessor(&m->cp3);
    mips_cleanup_coprocessor(&m->cp2);
    mips_cleanup_coprocessor(&m->cp1);
    mips_cleanup_coprocessor(&m->cp0);
    mips_cleanup_processor(&m->hw);
    
    m->mem.unmap(&m->mem);
    
    free(m);
}

int mips_exec(MIPS *m, uint32_t n)
{
    if ( m == NULL || m->decode == NULL )
    {
        mipsim_printf(IO_WARNING, "MIPS: Cannot run NULL machine");
        return MIPS_ERROR;
    }
    
    m->stop_reason = MIPS_OK;
    
    while ( (m->stop_reason == MIPS_OK) && n-- )
    {
        m->decode(m);
    }
    
    return m->stop_reason;
}

void mips_stop(MIPS *m, int reason)
{
    if ( m->stop_reason == MIPS_OK ) 
    {
        m->stop_reason = reason;
    } else {
        printf("should be stopped already...\n");
    }
}

uint8_t  mips_read_b(MIPS *m, MIPS_Addr a, int *stat)
{
    return m->mem.read_b(&m->mem, a, stat);
}

uint16_t mips_read_h(MIPS *m, MIPS_Addr a, int *stat)
{
    return m->mem.read_h(&m->mem, a, stat);
}

uint32_t mips_read_w(MIPS *m, MIPS_Addr a, int *stat)
{
    return m->mem.read_w(&m->mem, a, stat);
}

uint64_t mips_read_d(MIPS *m, MIPS_Addr a, int *stat)
{
    return m->mem.read_d(&m->mem, a, stat);
}

void mips_write_b(MIPS *m, MIPS_Addr a, uint8_t b,  int *stat)
{
    m->mem.write_b(&m->mem, a, b, stat);
}

void mips_write_h(MIPS *m, MIPS_Addr a, uint16_t h, int *stat)
{
    m->mem.write_h(&m->mem, a, h, stat);
}

void mips_write_w(MIPS *m, MIPS_Addr a, uint32_t w, int *stat)
{
    m->mem.write_w(&m->mem, a, w, stat);
}

void mips_write_d(MIPS *m, MIPS_Addr a, uint64_t d, int *stat)
{
    m->mem.write_d(&m->mem, a, d, stat);
}
