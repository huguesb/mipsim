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

typedef uint64_t MIPS_Addr;

typedef struct _MIPS_Memory MIPS_Memory;
typedef struct _MIPS_Hardware MIPS_Hardware;

typedef int (*mem_map_ro)(MIPS_Memory *m, uint64_t a, uint32_t s, uint8_t *d);
typedef int (*mem_map_rw)(MIPS_Memory *m, uint64_t a, uint32_t s, uint8_t *d);
typedef int (*mem_map_redir)(MIPS_Memory *m, uint64_t a, uint32_t s, MIPS_Memory *r);

typedef uint8_t  (*mem_read_byte) (MIPS_Memory *m, uint64_t a);
typedef uint16_t (*mem_read_half) (MIPS_Memory *m, uint64_t a);
typedef uint32_t (*mem_read_word) (MIPS_Memory *m, uint64_t a);
typedef uint64_t (*mem_read_dword)(MIPS_Memory *m, uint64_t a);

typedef void (*mem_write_byte) (MIPS_Memory *m, uint64_t a, uint8_t b);
typedef void (*mem_write_half) (MIPS_Memory *m, uint64_t a, uint16_t h);
typedef void (*mem_write_word) (MIPS_Memory *m, uint64_t a, uint32_t w);
typedef void (*mem_write_dword)(MIPS_Memory *m, uint64_t a, uint64_t d);

struct _MIPS_Memory {
    void *data;
    
    mem_map_ro    map_ro;
    mem_map_rw    map_rw;
    mem_map_redir map_redir;
    
    mem_read_byte  read_b;
    mem_read_half  read_h;
    mem_read_word  read_w;
    mem_read_dword read_d;
    
    mem_write_byte  write_b;
    mem_write_half  write_h;
    mem_write_word  write_w;
    mem_write_dword write_d;
};


struct _MIPS_Hardware {
    uint64_t r[32];
    uint64_t hi, lo;
    
    uint64_t pc;
    uint32_t ir;
};


enum {
    MIPS_OK,
    MIPS_INVALID,
    MIPS_UNSUPPORTED,
    MIPS_TRAP,
    MIPS_BREAK,
    MIPS_ERROR
};

typedef struct _MIPS MIPS;
typedef int (*mips_decode)(MIPS *m);

struct _MIPS {
    int architecture;
    
    int stop_reason;
    
    MIPS_Memory mem;
    MIPS_Hardware hw;
    
    mips_decode decode;
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

MIPS* mips_create(int arch);
void mips_destroy(MIPS *m);

int mips_exec(MIPS *m, uint32_t n);

#endif
