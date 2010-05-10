/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "mips.h"

enum {
    MAP_NONE,
    MAP_STATIC_RO,
    MAP_STATIC_RW,
    MAP_BLACKBOX
};

typedef struct _MemMapping MemMapping;

struct _MemMapping {
    int type;
    void *mapped;
    uint64_t start, end;
    
    MemMapping *next;
};

MemMapping* mips_mapping(MIPS_Memory *m, uint64_t a)
{
    MemMapping *mm = m ? m->data : NULL;
    
    while ( mm != NULL )
    {
        if ( mm->start <= a && a < mm->end )
            return mm;
        
        mm = mm->next;
    }
    
    return NULL;
}

int mips_simple_map_ro(MIPS_Memory *m, uint64_t a, uint32_t s, uint8_t *d)
{
    MemMapping *mm = (MemMapping*)malloc(sizeof(MemMapping));
    mm->type = MAP_STATIC_RO;
    mm->start = a;
    mm->end = a + s;
    mm->mapped = d;
    
    mm->next = (MemMapping*)m->data;
    m->data = mm;
    
    return 0;
}

int mips_simple_map_rw(MIPS_Memory *m, uint64_t a, uint32_t s, uint8_t *d)
{
    MemMapping *mm = (MemMapping*)malloc(sizeof(MemMapping));
    mm->type = MAP_BLACKBOX;
    mm->start = a;
    mm->end = a + s;
    mm->mapped = d;
    
    mm->next = (MemMapping*)m->data;
    m->data = mm;
    
    return 0;
}

int mips_simple_map_redir(MIPS_Memory *m, uint64_t a, uint32_t s, MIPS_Memory *r)
{
    MemMapping *mm = (MemMapping*)malloc(sizeof(MemMapping));
    mm->type = MAP_STATIC_RW;
    mm->start = a;
    mm->end = a + s;
    mm->mapped = r;
    
    mm->next = (MemMapping*)m->data;
    m->data = mm;
    
    return 0;
}

uint8_t mips_simple_read_b(MIPS_Memory *m, uint64_t a)
{
    MemMapping *mm = mips_mapping(m, a);
    
    if ( mm == NULL )
    {
        printf("MIPS: Failed to read memory byte @ 0x%8x:%8x\n", (uint32_t)(a >> 32), (uint32_t)(a & 0xFFFFFFFF));
        return 0;
    }
    
    if ( mm->type == MAP_BLACKBOX )
        return ((MIPS_Memory*)mm->mapped)->read_b(((MIPS_Memory*)mm->mapped), a);
    
    return ((uint8_t*)mm->mapped)[a - mm->start];
}

uint16_t mips_simple_read_h(MIPS_Memory *m, uint64_t a)
{
    uint8_t b0 = mips_simple_read_b(m, a);
    uint8_t b1 = mips_simple_read_b(m, a + 1);
    
    return ((uint16_t)b0 << 8) | b1;
}

uint32_t mips_simple_read_w(MIPS_Memory *m, uint64_t a)
{
    uint16_t h0 = mips_simple_read_h(m, a);
    uint16_t h1 = mips_simple_read_h(m, a + 2);
    
    return ((uint32_t)h0 << 16) | h1;
}

uint64_t mips_simple_read_d(MIPS_Memory *m, uint64_t a)
{
    uint32_t w0 = mips_simple_read_w(m, a);
    uint32_t w1 = mips_simple_read_w(m, a + 4);
    
    return ((uint64_t)w0 << 32) | w1;
}


void mips_simple_write_b(MIPS_Memory *m, uint64_t a, uint8_t b)
{
    MemMapping *mm = mips_mapping(m, a);
    
    if ( mm == NULL )
    {
        printf("MIPS: Failed to write memory byte @ 0x%8x:%8x\n", (uint32_t)(a >> 32), (uint32_t)(a & 0xFFFFFFFF));
        return;
    }
    
    if ( mm->type == MAP_BLACKBOX )
        ((MIPS_Memory*)mm->mapped)->write_b(((MIPS_Memory*)mm->mapped), a, b);
    else if ( mm->type == MAP_STATIC_RW )
        ((uint8_t*)mm->mapped)[a - mm->start] = b;
    else
        printf("MIPS: Failed to write memory byte @ 0x%8x:%8x\n", (uint32_t)(a >> 32), (uint32_t)(a & 0xFFFFFFFF));
}

void mips_simple_write_h(MIPS_Memory *m, uint64_t a, uint16_t h)
{
    mips_simple_write_b(m, a,     h >> 8);
    mips_simple_write_b(m, a + 1, h & 0x00ff);
}

void mips_simple_write_w(MIPS_Memory *m, uint64_t a, uint32_t w)
{
    mips_simple_write_h(m, a,     w >> 16);
    mips_simple_write_h(m, a + 1, w & 0x0000ffff);
}

void mips_simple_write_d(MIPS_Memory *m, uint64_t a, uint64_t d)
{
    mips_simple_write_w(m, a,     d >> 32);
    mips_simple_write_w(m, a + 1, d & 0x00000000ffffffffL);
}

void mips_init_memory(MIPS_Memory *mem)
{
    mem->data = NULL;
    
    mem->map_ro = mips_simple_map_ro;
    mem->map_rw = mips_simple_map_rw;
    mem->map_redir = mips_simple_map_redir;
    
    mem->read_b = mips_simple_read_b;
    mem->read_h = mips_simple_read_h;
    mem->read_w = mips_simple_read_w;
    mem->read_d = mips_simple_read_d;
    
    mem->write_b = mips_simple_write_b;
    mem->write_h = mips_simple_write_h;
    mem->write_w = mips_simple_write_w;
    mem->write_d = mips_simple_write_d;
}
