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
#include "util.h"
#include "decode.h"

#include <string.h>

extern void mips_init_memory(MIPS *m);
extern void mips_init_processor(MIPS *m);
extern void mips_init_coprocessor(MIPS *m, int n);

//extern void mips_cleanup_memory(MIPS_Memory *mem);
extern void mips_cleanup_processor(MIPS_Processor *hw);
extern void mips_cleanup_coprocessor(MIPS_Coprocessor *hw);

extern int mips_universal_decode(MIPS *m);

static const char *mips_isa_names[] = {
    NULL,
    "mips1",
    "mips2",
    "mips3",
    "mips4",
    "mips5",
    "mips32",
    "mips32r2",
    "mips64",
    "mips64r2"
};

/*!
    \brief Give the verbose name of an ISA version
*/
int mips_isa_id(const char* name)
{
    for ( int i = MIPS_ARCH_FIRST; i < MIPS_ARCH_LAST; ++i )
    {
        const char *isa = mips_isa_names[i];
        
        // allow "MIPS" ommision
        if ( *name != 'm' )
            isa += 4;
        
        if ( !strcmp(isa, name) )
            return i;
    }
    
    return MIPS_ARCH_NONE;
}

/*!
    \brief Give the verbose name of an ISA version
*/
const char* mips_isa_name(int isa)
{
    return isa >= MIPS_ARCH_FIRST && isa < MIPS_ARCH_LAST ? mips_isa_names[isa - MIPS_ARCH_FIRST] : NULL;
}

static const char *mips_default_reg_names[32] = {
    "$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",
    "$8",  "$9",  "$10", "$11", "$12", "$13", "$14", "$15",
    "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
    "$24", "$25", "$26", "$27", "$28", "$29", "$30", "$31"
};

static const char *mips_gpr_names[32] = {
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0",   "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0",   "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8",   "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

static const char *mips_spr_names[3] = {
    "pc", "hi", "lo"
};

static const char *mips_cp0_names[32] = {
    "c0_index", "c0_random", "c0_entrylo", "c0_r3",  "c0_context", "c0_r5",    "c0_r6",  "c0_r7",
    "c0_vaddr", "c0_r9",     "c0_entryhi", "c0_r11", "c0_status",  "c0_cause", "c0_epc", "c0_r15",
    "c0_r16",   "c0_r17",    "c0_r18",     "c0_r19", "c0_r20",     "c0_r21",   "c0_r22", "c0_r23",
    "c0_r24",   "c0_r25",    "c0_r26",     "c0_r27", "c0_r28",     "c0_r29",   "c0_r30", "c0_r31"
};

static const char *mips_fpr_names[32] = {
    "fp0",  "fp1",  "fp2",  "fp3",  "fp4",  "fp5",  "fp6",  "fp7",
    "fp8",  "fp9",  "fp10", "fp11", "fp12", "fp13", "fp14", "fp15",
    "fp16", "fp17", "fp18", "fp19", "fp20", "fp21", "fp22", "fp23",
    "fp24", "fp25", "fp26", "fp27", "fp28", "fp29", "fp30", "fp31"
};

static const char *mips_cp2_names[32] = {
    "c2_r0",  "c2_r1",  "c2_r2",  "c2_r3",  "c2_r4",  "c2_r5",  "c2_r6",  "c2_r7",
    "c2_r8",  "c2_r9",  "c2_r3",  "c2_r11", "c2_r12", "c2_r13", "c2_r14", "c2_r15",
    "c2_r16", "c2_r17", "c2_r18", "c2_r19", "c2_r20", "c2_r21", "c2_r22", "c2_r23",
    "c2_r24", "c2_r25", "c2_r26", "c2_r27", "c2_r28", "c2_r29", "c2_r30", "c2_r31"
};

static const char *mips_cp3_names[32] = {
    "c3_r0",  "c3_r1",  "c3_r2",  "c3_r3",  "c3_r4",  "c3_r5",  "c3_r6",  "c3_r7",
    "c3_r8",  "c3_r9",  "c3_r3",  "c3_r11", "c3_r12", "c3_r13", "c3_r14", "c3_r15",
    "c3_r16", "c3_r17", "c3_r18", "c3_r19", "c3_r20", "c3_r21", "c3_r22", "c3_r23",
    "c3_r24", "c3_r25", "c3_r26", "c3_r27", "c3_r28", "c3_r29", "c3_r30", "c3_r31"
};

/*!
    \brief Convert a register name into a register ID
*/
int mips_reg_id(const char *name)
{
    if ( name == NULL )
        return INVALID_REG;
    
    if ( *name == '$' )
        ++name;
    
    if ( is_number(*name) )
    {
        // simple atoi from reg number
        
        int error;
        const char *end;
        int id = str_to_num(name, &end, &error);
        
        if ( !*end && !error && id < 32 )
            return id;
        
    } else if ( is_letter(*name) ) {
        // string lookup from reg name
        
        for ( int i = 0; i < 32; ++i )
            if ( !strcmp(name, mips_gpr_names[i]) )
                return i;
        
        for ( int i = 0; i < 3; ++i )
            if ( !strcmp(name, mips_spr_names[i]) )
                return i | SPR;
        
        for ( int i = 0; i < 32; ++i )
            if ( !strcmp(name, mips_fpr_names[i]) )
                return i | CP1;
    }
    
    return INVALID_REG;
}

/*!
    \brief Convert a register ID into a register name
*/
const char* mips_reg_name(int reg)
{
    int flags = reg & ~0xFF;
    reg &= 0xFF;
    
    // TODO : support for CPx ctrl regs
    
    if ( !flags )
        return reg < 32 ? mips_gpr_names[reg] : NULL;
    else if ( flags == SPR )
        return reg < 3 ? mips_spr_names[reg] : NULL;
    else if ( flags == CP0 )
        return reg < 32 ? mips_cp0_names[reg] : NULL;
    else if ( flags == CP1 )
        return reg < 32 ? mips_fpr_names[reg] : NULL;
    else if ( flags == CP2 )
        return reg < 32 ? mips_cp2_names[reg] : NULL;
    else if ( flags == CP3 )
        return reg < 32 ? mips_cp3_names[reg] : NULL;
    
    return NULL;
}

/*!
    \brief Creates a simulated machine
    \param arch Architecture to simulate
*/
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
    
    m->breakpoints = NULL;
    
    mips_init_memory(m);
    mips_init_processor(m);
    mips_init_coprocessor(m, 0);
    mips_init_coprocessor(m, 1);
    mips_init_coprocessor(m, 2);
    mips_init_coprocessor(m, 3);
    
    return m;
}

/*!
    \brief Reset the state of a simulated machine
*/
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

/*!
    \brief Release all memory used by a simulated machine
*/
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

/*!
    \brief Simulate a machine
    \param m machine
    \param n number of instructions to simulate
    \param skip_proc whether to skip procedure calls
    \return stop reason
    
    If \a skip_proc is non-zero, procdedure calls will be skipped.
    Please note though that only ABI-conforming procedure calls
    will be detected and correctly skipped.
*/
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

/*!
    \brief Stop the execution of a simulated machine
    \param reason Stop reason
*/
void mips_stop(MIPS *m, int reason)
{
    if ( m->stop_reason == MIPS_OK ) 
    {
        m->stop_reason = reason;
    } else {
        printf("should be stopped already...\n");
    }
}

/*!
    \brief Getter to simulated machine registers
*/
MIPS_Native mips_get_reg(MIPS *m, int id)
{
    int flags = id & ~0xFF;
    int reg = id & 0xFF;
    
    if ( !flags )
    {
        return m->hw.get_reg(&m->hw, reg);
    } else if ( flags == SPR ) {
        if ( reg == (PC & 0xFF) )
            return m->hw.get_pc(&m->hw);
        else if ( reg == (HI & 0xFF) )
            return m->hw.get_hi(&m->hw);
        else if ( reg == (LO & 0xFF) )
            return m->hw.get_lo(&m->hw);
    } else if ( flags & CP_BIT ) {
        MIPS_Coprocessor *cp = m->cp + ((flags >> CP_SHIFT) & 3);
        return flags & COND_BIT ? cp->get_ctrl(cp, id) : cp->get_reg(cp, id);
    }
    
    mipsim_printf(IO_WARNING, "Trying to read inexistant register : %08x\n", id);
    
    return 0;
}

/*!
    \brief Setter to simulated machine registers
*/
void mips_set_reg(MIPS *m, int id, MIPS_Native v)
{
    int flags = id & ~0xFF;
    int reg = id & 0xFF;
    
    if ( !flags )
    {
        return m->hw.set_reg(&m->hw, reg, v);
    } else if ( flags == SPR ) {
        if ( reg == (PC & 0xFF) )
            return m->hw.set_pc(&m->hw, v);
        else if ( reg == (HI & 0xFF) )
            return m->hw.set_hi(&m->hw, v);
        else if ( reg == (LO & 0xFF) )
            return m->hw.set_lo(&m->hw, v);
    } else if ( flags & CP_BIT ) {
        MIPS_Coprocessor *cp = m->cp + ((flags >> CP_SHIFT) & 3);
        return flags & COND_BIT ? cp->set_ctrl(cp, id, v) : cp->set_reg(cp, id, v);
    }
}

/*!
    \brief Helper accessor to simulated machine memory
*/
uint8_t  mips_read_b(MIPS *m, MIPS_Addr a, int *stat)
{
    return m->mem.read_b(&m->mem, a, stat);
}

/*!
    \brief Helper accessor to simulated machine memory
*/
uint16_t mips_read_h(MIPS *m, MIPS_Addr a, int *stat)
{
    return m->mem.read_h(&m->mem, a, stat);
}

/*!
    \brief Helper accessor to simulated machine memory
*/
uint32_t mips_read_w(MIPS *m, MIPS_Addr a, int *stat)
{
    return m->mem.read_w(&m->mem, a, stat);
}

/*!
    \brief Helper accessor to simulated machine memory
*/
uint64_t mips_read_d(MIPS *m, MIPS_Addr a, int *stat)
{
    return m->mem.read_d(&m->mem, a, stat);
}

/*!
    \brief Helper accessor to simulated machine memory
*/
void mips_write_b(MIPS *m, MIPS_Addr a, uint8_t b,  int *stat)
{
    m->mem.write_b(&m->mem, a, b, stat);
}

/*!
    \brief Helper accessor to simulated machine memory
*/
void mips_write_h(MIPS *m, MIPS_Addr a, uint16_t h, int *stat)
{
    m->mem.write_h(&m->mem, a, h, stat);
}

/*!
    \brief Helper accessor to simulated machine memory
*/
void mips_write_w(MIPS *m, MIPS_Addr a, uint32_t w, int *stat)
{
    m->mem.write_w(&m->mem, a, w, stat);
}

/*!
    \brief Helper accessor to simulated machine memory
*/
void mips_write_d(MIPS *m, MIPS_Addr a, uint64_t d, int *stat)
{
    m->mem.write_d(&m->mem, a, d, stat);
}

/*!
    \brief Add a breakpoint to a simulated machine
    \param m simulated machine
    \param type breakpoint type
    \param start start of break range
    \param end end of break range
    \param mask break range mask
    \return breakpoint id
    
    Breakpoint will cause simulation interruption if the value V being considered verifies :
    
    start <= (V & mask) <= end
    
    Where the value V can be a memory address or an opcode, depending on the breakpoint type.
*/
int mips_breakpoint_add(MIPS *m, int type, MIPS_Addr start, MIPS_Addr end, MIPS_Addr mask)
{
    BreakpointList *l = malloc(sizeof(BreakpointList));
    l->d.id    = m->breakpoints != NULL ? m->breakpoints->d.id + 1 : 0;
    l->d.type  = type;
    l->d.start = start;
    l->d.end   = end;
    l->d.mask  = mask;
    l->next    = m->breakpoints;
    m->breakpoints = l;
    
    return l->d.id;
}

/*!
    \brief Remove a breakpoint from a simulated machine
    \param m simulated machine
    \param id breakpoint id, as returned by mips_breakpoint_add
*/
void mips_breakpoint_remove(MIPS *m, int id)
{
    BreakpointList *prev = NULL, *bkpt = m->breakpoints;
    
    while ( bkpt != NULL )
    {
        if ( bkpt->d.id == id )
        {
            if ( prev != NULL )
                prev->next = bkpt->next;
            else
                m->breakpoints = bkpt->next;
            
            free(bkpt);
            break;
        }
        
        prev = bkpt;
        bkpt = bkpt->next;
    }
}

/*!
    \internal
    \brief Recursively deletes the content of a linked list of breakpoints
*/
void mips_breakpoint_list_delete(BreakpointList *l)
{
    if ( l != NULL )
    {
        mips_breakpoint_list_delete(l->next);
        free(l);
    }
}

/*!
    \brief Remove all breakpoints attached to a simulated machine
    \param m simulated machine
*/
void mips_breakpoint_clear(MIPS *m)
{
    mips_breakpoint_list_delete(m->breakpoints);
    m->breakpoints = NULL;
}

/*!
    \brief Count breakpoints
*/
int mips_breakpoint_count(MIPS *m, int type)
{
    int n = 0;
    BreakpointList *bkpt = m->breakpoints;
    
    while ( bkpt != NULL )
    {
        ++n;
        bkpt = bkpt->next;
    }
    
    return n;
}

/*!
    \brief Access breakpoint data
    \param m simulated machine
    \param id breakpoint id, as returned by mips_breakpoint_add
    
    Any changes to the values of this structure will be taken into account for future breakpoint tests
*/
Breakpoint* mips_breakpoint(MIPS *m, int id)
{
    BreakpointList *bkpt = m->breakpoints;
    
    while ( bkpt != NULL )
    {
        if ( bkpt->d.id == id )
            return &bkpt->d;
        
        bkpt = bkpt->next;
    }
    
    return NULL;
}
