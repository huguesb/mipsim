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

/*!
    \file io.c
    \brief I/O abstraction layer
    \author Hugues Bruant
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*!
    \brief File I/O gateway
*/
int mipsim_open (int cxt, const char *path, int flags)
{
    (void)cxt; (void)path; (void)flags;
    
    printf("Unexpected gateway open\n");
    return 0; //open(path, flags);
}

/*!
    \brief File I/O gateway
*/
int mipsim_read (int cxt, int file, char *d, int len)
{
    if ( cxt == IO_MONITOR )
    {
        if ( file == 0 )
        {
            char *ret = fgets(d, len, mipsim_config()->mon_in);
            
            // remove extra LF
            // d[strlen(d) < (unsigned int)len ? strlen(d) : (unsigned int)len - 1] = 0;
            
            return ret != NULL;
        } else {
            printf("Unexpected monitor read on fd %d\n", file);
        }
    } else {
        printf("Unexpected gateway read\n");
    }
    
    return 0;
}

/*!
    \brief File I/O gateway
*/
int mipsim_write(int cxt, int file, char *d, int len)
{
    if ( cxt == IO_MONITOR )
    {
        (void)file;
        
        int ret = fwrite(d, sizeof(char), len, mipsim_config()->mon_out);
        fflush(mipsim_config()->mon_out);
        return ret;
    } else {
        printf("Unexpected gateway write\n");
    }
    
    return 0;
}

/*!
    \brief File I/O gateway
*/
int mipsim_close(int cxt, int file)
{
    if ( cxt == IO_MONITOR )
    {
        if ( file < 0 || file > 3 )
            printf("Unexpected monitor close\n");
    } else {
        printf("Unexpected gateway close\n");
    }
    
    return 0; //close(file);
}

/*!
    \brief Console I/O gateway
*/
char mipsim_inbyte (int cxt)
{
    if ( cxt == IO_MONITOR )
    {
        return fgetc(mipsim_config()->mon_in);
    } else {
        printf("Unexpected gateway inbyte\n");
    }
    
    return 0;
}

/*!
    \brief Console I/O gateway
*/
void mipsim_outbyte(int cxt, char c)
{
    if ( cxt == IO_MONITOR )
    {
        fputc(c, mipsim_config()->mon_in);
    } else {
        printf("Unexpected gateway outbyte\n");
    }
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
        case IO_MONITOR :
            vfprintf(cfg->mon_out, fmt, args);
            
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
