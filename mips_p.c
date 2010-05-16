/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "mips_p.h"

#include "io.h"

typedef struct _MIPS_Processor_Private {
    MIPS_Addr pc;
    
    MIPS_Native r[32];
    
    MIPS_Native hi, lo;
    int hi_lo_status;
    
    uint32_t ir;
} MIPS_Processor_Private;

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
            mipsim_printf(IO_TRACE, "\t%s = 0x%08x\n", mips_reg_name(gpr & 31), value);
            
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

void mips_init_processor(MIPS_Processor *hw)
{
    hw->signal_exception = _mips_sig_ex;
    
    hw->get_pc = _mips_get_pc;
    hw->set_pc = _mips_set_pc;
    
    hw->get_hi = _mips_get_hi;
    hw->set_hi = _mips_set_hi;
    hw->get_lo = _mips_get_lo;
    hw->set_lo = _mips_set_lo;
    
    hw->get_reg = _mips_get_gpr_p;
    hw->set_reg = _mips_set_gpr_p;
    
    hw->d = malloc(sizeof(MIPS_Processor_Private));
    memset(hw->d, 0, sizeof(MIPS_Processor_Private));
}

void mips_cleanup_processor(MIPS_Processor *hw)
{
    free(hw->d);
}

void mips_init_coprocessor(MIPS_Coprocessor *hw)
{
    hw->get_reg = NULL;
    hw->set_reg = NULL;
    hw->d = NULL;
}

void mips_cleanup_coprocessor(MIPS_Coprocessor *hw)
{
    //free(hw->d);
}