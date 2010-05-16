/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "cli.h"

#include "io.h"
#include "config.h"

#include <stdio.h>
#include <string.h>

#define CMD_BUF_SZ 256

typedef int (*command_handler)(const char *cmd, MIPS *m);

typedef struct _Command {
    const char *name;
    command_handler handler; 
} Command;

int mipsim_cli(MIPS *m)
{
    char cmd_buf[CMD_BUF_SZ];
    
    while ( 1 )
    {
        printf("mipsim> ");
        
        // very important to flush stdout before waiting for input
        // not doing it causes all sort of issues in interpocess I/O
        fflush(stdout);
        
        if ( fgets(cmd_buf, CMD_BUF_SZ, stdin) == cmd_buf )
        {
            // remove trailing EOL left by fgets
            cmd_buf[strlen(cmd_buf) - 1] = '\0';
            
            int ret = exec_command(cmd_buf, m);
            
            if ( ret == -1 )
                printf("Unknown command...\n");
            else if ( ret == -2 )
                break;
        } else {
            printf("Error");
        }
    }
    
    return 0;
}


int cli_quit(const char *cmd, MIPS *m)
{
    return -2;
}

void print_status(MIPS *m)
{
    switch ( m->stop_reason )
    {
        case MIPS_OK :
            break;
            
        case MIPS_INVALID :
            printf("Invalid instruction\n");
            break;
            
        case MIPS_UNSUPPORTED :
            printf("Unsupported instruction\n");
            break;
            
        case MIPS_TRAP :
            printf("Trap\n");
            break;
            
        case MIPS_BREAK :
            printf("Break\n");
            break;
            
        case MIPS_EXCEPTION :
            printf("Exception\n");
            break;
            
        case MIPS_ERROR :
            printf("Internal error\n");
            break;
            
        case MIPS_UNPREDICTABLE :
            printf("Unpredictable behavior\n");
            break;
            
        default:
            printf("Unknown status\n");
            break;
    }
}

int cli_run(const char *cmd, MIPS *m)
{
    int ret;
    
    do {
        ret = mips_exec(m, 1);
    } while ( ret == MIPS_OK );
    
    print_status(m);
    
    return ret;
}

int cli_step(const char *cmd, MIPS *m)
{
    int n = 0;
    const char *sep = strchr(cmd, ' ');
    
    if ( sep != NULL )
    {
        while ( *(++sep) != '\0' )
        {
            if ( *sep >= '0' && *sep <= '9' )
                n = n * 10 + (*sep - '0');
            else
                break;
        }
    }
    
    return mips_exec(m, n != 0 ? n : 1);
}

int cli_trace(const char *cmd, MIPS *m)
{
    int n = 0;
    const char *sep = strchr(cmd, ' ');
    
    if ( sep != NULL )
    {
        while ( *(++sep) != '\0' )
        {
            if ( *sep >= '0' && *sep <= '9' )
                n = n * 10 + (*sep - '0');
            else
                break;
        }
    }
    
    if ( n )
        mipsim_config()->io_mask |= IO_TRACE;
    else
        mipsim_config()->io_mask &= ~IO_TRACE;
}

int cli_dump(const char *cmd, MIPS *m)
{
    printf("    pc = 0x%08x\n", m->hw.get_pc(&m->hw));
    
    printf("    hi = 0x%08x        lo = 0x%08x\n", m->hw.get_hi(&m->hw), m->hw.get_lo(&m->hw));
    
    for ( int i = 0; i < 8; ++i )
    {
        printf("%6s = 0x%08x    %6s = 0x%08x    %6s = 0x%08x    %6s = 0x%08x\n",
               mips_reg_name(4*i),     m->hw.get_reg(&m->hw, 4*i),
               mips_reg_name(4*i + 1), m->hw.get_reg(&m->hw, 4*i+1),
               mips_reg_name(4*i + 2), m->hw.get_reg(&m->hw, 4*i+2),
               mips_reg_name(4*i + 3), m->hw.get_reg(&m->hw, 4*i+3));
    }
    
    return 0;
}

static const Command commands[] = {
    // shortcuts
    {"r", cli_run},
    {"s", cli_step},
    {"d", cli_dump},
    {"t", cli_trace},
    {"q", cli_quit},
    
    // full names
    {"run", cli_run},
    {"step", cli_step},
    {"dump", cli_dump},
    {"trace", cli_trace},
    {"quit", cli_quit},
    {NULL, NULL}
};

int exec_command(const char *cmd, MIPS *m)
{
    const Command *c = commands;
    
    while ( c->name != NULL && c->handler != NULL )
    {
        for ( int i = 0; ; ++i )
        {
            if ( c->name[i] == '\0' && (cmd[i] == '\0' || cmd[i] == ' ') )
            {
                return c->handler(cmd, m);
            } else if ( c->name[i] != cmd[i] ) {
                break;
            }
        }
        
        ++c;
    }
    
    return -1;
}
