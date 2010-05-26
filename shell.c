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
#include "util.h"
#include "config.h"
#include "mipself.h"

typedef struct _Shell_Env {
    MIPS *m;
    ELF_File *f;
} Shell_Env;

uint32_t symbol_value(const char *n, void *d, int *error)
{
    Shell_Env *e = (Shell_Env*)d;
    
    return 0;
}

typedef int (*command_handler)(int argc, char **argv, Shell_Env *e);

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

int shell_load(int argc, char **argv, Shell_Env *e)
{
    if ( argc <= 1 )
    {
        printf("load <filepath>\n");
        return 1;
    }
    
    if ( e->m != NULL )
    {
        /*
            Create simulator structures
            TODO : specify arch via a command line switch and alter if needed...
        */
        mips_reset(e->m);
    } else {
        /*
            Create simulator structures
            TODO : specify arch via a command line switch...
        */
        e->m = mips_create(mipsim_config()->arch);
        
        if ( e->m == NULL )
        {
            printf("MIPSim: Failed to allocate memory for MIPS machine\n");
            return COMMAND_FAIL;
        }
    }
    
    /*
        discard any previously loaded program
    */
    if ( e->f != NULL )
    {
        elf_file_destroy(e->f);
        e->f = NULL;
    }
    
    e->f = elf_file_create();
    
    if ( e->f == NULL )
    {
        printf("MIPSim: Failed to allocate memory for ELF file\n");
        return COMMAND_FAIL;
    }
    
    /*
        Load ELF file
    */
    if ( elf_file_load(e->f, argv[1]) )
        return COMMAND_FAIL;
    
    /*
        Map ELF file content to emulated machine memory
    */
    mips_load_elf(e->m, e->f);
    
    return COMMAND_OK;
}

int shell_reset(int argc, char **argv, Shell_Env *e)
{
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    (void)argc; (void)argv;
    
    m->hw.reset(&m->hw);
    for ( int i = 0; i < 4; ++i )
        m->cp[i].reset(&m->cp[i]);
    
    return COMMAND_OK;
}

int shell_quit(int argc, char **argv, Shell_Env *e)
{
    (void)argc; (void)argv; (void)e;
    
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

int shell_run(int argc, char **argv, Shell_Env *e)
{
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    if ( argc == 2 )
    {
        int error;
        MIPS_Addr addr = eval_expr(argv[1], symbol_value, e, &error);
        
        if ( error )
        {
            printf("Invalid <address> parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        printf("running from 0x%08x\n", addr);
        mips_set_reg(m, PC, addr);
    } else if ( argc != 1 ) {
        printf("run [address]\n");
        return COMMAND_PARAM_COUNT;
    }
    
    int ret;
    
    do {
        ret = mips_exec(m, 1, 0);
    } while ( ret == MIPS_OK );
    
    print_status(m);
    
    return ret ? COMMAND_FAIL : COMMAND_OK;
}

int shell_step(int argc, char **argv, Shell_Env *e)
{
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    int n = 1;
    
    if ( argc == 2 )
    {
        int error;
        n = eval_expr(argv[1], symbol_value, e, &error);
        
        if ( error )
        {
            printf("Invalid <count> parameter\n");
            return COMMAND_PARAM_TYPE;
        }
    } else if ( argc != 1 ) {
        printf("step [count]\n");
        return COMMAND_PARAM_COUNT;
    }
    
    return mips_exec(m, n, 1) ? COMMAND_FAIL : COMMAND_OK;
}

int shell_stepi(int argc, char **argv, Shell_Env *e)
{
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    int n = 1;
    
    if ( argc == 2 )
    {
        int error;
        n = eval_expr(argv[1], symbol_value, e, &error);
        
        if ( error )
        {
            printf("Invalid <count> parameter\n");
            return COMMAND_PARAM_TYPE;
        }
    } else if ( argc != 1 ) {
        printf("stepi [count]\n");
        return COMMAND_PARAM_COUNT;
    }
    
    return mips_exec(m, n, 0) ? COMMAND_FAIL : COMMAND_OK;
}

int shell_trace(int argc, char **argv, Shell_Env *e)
{
    (void)e;
    
    int n = mipsim_config()->io_mask & IO_TRACE ? 0 : 1;
    
    if ( argc == 2 )
    {
        const char *param = argv[1];
        
        if ( *param && !param[1] )
        {
            n = (*param - '0');
        } else {
            printf("trace [1 | 0]\n");
            return COMMAND_PARAM_TYPE;
        }
    } else if ( argc != 1 ) {
        printf("trace [1 | 0]\n");
        return COMMAND_PARAM_COUNT;
    }
    
    if ( n )
        mipsim_config()->io_mask |= IO_TRACE;
    else
        mipsim_config()->io_mask &= ~IO_TRACE;
    
    return COMMAND_OK;
}

int shell_dump(int argc, char **argv, Shell_Env *e)
{
    (void)argc; (void)argv;
    
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    printf("    pc = 0x%08x\n", mips_get_reg(m, PC));
    
    printf("    hi = 0x%08x        lo = 0x%08x\n",
           mips_get_reg(m, HI),
           mips_get_reg(m, LO));
    
    for ( int i = 0; i < 8; ++i )
    {
        printf("%6s = 0x%08x    %6s = 0x%08x    %6s = 0x%08x    %6s = 0x%08x\n",
               mips_reg_name(4*i),     mips_get_reg(m, 4*i),
               mips_reg_name(4*i + 1), mips_get_reg(m, 4*i+1),
               mips_reg_name(4*i + 2), mips_get_reg(m, 4*i+2),
               mips_reg_name(4*i + 3), mips_get_reg(m, 4*i+3));
    }
    
    return COMMAND_OK;
}

const char* find_symbol(MIPS_Addr org, MIPS_Addr val, void *d)
{
    Shell_Env *e = (Shell_Env*)d;
    
    // TODO : use relocation information when available to avoid ambiguities ?
    
    MIPS *m = e->m;
    ELF_File *elf = e->f;
    
    int stat;
    const char *s = elf_symbol_name(elf, val, &stat);
    
    return s;
}

int shell_dasm(int argc, char **argv, Shell_Env *e)
{
    (void)argc; (void)argv;
    
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    MIPSIM_Config *cfg = mipsim_config();
    
    int error = 0;
    MIPS_Addr start, end;
    
    ELF_Section *s = elf_section(e->f, ".text");
    
    if ( s == NULL )
        return COMMAND_NEED_TARGET;
    
    start = s->s_addr ? s->s_addr : s->s_reloc;
    end = start + s->s_size - 1;
    
    if ( argc > 1 )
    {
        start = eval_expr(argv[1], symbol_value, e, &error);
        if ( error )
        {
            printf("Invalid <start> parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        end = start;
        
        if ( argc == 3 )
        {
            end = eval_expr(argv[2], symbol_value, e, &error);
            if ( error )
            {
                printf("Invalid <end> parameter\n");
                return COMMAND_PARAM_TYPE;
            }
        } else if ( argc != 2 ) {
            printf("dasm [start] [end]\n");
            return COMMAND_PARAM_COUNT;
        }
    }
    
    if ( end < start )
    {
        printf("Invalid parameters\n");
        return COMMAND_PARAM_TYPE;
    }
    
    MIPS_Addr a = start;
    
    while ( a <= end )
    {
        const char *lbl = elf_symbol_name(e->f, a, NULL);
        
        if ( lbl != NULL )
            printf("\n%s:\n", lbl);
        
        char *s = mips_disassemble(m, a, find_symbol, e);
        
        if ( s != NULL )
        {
            printf("%08x:\t%08x\t%s\n", a, mips_read_w(m, a, NULL), s);
            free(s);
        } else {
            printf("%08x : xxxxxxxx\n", a);
        }
        
        a += 4;
    }
    
    return COMMAND_OK;
}

int shell_dreg(int argc, char **argv, Shell_Env *e)
{
    (void)argc; (void)argv;
    
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    if ( argc == 1 )
    {
        return shell_dump(argc, argv, e);
    } else if ( argc != 2 ) {
        printf("dreg <register>\n");
        return COMMAND_PARAM_COUNT;
    }
    
    int id = mips_reg_id(argv[1]);
    
    if ( id == INVALID_REG )
    {
        printf("Invalid <register> parameter\n");
        return COMMAND_PARAM_TYPE;
    }
    
    printf("%s = 0x%08x\n", mips_reg_name(id), mips_get_reg(m, id));
    
    return COMMAND_OK;
}

int shell_sreg(int argc, char **argv, Shell_Env *e)
{
    (void)argc; (void)argv;
    
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    if ( argc != 3 )
    {
        printf("dreg <register> <value>\n");
        return COMMAND_PARAM_COUNT;
    }
    
    int id = mips_reg_id(argv[1]);
    
    if ( id == INVALID_REG )
    {
        printf("Invalid <register> parameter\n");
        return COMMAND_PARAM_TYPE;
    }
    
    int error;
    uint32_t value = eval_expr(argv[2], symbol_value, e, &error);
    
    if ( error )
    {
        printf("Invalid <value> parameter\n");
        return COMMAND_PARAM_TYPE;
    }
    
    mips_set_reg(m, id, value);
    
    return COMMAND_OK;
}

int shell_dmem(int argc, char **argv, Shell_Env *e)
{
    (void)argc; (void)argv;
    
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    if ( argc >= 2 && argc <= 3 )
    {
        int error;
        
        MIPS_Addr start = eval_expr(argv[1], symbol_value, e, &error);
        MIPS_Addr end   = start + (3 << 4);
        
        if ( error )
        {
            printf("Invalid <start> parameters\n");
            return COMMAND_PARAM_TYPE;
        }
        
        if ( argc == 3 )
            end = eval_expr(argv[2], symbol_value, e, &error);
        
        if ( error || end < start )
        {
            printf("Invalid <end> parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        int stat;
        MIPS_Addr a = start;
        for ( MIPS_Addr i = 0; i < ((end - start) >> 4); ++i )
        {
            printf("%08x : ", a);
            for ( int j = 0; j < 16; ++j )
                printf(" %02x", mips_read_b(m, a++, &stat));
            printf("\n");
        }
        
        if ( ((end - start) & 15) || !((end - start) & ~15) )
        {
            printf("%08x : ", a);
            for ( MIPS_Addr i = 0; i <= ((end - start) & 15); ++i )
                printf(" %02x", mips_read_b(m, a++, &stat));
            printf("\n");
        }
    } else {
        printf("dmem <start> [end]\n");
        return COMMAND_PARAM_COUNT;
    }
    
    return COMMAND_OK;
}

int shell_smem(int argc, char **argv, Shell_Env *e)
{
    (void)argc; (void)argv;
    
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    if ( argc >= 3 && argc <= 4 )
    {
        int error;
        MIPS_Addr addr = eval_expr(argv[1], symbol_value, e, &error);
        
        if ( error )
        {
            printf("Invalid <addr> parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        uint32_t value = eval_expr(argv[2], symbol_value, e, &error);
        
        if ( error )
        {
            printf("Invalid <value> parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        int n = 1;
        
        if ( argc == 4 )
            n = eval_expr(argv[3], symbol_value, e, &error);
        
        if ( error )
        {
            printf("Invalid [bytecount] parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        if ( !bit_fit(value, n << 3) )
        {
            printf("<value> too large for given [bytecount]\n");
            return COMMAND_PARAM_TYPE;
        }
        
        int stat;
        
        if ( n == 1 )
            mips_write_b(m, addr, value, &stat);
        else if ( n == 2 )
            mips_write_h(m, addr, value, &stat);
        else if ( n == 4 )
            mips_write_w(m, addr, value, &stat);
        else {
            printf("Invalid [bytecount] : %d not in {1, 2, 4}\n", n);
            return COMMAND_INVALID;
        }
    } else {
        printf("smem <address> <value> [bytecount]\n");
        return COMMAND_PARAM_COUNT;
    }
    
    return COMMAND_OK;
}

int shell_help(int argc, char **argv, Shell_Env *e);

static const Command commands[] = {
    // shortcuts
    {"l",  shell_load, ""},
    {"r",  shell_run, ""},
    {"s",  shell_step, ""},
    {"si", shell_stepi, ""},
    {"d",  shell_dump, ""},
    {"t",  shell_trace, ""},
    {"q",  shell_quit, ""},
    
    // full names
    {"load",  shell_load, ""},
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

int shell_help(int argc, char **argv, Shell_Env *e)
{
    (void)e;
    
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

/*!
    \internal
    \brief Auxiliary function for shell command dispatch
*/
int execute_command(int argc, char **argv, Shell_Env *e)
{
    const Command *c = commands;
    
    while ( c->name != NULL && c->handler != NULL )
    {
        for ( int i = 0; ; ++i )
        {
            if ( c->name[i] == '\0' && (argv[0][i] == '\0' || argv[0][i] == ' ') )
            {
                return c->handler(argc, argv, e);
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
    \brief shell interface to simulator
    \param cli_argc remaining CLI parameter count
    \param cli_argv remaining CLI parameter values
    
    Present a command prompt on the console to the user
    offering various commands controlling the simulator.
*/
int mipsim_shell(int cli_argc, char **cli_argv)
{
    int argc, ret = COMMAND_OK;
    char *cmd, **argv;
    
    Shell_Env env;
    env.m = NULL;
    env.f = NULL;
    
    // try to load a file with leftover cli params
    argv = malloc(argc * sizeof(char*));
    argc = 0;
    for ( int i = 0; i < cli_argc; ++i )
        if ( *cli_argv[i] )
            argv[argc++] = cli_argv[i];
    
    shell_load(argc, argv, &env);
    free(argv);
    
    while ( ret != COMMAND_EXIT )
    {
        cmd = readline("mipsim> ");
        
        if ( cmd && *cmd )
        {
            add_history(cmd);
            
            argv = tokenize(cmd, &argc);
            
            ret = execute_command(argc, argv, &env);
            
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
    mips_destroy(env.m);
    elf_file_destroy(env.f);
    
    return 0;
}
