/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "mips.h"

#include "io.h"

enum {
    MAP_NONE,
    MAP_STATIC,
    MAP_BLACKBOX,
    MAP_DYNAMIC
};

typedef struct _MemMapping MemMapping;
typedef struct _TMemMapping TMemMapping;

struct _MemMapping {
    short type;
    short flags;
    void *mapped;
    MIPS_Addr start, end;
    
    MemMapping *next;
};

/*
struct _TMemMapping {
    int type;
    void *mapped;
    MIPS_Addr start, end;
    
    TMemMapping *left, *right;
};

TMemMapping* mips_tree_mapping(MIPS_Memory *m, MIPS_Addr a)
{
    TMemMapping *mm = m ? m->d : NULL;
    
    while ( mm != NULL )
    {
        if ( a < mm->start )
            mm = mm->left;
        else if ( a >= mm->end )
            mm = mm->right;
        else
            return mm;
    }
    
    return NULL;
}

void _tree_unmap(TMemMapping *mm)
{
    if ( mm != NULL )
    {
        if ( mm->type == MAP_BLACKBOX )
            ((MIPS_Memory*)mm->mapped)->unmap((MIPS_Memory*)mm->mapped);
        else if ( mm->type == MAP_DYNAMIC )
            free(mm->mapped);
        
        _tree_unmap(mm->left);
        _tree_unmap(mm->right);
        free(mm);
    }
}

void mips_tree_unmap(MIPS_Memory *m)
{
    if ( m != NULL )
    {
        _tree_unmap(m->d);
        m->d = NULL;
    }
}

int mips_tree_map_ro(MIPS_Memory *m, MIPS_Addr a, uint32_t s, uint8_t *d)
{
//     TMemMapping *mm = (TMemMapping*)malloc(sizeof(TMemMapping));
//     mm->type = MAP_STATIC_RO;
//     mm->start = a;
//     mm->end = a + s;
//     mm->mapped = d;
//     
//     mm->next = (TMemMapping*)m->d;
//     m->d = mm;
//     
    return 0;
}
*/

////////////////////////////////////////////////////////////////////////////////////

void mips_simple_init(MIPS_Memory *mem);

void mips_simple_dump_mapping(FILE *f, const char *indent, MIPS_Memory *m)
{
    MemMapping *mm = m ? m->d : NULL;
    
    while ( mm != NULL )
    {
        if ( mm->type == MAP_BLACKBOX )
        {
            fprintf(f, "%s R%c%c %08x %08x [BB]\n",
                   indent,
                   mm->flags & MEM_READONLY ? ' ' : 'W',
                   mm->flags & MEM_NOEXEC ? ' ' : 'X',
                   mm->start,
                   mm->end - 1);
            char sident[strlen(indent) + 2];
            strcpy(sident, indent);
            strcat(sident, "  ");
            mips_simple_dump_mapping(f, sident, (MIPS_Memory*)mm->mapped);
        } else {
            fprintf(f, "%s R%c%c %08x %08x\n",
                   indent,
                   mm->flags & MEM_READONLY ? ' ' : 'W',
                   mm->flags & MEM_NOEXEC ? ' ' : 'X',
                   mm->start,
                   mm->end - 1);
        }
        
        mm = mm->next;
    }
}

int mips_lazy_alloc_pagefault(MIPS_Memory *m, MIPS_Addr a)
{
    MIPS_Addr base = a & 0xFFFFF000;
    
    mipsim_printf(IO_DEBUG, "Page fault @ %08x => lazy alloc [%08x-%08x]\n", a, base, base + 0xFFF);
    
    return m->map_alloc(m, base, 0x1000, 0);
}

MemMapping* mips_simple_mapping(MIPS_Memory *m, MIPS_Addr a)
{
    MemMapping *mm = m ? m->d : NULL;
    
    while ( mm != NULL )
    {
        if ( mm->start <= a && a < mm->end )
            return mm;
        
        mm = mm->next;
    }
    
    if ( m->pagefault != NULL )
    {
        if ( !m->pagefault(m, a) )
        {
            mem_pagefault pf = m->pagefault;
            m->pagefault = NULL;
            mm = mips_simple_mapping(m, a);
            m->pagefault = pf;
            return mm;
        }
    }
    
    return NULL;
}

int mips_mapping_overlap(MIPS_Memory *m, MIPS_Addr start, MIPS_Addr end)
{
    MemMapping *mm = m ? m->d : NULL;
    
    while ( mm != NULL )
    {
        if ( mm->start <= end && start < mm->end )
            return 1;
        
        mm = mm->next;
    }
    
    return 0;
}

void mips_simple_unmap(MIPS_Memory *m)
{
    MemMapping *mm = m ? m->d : NULL, *tmp;
    
    while ( mm != NULL )
    {
        if ( mm->type == MAP_BLACKBOX )
        {
            ((MIPS_Memory*)mm->mapped)->unmap((MIPS_Memory*)mm->mapped);
            free(mm->mapped);
        } else if ( mm->type == MAP_DYNAMIC ) {
            free(mm->mapped);
        }
        
        tmp = mm->next;
        free(mm);
        mm = tmp;
    }
    
    if ( m != NULL )
        m->d = NULL;
}

int mips_simple_map_static(MIPS_Memory *m, MIPS_Addr a, uint32_t s, uint8_t *d, short flags)
{
    if ( mips_mapping_overlap(m, a, a + s) )
        return 1;
    
    MemMapping *mm = (MemMapping*)malloc(sizeof(MemMapping));
    mm->type   = MAP_STATIC;
    mm->flags  = flags;
    mm->start  = a;
    mm->end    = a + s;
    mm->mapped = d;
    mm->next = (MemMapping*)m->d;
    m->d = mm;
    
    return 0;
}

int mips_simple_map_redir(MIPS_Memory *m, MIPS_Addr a, uint32_t s, MIPS_Memory *r, short flags)
{
    if ( mips_mapping_overlap(m, a, a + s) )
        return 1;
    
    MemMapping *mm = (MemMapping*)malloc(sizeof(MemMapping));
    mm->type   = MAP_BLACKBOX;
    mm->flags  = flags;
    mm->start  = a;
    mm->end    = a + s;
    mm->mapped = r;
    mm->next = (MemMapping*)m->d;
    m->d = mm;
    
    return 0;
}

int mips_simple_map_alloc(MIPS_Memory *m, MIPS_Addr a, uint32_t s, short flags)
{
    if ( mips_mapping_overlap(m, a, a + s) )
        return 1;
    
    if ( flags & MEM_LAZY )
    {
        MIPS_Memory *r = (MIPS_Memory*)malloc(sizeof(MIPS_Memory));
        
        // init block
        mips_simple_init(r);
        
        r->pagefault = mips_lazy_alloc_pagefault;
        
        int ret = mips_simple_map_redir(m, a, s, r, flags);
        
        if ( ret )
        {
            mipsim_printf(IO_WARNING,
                          "Failed creating lazily allocated chunk [%08x-%08x]\n",
                          a, a + s);
            free(r);
            return ret;
        }
    } else {
        MemMapping *mm = (MemMapping*)malloc(sizeof(MemMapping));
        mm->type   = MAP_DYNAMIC;
        mm->flags  = flags;
        mm->start  = a;
        mm->end    = a + s;
        mm->mapped = malloc(s);
        mm->next   = m->d;
        m->d = mm;
    }
    
    return 0;
}

uint8_t mips_simple_read_b(MIPS_Memory *m, MIPS_Addr a, int *stat)
{
    MemMapping *mm = mips_simple_mapping(m, a);
    
    if ( mm == NULL )
    {
        if ( stat != NULL )
            *stat = MEM_UNMAPPED;
        
        return 0;
    }
    
    if ( stat != NULL )
        *stat = mm->flags;
    
    if ( mm->type == MAP_BLACKBOX )
    {
        int hstat;
        uint8_t r = ((MIPS_Memory*)mm->mapped)->read_b(((MIPS_Memory*)mm->mapped), a, &hstat);
        if ( stat != NULL )
            *stat |= hstat;
        return r;
    }
    
    return ((uint8_t*)mm->mapped)[a - mm->start];
}

uint16_t mips_simple_read_h(MIPS_Memory *m, MIPS_Addr a, int *stat)
{
    int hstat;
    uint8_t b0 = mips_simple_read_b(m, a, &hstat);
    if ( stat != NULL )
        *stat = hstat;
    uint8_t b1 = mips_simple_read_b(m, a + 1, &hstat);
    if ( stat != NULL )
        *stat |= hstat;
    
    return ((uint16_t)b0 << 8) | b1;
}

uint32_t mips_simple_read_w(MIPS_Memory *m, MIPS_Addr a, int *stat)
{
    int hstat;
    uint16_t h0 = mips_simple_read_h(m, a, &hstat);
    if ( stat != NULL )
        *stat = hstat;
    uint16_t h1 = mips_simple_read_h(m, a + 2, &hstat);
    if ( stat != NULL )
        *stat |= hstat;
    
    return ((uint32_t)h0 << 16) | h1;
}

uint64_t mips_simple_read_d(MIPS_Memory *m, MIPS_Addr a, int *stat)
{
    int hstat;
    uint32_t w0 = mips_simple_read_w(m, a, &hstat);
    if ( stat != NULL )
        *stat = hstat;
    uint32_t w1 = mips_simple_read_w(m, a + 4, &hstat);
    if ( stat != NULL )
        *stat |= hstat;
    
    return ((uint64_t)w0 << 32) | w1;
}


void mips_simple_write_b(MIPS_Memory *m, MIPS_Addr a, uint8_t b, int *stat)
{
    MemMapping *mm = mips_simple_mapping(m, a);
    
    if ( mm == NULL )
    {
        if ( stat != NULL )
            *stat = MEM_UNMAPPED;
        
        return;
    }
    
    if ( stat != NULL )
        *stat = mm->flags;
    
    if ( mm->type == MAP_BLACKBOX )
    {
        int hstat;
        ((MIPS_Memory*)mm->mapped)->write_b(((MIPS_Memory*)mm->mapped), a, b, &hstat);
        if ( stat != NULL )
            *stat |= hstat;
        
    } else {
        ((uint8_t*)mm->mapped)[a - mm->start] = b;
    }
}

void mips_simple_write_h(MIPS_Memory *m, MIPS_Addr a, uint16_t h, int *stat)
{
    int hstat;
    mips_simple_write_b(m, a,     h >> 8, &hstat);
    if ( stat != NULL )
        *stat = hstat;
    mips_simple_write_b(m, a + 1, h & 0x00ff, &hstat);
    if ( stat != NULL )
        *stat |= hstat;
}

void mips_simple_write_w(MIPS_Memory *m, MIPS_Addr a, uint32_t w, int *stat)
{
    int hstat;
    mips_simple_write_h(m, a,     w >> 16, &hstat);
    if ( stat != NULL )
        *stat = hstat;
    mips_simple_write_h(m, a + 2, w & 0X0000FFFF, &hstat);
    if ( stat != NULL )
        *stat |= hstat;
}

void mips_simple_write_d(MIPS_Memory *m, MIPS_Addr a, uint64_t d, int *stat)
{
    int hstat;
    mips_simple_write_w(m, a,     d >> 32, &hstat);
    if ( stat != NULL )
        *stat = hstat;
    mips_simple_write_w(m, a + 4, d & 0x00000000FFFFFFFFL, &hstat);
    if ( stat != NULL )
        *stat |= hstat;
}

void mips_init_memory(MIPS *m)
{
    mips_simple_init(&m->mem);
}

void mips_simple_init(MIPS_Memory *mem)
{
    mem->d = NULL;
    
    mem->unmap  = mips_simple_unmap;
    
    mem->pagefault = NULL;
    
    mem->map_static = mips_simple_map_static;
    mem->map_redir  = mips_simple_map_redir;
    mem->map_alloc  = mips_simple_map_alloc;
    
    mem->read_b = mips_simple_read_b;
    mem->read_h = mips_simple_read_h;
    mem->read_w = mips_simple_read_w;
    mem->read_d = mips_simple_read_d;
    
    mem->write_b = mips_simple_write_b;
    mem->write_h = mips_simple_write_h;
    mem->write_w = mips_simple_write_w;
    mem->write_d = mips_simple_write_d;
    
    mem->dump_mapping = mips_simple_dump_mapping;
}
