/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "mips_p.h"

#include "io.h"
#include "monitor.h"

typedef struct _MIPS_Processor_Private {
    MIPS *m;
    
    MIPS_Addr pc;
    
    MIPS_Native r[32];
    
    MIPS_Native hi, lo;
    int hi_lo_status;
    
    uint32_t ir;
} MIPS_Processor_Private;

void _mips_reset_p(MIPS_Processor *p)
{
    MIPS_Processor_Private *d = (MIPS_Processor_Private*)p->d;
    
    d->pc = 0;
    memset(&d->r, 0, 32 * sizeof(MIPS_Native));
    d->hi = d->lo = 0;
    d->hi_lo_status = 0;
    d->ir = 0;
}

MIPS_Native _mips_get_pc(MIPS_Processor *p)
{
    if ( p != NULL && p->d != NULL )
    {
        return ((MIPS_Processor_Private*)p->d)->pc;
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
        return 0;
    }
}

void _mips_set_pc(MIPS_Processor *p, MIPS_Native value)
{
    if ( p != NULL && p->d != NULL )
    {
        if ( value & 3 )
            p->signal_exception(p, MIPS_E_ADDRESS_ERROR);
        else
            ((MIPS_Processor_Private*)p->d)->pc = value;
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
}

uint32_t _mips_fetch(MIPS_Processor *p, int *stat)
{
    MIPS_Processor_Private *d = (MIPS_Processor_Private*)p->d;
    
    MIPS_Memory *m = &d->m->mem;
    
    if ( (d->pc & 0xFFFFF003) == 0xBFC00000 )
    {
        // monitor (I/O) entry points
        
        // execute requested operation (out of emulation)
        mips_monitor(d->m, (d->pc >> 2) & 0x1FF);
        
        // go back to emulation (j r31)
        d->pc = d->r[31];
        
        if ( stat )
            *stat = MEM_FWMON;
        
        return 0;
    }
    
    d->ir = m->read_w(m, d->pc, stat);
    
    return d->ir;
}

MIPS_Native _mips_get_hi(MIPS_Processor *p)
{
    if ( p != NULL && p->d != NULL )
    {
        return ((MIPS_Processor_Private*)p->d)->hi;
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
        return -1;
    }
}

void _mips_set_hi(MIPS_Processor *p, MIPS_Native value)
{
    if ( p != NULL && p->d != NULL )
    {
        ((MIPS_Processor_Private*)p->d)->hi = value;
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
}

MIPS_Native _mips_get_lo(MIPS_Processor *p)
{
    if ( p != NULL && p->d != NULL )
    {
        return ((MIPS_Processor_Private*)p->d)->lo;
    } else {
        mipsim_printf(IO_WARNING, "");
        return -1;
    }
}

void _mips_set_lo(MIPS_Processor *p, MIPS_Native value)
{
    if ( p != NULL && p->d != NULL )
    {
        ((MIPS_Processor_Private*)p->d)->lo = value;
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
}

MIPS_Native _mips_get_gpr_p(MIPS_Processor *p, int gpr)
{
    if ( p != NULL && p->d != NULL )
    {
        if ( gpr > 0 && gpr < 32 )
        {
            return ((MIPS_Processor_Private*)p->d)->r[gpr];
        } else if ( gpr ) {
            mipsim_printf(IO_WARNING, "");
        } else {
            return 0;
        }
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
    
    return -1;
}

void _mips_set_gpr_p(MIPS_Processor *p, int gpr, MIPS_Native value)
{
    if ( p != NULL && p->d != NULL )
    {
        if ( gpr > 0 && gpr < 32 )
        {
            mipsim_printf(IO_TRACE, "\t%s = 0x%08x\n", mips_gpr_name(gpr & 31), value);
            
            ((MIPS_Processor_Private*)p->d)->r[gpr] = value;
        } else if ( gpr ) {
            mipsim_printf(IO_WARNING, "");
        }
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
}

void _mips_sig_ex(MIPS_Processor *p, int exception)
{
    mipsim_printf(IO_WARNING, "exception %d\n", exception);
}

void mips_init_processor(MIPS *m)
{
    MIPS_Processor *hw = &m->hw;
    
    hw->reset = _mips_reset_p;
    
    hw->signal_exception = _mips_sig_ex;
    
    hw->get_pc = _mips_get_pc;
    hw->set_pc = _mips_set_pc;
    
    hw->fetch  = _mips_fetch;
    
    hw->get_hi = _mips_get_hi;
    hw->set_hi = _mips_set_hi;
    hw->get_lo = _mips_get_lo;
    hw->set_lo = _mips_set_lo;
    
    hw->get_reg = _mips_get_gpr_p;
    hw->set_reg = _mips_set_gpr_p;
    
    hw->d = malloc(sizeof(MIPS_Processor_Private));
    memset(hw->d, 0, sizeof(MIPS_Processor_Private));
    ((MIPS_Processor_Private*)hw->d)->m = m;
}

void mips_cleanup_processor(MIPS_Processor *hw)
{
    free(hw->d);
}

/////////////////////////////////////////////////////////////


typedef struct _MIPS_Coprocessor_Private {
    MIPS * m;
    
    int type;
    MIPS_Native r[32];
} MIPS_Coprocessor_Private;

typedef struct _MIPS_CP0_Private {
    MIPS_Coprocessor_Private p; // keep it on top : pseudo-polymorphism...
    
} MIPS_CP0_Private;

enum {
    FCSR_FCC_MASK      = 0xFE800000,
    FCSR_FS_MASK       = 0x01000000,
    FCSR_ZERO_MASK     = 0x007C0000,
    FCSR_CAUSE_MASK    = 0x0003F000,
    FCSR_ENABLES_MASK  = 0x00000F80,
    FCSR_FLAGS_MASK    = 0x0000007C,
    FCSR_RM_MASK       = 0x00000003,
    
    FCSR_FCC_SHIFT     = 23,
    FCSR_FS_SHIFT      = 22,
    FCSR_CAUSE_SHIFT   = 12,
    FCSR_ENABLES_SHIFT = 7,
    FCSR_FLAGS_SHIFT   = 2,
    FCSR_RM_SHIFT      = 0
};

typedef struct _MIPS_FPU_Private {
    MIPS_Coprocessor_Private p; // keep it on top : pseudo-polymorphism...
    
    MIPS_Native fir, fcsr;
} MIPS_FPU_Private;

void _mips_reset_cp(MIPS_Coprocessor *p)
{
    MIPS_Coprocessor_Private *d = (MIPS_Coprocessor_Private*)p->d;
    
    memset(&d->r, 0, 32 * sizeof(MIPS_Native));
}

void _mips_reset_fpu(MIPS_Coprocessor *p)
{
    _mips_reset_cp(p);
    
    MIPS_FPU_Private *d = (MIPS_FPU_Private*)p->d;
    
    d->fir = d->fcsr = 0;
}

MIPS_Native _mips_get_gpr_cp(MIPS_Coprocessor *p, int gpr)
{
    if ( p != NULL && p->d != NULL )
    {
        if ( gpr >= 0 && gpr < 32 )
        {
            return ((MIPS_Coprocessor_Private*)p->d)->r[gpr];
        } else {
            mipsim_printf(IO_WARNING, "");
            return 0;
        }
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
    
    return -1;
}

void _mips_set_gpr_cp(MIPS_Coprocessor *p, int gpr, MIPS_Native value)
{
    if ( p != NULL && p->d != NULL )
    {
        if ( gpr >= 0 && gpr < 32 )
        {
            mipsim_printf(IO_TRACE, "\t%s = 0x%08x\n", mips_fpr_name(gpr & 31), value);
            
            ((MIPS_Coprocessor_Private*)p->d)->r[gpr] = value;
        } else {
            mipsim_printf(IO_WARNING, "");
        }
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
}

MIPS_Native _mips_get_ctrl_fpu(MIPS_Coprocessor *p, int gpr)
{
    if ( p != NULL && p->d != NULL )
    {
        MIPS_FPU_Private *fpu = ((MIPS_FPU_Private*)p->d);
        
        // see MIPS ISA for details of this weirdness
        
        if ( gpr == 0 )
        {
            // FP Implementation / Revision
            return fpu->fir;
        } else if ( gpr == 25 ) {
            // FP Condition Codes
            MIPS_Native fcc = (fpu->fcsr & FCSR_FCC_MASK) >> FCSR_FCC_SHIFT;
            return (fcc & 1) | (fcc >> 1);
        } else if ( gpr == 26 ) {
            // FP Exceptions
            return (fpu->fcsr & (FCSR_CAUSE_MASK | FCSR_FLAGS_MASK));
        } else if ( gpr == 28 ) {
            // FP Enables
            return ((fpu->fcsr & FCSR_FS_MASK) >> FCSR_FS_SHIFT) | (fpu->fcsr & (FCSR_ENABLES_MASK | FCSR_RM_MASK));
        } else if ( gpr == 31 ) {
            // FP Control / Status
            return fpu->fcsr;
        } else {
            mipsim_printf(IO_WARNING, "Trying to access inexistant FPU ctrl register");
//             mips_stop(m, MIPS_UNPREDICTABLE);
            return 0;
        }
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
    
    return -1;
}

void _mips_set_ctrl_fpu(MIPS_Coprocessor *p, int gpr, MIPS_Native value)
{
    if ( p != NULL && p->d != NULL )
    {
        MIPS_FPU_Private *fpu = ((MIPS_FPU_Private*)p->d);
        
        if ( gpr == 25 )
        {
            // FP Condition Codes
            fpu->fcsr &= ~FCSR_FCC_MASK;
            fpu->fcsr |= (((value & 1) | (value << 1)) << FCSR_FCC_SHIFT) & FCSR_FCC_MASK;
        } else if ( gpr == 26 ) {
            // FP Exceptions
            fpu->fcsr &= ~(FCSR_CAUSE_MASK | FCSR_FLAGS_MASK);
            fpu->fcsr |= value & (FCSR_CAUSE_MASK | FCSR_FLAGS_MASK);
            
            // TODO : test for exception trigger
            
        } else if ( gpr == 28 ) {
            // FP Enables
            fpu->fcsr &= ~(FCSR_FS_MASK | FCSR_ENABLES_MASK | FCSR_RM_MASK);
            fpu->fcsr |= (value & (FCSR_ENABLES_MASK | FCSR_RM_MASK)) | ((value << FCSR_FS_SHIFT) & FCSR_FS_MASK);
            
            // TODO : test for exception trigger
            
        } else if ( gpr == 31 ) {
            // FP Control / Status
            fpu->fcsr = value;
            
            // TODO : test for exception trigger
            
        } else {
            mipsim_printf(IO_WARNING, "Trying to write inexistant FPU ctrl register");
        }
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
}

MIPS_Native _mips_get_ctrl_cp0(MIPS_Coprocessor *p, int gpr)
{
    if ( p != NULL && p->d != NULL )
    {
        MIPS_CP0_Private *cp0 = ((MIPS_CP0_Private*)p->d);
        
        // see MIPS ISA for details of this weirdness
        
        if ( 0 )
        {
            
        } else {
            mipsim_printf(IO_WARNING, "Trying to access inexistant CP0 ctrl register");
//             mips_stop(m, MIPS_UNPREDICTABLE);
            return 0;
        }
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
    
    return -1;
}

void _mips_set_ctrl_cp0(MIPS_Coprocessor *p, int gpr, MIPS_Native value)
{
    if ( p != NULL && p->d != NULL )
    {
        MIPS_CP0_Private *cp0 = ((MIPS_CP0_Private*)p->d);
        
        if ( 0 )
        {
            
        } else if ( gpr ) {
            mipsim_printf(IO_WARNING, "Trying to write inexistant CP0 ctrl register");
        }
    } else {
        mipsim_printf(IO_WARNING, "(NULL)\n");
    }
}

void mips_init_coprocessor(MIPS *m, int n)
{
    MIPS_Coprocessor *hw = &m->cp[n];
    
    hw->reset   = _mips_reset_cp;
    
    hw->get_reg = _mips_get_gpr_cp;
    hw->set_reg = _mips_set_gpr_cp;
    
    switch ( n )
    {
        case 0 :
            hw->get_ctrl = _mips_get_ctrl_cp0;
            hw->set_ctrl = _mips_set_ctrl_cp0;
            hw->d = malloc(sizeof(MIPS_CP0_Private));
            memset(hw->d, 0, sizeof(MIPS_CP0_Private));
            break;
            
        case 1 :
            hw->reset    = _mips_reset_fpu;
            hw->get_ctrl = _mips_get_ctrl_fpu;
            hw->set_ctrl = _mips_set_ctrl_fpu;
            hw->d = malloc(sizeof(MIPS_FPU_Private));
            memset(hw->d, 0, sizeof(MIPS_FPU_Private));
            break;
            
        default:
            hw->get_ctrl = _mips_get_gpr_cp;
            hw->set_ctrl = _mips_set_gpr_cp;
            hw->d = malloc(sizeof(MIPS_Coprocessor_Private));
            memset(hw->d, 0, sizeof(MIPS_Coprocessor_Private));
            break;
    }
    
    ((MIPS_Coprocessor_Private*)hw->d)->m = m;
}

void mips_cleanup_coprocessor(MIPS_Coprocessor *hw)
{
    free(hw->d);
}
