
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../util.h"

enum {
    CMD_BUF_SZ = 1024
};

typedef struct {
    char *name;
    uint32_t value;
} Symbol;

typedef struct _SymbolList SymbolList;

struct _SymbolList {
    Symbol d;
    SymbolList *next;
};

char* get_cmd(char *buf, int len);

void clear_symbols(SymbolList *l);
void save_symbol(char *n, uint32_t value, SymbolList *l);
uint32_t symbol_value(const char *n, void *d, int *error);

int main()
{
    SymbolList l;
    char cmd_buf[CMD_BUF_SZ];
    
    l.d.name = NULL;
    l.d.value = 0;
    
    uint32_t unamed = 0;
    
    printf("MIPS Calculator\n");
    
    while ( 1 )
    {
        printf("> ");
        fflush(stdout);
        
        char *stat = get_cmd(cmd_buf, CMD_BUF_SZ);
        
        if ( stat != NULL )
        {
            char *name = NULL, *expr = cmd_buf;
            
            if ( *expr == '=' )
            {
                ++expr;
                
                while ( *expr == ' ' )
                    ++expr;
                
                name = expr;
                
                while ( *expr && *expr != ' ' )
                    ++expr;
                
                if ( *expr )
                {
                    *expr = 0;
                    ++expr;
                } else {
                    printf("Error encountered\n");
                    continue;
                }
            } else if ( *expr == 'q' ) {
                printf("Exiting calculator\n");
                break;
            }
            
            int error;
            uint32_t value = eval_expr(expr, symbol_value, &l, &error);
            
            if ( error )
            {
                printf("Error encountered : %d\n", error);
            } else if ( name != NULL ) {
                printf(" %s = %d\n", name, value);
                char *sname = malloc(strlen(name) * sizeof(char));
                strcpy(sname, name);
                save_symbol(sname, value, &l);
            } else {
                printf(" $ = %d\n", value);
            }
            
            // symbol refering to last result
            l.d.name  = "$";
            l.d.value = value;
        }
    }
    
    clear_symbols(l.next);
    
    return 0;
}

char* get_cmd(char *buf, int len)
{
    __asm__(
        "addiu $v0, $zero, 8\t\n"
        "syscall\t\n"
    );
    
    return buf;
}

uint32_t symbol_value(const char *n, void *d, int *error)
{
    SymbolList *l = (SymbolList*)d;
    
    printf("sym lookup : %s\n", n);
    
    while ( l != NULL )
    {
        printf(" ? %s\n", l->d.name);
        
        if ( l->d.name != NULL && !strcmp(l->d.name, n) )
        {
            if ( error != NULL )
                *error = E_NONE;
            
            return l->d.value;
        }
        
        l = l->next;
    }
    
    if ( error )
        *error = E_UNDEFINED;
    
    return 0;
}

void save_symbol(char *n, uint32_t value, SymbolList *l)
{
    if ( l == NULL )
        return;
    
    SymbolList *s = (SymbolList*)malloc(sizeof(SymbolList));
    
    s->d.name  = n;
    s->d.value = value;
    
    s->next = l->next;
    l->next = s;
}

void clear_symbols(SymbolList *l)
{
    if ( l != NULL )
    {
        free(l->d.name);
        
        clear_symbols(l->next);
    }
}

int mipsim_printf(int cxt, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    vprintf(fmt, args);
    
    va_end(args);
}

#include "../util.c"
