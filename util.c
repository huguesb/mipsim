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

#include <string.h>

enum {
    CASE_MASK = ~('a' - 'A'),
    NUM_ADJUST = -('A' - '0' - 10),
    STR_ADJUST = ('A' - 10)
};

/*!
    \internal
    \brief auxiliary function for str->num conversion
*/
uint8_t value(uint8_t c)
{
    c -= '0';
    
    if ( c > 9 ) {
        c &= CASE_MASK;  // discard case
        c += NUM_ADJUST; // adjust
    }
    
    return c;
}



/*!
    \internal
    \brief auxiliary function for num-> conversion
*/
uint8_t character(uint8_t c)
{
    return c <= 9 ? c + '0' : c + STR_ADJUST;
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
        base = 8;
        
        if ( *s == 'x' )
        {
            base += base;
            ++s;
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
            return 0;
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
    c &= CASE_MASK;
    return c >= 'A' && c <= 'Z';
}

int is_ident(char c)
{
    return is_letter(c) || c == '$' || c == '_';
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
    uint32_t r = 0;
    
//     while ( *s && *s <= ' ' )
//         ++s;
//     
//     if ( !(is_ident(*s) || *s == '(' || *s == '~') )
//     {
//         if ( error != NULL )
//             *error = E_SYNTAX;
//         return 0;
//     }
    
    
    
    return str_to_num(s, NULL, error);
}
