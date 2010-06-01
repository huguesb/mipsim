/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "monitor.h"

/*!
    \file monitor.c
    \brief Monitor / syscall entry points for use by simulator
    \author Hugues Bruant
*/

#include "io.h"
#include "util.h"
#include "config.h"

/*!
    \internal
    \brief Fetch a NULL-terminated string from the memory of a simulated machine
*/
char* fetch_str(MIPS *m, MIPS_Addr a)
{
    MIPS_Native len = 0;
    
    while ( mips_read_b(m, a + len, NULL) )
        ++len;
    
    char *s = (char*)malloc(len * sizeof(char));
    
    for ( MIPS_Native i = 0; i <= len; ++i )
        s[i] = mips_read_b(m, a + i, NULL);
    
    return s;
}

enum {
    SYSCALL_BUF_SZ = 128
};

/*!
    \brief Syscall handler
    
    Support for some syscalls required by project spec.
*/
int mips_syscall(MIPS *m, int code)
{
    switch ( code )
    {
        case 1 :
        {
            MIPS_Native a0 = mips_get_reg(m, A0);
            mipsim_printf(IO_MONITOR, "%d", a0);
            break;
        }
            
        case 4 :
        {
            MIPS_Native a0 = mips_get_reg(m, A0);
            char *s = fetch_str(m, a0);
            mipsim_printf(IO_MONITOR, "%s", s);
            free(s);
            break;
        }
            
        case 5 :
        {
            char buf[SYSCALL_BUF_SZ];
            mipsim_read(IO_MONITOR, 0, buf, SYSCALL_BUF_SZ);
            
            mips_set_reg(m, V0, str_to_num(buf, NULL, NULL));
            break;
        }
            
        case 8 :
        {
            MIPS_Native a0 = mips_get_reg(m, A0);
            MIPS_Native a1 = mips_get_reg(m, A1);
            
            char *buffer = (char*)malloc(a1);
            mipsim_read(IO_MONITOR, 0, buffer, a1);
            for ( MIPS_Native i = 0; i < a1; ++i )
                mips_write_b(m, a0 + i, buffer[i], NULL);
            free(buffer);
            
            break;
        }
            
        case 10 :
            mips_stop(m, MIPS_QUIT);
            return MIPS_QUIT;
            break;
            
        default:
            mipsim_printf(IO_WARNING, "Unknown syscall %d\n", code);
            break;
    }
    
    return MIPS_OK;
}

/*!
    \brief Monitor handler
    
    Support for some idt/pmon entry points required for C/newlib support. 
*/
int mips_monitor(MIPS *m, int entry)
{
    MIPSIM_Config *cfg = mipsim_config();
    
    switch ( entry )
    {
        case 12 :
        {
            /* int open(char *path,int flags) */
            MIPS_Native a0 = mips_get_reg(m, A0);
            MIPS_Native a1 = mips_get_reg(m, A1);
            
            mipsim_printf(IO_TRACE, "open : %08x, %08x\n", a0, a1);
            
            char *s = fetch_str(m, a0);
            mips_set_reg(m, V0, mipsim_open(IO_MONITOR, s, a1));
            free(s);
            
            break;
        }
            
        case 14 :
        {
            /* int read(int file,char *ptr,int len) */
            MIPS_Native a0 = mips_get_reg(m, A0);
            MIPS_Native a1 = mips_get_reg(m, A1);
            MIPS_Native a2 = mips_get_reg(m, A2);
            
            mipsim_printf(IO_TRACE, "read : %08x, %08x, %08x\n", a0, a1, a2);
            
            char *buffer = (char*)malloc(a2);
            mips_set_reg(m, V0, mipsim_read(IO_MONITOR, a0, buffer, a2));
            for ( MIPS_Native i = 0; i < a2; ++i )
                mips_write_b(m, a1 + i, buffer[i], NULL);
            free(buffer);
            
            break;
        }
            
        case 16 :
        {
            /* int write(int file,char *ptr,int len) */
            MIPS_Native a0 = mips_get_reg(m, A0);
            MIPS_Native a1 = mips_get_reg(m, A1);
            MIPS_Native a2 = mips_get_reg(m, A2);
            
            mipsim_printf(IO_TRACE, "write : %08x, %08x, %08x\n", a0, a1, a2);
            
            char *buffer = (char*)malloc(a2);
            for ( MIPS_Native i = 0; i < a2; ++i )
                buffer[i] = mips_read_b(m, a1 + i, NULL);
            mips_set_reg(m, V0, mipsim_write(IO_MONITOR, a0, buffer, a2));
            free(buffer);
            
            break;
        }
            
        case 20 :
        {
            /* int close(int file) */
            MIPS_Native a0 = mips_get_reg(m, A0);
            
            mipsim_printf(IO_TRACE, "close : %08x\n", a0);
            
            mips_set_reg(m, V0, mipsim_close(IO_MONITOR, a0));
            break;
        }
            
        case 22 :
            /* char inbyte(void) */
            
            mipsim_printf(IO_TRACE, "inbyte\n");
            
            mips_set_reg(m, V0, mipsim_inbyte(IO_MONITOR));
            break;
            
        case 24 :
        {
            /* void outbyte(char chr) : write a byte to "stdout" */
            MIPS_Native a0 = mips_get_reg(m, A0);
            
            mipsim_printf(IO_TRACE, "outbyte : %08x\n", a0);
            
            mipsim_outbyte(IO_MONITOR, a0);
            break;
        }
            
        case 34 :
            /* void _exit() */
            
            mipsim_printf(IO_TRACE, "_exit");
            mips_stop(m, MIPS_QUIT);
            return MIPS_QUIT;
            
        case 110 :
        {
            /* void get_mem_info(unsigned int *ptr) */
            /* in:  A0 = pointer to three word memory location */
            /* out: [A0 + 0] = size */
            /*      [A0 + 4] = instruction cache size */
            /*      [A0 + 8] = data cache size */
            
            MIPS_Native a = mips_get_reg(m, A0);
            
            mipsim_printf(IO_TRACE, "get_mem_info : 0x%08x", a);
            
            mips_write_w(m, a + 0, cfg->newlib_stack_size, NULL);
            mips_write_w(m, a + 4, 0, NULL);
            mips_write_w(m, a + 8, 0, NULL);
            
            break;
        }
            
        case 316 :
        {
            /* PMON printf */
            /* in:  A0 = pointer to format string */
            /*      A1 = optional argument 1 */
            /*      A2 = optional argument 2 */
            /*      A3 = optional argument 3 */
            /* out: void */
            MIPS_Addr a0 = mips_get_reg(m, A0);
            
            mipsim_printf(IO_TRACE, "printf : %08x\n", a0);
            break;
        }
            
        default:
            mipsim_printf(IO_WARNING, "mon : %d\n", entry);
            
            break;
    }
    
    return 0;
}
