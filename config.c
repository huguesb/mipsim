/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "config.h"

#include <string.h>

#include "mips.h"
#include "io.h"
#include "util.h"

/*!
    \brief Accessor to the global configuration of the application
*/
MIPSIM_Config* mipsim_config()
{
    static MIPSIM_Config cfg;
    return &cfg;
}

/*!
    \brief Initialize the global configuration based on CLI parameters
    \param argc argument count
    \param argv argument values
    
    \note Consumed argument are nullified (the first character of each
    consumed value is set to 0 to make them empty strings).
*/
int mipsim_config_init(int argc, char **argv)
{
    MIPSIM_Config *cfg = mipsim_config();
    
    cfg->io_mask = 0;
    cfg->trace_log = NULL;
    cfg->debug_log = NULL;
    
    cfg->arch = MIPS_I;
    
    cfg->reloc_text  = 0x00400000;
    cfg->reloc_data  = 0xFFFFFFFF;
    
    cfg->phys_memory_size  = 0x00100000;
    cfg->newlib_stack_size = 0x00800000;
    
    int error;
    
    for ( int i = 1; i < argc; ++i )
    {
        char *arg = argv[i];
        
        if ( !strcmp(arg, "--version") )
        {
            return 1;
        } else if ( !strcmp(arg, "--debug") ) {
            *argv[i] = 0;
            cfg->io_mask |= IO_DEBUG;
        } else if ( !strcmp(arg, "--trace") ) {
            *argv[i] = 0;
            cfg->io_mask |= IO_TRACE;
        } else if ( !strcmp(arg, "--debug-log") ) {
            *argv[i] = 0;
            if ( i+1 < argc )
            {
                cfg->debug_log = fopen(argv[++i], "wt");
                
                if ( cfg->debug_log == NULL )
                {
                    mipsim_printf(IO_WARNING, "CLI: unable to open %s for writing\n", argv[i]);
                }
                
                *argv[i] = 0;
            } else {
                mipsim_printf(IO_WARNING, "CLI: missing value for --debug-log switch\n");
            }
        } else if ( !strcmp(arg, "--trace-log") ) {
            *argv[i] = 0;
            if ( i+1 < argc )
            {
                cfg->trace_log = fopen(argv[++i], "wt");
                
                if ( cfg->trace_log == NULL )
                {
                    mipsim_printf(IO_WARNING, "CLI: unable to open %s for writing\n", argv[i]);
                }
                
                *argv[i] = 0;
            } else {
                mipsim_printf(IO_WARNING, "CLI: missing value for --trace-log switch\n");
            }
        } else if ( !strcmp(arg, "-t") ) {
            *argv[i] = 0;
            if ( i+1 < argc )
            {
                cfg->reloc_text = str_to_num(argv[++i], NULL, &error);
                *argv[i] = 0;
                
                if ( error )
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
                cfg->reloc_data = str_to_num(argv[++i], NULL, &error);
                *argv[i] = 0;
                
                if ( error )
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
                cfg->phys_memory_size = str_to_num(argv[++i], NULL, &error);
                *argv[i] = 0;
                
                if ( error )
                {
                    mipsim_printf(IO_WARNING, "CLI: invalid value for -s switch\n");
                }
            } else {
                mipsim_printf(IO_WARNING, "CLI: missing value for -s switch\n");
            }
        } else if ( !strcmp(arg, "-nss") ) {
            *argv[i] = 0;
            if ( i+1 < argc )
            {
                cfg->newlib_stack_size = str_to_num(argv[++i], NULL, &error);
                *argv[i] = 0;
                
                if ( error )
                {
                    mipsim_printf(IO_WARNING, "CLI: invalid value for -nss switch\n");
                }
            } else {
                mipsim_printf(IO_WARNING, "CLI: missing value for -nss switch\n");
            }
        }
    }
    
    return 0;
}

/*!
    \brief Release all memory used by the global configuration
*/
int mipsim_config_fini()
{
    MIPSIM_Config *cfg = mipsim_config();
    
    if ( cfg->trace_log != NULL )
    {
        fclose(cfg->trace_log);
    }
    
    if ( cfg->debug_log != NULL )
    {
        fclose(cfg->debug_log);
    }
    
    return 0;
}
