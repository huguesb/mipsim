/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#ifndef _MIPSIM_CONFIG_H_
#define _MIPSIM_CONFIG_H_

#include <stdio.h>

typedef struct _MIPSIM_Config {
    int io_mask;
    FILE *trace_log;
    FILE *debug_log;
    
} MIPSIM_Config;

MIPSIM_Config* mipsim_config();

#endif
