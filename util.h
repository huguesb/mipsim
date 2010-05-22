/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#ifndef _MIPSIM_UTIL_H_
#define _MIPSIM_UTIL_H_

#include <stdint.h>

enum {
    E_NONE,
    E_OVERFLOW,
    E_INVALID,
    E_SYNTAX,
    E_UNDEFINED
};

static inline uint32_t s32_to_u32(int32_t v)
{ return *((uint32_t*)&v); }

static inline int32_t u32_to_s32(uint32_t v)
{ return *((int32_t*)&v); }

static inline int bit_fit(uint32_t v, int bits)
{ return bits > 0 ? (bits < 32 ? (v >> bits ? 0 : 1) : 1) : 0;}

uint32_t str_2_num(const char *s, const char **end, int *error);

typedef uint32_t (*_symbol_value)(const char *n, void *d, int *error);

int is_number(char c);
int is_letter(char c);

uint32_t eval_expr(const char *s, _symbol_value eval_sym, void *d, int *error);

#endif
