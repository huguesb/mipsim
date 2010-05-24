/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "io.h"

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/*!
    \brief File I/O gateway
*/
int mipsim_open (int cxt, const char *path, int flags)
{
    return 0; //open(path, flags);
}

/*!
    \brief File I/O gateway
*/
int mipsim_read (int cxt, int file, char *d, int len)
{
    if ( cxt == IO_MONITOR && file == 0 )
    {
        return read(0, d, len);
    } else {
        printf("Damn I/O!");
    }
    
    //int ret = fread(d, sizeof(char), len, stdin);
    return 0;
}

/*!
    \brief File I/O gateway
*/
int mipsim_write(int cxt, int file, char *d, int len)
{
    if ( cxt == IO_MONITOR )
    {
        int ret = fwrite(d, sizeof(char), len, stdout);
        fflush(stdout);
        return ret;
    } else {
        printf("Damn I/O!");
    }
    
    return 0;
}

/*!
    \brief File I/O gateway
*/
int mipsim_close(int cxt, int file)
{
    return 0; //close(file);
}

/*!
    \brief Console I/O gateway
*/
char mipsim_inbyte (int cxt)
{
    return fgetc(stdin);
}

/*!
    \brief Console I/O gateway
*/
void mipsim_outbyte(int cxt, char c)
{
//     putc(c);
}

/*!
    \brief printf gateway
    
    This function redirects printf output to the appropriate medium based on \a cxt and
    the global configuration
    
    \see mipsim_config
*/
int mipsim_printf(int cxt, const char *fmt, ...)
{
    int ret = 0;
    MIPSIM_Config *cfg = mipsim_config();
    
    va_list args;
    va_start(args, fmt);
    
    switch ( cxt )
    {
        case IO_TRACE :
            if ( cfg->io_mask & IO_TRACE )
                ret = cfg->trace_log != NULL ? vfprintf(cfg->trace_log, fmt, args) : vprintf(fmt, args);
            break;
            
        case IO_DEBUG :
            if ( cfg->io_mask & IO_DEBUG )
                ret = cfg->debug_log != NULL ? vfprintf(cfg->debug_log, fmt, args) : vprintf(fmt, args);
            break;
            
        default:
            ret = vprintf(fmt, args);
            break;
    }
    
    va_end(args);
    
    return ret;
}
