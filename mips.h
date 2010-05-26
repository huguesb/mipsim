/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
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
    MEM_OK       = 0,
    MEM_UNMAPPED = 1,
    MEM_READONLY = 2,
    MEM_NOEXEC   = 4,
    MEM_FWMON    = 8,
    
    MEM_RWX      = 0,
    MEM_RW       = MEM_NOEXEC,
    MEM_RX       = MEM_READONLY,
    MEM_R        = MEM_READONLY | MEM_NOEXEC
};

typedef void (*mem_unmap)(MIPS_Memory *m);

typedef int (*mem_map_alloc)(MIPS_Memory *m, MIPS_Addr a, uint32_t s, short flags);
typedef int (*mem_map_static)(MIPS_Memory *m, MIPS_Addr a, uint32_t s, uint8_t *d, short flags);
typedef int (*mem_map_redir)(MIPS_Memory *m, MIPS_Addr a, uint32_t s, MIPS_Memory *r, short flags);

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
    
    mem_map_static map_static;
    mem_map_alloc  map_alloc;
    mem_map_redir  map_redir;
    
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
    RA,
    
    SPR = 0x100,
    PC = SPR,
    HI,
    LO,
    
    CP_BIT   = 0x400,
    CP_SHIFT = 8,
    COND_BIT = 0x80,
    
    CP0 = 0x400,
    CP1 = 0x500,
    CP2 = 0x600,
    CP3 = 0x700,
    
    INVALID_REG = -1
};

enum {
    MIPS_E_ADDRESS_ERROR,
};

typedef void (*_reset_p)(MIPS_Processor *p);

typedef void (*_sig_ex)(MIPS_Processor *p, int exception);

typedef uint32_t (*_fetch_instr)(MIPS_Processor *p, int *stat);

typedef MIPS_Native (*_get_spr_p)(MIPS_Processor *p);
typedef void (*_set_spr_p)(MIPS_Processor *p, MIPS_Native value);

typedef MIPS_Native (*_get_gpr_p)(MIPS_Processor *p, int gpr);
typedef void (*_set_gpr_p)(MIPS_Processor *p, int gpr, MIPS_Native value);

struct _MIPS_Processor {
    _reset_p reset;
    
    _fetch_instr fetch;
    
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

typedef void (*_reset_cp)(MIPS_Coprocessor *p);

typedef MIPS_Native (*_get_gpr_cp)(MIPS_Coprocessor *p, int gpr);
typedef void (*_set_gpr_cp)(MIPS_Coprocessor *p, int gpr, MIPS_Native value);

struct _MIPS_Coprocessor {
    _reset_cp reset;
    
    _get_gpr_cp get_reg;
    _set_gpr_cp set_reg;
    
    _get_gpr_cp get_ctrl;
    _set_gpr_cp set_ctrl;
    
    void *d;
};

enum {
    MIPS_OK,
    MIPS_QUIT,
    MIPS_INVALID,
    MIPS_UNSUPPORTED,
    MIPS_TRAP,
    MIPS_BREAK,
    MIPS_EXCEPTION,
    MIPS_ERROR,
    MIPS_UNPREDICTABLE,
    MIPS_BKPT
};

typedef struct _MIPS MIPS;
typedef int (*mips_decode)(MIPS *m);

enum {
    BKPT_NONE   = 0,
    BKPT_MEM_X  = 1,
    BKPT_MEM_R  = 2,
    BKPT_MEM_W  = 4,
    BKPT_OPCODE = 8,
    
    BKPT_TYPE_MASK = 0xFFFF,
    
    BKPT_DISABLED = 0x10000
};

typedef struct _Breakpoint {
    int id, type;
    MIPS_Addr start, end, mask;
} Breakpoint;

typedef struct _BreakpointList BreakpointList;

struct _BreakpointList {
    Breakpoint d;
    BreakpointList *next;
};

struct _MIPS {
    MIPS_Memory mem;
    MIPS_Processor hw;
    MIPS_Coprocessor cp[4];
    
    mips_decode decode;
    
    int architecture;
    
    int stop_reason;
    int breakpoint_hit;
    
    BreakpointList *breakpoints;
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

int mips_reg_id(const char *name);
const char* mips_reg_name(int reg);

MIPS* mips_create(int arch);
void mips_destroy(MIPS *m);
void mips_reset(MIPS *m);

int mips_exec(MIPS *m, uint32_t n, int skip_proc);
void mips_stop(MIPS *m, int reason);

/*
    helpers to abstract away some of the not so nice implementation details
*/

MIPS_Native mips_get_reg(MIPS *m, int id);
void mips_set_reg(MIPS *m, int id, MIPS_Native v);

uint8_t  mips_read_b(MIPS *m, MIPS_Addr a, int *stat);
uint16_t mips_read_h(MIPS *m, MIPS_Addr a, int *stat);
uint32_t mips_read_w(MIPS *m, MIPS_Addr a, int *stat);
uint64_t mips_read_d(MIPS *m, MIPS_Addr a, int *stat);

void mips_write_b(MIPS *m, MIPS_Addr a, uint8_t b,  int *stat);
void mips_write_h(MIPS *m, MIPS_Addr a, uint16_t h, int *stat);
void mips_write_w(MIPS *m, MIPS_Addr a, uint32_t w, int *stat);
void mips_write_d(MIPS *m, MIPS_Addr a, uint64_t d, int *stat);

/*
    Breakpoint management
*/
int mips_breakpoint_add(MIPS *m, int type, MIPS_Addr start, MIPS_Addr end, MIPS_Addr mask);
void mips_breakpoint_remove(MIPS *m, int id);

int mips_breakpoint_count(MIPS *m, int type);
Breakpoint* mips_breakpoint(MIPS *m, int id);

/*
    Disassembly
*/
typedef const char* (symbol_name)(MIPS_Addr org, MIPS_Addr val, void *d);
char* mips_disassemble(MIPS *m, MIPS_Addr a, symbol_name sym_name, void *sym_data);

#endif
