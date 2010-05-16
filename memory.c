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

#include "io.h"

enum {
    MAP_NONE,
    MAP_STATIC_RO,
    MAP_STATIC_RW,
    MAP_BLACKBOX,
    MAP_DYNAMIC
};

typedef struct _MemMapping MemMapping;

struct _MemMapping {
    int type;
    void *mapped;
    MIPS_Addr start, end;
    
    MemMapping *next;
};

MemMapping* mips_mapping(MIPS_Memory *m, MIPS_Addr a)
{
    MemMapping *mm = m ? m->d : NULL;
    
    while ( mm != NULL )
    {
        if ( mm->start <= a && a < mm->end )
            return mm;
        
        mm = mm->next;
    }
    
    return NULL;
}

void mips_simple_unmap(MIPS_Memory *m)
{
    MemMapping *mm = m->d, *tmp;
    
    while ( mm != NULL )
    {
        if ( mm->type == MAP_BLACKBOX )
            ((MIPS_Memory*)mm->mapped)->unmap((MIPS_Memory*)mm->mapped);
        else if ( mm->type == MAP_DYNAMIC )
            free(mm->mapped);
        
        tmp = mm->next;
        free(mm);
        mm = tmp;
    }
    
    m->d = NULL;
}

int mips_simple_map_ro(MIPS_Memory *m, MIPS_Addr a, uint32_t s, uint8_t *d)
{
    MemMapping *mm = (MemMapping*)malloc(sizeof(MemMapping));
    mm->type = MAP_STATIC_RO;
    mm->start = a;
    mm->end = a + s;
    mm->mapped = d;
    
    mm->next = (MemMapping*)m->d;
    m->d = mm;
    
    return 0;
}

int mips_simple_map_rw(MIPS_Memory *m, MIPS_Addr a, uint32_t s, uint8_t *d)
{
    MemMapping *mm = (MemMapping*)malloc(sizeof(MemMapping));
    mm->type = MAP_STATIC_RW;
    mm->start = a;
    mm->end = a + s;
    mm->mapped = d;
    
    mm->next = (MemMapping*)m->d;
    m->d = mm;
    
    return 0;
}

int mips_simple_map_redir(MIPS_Memory *m, MIPS_Addr a, uint32_t s, MIPS_Memory *r)
{
    MemMapping *mm = (MemMapping*)malloc(sizeof(MemMapping));
    mm->type = MAP_BLACKBOX;
    mm->start = a;
    mm->end = a + s;
    mm->mapped = r;
    
    mm->next = (MemMapping*)m->d;
    m->d = mm;
    
    return 0;
}

int mips_simple_map_alloc(MIPS_Memory *m, MIPS_Addr a, uint32_t s)
{
    MemMapping *mm = (MemMapping*)malloc(sizeof(MemMapping));
    mm->type   = MAP_DYNAMIC;
    mm->start  = a;
    mm->end    = a + s;
    mm->mapped = malloc(s);
    mm->next   = m->d;
    
    m->d = mm;
    
    return 0;
}

uint8_t mips_simple_read_b(MIPS_Memory *m, MIPS_Addr a, int *stat)
{
    MemMapping *mm = mips_mapping(m, a);
    
    if ( mm == NULL )
    {
        mipsim_printf(IO_WARNING, "MIPS: Failed to read memory byte @ 0x%x\n", a);
        
        if ( stat != NULL )
            *stat = MEM_UNMAPPED;
        
        return 0;
    }
    
    if ( mm->type == MAP_BLACKBOX )
        return ((MIPS_Memory*)mm->mapped)->read_b(((MIPS_Memory*)mm->mapped), a, stat);
    
    if ( stat != NULL )
        *stat = MEM_OK;
    
    return ((uint8_t*)mm->mapped)[a - mm->start];
}

uint16_t mips_simple_read_h(MIPS_Memory *m, MIPS_Addr a, int *stat)
{
    uint8_t b0 = mips_simple_read_b(m, a, stat);
    if ( stat != NULL && *stat != MEM_OK )
        return 0;
    uint8_t b1 = mips_simple_read_b(m, a + 1, stat);
    
    return ((uint16_t)b0 << 8) | b1;
}

uint32_t mips_simple_read_w(MIPS_Memory *m, MIPS_Addr a, int *stat)
{
    uint16_t h0 = mips_simple_read_h(m, a, stat);
    if ( stat != NULL && *stat != MEM_OK )
        return 0;
    uint16_t h1 = mips_simple_read_h(m, a + 2, stat);
    
    return ((uint32_t)h0 << 16) | h1;
}

uint64_t mips_simple_read_d(MIPS_Memory *m, MIPS_Addr a, int *stat)
{
    uint32_t w0 = mips_simple_read_w(m, a, stat);
    if ( stat != NULL && *stat != MEM_OK )
        return 0;
    uint32_t w1 = mips_simple_read_w(m, a + 4, stat);
    
    return ((uint64_t)w0 << 32) | w1;
}


void mips_simple_write_b(MIPS_Memory *m, MIPS_Addr a, uint8_t b, int *stat)
{
    MemMapping *mm = mips_mapping(m, a);
    
    if ( mm == NULL )
    {
        mipsim_printf(IO_WARNING, "MIPS: Failed to write memory byte @ 0x%x\n", a);
        
        if ( stat != NULL )
            *stat = MEM_UNMAPPED;
        
        return;
    }
    
    if ( mm->type == MAP_BLACKBOX )
        ((MIPS_Memory*)mm->mapped)->write_b(((MIPS_Memory*)mm->mapped), a, b, stat);
    else if ( mm->type != MAP_STATIC_RO ) {
        ((uint8_t*)mm->mapped)[a - mm->start] = b;
        if ( stat != NULL )
            *stat = MEM_OK;
    } else {
        if ( stat != NULL )
            *stat = MEM_READONLY;
        mipsim_printf(IO_WARNING, "MIPS: Tried to write in read only memory @ 0x%x\n", a);
    }
}

void mips_simple_write_h(MIPS_Memory *m, MIPS_Addr a, uint16_t h, int *stat)
{
    mips_simple_write_b(m, a,     h >> 8, stat);
    if ( stat != NULL && *stat != MEM_OK )
        return;
    mips_simple_write_b(m, a + 1, h & 0x00ff, stat);
}

void mips_simple_write_w(MIPS_Memory *m, MIPS_Addr a, uint32_t w, int *stat)
{
    mips_simple_write_h(m, a,     w >> 16, stat);
    if ( stat != NULL && *stat != MEM_OK )
        return;
    mips_simple_write_h(m, a + 2, w & 0X0000FFFF, stat);
}

void mips_simple_write_d(MIPS_Memory *m, MIPS_Addr a, uint64_t d, int *stat)
{
    mips_simple_write_w(m, a,     d >> 32, stat);
    if ( stat != NULL && *stat != MEM_OK )
        return;
    mips_simple_write_w(m, a + 4, d & 0x00000000FFFFFFFFL, stat);
}

void mips_init_memory(MIPS_Memory *mem)
{
    mem->d = NULL;
    
    mem->unmap  = mips_simple_unmap;
    
    mem->map_ro = mips_simple_map_ro;
    mem->map_rw = mips_simple_map_rw;
    mem->map_redir = mips_simple_map_redir;
    mem->map_alloc = mips_simple_map_alloc;
    
    mem->read_b = mips_simple_read_b;
    mem->read_h = mips_simple_read_h;
    mem->read_w = mips_simple_read_w;
    mem->read_d = mips_simple_read_d;
    
    mem->write_b = mips_simple_write_b;
    mem->write_h = mips_simple_write_h;
    mem->write_w = mips_simple_write_w;
    mem->write_d = mips_simple_write_d;
    
    // init default memory layout
    
    // Stack space : 0x7fff8000-0x80000000
    mem->map_alloc(mem, 0x7fff8000, 0x8000);
    
    // Some memory space apparently needed by newlib CRT or IDT monitor...
    // knowledge obtained after digging gdb sources to determine the reason
    // of an puzzling simulation mismatch...
    mem->map_alloc(mem, 0x80000000, 0x00800000);
}
