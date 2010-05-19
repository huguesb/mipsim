/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "shell.h"

#include "io.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef int (*command_handler)(int argc, char **argv, MIPS *m);

enum {
    COMMAND_OK,
    COMMAND_FAIL,
    COMMAND_INVALID,
    COMMAND_PARAM_COUNT,
    COMMAND_PARAM_TYPE,
    
    COMMAND_EXIT = -1
};

typedef struct _Command {
    const char *name;
    command_handler handler; 
} Command;

int cli_quit(int argc, char **argv, MIPS *m)
{
    (void)argc; (void)argv; (void)m;
    
    return COMMAND_EXIT;
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

int cli_run(int argc, char **argv, MIPS *m)
{
    (void)argc; (void)argv;
    
    int ret;
    
    do {
        ret = mips_exec(m, 1);
    } while ( ret == MIPS_OK );
    
    print_status(m);
    
    return ret ? COMMAND_FAIL : COMMAND_OK;
}

int cli_step(int argc, char **argv, MIPS *m)
{
    int n = 0;
    
    if ( argc == 2 )
    {
        const char *param = argv[1];
        
        while ( *param != '\0' )
        {
            if ( *param < '0' || *param > '9' )
            {
                printf("step expects an integer as optional parameter\n");
                return COMMAND_PARAM_TYPE;
            }
            
            n = n * 10 + (*param - '0');
            
            ++param;
        }
    } else if ( argc != 1 ) {
        printf("step expects at most one integer parameter\n");
        return COMMAND_PARAM_COUNT;
    } else {
        n = 1;
    }
    
    return mips_exec(m, n) ? COMMAND_FAIL : COMMAND_OK;
}

int cli_trace(int argc, char **argv, MIPS *m)
{
    (void)m;
    
    int n = 0;
    
    if ( argc == 2 )
    {
        const char *param = argv[1];
        
        if ( *param && !param[1] )
        {
            n = (*param - '0');
        } else {
            printf("trace expects a boolean parameter\n");
            return COMMAND_PARAM_TYPE;
        }
    } else {
        printf("trace expects exactly one boolean parameter");
        return COMMAND_PARAM_COUNT;
    }
    
    if ( n )
        mipsim_config()->io_mask |= IO_TRACE;
    else
        mipsim_config()->io_mask &= ~IO_TRACE;
    
    return COMMAND_OK;
}

int cli_dump(int argc, char **argv, MIPS *m)
{
    (void)argc; (void)argv;
    
    printf("    pc = 0x%08x\n", m->hw.get_pc(&m->hw));
    
    printf("    hi = 0x%08x        lo = 0x%08x\n", m->hw.get_hi(&m->hw), m->hw.get_lo(&m->hw));
    
    for ( int i = 0; i < 8; ++i )
    {
        printf("%6s = 0x%08x    %6s = 0x%08x    %6s = 0x%08x    %6s = 0x%08x\n",
               mips_gpr_name(4*i),     m->hw.get_reg(&m->hw, 4*i),
               mips_gpr_name(4*i + 1), m->hw.get_reg(&m->hw, 4*i+1),
               mips_gpr_name(4*i + 2), m->hw.get_reg(&m->hw, 4*i+2),
               mips_gpr_name(4*i + 3), m->hw.get_reg(&m->hw, 4*i+3));
    }
    
    return COMMAND_OK;
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

int execute_command(int argc, char **argv, MIPS *m)
{
    const Command *c = commands;
    
    while ( c->name != NULL && c->handler != NULL )
    {
        for ( int i = 0; ; ++i )
        {
            if ( c->name[i] == '\0' && (argv[0][i] == '\0' || argv[0][i] == ' ') )
            {
                return c->handler(argc, argv, m);
            } else if ( c->name[i] != argv[0][i] ) {
                break;
            }
        }
        
        ++c;
    }
    
    return COMMAND_INVALID;
}

/*
    input : a space-separated command line
    output : a list of pointers to start of each "token" of the command line
    
    notes :
        * all whitespaces are turned into string terminators in the input
        * output is malloc'd and must be free'd by caller
        * quoted whitespaces are preserved
*/
char** tokenize(char *cmd, int *argc)
{
    int n = 0, fresh = 1, quoted = 0;
    char **argv, *iter = cmd;
    
    // nullify separators and count entities
    while ( *iter )
    {
        if ( *iter <= ' ' && !quoted )
        {
            *iter = 0;
            fresh = 1;
        } else {
            if ( *iter == '\"' )
                quoted = !quoted;
            
            if ( fresh )
                ++n;
            
            fresh = 0;
        }
        
        ++iter;
    }
    
    int i = 0;
    
    // allocate output array
    argv = malloc(n * sizeof(char*));
    
    // fill output array
    while ( cmd < iter )
    {
        // skip null terminators
        while ( !*cmd && cmd < iter )
            ++cmd;
        
        // store the start of one entity
        if ( cmd < iter )
        {
            argv[i] = cmd;
            
            // move past the entity
            do {
                ++cmd;
            } while ( *cmd && cmd < iter );
        }
        
        ++i;
    }
    
    if ( argc )
        *argc = n;
    
    return argv;
}

int mipsim_cli(MIPS *m)
{
    int argc, ret = COMMAND_OK;
    char *cmd, **argv;
    
    while ( ret != COMMAND_EXIT )
    {
        cmd = readline("mipsim> ");
        
        if ( cmd && *cmd )
        {
            add_history(cmd);
            
            argv = tokenize(cmd, &argc);
            
            ret = execute_command(argc, argv, m);
            
            switch ( ret )
            {
                case COMMAND_INVALID :
                    printf("Invalid command.\n");
                    break;
                    
                default:
                    break;
            }
            
            free(argv);
        } else {
            printf("I/O Error\n");
        }
        
        free(cmd);
    }
    
    return 0;
}
