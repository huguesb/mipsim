/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "monitor.h"

#include "io.h"

char* fetch_str(MIPS *m, MIPS_Addr a)
{
    MIPS_Native len = 0;
    
    while ( mips_read_b(m, a + len, NULL) )
        ++len;
    
    char *s = (char*)malloc(len * sizeof(char));
    
    for ( MIPS_Native i = 0; i <= len; ++i )
        s[i] = mips_read_b(m, a + i, NULL);
}

int mips_monitor(MIPS *m, int entry)
{
    switch ( entry )
    {
        case 12 :
        {
            /* int open(char *path,int flags) */
            MIPS_Addr a0 = m->hw.get_reg(&m->hw, A0);
            MIPS_Addr a1 = m->hw.get_reg(&m->hw, A1);
            
            mipsim_printf(IO_TRACE, "open : %08x, %08x\n", a0, a1);
            
            char *s = fetch_str(m, a0);
            m->hw.set_reg(&m->hw, V0, mipsim_open(IO_MONITOR, s, a1));
            free(s);
            
            break;
        }
            
        case 14 :
        {
            /* int read(int file,char *ptr,int len) */
            MIPS_Addr a0 = m->hw.get_reg(&m->hw, A0);
            MIPS_Addr a1 = m->hw.get_reg(&m->hw, A1);
            MIPS_Addr a2 = m->hw.get_reg(&m->hw, A2);
            
            mipsim_printf(IO_TRACE, "read : %08x, %08x, %08x\n", a0, a1, a2);
            
            char *buffer = (char*)malloc(a2);
            m->hw.set_reg(&m->hw, V0, mipsim_read(IO_MONITOR, a0, buffer, a2));
            for ( MIPS_Native i = 0; i < a2; ++i )
                mips_write_b(m, a1 + i, buffer[i], NULL);
            free(buffer);
            
            break;
        }
            
        case 16 :
        {
            /* int write(int file,char *ptr,int len) */
            MIPS_Addr a0 = m->hw.get_reg(&m->hw, A0);
            MIPS_Addr a1 = m->hw.get_reg(&m->hw, A1);
            MIPS_Addr a2 = m->hw.get_reg(&m->hw, A2);
            
            mipsim_printf(IO_TRACE, "write : %08x, %08x, %08x\n", a0, a1, a2);
            
            char *buffer = (char*)malloc(a2);
            for ( MIPS_Native i = 0; i < a2; ++i )
                buffer[i] = mips_read_b(m, a1 + i, NULL);
            m->hw.set_reg(&m->hw, V0, mipsim_write(IO_MONITOR, a0, buffer, a2));
            free(buffer);
            
            break;
        }
            
        case 20 :
        {
            /* int close(int file) */
            MIPS_Addr a0 = m->hw.get_reg(&m->hw, A0);
            
            mipsim_printf(IO_TRACE, "close : %08x\n", a0);
            
            m->hw.set_reg(&m->hw, V0, mipsim_close(IO_MONITOR, a0));
            break;
        }
            
        case 22 :
            /* char inbyte(void) */
            
            mipsim_printf(IO_TRACE, "inbyte\n");
            
            m->hw.set_reg(&m->hw, V0, mipsim_inbyte(IO_MONITOR));
            break;
            
        case 24 :
        {
            /* void outbyte(char chr) : write a byte to "stdout" */
            MIPS_Addr a0 = m->hw.get_reg(&m->hw, A0);
            
            mipsim_printf(IO_TRACE, "outbyte : %08x\n", a0);
            
            mipsim_outbyte(IO_MONITOR, a0);
            break;
        }
            
        case 34 :
            /* void _exit() */
            
            mipsim_printf(IO_TRACE, "_exit");
            
            break;
            
        case 110 :
        {
            /* void get_mem_info(unsigned int *ptr) */
            /* in:  A0 = pointer to three word memory location */
            /* out: [A0 + 0] = size */
            /*      [A0 + 4] = instruction cache size */
            /*      [A0 + 8] = data cache size */
            
            MIPS_Native a = m->hw.get_reg(&m->hw, A0);
            
            mipsim_printf(IO_TRACE, "get_mem_info : 0x%08x", a);
            
            // hardcoded mem info
            // see memory.c:mips_init_memory() for more details
            // without this newlib CRT goes nuts...
            mips_write_w(m, a + 0, 0x00800000, NULL);
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
            MIPS_Addr a0 = m->hw.get_reg(&m->hw, A0);
            
            mipsim_printf(IO_TRACE, "printf : %08x\n", a0);
            break;
        }
            
        default:
            mipsim_printf(IO_TRACE, "mon : %d\n", entry);
            
            break;
    }
    
    return 0;
}