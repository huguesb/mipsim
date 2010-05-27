/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#ifndef _MIPSIM_CONFIG_H_
#define _MIPSIM_CONFIG_H_

#include <stdio.h>
#include <inttypes.h>

typedef struct _MIPSIM_Config {
    int io_mask;
    FILE *trace_log;
    FILE *debug_log;
    
    int arch;
    
    uint32_t reloc_text, reloc_data;
    
    uint32_t phys_memory_size;
    uint32_t newlib_stack_size;
} MIPSIM_Config;

MIPSIM_Config* mipsim_config();
int mipsim_config_init(int argc, char **argv);
int mipsim_config_fini();

#endif
