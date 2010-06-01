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

#ifdef _SHELL_USE_READLINE_
#include <readline/readline.h>
#include <readline/history.h>
#endif

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
    if ( n == NULL )
    {
        if ( error != NULL )
            *error = E_INVALID;
        return 0;
    }
    
    Shell_Env *e = (Shell_Env*)d;
    MIPS *m = e->m;
    ELF_File *f = e->f;
    
    if ( error != NULL )
        *error = E_NONE;
    
    int found = 0;
    int deref = 0;
    uint32_t value = 0;
    
    while ( *n == '*' )
    {
        ++deref;
        ++n;
    }
    
    if ( m != NULL )
    {
        int id = mips_reg_id(n);
        
        if ( id != INVALID_REG )
        {
            value = mips_get_reg(m, id);
            found = 1;
        }
    }
    
    if ( !found && f != NULL )
    {
        int stat;
        value = elf_symbol_value(f, n, &stat);
        
        if ( stat != -1 )
            found = 1;
    }
    
    if ( !found )
    {
        int stat;
        const char *end;
        value = str_to_num(n, &end, &stat);
        
        found = !(*end || stat);
    }
    
    if ( !found )
    {
        *error = E_UNDEFINED;
        
        return 0;
    }
    
    while ( deref-- )
    {
        int stat;
        value = mips_read_w(m, value, &stat);
        
        if ( stat )
        {
            *error = E_UNDEFINED;
            return 0;
        }   
    }
    
    return value;
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
    const char *shorthand;
    command_handler handler; 
    const char *params;
    const char *help;
} Command;

int shell_load(int argc, char **argv, Shell_Env *e)
{
    if ( argc < 2 || argc > 3 )
        return COMMAND_PARAM_COUNT;
    
    int arch = mipsim_config()->arch;
    
    if ( argc == 3 )
    {
        arch = mips_isa_id(argv[2]);
        
        if ( arch == MIPS_ARCH_NONE )
        {
            printf("Invalid [arch] parameter\n");
            return COMMAND_PARAM_TYPE;
        }
    }
    
    if ( e->m != NULL )
    {
        /*
            Reset simulator structures
        */
        mips_reset(e->m);
        e->m->architecture = arch;
    } else {
        /*
            Create simulator structures
        */
        e->m = mips_create(arch);
        
        if ( e->m == NULL )
        {
            printf("Failed to allocate memory for MIPS machine\n");
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
        printf("Failed to allocate memory for ELF file\n");
        return COMMAND_FAIL;
    }
    
    /*
        Load ELF file
    */
    if ( elf_file_load(e->f, argv[1]) )
    {
        printf("Invalid <filepath> parameter\n");
        return COMMAND_FAIL;
    }
    
    /*
        Map ELF file content to emulated machine memory
    */
    if ( mips_load_elf(e->m, e->f) )
    {
        printf("Invalid <filepath> parameter\n");
        return COMMAND_FAIL;
    }
    
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

int shell_print(int argc, char **argv, Shell_Env *e)
{
    for ( int i = 1; i < argc; ++i )
    {
        int error;
        uint32_t val = eval_expr(argv[i], symbol_value, e, &error);
        
        if ( error )
            printf("  %d : error %d\n", i, error);
        else
            printf("  %d : 0x%08x\n", i, val);
    }
    
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
            
        case MIPS_QUIT :
            printf("Quit\n");
            break;
            
        case MIPS_BKPT :
            printf("Hit breakpoint %d\n", m->breakpoint_hit);
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
            return COMMAND_PARAM_TYPE;
        }
    } else if ( argc != 1 ) {
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
    (void)org;
    
    Shell_Env *e = (Shell_Env*)d;
    
    // TODO : use relocation information when available to avoid ambiguities ?
    
    ELF_File *elf = e->f;
    
    if ( elf == NULL )
        return NULL;
    
    int stat;
    const char *s = elf_symbol_name(elf, val, &stat);
    
    // TODO : on failure look for neighbouring symbols and add offsets...
    
    return s;
}

int shell_dasm(int argc, char **argv, Shell_Env *e)
{
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
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
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    if ( argc == 1 )
    {
        return shell_dump(argc, argv, e);
    } else if ( argc != 2 ) {
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
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    if ( argc != 3 )
        return COMMAND_PARAM_COUNT;
    
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
        return COMMAND_PARAM_COUNT;
    }
    
    return COMMAND_OK;
}

int shell_smem(int argc, char **argv, Shell_Env *e)
{
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
        return COMMAND_PARAM_COUNT;
    }
    
    return COMMAND_OK;
}

int shell_addbp(int argc, char **argv, Shell_Env *e)
{
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    int error, type;
    MIPS_Addr start, end, mask;
    
    if ( argc >= 2 && argc <= 5 )
    {
        start = eval_expr(argv[1], symbol_value, e, &error);
        
        if ( error )
        {
            printf("Invalid <start> parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        end = argc > 2 ? eval_expr(argv[2], symbol_value, e, &error) : start;
        
        if ( error )
        {
            printf("Invalid [end] parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        mask = argc > 3 ? eval_expr(argv[3], symbol_value, e, &error) : 0xFFFFFFFF;
        
        if ( error )
        {
            printf("Invalid [mask] parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        type = BKPT_MEM_X;
        
        if ( argc == 5 )
        {
            if ( !strcmp(argv[4], "memr") )
                type = BKPT_MEM_R;
            else if ( !strcmp(argv[4], "memw") )
                type = BKPT_MEM_W;
            else if ( !strcmp(argv[4], "op") )
                type = BKPT_OPCODE;
            else {
                printf("Invalid [type] parameter : not in {memr, memw, op}\n");
                return COMMAND_PARAM_TYPE;
            }
        }
    } else {
        return COMMAND_PARAM_COUNT;
    }
    
    int id = mips_breakpoint_add(m, type, start, end, mask);
    
    printf("added breakpoint %d\n", id);
    
    return COMMAND_OK;
}

int shell_rmbp(int argc, char **argv, Shell_Env *e)
{
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    if ( argc == 1 )
    {
        mips_breakpoint_clear(m);
    } else if ( argc == 2 ) {
        int error;
        int id = eval_expr(argv[1], symbol_value, e, &error);
        
        if ( error )
        {
            printf("Invalid [id] parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        mips_breakpoint_remove(m, id);
    } else {
        return COMMAND_PARAM_COUNT;
    }
    
    return COMMAND_OK;
}


void dbp(Breakpoint *b)
{
    printf("%4d  0x%08x  0x%08x  0x%08x\n", b->id, b->start, b->end, b->mask);
}

void dbpl(BreakpointList *l)
{
    if ( l == NULL )
        return;
    
    dbpl(l->next);
    dbp (&l->d);
}

int shell_dbp(int argc, char **argv, Shell_Env *e)
{
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    if ( argc > 2 )
        return COMMAND_PARAM_COUNT;
    
    if ( argc == 2 )
    {
        int error;
        int id = eval_expr(argv[1], symbol_value, e, &error);
        
        if ( error )
        {
            printf("Invalid [id] parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        Breakpoint *b = mips_breakpoint(m, id);
        
        if ( b == NULL )
        {
            printf("No breakpoint with id=%d\n", id);
            return COMMAND_FAIL;
        }
        
        dbp(b);
    } else {
        printf("----------------------------------------\n");
        printf("  ID     start        end        mask   \n");
        printf("----------------------------------------\n");
        dbpl(m->breakpoints);
        printf("----------------------------------------\n");
    }
    
    return COMMAND_OK;
}

int shell_mmap(int argc, char **argv, Shell_Env *e)
{
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    MIPS_Memory *mem = &e->m->mem;
    if ( argc == 1 )
    {
        printf("Memory mappings :\n");
        mem->dump_mapping(stdout, " ", mem);
    } else if ( argc <= 4 ) {
        int error;
        MIPS_Addr start = eval_expr(argv[1], symbol_value, e, &error);
        
        if ( error )
        {
            printf("Invalid [start] parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        MIPS_Addr size = argc > 2
                    ? eval_expr(argv[2], symbol_value, e, &error)
                    : ((start & 0xFFF) ? 0x1000 - (start & 0xFFF) : 0x1000);
        
        if ( error )
        {
            printf("Invalid [size] parameter\n");
            return COMMAND_PARAM_TYPE;
        }
        
        int flags = MEM_NOEXEC;
        
        if ( argc == 4 )
        {
            if ( !strcmp(argv[3], "x") )
                flags = MEM_READONLY;
            else if ( !strcmp(argv[3], "w") )
                flags = MEM_NOEXEC;
            else if ( !strcmp(argv[3], "wx") || !strcmp(argv[3], "xw") )
                flags = 0;
            else {
                printf("Invalid [flags] parameter\n");
                return COMMAND_PARAM_TYPE;
            }
        }
        
        int ret = mem->map_alloc(mem, start, size, flags);
        
        if ( ret )
            return COMMAND_FAIL;
        
    } else {
        return COMMAND_PARAM_COUNT;
    }
    
    return COMMAND_OK;
}

int shell_status(int argc, char **argv, Shell_Env *e)
{
    MIPS *m = e->m;
    if ( m == NULL )
        return COMMAND_NEED_TARGET;
    
    (void)argv;
    
    if ( argc != 1 )
        return COMMAND_PARAM_COUNT;
    
    print_status(m);
    
    return COMMAND_OK;
}

int shell_dsym(int argc, char **argv, Shell_Env *e)
{
    ELF_File *elf = e->f;
    if ( elf == NULL )
        return COMMAND_NEED_TARGET;
    
    (void)argv;
    
    if ( argc != 1 )
        return COMMAND_PARAM_COUNT;
    
    elf_dump_symbols(elf, stdout);
    
    return COMMAND_OK;
}

int shell_drel(int argc, char **argv, Shell_Env *e)
{
    ELF_File *elf = e->f;
    if ( elf == NULL )
        return COMMAND_NEED_TARGET;
    
    (void)argv;
    
    if ( argc != 1 )
        return COMMAND_PARAM_COUNT;
    
    elf_dump_relocations(elf, stdout);
    
    return COMMAND_OK;
}

int shell_help(int argc, char **argv, Shell_Env *e);

static const Command commands[] = {
    {"load",  "l",  shell_load,     "<filepath>",
        " Loads an ELF binary (executable or relocatable) and map it into the memory of\n"
        " the simulated  machine. PC is moved to the entry point or, if none exists, to\n"
        " the start of the .text section.\n"
        "\n"
        " Note : binaries MAY NOT contain references to external symbols.\n"
        "\n"
        " The optional isa parameter indicate which version of the MIPS instruction set\n"
        " should be allowed in the simulator. Valid values are :\n"
        "    mips1\n"
        "    mips2\n"
        "    mips3\n"
        "    mips4\n"
        "    mips5\n"
        "    mips32\n"
        "    mips64\n"
        "    mips32r2\n"
        "    mips64r2\n"
        "\n"
        " The \"mips\" prefix can be ommitted.\n"
        "\n"
        " Please note that, while all values are accepted, not all ISA are properly\n"
        " simulated yet. In particular there is no FPU and 64 bit support at all...\n"},
    {"print", "p",  shell_print,    "<expr>+",
        "Evaluate any number of expressions and print the results.\n"},
    {"dasm",  NULL, shell_dasm,     "[start] [end]",
        " Disassemble memory. Non-executable memory will be disassembled as .word\n"
        " directives.\n"
        " \n"
        " If no start parameter is provided the whole .text section will be disasembled.\n"
        " If only the start parameter is provided, end will default to start (only one\n"
        " instruction will  be shown).\n"},
    {"dreg",  NULL, shell_dreg,     "<reg>",
        " Display the content of a register. The parameter can be a register number or a\n"
        " mnemonic, with or without a $ prefix. Using mnemonic, one can access the\n"
        " contents of PC, HI, LO and coprocessor registers.\n"},
    {"sreg",  NULL, shell_sreg,     "<reg> <value>",
        " Set the content of a register. The parameter can be a register number or a\n"
        " mnemonic, with or without a $ prefix. Using mnemonic, one can access the\n"
        " contents of PC, HI, LO and coprocessor registers.\n"},
    {"dmem",  NULL, shell_dmem,     "<start> <end>",
        " Display the content of memory region delimited by start and end parameters. If\n"
        " not provided, end default to start + 3*16\n"},
    {"smem",  NULL, shell_smem,     "<address> <value> [bytecount]",
        " Set the content of a memory cell. If value is larger than a byte, a valid\n"
        " bytecount parameter must be provided. Bytecount can be either 1, 2 or 4.\n"},
    {"run",   "r",  shell_run,      "[address]",
        " Resume the execution of the simulated machine from the supplied address (from\n"
        " the current  value of the PC by default)\n"},
    {"step",  "s",  shell_step,     "[count]",
        " Executes [count] instructions (default is 1). A branch and its delay slot are\n"
        " considered as a  single instruction.\n"
        "\n"
        " A procedure call and all the subsequent instructions until procedure return are\n"
        " considered as  a single instruction.\n"},
    {"stepi", "si", shell_stepi,    "[count]",
        " Executes [count] instructions (default is 1). A branch and its delay slot are\n"
        " considered as a single instruction.\n"},
    {"addbp", NULL, shell_addbp,    "<start> [end] [mask] [type]",
        " Create a breakpoint.\n"
        "\n"
        " Breakpoint is hit when tested value V verifies :\n"
        "    start <= (V & mask) <= end\n"
        "\n"
        " Value being tested depends on the breakpoint type :\n"
        "\n"
        "   type |  value tested\n"
        " ----------------------\n"
        "   memr | memory address passed to lb, lh, lw...\n"
        "   memw | memory address passed to sb, sh, sw...\n"
        "   memx | memory address of current instruciton (i.e PC)\n"
        " opcode | current instruction\n"
        "\n"
        " Defaults :\n"
        "   end  = start\n"
        "   mask = 0xFFFFFFFF\n"
        "   type = memx\n"},
    {"rmbp",  NULL, shell_rmbp,     "[id]",
        " Remove a breakpoint when given a parameter, all breakpoints otherwise\n"},
    {"dbp",   NULL, shell_dbp,      "[id]",
        " Display information about a breakpoint when given a parameter, about all\n"
        " breakpoints otherwise\n"},
    {"dump",  "d",  shell_dump,     "",
        " Dump informations about all processor registers.\n"},
    {"trace", "t",  shell_trace,    "[1 | 0]",
        " Toggle trace output.\n"},
    {"mmap",  NULL, shell_mmap,     "[address] [size] [flags]",
        " When invoked without parameters, display all memory mappings of the simulated\n"
        " machine, Otherwise create a new mapping.\n"
        "\n"
        " Mappings MAY NOT overlap.\n"
        "\n"
        " Default size value is the amount of memory needed to reach next page boundary.\n"
        "\n"
        " Flags is in {w, x, wx}. wx is accepted as an alias to wx but its use is\n"
        " discouraged.\n"
        "\n"
        " Default flags value is w.\n"},
    {"status",  NULL, shell_status, "",
        " Show target status"},
    {"dsym",  NULL, shell_dsym, "",
        " Show symbols from ELF file"},
    {"drel",  NULL, shell_drel, "",
        " Show relocation informations from ELF file"},
    {"quit",  "q",  shell_quit,     "",
        " Quit MIPSim"},
    {"exit",  NULL, shell_quit,     "",
        " Quit MIPSim"},
    {"help",  "h",  shell_help,     "",
        " Display a list of supported commands if no parameter passed. Display command\n"
        " specific help  when given a parameter.\n"},
    {NULL, NULL, NULL, NULL, NULL}
};

int shell_help(int argc, char **argv, Shell_Env *e)
{
    (void)e;
    
    if ( argc == 2 )
    {
        const Command *c = commands;
        
        while ( c->name != NULL && c->help != NULL )
        {
            if ( !strcmp(c->name, argv[1]) )
            {
                printf("%s %s\n"
                       "--------------------------------------------------------------------------------\n"
                        "%s",
                       c->name, c->params, c->help);
                break;
            }
            
            ++c;
        }
        
        if ( c->name == NULL )
        {
            printf("Unrecognized command : %s\n", argv[1]);
            return COMMAND_PARAM_TYPE;
        }
    } else if ( argc == 1 ){
        printf("Supported commands :\n");

        const Command *c = commands;
        
        while ( c->name != NULL && c->help != NULL )
        {
            printf("  %s", c->name);
            
            int n = 10 - strlen(c->name);
            while ( n-- )
                printf(" ");
            
            printf("%s\n", c->params);
            
            ++c;
        }
        
        printf("\ntype : \"help\" followed by a command name for specific help.\n");
    } else {
        return COMMAND_PARAM_COUNT;
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
        if ( !strcmp(c->name, *argv)
            || (c->shorthand != NULL && !strcmp(c->shorthand, *argv)) )
        {
            int ret = c->handler(argc, argv, e);
            
            if ( ret == COMMAND_PARAM_COUNT )
                printf("%s\t%s\n", c->name, c->params);
            
            return ret;
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
            {
                *iter = 0;
                quoted = !quoted;
            }
            
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
    argv = malloc(cli_argc * sizeof(char*));
    argc = 0;
    for ( int i = 0; i < cli_argc; ++i )
        if ( *cli_argv[i] )
            argv[argc++] = cli_argv[i];
    
    shell_load(argc, argv, &env);
    free(argv);
    
    #ifdef _SHELL_USE_READLINE_
    using_history();
    #endif
    
    while ( ret != COMMAND_EXIT )
    {
        #ifdef _SHELL_USE_READLINE_
        cmd = readline("mipsim> ");
        #else
        printf("mipsim> ");
        fflush(stdout);
        cmd = malloc(256*sizeof(char));
        fgets(cmd, 255, stdin);
        cmd[strlen(cmd)-1] = 0;
        #endif
        
        if ( cmd && *cmd )
        {
            #ifdef _SHELL_USE_READLINE_
            add_history(cmd);
            #endif
            
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
    
    #ifdef _SHELL_USE_READLINE_
    clear_history();
    #endif
    
    /*
        always destroy emulated machine before ELF file
    */
    mips_destroy(env.m);
    elf_file_destroy(env.f);
    
    return 0;
}
