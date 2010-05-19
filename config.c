/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "config.h"

#include <string.h>

#include "mips.h"
#include "io.h"

MIPSIM_Config* mipsim_config()
{
    static MIPSIM_Config cfg;
    return &cfg;
}

int mipsim_config_init(int argc, char **argv)
{
    MIPSIM_Config *cfg = mipsim_config();
    
    cfg->io_mask = 0;
    cfg->trace_log = NULL;
    cfg->debug_log = NULL;
    
    cfg->arch = MIPS_I;
    
    cfg->reloc_text  = 0x00400000;
    cfg->reloc_data  = 0xFFFFFFFF;
    cfg->address_max = 0x00100000;
    
    for ( int i = 1; i < argc; ++i )
    {
        char *arg = argv[i];
        
        if ( !strcmp(arg, "--version") )
        {
            return 1;
        } else if ( !strcmp(arg, "--debug") ) {
            *argv[i] = 0;
            cfg->io_mask |= IO_DEBUG;
        } else if ( !strcmp(arg, "-t") ) {
            *argv[i] = 0;
            if ( i+1 < argc )
            {
                cfg->reloc_text = strtoul(argv[++i], &arg, 0);
                *argv[i] = 0;
                
                if ( !cfg->reloc_text )
                {
                    mipsim_printf(IO_WARNING, "CLI: invalid value for -t switch\n");
                }
            } else {
                mipsim_printf(IO_WARNING, "CLI: missing value for -t switch\n");
            }
        } else if ( !strcmp(arg, "-d") ) {
            *argv[i] = 0;
            if ( i+1 < argc )
            {
                cfg->reloc_data = strtoul(argv[++i], &arg, 0);
                *argv[i] = 0;
                
                if ( !cfg->reloc_data )
                {
                    mipsim_printf(IO_WARNING, "CLI: invalid value for -d switch\n");
                }
            } else {
                mipsim_printf(IO_WARNING, "CLI: missing value for -d switch\n");
            }
        } else if ( !strcmp(arg, "-s") ) {
            *argv[i] = 0;
            if ( i+1 < argc )
            {
                cfg->address_max = strtoul(argv[++i], &arg, 0);
                *argv[i] = 0;
                
                if ( !cfg->address_max )
                {
                    mipsim_printf(IO_WARNING, "CLI: invalid value for -s switch\n");
                }
            } else {
                mipsim_printf(IO_WARNING, "CLI: missing value for -s switch\n");
            }
        }
    }
    
    return 0;
}
