/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "mipself.h"

#include "io.h"
#include "config.h"

static ELF32_Addr section_addr_end[] = {
    0, 0
};

/*!
    \brief Auxiliary function for section placement
    \param name section name
    \param size section size
    \return address at which the section should be placed
    
    This function uses *ugly* static variables. It is neither thread-safe
    nor reentrant.
*/
ELF32_Addr section_addr(const char *name, ELF32_Word size)
{
    ELF32_Addr a = 0;
    int n = -1;
    
    if ( !strcmp(name, ".text") )
    {
        n = 0;
    } else if ( !strcmp(name, ".data")
                || !strcmp(name, ".rodata")
                || !strcmp(name, ".bss") ) {
        n = section_addr_end[1] == 0xFFFFFFFF ? 0 : 1;
    }
    
    if ( n != -1 )
    {
        a = section_addr_end[n];
        
        if ( a & 0xFFF )
            section_addr_end[n] = a = (a & ~0xFFF) + 0x1000;
        
        section_addr_end[n] += size;
    } else {
        mipsim_printf(IO_WARNING, "Asked to place section %s. No idea what to do...\n", name);
    }
    
    return a;
}

/*!
    \brief "Place" all relocatable sections of an ELF file
    \param elf file to place
    \return 0 on success
    
    Placement follows config constraints (text/data positions and size) but no overlap
    chekc is performed
    
    \note This function uses \ref section_addr and suffers from the same limitations
*/
int place_sections(ELF_File *elf)
{
    MIPSIM_Config *cfg = mipsim_config();
    section_addr_end[0] = cfg->reloc_text;
    section_addr_end[1] = cfg->reloc_data;
    
    if ( cfg->reloc_data < cfg->reloc_text )
    {
        mipsim_printf(IO_WARNING, "Invalid placement constraint on .data section\n");
        return 1;
    }
    
    /*
        ensure that all SHF_ALLOC sections are placed
    */
    for ( ELF32_Word i = 0; i < elf->nsection; ++i )
    {
        ELF_Section *s = elf->sections[i];
        
        if ( s == NULL || s->s_data == NULL )
            continue;
        
        if ( (s->s_flags & SHF_ALLOC) && !s->s_addr )
        {
            ELF32_Word size;
            const char *name = elf_section_name(elf, i, &size);
            s->s_addr = section_addr(name, size);
            /*
            if ( s->s_addr + s->s_size > cfg->address_max )
            {
                mipsim_printf(IO_WARNING, "Section %s goes beyond last valid address\n", name);
                return 1;
            }
            */
            mipsim_printf(IO_DEBUG, "Placed %s @ 0x%08x\n", name, s->s_addr);
        }
    }
    
    return 0;
}

/*!
    \brief Load an executable from a ELF file into a simulated machine
    \param m target machine
    \param f ELF file
    
    Both executable and relocatable ELF files can be loaded, provided they
    don't have any external dependencies.
*/
int mips_load_elf(MIPS *m, ELF_File *f)
{
    ELF32_Word sz = 0;
    ELF32_Addr last = 0;
    MIPSIM_Config *cfg = mipsim_config();
    
    if ( f->header->e_type == ET_EXEC )
    {
        // when loading a program : 
        //  - load segments and not sections
        //  - assume GCC/newlib memory layout expected...
        //  - bypass any -t -d -s switches...
        
        for ( ELF32_Word i = 0; i < f->nsegment; ++i )
        {
            ELF_Segment *s = f->segments[i];
            
            if ( s == NULL )
                continue;
            
            sz += s->p_memsz;
            
            if ( sz >= cfg->phys_memory_size )
            {
                mipsim_printf(IO_WARNING,
                              "Program too large (availeable=%08x, required>=%08x)\n",
                              cfg->phys_memory_size, sz);
                return 1;
            }
            
            // Load sections to fine tune RO/RW status...
            mipsim_printf(IO_DEBUG, "Loading segment %i : [%08x-%08x]\n",
                          i, s->p_vaddr, s->p_vaddr + s->p_memsz);
            
            int flags = MEM_RWX;
            
            if ( m->mem.map_static(&m->mem, s->p_vaddr, s->p_memsz, s->p_data, flags) )
            {
                mipsim_printf(IO_WARNING, "Failed mapping program segment in simulator memory\n");
                return 1;
            }
            
            if ( last < s->p_vaddr + s->p_memsz )
                last = s->p_vaddr + s->p_memsz;
        }
        
        // default GCC/newlib memory layout
        
        // Stack space : 0x7fff8000-0x80000000
        // TODO : alloc on demand
        m->mem.map_alloc(&m->mem, 0x7fff8000, 0x8000, MEM_RW);
        
        // Some memory space apparently needed by newlib CRT or IDT monitor...
        // knowledge obtained after digging gdb sources to determine the reason
        // of a puzzling simulation mismatch...
        // TODO : allow tweaking the size of this region
        // TODO : alloc on demand
        m->mem.map_alloc(&m->mem, 0x80000000, 0x00800000, MEM_RW);
        
        // alloc data beyond bss for dynamic data
        // TODO : alloc on-demand
        if ( sz < cfg->phys_memory_size )
        {
            m->mem.map_alloc(&m->mem, last, cfg->phys_memory_size - sz, MEM_RW);
        } else {
            mipsim_printf(IO_WARNING, "Program occupies all physical memory : no space left for dyn data\n");
        }
        
        if ( f->header )
            mips_set_reg(m, PC, f->header->e_entry);
    } else if ( f->header->e_type == ET_REL ) {
        // when loading a relocatable binary :
        //  - perform section placement according to constraints
        //  - perform relocations
        //  - map code on a section basis, check for overlap
        //    honor WX attributes [TODO]
        
        // TODO : different alloc/map strategy
        
        if ( place_sections(f) )
        {
            mipsim_printf(IO_WARNING, "Section placement failed\n");
            return 1;
        }
        
        if ( elf_file_relocate(f) )
        {
            mipsim_printf(IO_WARNING, "Relocation failed\n");
            return 1;
        }
        
        for ( ELF32_Word i = 0; i < f->nsection; ++i )
        {
            ELF_Section *s = f->sections[i];
            
            if ( s == NULL || !(s->s_flags & SHF_ALLOC) )
                continue;
            
            sz += s->s_size;
            
            if ( sz >= cfg->phys_memory_size )
            {
                mipsim_printf(IO_WARNING,
                              "Program too large (availeable=%08x, required>=%08x)\n",
                              cfg->phys_memory_size, sz);
                return 1;
            }
            
            // Load sections to fine tune RO/RW status...
            mipsim_printf(IO_DEBUG, "Loading section %i : [%08x-%08x]\n",
                          i, s->s_addr, s->s_addr + s->s_size);
            
            int flags = 0;
            if ( !(s->s_flags & SHF_WRITE) )
                flags |= MEM_READONLY;
            if ( !(s->s_flags & SHF_EXECINSTR) )
                flags |= MEM_NOEXEC;
            
            if ( m->mem.map_static(&m->mem, s->s_addr, s->s_size, s->s_data, flags) )
            {
                mipsim_printf(IO_WARNING, "Failed mapping program segment in simulator memory\n");
                return 1;
            }
        }
        
        // allocate remaining space between static data and first invalid address (used for both
        // stack and dynamic data)
        // TODO : alloc on demand
        ELF32_Addr dla = section_addr_end[1] == 0xFFFFFFFF ? section_addr_end[0] : section_addr_end[1];
        ELF32_Addr la = dla + (cfg->phys_memory_size - sz);
        
        if ( cfg->phys_memory_size > sz )
        {
            m->mem.map_alloc(&m->mem, dla, cfg->phys_memory_size - sz, MEM_RW);
        } else {
            mipsim_printf(IO_WARNING, "Program occupies all physical memory : no space left for dyn data\n");
        }
        
        // default register values
        mips_set_reg(m, SP, la);
        mips_set_reg(m, PC, cfg->reloc_text);
    }
    
    return 0;
}

