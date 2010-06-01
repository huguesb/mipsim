/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "util.h"

#include <stdlib.h>
#include <string.h>

#include "io.h"

/*!
    \internal
    \brief auxiliary function for str->num conversion
*/
uint8_t value(uint8_t c)
{
    if ( c >= '0' && c <= '9' )
        return c - '0';
    else if ( c >= 'A' && c <= 'Z' )
        return c - 'A' + 10;
    else if ( c >= 'a' && c <= 'z' )
        return c - 'a' + 10;
    
    return 0xFF;
}



/*!
    \internal
    \brief auxiliary function for num-> conversion
*/
uint8_t character(uint8_t c)
{
    return c <= 9 ? c + '0' : c + 'A' - 10;
}

void cat_num(uint32_t n, uint8_t base, char *s, uint8_t pad)
{
    int sz = pad;
    
    do {
        s[--sz] = character(n % base);
        n /= base;
    } while ( n && sz );
    
    while ( sz )
        s[--sz] = '0';
    
}

/*!
    \brief convert an integer to a string
    \param n number to convert
    \param base base to use (valid values in 2-36)
    \return NULL on failure (invalid base), a string representation of the input otherwise
    
    The caller is responsible for freeing the returned string.
*/
char* num_to_str(uint32_t n, uint8_t base)
{
    int flags = base & 0xC0;
    base &= 63;
    
    if ( base < 2 || base > 36 )
        return NULL;
    
    int sz = 1, tmp = n;
    
    do {
        tmp /= base;
        ++sz;
    } while ( tmp );
    
    if ( flags & C_PREFIX )
    {
        if ( base == 8 )
            ++sz;
        else if ( base == 16 )
            sz += 2;
    }
    
    char *s = malloc(sz * sizeof(char));
    s[--sz] = 0;
    
    do {
        s[--sz] = character(n % base);
        n /= base;
    } while ( n );
    
    if ( flags & C_PREFIX )
    {
        if ( base == 8 )
            s[0] = '0';
        else if ( base == 16 ) {
            s[0] = '0';
            s[1] = 'x';
        }
    }
    
    return s;
}

/*!
    \brief convert a string to an integer, with error checks
    \param s string to convert
    \param end if non NULL, will be set to last read character
    \param error, if non NULL, will hold error code
    \return value represented by the string
    
    In case of error, the return value will be zero.
    
    The input string can be in either octal, decimal or hexadecimal
    format, with proper C prefixes.
*/
uint32_t str_to_num(const char *s, const char **end, int *error)
{
    uint32_t base = 10;
    
    if ( *s == '0' )
    {
        ++s;
        
        if ( *s == 'x' )
        {
            base = 16;
            ++s;
        } else {
            base = 8;
        }
    }
    
    uint32_t r = 0;
    
    while ( *s && *s > ' ' )
    {
        uint8_t v = value(*(uint8_t*)s);
        
        if ( v >= base )
        {
            if ( error != NULL )
                *error = E_INVALID;
            if ( end != NULL )
                *end = s;
            return r;
        }
        
        if ( (r * base) / base != r )
        {
            if ( error != NULL )
                *error = E_OVERFLOW;
            if ( end != NULL )
                *end = s;
            return 0;
        }
        
        r *= base;
        r += v;
        
        ++s;
    }
    
    if ( error != NULL )
        *error = E_NONE;
    if ( end != NULL )
        *end = s;
    
    return r;
}

int is_number(char c)
{
    return c >= '0' && c <= '9';
}

int is_letter(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int is_ident(char c)
{
    return is_letter(c) || c == '$' || c == '_' || c == '.';
}

uint32_t eval_top(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_ternary(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_or(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_xor(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_and(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_equ(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_rel(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_shift(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_addsub(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_multdiv(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_fact(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_power(const char **s, _symbol_value eval_sym, void *d, int *error);
uint32_t eval_literal(const char **s, _symbol_value eval_sym, void *d, int *error);

void skip_ws(const char **s)
{
    while ( **s == ' ' || **s == '\t' || **s == '\n' )
        ++*s;
}

void next_non_ws(const char **s)
{
    ++*s;
    skip_ws(s);
}

/*!
    \brief Evaluate an expression to an integer
    \param s expression
    \param eval_sym auxiliary function for symbol evaluation
    \param[out] error where to store error code if not NULL
    \return result of epxression evaluation
*/
uint32_t eval_expr(const char *s, _symbol_value eval_sym, void *d, int *error)
{
    skip_ws(&s);
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "evaluating : %s\n", s);
    #endif
    
    uint32_t v = eval_top(&s, eval_sym, d, error);
    
    skip_ws(&s);
    
    if ( *s )
    {
        #ifdef _DEBUG_EVAL_
        mipsim_printf(IO_DEBUG, "unevaluated : %s\n", s);
        #endif
        
        *error = E_SYNTAX;
        return 0;
    }
    
    return v;
}

/*
    Top-Down expression evaluation :
    
    Top
    Ternary
    LogOr
    LogXor
    LogAnd
    Equ
    Rel
    Shift
    AddSub
    MultDiv
    Literal -> Paren : Top
    
*/

uint32_t eval_top(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "top: %s\n", *s);
    #endif
    
    return eval_ternary(s, eval_sym, d, error);
}

uint32_t eval_ternary(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "ternary: %s\n", *s);
    #endif
    
    uint32_t cond = eval_or(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    skip_ws(s);
    
    if ( **s != '?' )
        return cond;
    
    next_non_ws(s);
    
    uint32_t val1 = eval_top(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    skip_ws(s);
    
    if ( **s != ':' )
    {
        *error = E_SYNTAX;
        return 0;
    }
    
    next_non_ws(s);
    
    uint32_t val2 = eval_top(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "ternary : %08x ? %08x : %08x\n", cond, val1, val2);
    #endif
    
    return cond ? val1 : val2;
}

uint32_t eval_or(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "or: %s\n", *s);
    #endif
    
    uint32_t val = eval_xor(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    skip_ws(s);
    
    while ( **s == '|' )
    {
        next_non_ws(s);
        
        val |= eval_xor(s, eval_sym, d, error);
        
        if ( *error )
            return 0;
        
        skip_ws(s);
    }
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "or : %08x\n", val);
    #endif
    
    return val;
}

uint32_t eval_xor(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "xor: %s\n", *s);
    #endif
    
    uint32_t val = eval_and(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    skip_ws(s);
    
    while ( **s == '^' )
    {
        next_non_ws(s);
        
        val ^= eval_and(s, eval_sym, d, error);
        
        if ( *error )
            return 0;
        
        skip_ws(s);
    }
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "xor : %08x\n", val);
    #endif
    
    return val;
}

uint32_t eval_and(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "and: %s\n", *s);
    #endif
    
    uint32_t val = eval_equ(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    skip_ws(s);
    
    while ( **s == '&' )
    {
        next_non_ws(s);
        
        val &= eval_equ(s, eval_sym, d, error);
        
        if ( *error )
            return 0;
        
        skip_ws(s);
    }
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "and : %08x\n", val);
    #endif
    
    return val;
}

uint32_t eval_equ(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "equ: %s\n", *s);
    #endif
    
    uint32_t val = eval_rel(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    skip_ws(s);
    char c = **s;
    
    if ( c == '=' || c == '!' )
    {
        ++*s;
        
        if ( **s != '=' )
        {
            *error = E_SYNTAX;
            return 0;
        }
        
        next_non_ws(s);
        
        uint32_t tmp = eval_rel(s, eval_sym, d, error);
        
        if ( *error )
            return 0;
        
        val = c == '=' ? (val == tmp) : (val != tmp);
        
        //skip_ws(s);
        //c = **s;
    }
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "equ : %08x\n", val);
    #endif
    
    return val;
}

uint32_t eval_rel(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "rel: %s\n", *s);
    #endif
    
    uint32_t val = eval_shift(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    int equ = 0;
    
    skip_ws(s);
    char c = **s;
    
    if ( c == '<' || c == '>' )
    {
        if ( *(*s + 1) == '=' )
        {
            ++*s;
            equ = 1;
        }
        
        next_non_ws(s);
        
        uint32_t tmp = eval_shift(s, eval_sym, d, error);
        
        if ( *error )
            return 0;
        
        val = (c == '<' ? (val < tmp) : (val > tmp)) || ((val == tmp) && equ);
        
        //skip_ws(s);
        //c = **s;
    }
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "rel: %08x\n", val);
    #endif
    
    return val;
}

uint32_t eval_shift(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "shift: %s\n", *s);
    #endif
    
    uint32_t val = eval_addsub(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    skip_ws(s);
    char c = **s;
    
    while ( (c == '<' || c == '>') && (*(*s + 1) == c) )
    {
        ++*s;
        next_non_ws(s);
        
        uint32_t tmp = eval_addsub(s, eval_sym, d, error);
        
        if ( *error )
            return 0;
        
        val = c == '<' ? (val << tmp) : (val >> tmp);
        
        skip_ws(s);
        c = **s;
    }
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "shift : %08x\n", val);
    #endif
    
    return val;
}

uint32_t eval_addsub(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "add: %s\n", *s);
    #endif
    
    uint32_t val = eval_multdiv(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    skip_ws(s);
    char c = **s;
    
    while ( c == '+' || c == '-' )
    {
        next_non_ws(s);
        
        uint32_t tmp = eval_multdiv(s, eval_sym, d, error);
        
        if ( *error )
            return 0;
        
        if ( c == '+' )
            val += tmp;
        else
            val -= tmp;
        
        skip_ws(s);
        
        c = **s;
    }
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "add : %08x\n", val);
    #endif
    
    return val;
}

uint32_t eval_multdiv(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "mult: %s\n", *s);
    #endif
    
    uint32_t val = eval_literal(s, eval_sym, d, error);
    
    if ( *error )
        return 0;
    
    skip_ws(s);
    char c = **s;
    
    while ( c == '*' || c == '/' || c == '%' )
    {
        next_non_ws(s);
        
        uint32_t tmp = eval_literal(s, eval_sym, d, error);
        
        if ( *error )
            return 0;
        
        if ( c == '*' )
        {
            val *= tmp;
        } else if ( c == '/' ) {
            if ( !tmp )
            {
                *error = E_INVALID;
                return 0;
            }
            val /= tmp;
        } else {
            if ( !tmp )
            {
                *error = E_INVALID;
                return 0;
            }
            val %= tmp;
        }
        
        skip_ws(s);
        
        c = **s;
    }
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "mlt : %08x\n", val);
    #endif
    
    return val;
}

uint32_t eval_literal(const char **s, _symbol_value eval_sym, void *d, int *error)
{
    uint32_t val = 0;
    
    #ifdef _DEBUG_EVAL_
    mipsim_printf(IO_DEBUG, "lit: %s\n", *s);
    #endif
    
    if ( **s == '(' )
    {
        next_non_ws(s);
        
        val = eval_top(s, eval_sym, d, error);
        
        if ( *error )
            return 0;
        
        #ifdef _DEBUG_EVAL_
        mipsim_printf(IO_DEBUG, "() : %08x\n", val);
        #endif
        
        skip_ws(s);
        
        if ( **s != ')' )
        {
            *error = E_SYNTAX;
            return 0;
        }
        
        ++*s;
        
    } else if ( **s == '*' ) {
        // deref
        
        next_non_ws(s);
        val = eval_literal(s, eval_sym, d, error);
        
        if ( *error )
            return 0;
        
        char buf[12];
        buf[0] = '*';
        buf[1] = '0';
        buf[2] = 'x';
        buf[11] = 0;
        cat_num(val, 16, buf + 3, 8);
        
        val = eval_sym(buf, d, error);
        
        if ( *error )
            return 0;
        
    } else if ( is_number(**s) ) {
        val = str_to_num(*s, s, error);
        
        if ( *error == E_INVALID && !(is_number(**s) || is_letter(**s)) )
            *error = E_NONE;
        
        if ( *error )
            return 0;
        
        #ifdef _DEBUG_EVAL_
        mipsim_printf(IO_DEBUG, "lit : %08x\n", val);
        #endif
        
    } else if ( is_ident(**s) ) {
        int n = 1;
        
        while ( is_ident((*s)[n]) || is_number((*s)[n]) )
            ++n;
        
        char *ident = malloc((n+1) * sizeof(char));
        memcpy(ident, *s, n);
        ident[n] = 0;
        
        *s += n;
        
        val = eval_sym(ident, d, error);
        
        #ifdef _DEBUG_EVAL_
        mipsim_printf(IO_DEBUG, "sym : %s = %08x\n", ident, val);
        #endif
        
        free(ident);
        
        if ( *error )
            return 0;
        
        #ifdef _DEBUG_EVAL_
        mipsim_printf(IO_DEBUG, "remains : %s\n", *s);
        #endif
    } else {
        *error = E_SYNTAX;
    }
    
    return val;
}
