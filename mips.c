/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "mips.h"

#include "io.h"
#include "decode.h"

extern void mips_init_memory(MIPS *m);
extern void mips_init_processor(MIPS *m);
extern void mips_init_coprocessor(MIPS *m, int n);

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

static const char *mips_gpr_names[32] = {
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0",   "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0",   "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8",   "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

static const char *mips_fpr_names[32] = {
    "fp0",  "fp1",  "fp2",  "fp3",  "fp4",  "fp5",  "fp6",  "fp7",
    "fp8",  "fp9",  "fp10", "fp11", "fp12", "fp13", "fp14", "fp15",
    "fp16", "fp17", "fp18", "fp19", "fp20", "fp21", "fp22", "fp23",
    "fp24", "fp25", "fp26", "fp27", "fp28", "fp29", "fp30", "fp31"
};

const char* mips_gpr_name(int reg)
{
    return reg >= 0 && reg < 32 ? mips_gpr_names[reg & 31] : NULL;
}

const char* mips_fpr_name(int reg)
{
    return reg >= 0 && reg < 32 ? mips_fpr_names[reg & 31] : NULL;
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
    
    mips_init_memory(m);
    mips_init_processor(m);
    mips_init_coprocessor(m, 0);
    mips_init_coprocessor(m, 1);
    mips_init_coprocessor(m, 2);
    mips_init_coprocessor(m, 3);
    
    return m;
}

void mips_reset(MIPS *m)
{
    if ( m == NULL )
        return;
    
    m->cp[3].reset(&m->cp[3]);
    m->cp[2].reset(&m->cp[2]);
    m->cp[1].reset(&m->cp[1]);
    m->cp[0].reset(&m->cp[0]);
    m->hw.reset(&m->hw);
    
    m->mem.unmap(&m->mem);
    
    mips_init_memory(m);
}

void mips_destroy(MIPS *m)
{
    if ( m == NULL )
        return;
    
    mips_cleanup_processor(&m->hw);
    mips_cleanup_coprocessor(&m->cp[0]);
    mips_cleanup_coprocessor(&m->cp[1]);
    mips_cleanup_coprocessor(&m->cp[2]);
    mips_cleanup_coprocessor(&m->cp[3]);
    
    m->mem.unmap(&m->mem);
    
    free(m);
}

int mips_exec(MIPS *m, uint32_t n, int skip_proc)
{
    if ( m == NULL || m->decode == NULL )
    {
        mipsim_printf(IO_WARNING, "MIPS: Cannot run NULL machine");
        return MIPS_ERROR;
    }
    
    int nest = 0;
    m->stop_reason = MIPS_OK;
    
    while ( (m->stop_reason == MIPS_OK) && n )
    {
        MIPS_Native pc_pre = m->hw.get_pc(&m->hw);
        MIPS_Native ra_pre = m->hw.get_reg(&m->hw, RA);
        m->decode(m);
        MIPS_Native pc_post = m->hw.get_pc(&m->hw);
        MIPS_Native ra_post = m->hw.get_reg(&m->hw, RA);
        
        if ( skip_proc )
        {
            if ( ra_post != ra_pre && ra_post == pc_pre + 8 )
                ++nest;
            else if ( ra_post == ra_pre && pc_post == ra_pre )
                --nest;
        }
        
        if ( !nest )
            --n;
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
