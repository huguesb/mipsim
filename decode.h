/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#ifndef _MIPS_ARCH_H_
#define _MIPS_ARCH_H_

/*!
    \file decode.h
    \brief MIPS instruction decode/execute
    \author Hugues Bruant
*/

#include "mips.h"

enum {
    OPCODE_MASK    = 0xFC000000,
    OPCODE_UP_MASK = 0xF0000000,
    
    RS_MASK        = 0x03E00000,
    RT_MASK        = 0x001F0000,
    RD_MASK        = 0x0000F800,
    SH_MASK        = 0x000007C0,
    FN_MASK        = 0x0000003F,
    IMM_MASK       = 0x0000FFFF,
    ADDR_MASK      = 0x03FFFFFF,
    
    OPCODE_SHIFT   = 26,
    RS_SHIFT       = 21,
    RT_SHIFT       = 16,
    RD_SHIFT       = 11,
    SH_SHIFT       = 6,
    FN_SHIFT       = 0,
    
    FMT_MASK       = 0x03E00000,
    FS_MASK        = 0x0000F800,
    FD_MASK        = 0x000007C0,
    
    FMT_SHIFT      = 21,
    FS_SHIFT       = 11,
    FD_SHIFT       = 6
};

enum {
    ISA_NONE = 0,
    
    ISA_1 = 1 << MIPS_1,
    ISA_2 = 1 << MIPS_2,
    ISA_3 = 1 << MIPS_3,
    ISA_4 = 1 << MIPS_4,
    ISA_5 = 1 << MIPS_5,
    ISA_32 = 1 << MIPS_32,
    ISA_32_R2 = 1 << MIPS_32_R2,
    ISA_64 = 1 << MIPS_64,
    ISA_64_R2 = 1 << MIPS_64_R2,
    
    ISA_from_1 = ISA_1 | ISA_2 | ISA_3 | ISA_4 | ISA_5 | ISA_32 | ISA_32_R2 | ISA_64 | ISA_64_R2,
    ISA_from_2 =         ISA_2 | ISA_3 | ISA_4 | ISA_5 | ISA_32 | ISA_32_R2 | ISA_64 | ISA_64_R2,
    ISA_from_3 =                 ISA_3 | ISA_4 | ISA_5                      | ISA_64 | ISA_64_R2,
    ISA_from_4 =                         ISA_4 | ISA_5                      | ISA_64 | ISA_64_R2,
    ISA_from_5 =                                 ISA_5                      | ISA_64 | ISA_64_R2,
    ISA_from_32 =                                        ISA_32 | ISA_32_R2 | ISA_64 | ISA_64_R2
};

typedef int (*instr_decode)(MIPS *m, uint32_t ir);

typedef struct _MIPS_Instr {
    const char *mnemonic;
    const char *args;
    instr_decode decode;
    int isa;
} MIPS_Instr;

#endif
