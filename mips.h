/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#ifndef _MIPS_H_
#define _MIPS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

typedef uint32_t MIPS_Addr;
typedef int32_t MIPS_Native;
typedef int32_t MIPS_NativeU;

typedef struct _MIPS_Memory MIPS_Memory;

enum {
    MEM_OK,
    MEM_UNMAPPED,
    MEM_READONLY
};

typedef void (*mem_unmap)(MIPS_Memory *m);

typedef int (*mem_map_alloc)(MIPS_Memory *m, MIPS_Addr a, uint32_t s);
typedef int (*mem_map_ro)(MIPS_Memory *m, MIPS_Addr a, uint32_t s, uint8_t *d);
typedef int (*mem_map_rw)(MIPS_Memory *m, MIPS_Addr a, uint32_t s, uint8_t *d);
typedef int (*mem_map_redir)(MIPS_Memory *m, MIPS_Addr a, uint32_t s, MIPS_Memory *r);

typedef uint8_t  (*mem_read_byte) (MIPS_Memory *m, MIPS_Addr a, int *stat);
typedef uint16_t (*mem_read_half) (MIPS_Memory *m, MIPS_Addr a, int *stat);
typedef uint32_t (*mem_read_word) (MIPS_Memory *m, MIPS_Addr a, int *stat);
typedef uint64_t (*mem_read_dword)(MIPS_Memory *m, MIPS_Addr a, int *stat);

typedef void (*mem_write_byte) (MIPS_Memory *m, MIPS_Addr a, uint8_t b, int *stat);
typedef void (*mem_write_half) (MIPS_Memory *m, MIPS_Addr a, uint16_t h, int *stat);
typedef void (*mem_write_word) (MIPS_Memory *m, MIPS_Addr a, uint32_t w, int *stat);
typedef void (*mem_write_dword)(MIPS_Memory *m, MIPS_Addr a, uint64_t d, int *stat);

struct _MIPS_Memory {
    mem_unmap     unmap;
    
    mem_map_ro    map_ro;
    mem_map_rw    map_rw;
    mem_map_redir map_redir;
    mem_map_alloc map_alloc;
    
    mem_read_byte  read_b;
    mem_read_half  read_h;
    mem_read_word  read_w;
    mem_read_dword read_d;
    
    mem_write_byte  write_b;
    mem_write_half  write_h;
    mem_write_word  write_w;
    mem_write_dword write_d;
    
    void *d;
};

typedef struct _MIPS_Processor MIPS_Processor;
typedef struct _MIPS_Coprocessor MIPS_Coprocessor;

enum MIPS_Regnames {
    ZERO,
    AT,
    V0,
    V1,
    A0,
    A1,
    A2,
    A3,
    T0,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    S0,
    S1,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    T8,
    T9,
    K0,
    K1,
    GP,
    SP,
    FP,
    RA
};

enum {
    MIPS_E_ADDRESS_ERROR,
};

typedef void (*_sig_ex)(MIPS_Processor *p, int exception);

typedef MIPS_Native (*_get_spr_p)(MIPS_Processor *p);
typedef void (*_set_spr_p)(MIPS_Processor *p, MIPS_Native value);

typedef MIPS_Native (*_get_gpr_p)(MIPS_Processor *p, int gpr);
typedef void (*_set_gpr_p)(MIPS_Processor *p, int gpr, MIPS_Native value);

struct _MIPS_Processor {
    
    _sig_ex signal_exception;
    
    _get_spr_p get_pc;
    _set_spr_p set_pc;
    
    _get_spr_p get_hi;
    _set_spr_p set_hi;
   
    _get_spr_p get_lo;
    _set_spr_p set_lo;
    
    _get_gpr_p get_reg;
    _set_gpr_p set_reg;
    
    void *d;
};

typedef MIPS_Native (*_get_gpr_cp)(MIPS_Coprocessor *p, int gpr);
typedef void (*_set_gpr_cp)(MIPS_Coprocessor *p, int gpr, MIPS_Native value);

struct _MIPS_Coprocessor {
    
    _get_gpr_cp get_reg;
    _set_gpr_cp set_reg;
    
    void *d;
};

enum {
    MIPS_OK,
    MIPS_INVALID,
    MIPS_UNSUPPORTED,
    MIPS_TRAP,
    MIPS_BREAK,
    MIPS_EXCEPTION,
    MIPS_ERROR,
    MIPS_UNPREDICTABLE
};

typedef struct _MIPS MIPS;
typedef int (*mips_decode)(MIPS *m);

struct _MIPS {
    MIPS_Memory mem;
    MIPS_Processor hw;
    MIPS_Coprocessor cp0;
    MIPS_Coprocessor cp1;
    MIPS_Coprocessor cp2;
    MIPS_Coprocessor cp3;
    
    mips_decode decode;
    
    int architecture;
    
    int stop_reason;
};


enum MIPS_Architecture {
    MIPS_ARCH_NONE,
    
    MIPS_1,
    MIPS_2,
    MIPS_3,
    MIPS_4,
    MIPS_5,
    MIPS_32,
    MIPS_32_R2,
    MIPS_64,
    MIPS_64_R2,
    
    MIPS_ARCH_LAST,
    MIPS_ARCH_FIRST = MIPS_1,
    
    MIPS_ARCH_COUNT = MIPS_ARCH_LAST - MIPS_ARCH_FIRST,
    
    MIPS_I = MIPS_1,
    MIPS_II = MIPS_2,
    MIPS_III = MIPS_3,
    MIPS_IV = MIPS_4,
    MIPS_V = MIPS_5
};

const char* mips_isa_name(int isa);
const char* mips_reg_name(int reg);

MIPS* mips_create(int arch);
void mips_destroy(MIPS *m);

int mips_exec(MIPS *m, uint32_t n);
void mips_stop(MIPS *m, int reason);

uint8_t  mips_read_b(MIPS *m, MIPS_Addr a, int *stat);
uint16_t mips_read_h(MIPS *m, MIPS_Addr a, int *stat);
uint32_t mips_read_w(MIPS *m, MIPS_Addr a, int *stat);
uint64_t mips_read_d(MIPS *m, MIPS_Addr a, int *stat);

void mips_write_b(MIPS *m, MIPS_Addr a, uint8_t b,  int *stat);
void mips_write_h(MIPS *m, MIPS_Addr a, uint16_t h, int *stat);
void mips_write_w(MIPS *m, MIPS_Addr a, uint32_t w, int *stat);
void mips_write_d(MIPS *m, MIPS_Addr a, uint64_t d, int *stat);

#endif
