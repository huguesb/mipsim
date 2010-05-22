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
    NUM_ADJUST = -('A' - '0' - 10)
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
    \brief convert a string to an integer, with error checks
*/
uint32_t str_2_num(const char *s, const char **end, int *error)
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
    \brief Evaluate an expression ot an integer
    
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
    
    
    
    return str_2_num(s, NULL, error);
}
