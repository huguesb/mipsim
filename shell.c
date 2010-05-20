/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "shell.h"

#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "io.h"
#include "config.h"
#include "mipself.h"

typedef int (*command_handler)(int argc, char **argv, MIPS *m);

enum {
    COMMAND_OK,
    COMMAND_FAIL,
    COMMAND_INVALID,
    COMMAND_PARAM_COUNT,
    COMMAND_PARAM_TYPE,
    COMMAND_NEED_TARGET,
    
    COMMAND_EXIT = -1
};

typedef struct _Command {
    const char *name;
    command_handler handler; 
    const char *help;
} Command;

int shell_load(int argc, char **argv, MIPS **m, ELF_File **f)
{
    if ( argc <= 1 )
    {
        printf("load expects one string parameter\n");
        return 1;
    }
    
    if ( *m != NULL )
    {
        /*
            Create simulator structures
            TODO : specify arch via a command line switch and alter if needed...
        */
        mips_reset(*m);
    } else {
        /*
            Create simulator structures
            TODO : specify arch via a command line switch...
        */
        *m = mips_create(mipsim_config()->arch);
        
        if ( *m == NULL )
        {
            printf("MIPSim: Failed to allocate memory for MIPS machine\n");
            return COMMAND_FAIL;
        }
    }
    
    /*
        discard any previously loaded program
    */
    if ( *f != NULL )
    {
        elf_file_destroy(*f);
        *f = NULL;
    }
    
    *f = elf_file_create();
    
    if ( *f == NULL )
    {
        printf("MIPSim: Failed to allocate memory for ELF file\n");
        return COMMAND_FAIL;
    }
    
    /*
        Load ELF file
    */
    if ( elf_file_load(*f, argv[1]) )
        return COMMAND_FAIL;
    
    /*
        Map ELF file content to emulated machine memory
    */
    mips_load_elf(*m, *f);
    
    return COMMAND_OK;
}

int shell_reset(int argc, char **argv, MIPS *m)
{
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    (void)argc; (void)argv;
    
    m->hw.reset(&m->hw);
    for ( int i = 0; i < 4; ++i )
        m->cp[i].reset(&m->cp[i]);
    
    return COMMAND_OK;
}

int shell_quit(int argc, char **argv, MIPS *m)
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

int shell_run(int argc, char **argv, MIPS *m)
{
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    if ( argc == 2 )
    {
        MIPS_Addr addr = strtoul(argv[1], NULL, 0);
        printf("running from 0x%08x\n", addr);
        m->hw.set_pc(&m->hw, addr);
    } else if ( argc != 1 ) {
        printf("run expects at most one parameter\n");
        return COMMAND_PARAM_COUNT;
    }
    
    int ret;
    
    do {
        ret = mips_exec(m, 1, 0);
    } while ( ret == MIPS_OK );
    
    print_status(m);
    
    return ret ? COMMAND_FAIL : COMMAND_OK;
}

int shell_step(int argc, char **argv, MIPS *m)
{
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    int n = 0;
    
    if ( argc == 2 )
    {
        n = strtoul(argv[1], NULL, 0);
    } else if ( argc != 1 ) {
        printf("step expects at most one integer parameter\n");
        return COMMAND_PARAM_COUNT;
    } else {
        n = 1;
    }
    
    return mips_exec(m, n, 1) ? COMMAND_FAIL : COMMAND_OK;
}

int shell_stepi(int argc, char **argv, MIPS *m)
{
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    int n = 0;
    
    if ( argc == 2 )
    {
        n = strtoul(argv[1], NULL, 0);
    } else if ( argc != 1 ) {
        printf("step expects at most one integer parameter\n");
        return COMMAND_PARAM_COUNT;
    } else {
        n = 1;
    }
    
    return mips_exec(m, n, 0) ? COMMAND_FAIL : COMMAND_OK;
}

int shell_trace(int argc, char **argv, MIPS *m)
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

int shell_dasm(int argc, char **argv, MIPS *m)
{
    (void)argc; (void)argv;
    
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    return COMMAND_OK;
}

int shell_dreg(int argc, char **argv, MIPS *m)
{
    (void)argc; (void)argv;
    
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    return COMMAND_OK;
}

int shell_sreg(int argc, char **argv, MIPS *m)
{
    (void)argc; (void)argv;
    
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    return COMMAND_OK;
}

int shell_dmem(int argc, char **argv, MIPS *m)
{
    (void)argc; (void)argv;
    
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    return COMMAND_OK;
}

int shell_smem(int argc, char **argv, MIPS *m)
{
    (void)argc; (void)argv;
    
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    return COMMAND_OK;
}

int shell_dump(int argc, char **argv, MIPS *m)
{
    (void)argc; (void)argv;
    
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
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

int shell_help(int argc, char **argv, MIPS *m);

static const Command commands[] = {
    // shortcuts
    {"r",  shell_run, ""},
    {"s",  shell_step, ""},
    {"si", shell_stepi, ""},
    {"d",  shell_dump, ""},
    {"t",  shell_trace, ""},
    {"q",  shell_quit, ""},
    
    // full names
    {"reset", shell_reset, ""},
    {"dasm",  shell_dasm, ""},
    {"dreg",  shell_dreg, ""},
    {"sreg",  shell_sreg, ""},
    {"dmem",  shell_dmem, ""},
    {"smem",  shell_smem, ""},
    {"run",   shell_run, ""},
    {"step",  shell_step, ""},
    {"stepi", shell_stepi, ""},
    {"dump",  shell_dump, ""},
    {"trace", shell_trace, ""},
    {"quit",  shell_quit, ""},
    {"exit",  shell_quit, ""},
    {"help",  shell_help, ""},
    {NULL, NULL, NULL}
};

int shell_help(int argc, char **argv, MIPS *m)
{
    if ( argc > 1 )
    {
        const Command *c = commands;
        
        while ( c->name != NULL && c->help != NULL )
        {
            if ( !strcmp(c->name, argv[1]) )
            {
                printf("%s : %s", c->name, c->help);
                break;
            }
            
            ++c;
        }
    } else {
        printf(
            "Supported commands :\n"
            " load\n"
            " run\n"
            " step\n"
            " stepi\n"
            " dasm\n"
            " dreg\n"
            " dmem\n"
            " sreg\n"
            " smem\n"
            " exit\n"
            "\n"
            "type : \"help\" followed by a command name for specific help.\n"
        );
    }
    
    return COMMAND_OK;
}

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

/*!
    \brief Split a command into a list of arguments
    
    \param[in]  cmd  command line to process
    \param[out] argc number of elements in the resulting list
    \return          list of pointer to arguments
    
    \note the caller is reponsible for freeing the list
    \note each element of the list points inside the input string and MUST NOT be freed
    \note every whitespace in the input string is truned into a null terminator
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

/*!
    
*/
int mipsim_shell(int cli_argc, char **cli_argv)
{
    int argc, ret = COMMAND_OK;
    char *cmd, **argv;
    
    MIPS *m = NULL;
    ELF_File *f = NULL;
    
    // try to load a file with leftover cli params
    argv = malloc(argc * sizeof(char*));
    argc = 0;
    for ( int i = 0; i < cli_argc; ++i )
        if ( *cli_argv[i] )
            argv[argc++] = cli_argv[i];
    
    shell_load(argc, argv, &m, &f);
    free(argv);
    
    while ( ret != COMMAND_EXIT )
    {
        cmd = readline("mipsim> ");
        
        if ( cmd && *cmd )
        {
            add_history(cmd);
            
            argv = tokenize(cmd, &argc);
            
            if ( !strcmp(argv[0], "load") || !strcmp(argv[0], "l") )
            {
                // special handling of load commands as it needs direct write access to m and f
                ret = shell_load(argc, argv, &m, &f);
            } else {
                ret = execute_command(argc, argv, m);
            }
            
            switch ( ret )
            {
                case COMMAND_INVALID :
                    printf("Invalid command.\n");
                    break;
                    
                case COMMAND_NEED_TARGET :
                    printf("Please load an ELF file.\n");
                    break;
                    
                default:
                    break;
            }
            
            free(argv);
        }
        
        free(cmd);
    }
    
    /*
        always destroy emulated machine before ELF file
    */
    mips_destroy(m);
    elf_file_destroy(f);
    
    return 0;
}
