/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "mipsarch.h"

void mips_init_hardware(MIPS_Hardware *hw)
{
    /* default everything to 0 */
    memset(hw, 0, sizeof(MIPS_Hardware));
    
}

extern int decode_unknown(MIPS *m, uint32_t ir);

int decode_special (MIPS *m, uint32_t ir);
int decode_regimm  (MIPS *m, uint32_t ir);
int decode_j       (MIPS *m, uint32_t ir);
int decode_beq     (MIPS *m, uint32_t ir);
int decode_cp0     (MIPS *m, uint32_t ir);
int decode_cp1     (MIPS *m, uint32_t ir);
int decode_cp2     (MIPS *m, uint32_t ir);
int decode_cp3     (MIPS *m, uint32_t ir);
int decode_special2(MIPS *m, uint32_t ir);

const MIPS_Instr Rinstr[64] = {
    {"sll",     "d, t, <", decode_unknown,  ISA_from_1}, // 000000
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"srl",     "d, t, <", decode_unknown,  ISA_from_1}, // 000010
    {"sra",     "d, t, <", decode_unknown,  ISA_from_1},
    {"sllv",    "d, t, s", decode_unknown,  ISA_from_1}, // 000100
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"srlv",    "d, t, s", decode_unknown,  ISA_from_1},
    {"srav",    "d, t, s", decode_unknown,  ISA_from_1},
    {"jr",      "s",       decode_unknown,  ISA_from_1}, // 001000
    {"jalr",    "d, s",    decode_unknown,  ISA_from_1},
    {"movz",    "d, s, t", decode_unknown,  ISA_from_4 | ISA_32},
    {"movn",    "d, s, t", decode_unknown,  ISA_from_4 | ISA_32},
    {"syscall", "",        decode_unknown,  ISA_from_1},
    {"break",   "",        decode_unknown,  ISA_from_1},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"mfhi",    "d",       decode_unknown,  ISA_from_1}, // 010000
    {"mthi",    "s",       decode_unknown,  ISA_from_1},
    {"mflo",    "d",       decode_unknown,  ISA_from_1},
    {"mtlo",    "s",       decode_unknown,  ISA_from_1},
    {"dsllv",   "d, t, s", decode_unknown,  ISA_from_3},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"dsrlv",   "d, t, s", decode_unknown,  ISA_from_3},
    {"dsrav",   "d, t, s", decode_unknown,  ISA_from_3},
    {"mult",    "s, t",    decode_unknown,  ISA_from_1},
    {"multu",   "s, t",    decode_unknown,  ISA_from_1},
    {"div",     "s, t",    decode_unknown,  ISA_from_1},
    {"divu",    "s, t",    decode_unknown,  ISA_from_1},
    {"dmult",   "s, t",    decode_unknown,  ISA_from_3},
    {"dmultu",  "s, t",    decode_unknown,  ISA_from_3},
    {"ddiv",    "s, t",    decode_unknown,  ISA_from_3},
    {"ddivu",   "s, t",    decode_unknown,  ISA_from_3},
    {"add",     "d, s, t", decode_unknown,  ISA_from_1}, // 100000
    {"addu",    "d, s, t", decode_unknown,  ISA_from_1},
    {"sub",     "d, s, t", decode_unknown,  ISA_from_1},
    {"subu",    "d, s, t", decode_unknown,  ISA_from_1},
    {"and",     "d, s, t", decode_unknown,  ISA_from_1},
    {"or",      "d, s, t", decode_unknown,  ISA_from_1},
    {"xor",     "d, s, t", decode_unknown,  ISA_from_1},
    {"nor",     "d, s, t", decode_unknown,  ISA_from_1},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"slt",     "d, s, t", decode_unknown,  ISA_from_1},
    {"sltu",    "d, s, t", decode_unknown,  ISA_from_1},
    {"dadd",    "d, s, t", decode_unknown,  ISA_from_3},
    {"daddu",   "d, s, t", decode_unknown,  ISA_from_3},
    {"dsub",    "d, s, t", decode_unknown,  ISA_from_3},
    {"dsubu",   "d, s, t", decode_unknown,  ISA_from_3},
    {"tge",     "s, t",    decode_unknown,  ISA_from_2},
    {"tgeu",    "s, t",    decode_unknown,  ISA_from_2},
    {"tlt",     "s, t",    decode_unknown,  ISA_from_2},
    {"tltu",    "s, t",    decode_unknown,  ISA_from_2},
    {"teq",     "s, t",    decode_unknown,  ISA_from_2},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"tne",     "s, t",    decode_unknown,  ISA_from_2},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"dsll",    "d, t, <", decode_unknown,  ISA_from_3},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"dsrl",    "d, t, <", decode_unknown,  ISA_from_3},
    {"dsra",    "d, t, <", decode_unknown,  ISA_from_3},
    {"dsll32",  "d, t, <", decode_unknown,  ISA_from_3},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"dsrl32",  "d, t, <", decode_unknown,  ISA_from_3},
    {"dsra32",  "d, t, <", decode_unknown,  ISA_from_3}
};


const MIPS_Instr Rinstr2[64] = {
    {"madd",    "s, t",    decode_unknown, ISA_from_32}, // 000000
    {"maddu",   "s, t",    decode_unknown, ISA_from_32},
    {"mul",     "d, s, t", decode_unknown, ISA_from_32},
    {NULL,      NULL,      NULL          , ISA_NONE},
    {"msub",    "s, t",    decode_unknown, ISA_from_32},
    {"musbu",   "s, t",    decode_unknown, ISA_from_32},
    {NULL,      NULL, NULL, ISA_from_1},
    {NULL,      NULL, NULL, ISA_from_1},
    {NULL,      NULL,     NULL, ISA_from_1},
    {NULL,      NULL,  NULL, ISA_from_1},
    {NULL,      NULL, NULL, ISA_from_4 | ISA_32},
    {NULL,      NULL, NULL, ISA_from_4 | ISA_32},
    {NULL,      NULL,      NULL, ISA_from_1},
    {NULL,      NULL,      NULL, ISA_from_1},
    {NULL,      NULL,      NULL, ISA_NONE},
    {NULL,      NULL,      NULL, ISA_NONE},
    {NULL,      NULL,     NULL, ISA_from_1}, // 010000
    {NULL,      NULL,     NULL, ISA_from_1},
    {NULL,      NULL,     NULL, ISA_from_1},
    {NULL,      NULL,     NULL, ISA_from_1},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL,      NULL, ISA_NONE},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL,  NULL, ISA_from_1},
    {NULL,      NULL,  NULL, ISA_from_1},
    {NULL,      NULL,  NULL, ISA_from_1},
    {NULL,      NULL,  NULL, ISA_from_1},
    {NULL,      NULL,  NULL, ISA_from_3},
    {NULL,      NULL,  NULL, ISA_from_3},
    {NULL,      NULL,  NULL, ISA_from_3},
    {NULL,      NULL,  NULL, ISA_from_3},
    {"clz",     "d, s",    decode_unknown, ISA_from_32}, // 100000
    {"clo",     "d, s",    decode_unknown, ISA_from_32},
    {NULL,      NULL,      NULL          , ISA_NONE},
    {NULL,      NULL,      NULL          , ISA_NONE},
    {"dclz",    "d, s",    decode_unknown, ISA_from_32},
    {"dclo",    "d, s",    decode_unknown, ISA_from_32},
    {NULL,      NULL, NULL, ISA_from_1},
    {NULL,      NULL, NULL, ISA_from_1},
    {NULL,      NULL, NULL, ISA_from_1},
    {NULL,      NULL, NULL, ISA_from_1},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL,      NULL, ISA_NONE},
    {NULL,      NULL,      NULL, ISA_NONE},
    {NULL,      NULL,  NULL, ISA_from_2},
    {NULL,      NULL,  NULL, ISA_from_2},
    {NULL,      NULL,  NULL, ISA_from_2},
    {NULL,      NULL,  NULL, ISA_from_2},
    {NULL,      NULL,  NULL, ISA_from_2},
    {NULL,      NULL,      NULL, ISA_NONE},
    {NULL,      NULL,  NULL, ISA_from_2},
    {NULL,      NULL,      NULL, ISA_NONE},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL,      NULL, ISA_NONE},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL,      NULL, ISA_NONE},
    {NULL,      NULL, NULL, ISA_from_3},
    {NULL,      NULL, NULL, ISA_from_3}
};

const MIPS_Instr opcodes[64] = {
    {NULL,      NULL,      decode_special,  ISA_from_1},
    {NULL,      NULL,      decode_regimm,   ISA_from_1},
    {"j",       "a",       decode_j,        ISA_from_1},
    {"jal",     "a",       decode_j,        ISA_from_1},
    {"beq",     "s, t, p", decode_beq,      ISA_from_1}, // 000100
    {"bne",     "s, t, p", decode_unknown,  ISA_from_1},
    {"blez",    "s, p",    decode_unknown,  ISA_from_1},
    {"bgtz",    "s, p",    decode_unknown,  ISA_from_1},
    {"addi",    "t, s, i", decode_unknown,  ISA_from_1}, // 001000
    {"addiu",   "t, s, i", decode_unknown,  ISA_from_1},
    {"slti",    "t, s, i", decode_unknown,  ISA_from_1},
    {"sltiu",   "t, s, i", decode_unknown,  ISA_from_1},
    {"andi",    "t, s, i", decode_unknown,  ISA_from_1},
    {"ori",     "t, s, i", decode_unknown,  ISA_from_1},
    {"xori",    "t, s, i", decode_unknown,  ISA_from_1},
    {"lui",     "t, i",    decode_unknown,  ISA_from_1},
    {NULL,      NULL,      decode_cp0,      ISA_from_1}, // 010000
    {NULL,      NULL,      decode_cp1,      ISA_from_1},
    {NULL,      NULL,      decode_cp2,      ISA_from_1},
    {NULL,      NULL,      decode_cp3,      ISA_from_1},
    {"beql",    "s, t, p", decode_beq,      ISA_from_2},
    {"bnel",    "s, t, p", decode_beq,      ISA_from_2},
    {"blezl",   "s, p",    decode_unknown,  ISA_from_2},
    {"bgtzl",   "s, p",    decode_unknown,  ISA_from_2},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"ldl",     "t, i(s)", decode_unknown,  ISA_from_3},
    {"ldr",     "t, i(s)", decode_unknown,  ISA_from_3},
    {NULL,      NULL,      decode_special2, ISA_from_3},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"lb",      "t, i(s)", decode_unknown,  ISA_from_1}, // 100000
    {"lh",      "t, i(s)", decode_unknown,  ISA_from_1},
    {"lwl",     "t, i(s)", decode_unknown,  ISA_from_1},
    {"lw",      "t, i(s)", decode_unknown,  ISA_from_1},
    {"lbu",     "t, i(s)", decode_unknown,  ISA_from_1},
    {"lhu",     "t, i(s)", decode_unknown,  ISA_from_1},
    {"lwr",     "t, i(s)", decode_unknown,  ISA_from_1},
    {"lwu",     "t, i(s)", decode_unknown,  ISA_from_1},
    {"sb",      "t, i(s)", decode_unknown,  ISA_from_1},
    {"sh",      "t, i(s)", decode_unknown,  ISA_from_1},
    {"swl",     "t, i(s)", decode_unknown,  ISA_from_1},
    {"sw",      "t, i(s)", decode_unknown,  ISA_from_1},
    {"sdl",     "t, i(s)", decode_unknown,  ISA_from_3},
    {"sdr",     "t, i(s)", decode_unknown,  ISA_from_3},
    {"swr",     "t, i(s)", decode_unknown,  ISA_from_1},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"ll",      "t, i(s)", decode_unknown,  ISA_from_2}, // 110000
    {"lwc1",    "t, i(s)", decode_unknown,  ISA_from_1},
    {"lwc2",    "t, i(s)", decode_unknown,  ISA_from_1},
    {"pref",    "h, i(s)", decode_unknown,  ISA_from_4 | ISA_32},
    {"lld",     "t, i(s)", decode_unknown,  ISA_from_3},
    {"ldc1",    "t, i(s)", decode_unknown,  ISA_from_2},
    {"ldc2",    "t, i(s)", decode_unknown,  ISA_from_32},
    {"ld",      "t, i(s)", decode_unknown,  ISA_from_3},
    {"sc",      "t, i(s)", decode_unknown,  ISA_from_2}, // 111000
    {"swc1",    "t, i(s)", decode_unknown,  ISA_from_1}, 
    {"swc2",    "t, i(s)", decode_unknown,  ISA_from_1},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"scd",     "t, i(s)", decode_unknown,  ISA_from_3},
    {"sdc1",    "t, i(s)", decode_unknown,  ISA_from_2},
    {"sdc2",    "t, i(s)", decode_unknown,  ISA_from_32},
    {"sdi",     "t, i(s)", decode_unknown,  ISA_from_3}
};

#define DISASM_BUFFER_SIZE 128

const char* mips_disasm(const char *args, uint64_t pc, uint32_t ir)
{
    static char disasm_buffer[DISASM_BUFFER_SIZE];
    memset(disasm_buffer, 0, DISASM_BUFFER_SIZE);
    
    if ( !args )
        return disasm_buffer;
    
    const uint8_t sh = (ir & SH_MASK) >> SH_SHIFT;
    const uint8_t rd = (ir & RD_MASK) >> RD_SHIFT;
    const uint8_t rt = (ir & RT_MASK) >> RT_SHIFT;
    const uint8_t rs = (ir & RS_MASK) >> RS_SHIFT;
    const uint16_t imm = (ir & IMM_MASK);
    const uint32_t addr = (ir & ADDR_MASK);
    
    int j = 0;
    
    while ( *args && j < DISASM_BUFFER_SIZE )
    {
        char c = *args++;
        
        switch ( c )
        {
            case 's' :
                disasm_buffer[j++] = 'r';
                if ( rs >= 10 ) disasm_buffer[j++] = '0' + rs / 10;
                disasm_buffer[j++] = '0' + rs % 10;
                break;
                
            case 't' :
                disasm_buffer[j++] = 'r';
                if ( rt >= 10 ) disasm_buffer[j++] = '0' + rt / 10;
                disasm_buffer[j++] = '0' + rt % 10;
                break;
                
            case 'd' :
                disasm_buffer[j++] = 'r';
                if ( rd >= 10 ) disasm_buffer[j++] = '0' + rd / 10;
                disasm_buffer[j++] = '0' + rd % 10;
                break;
                
            case '<' :
                if ( sh >= 10 ) disasm_buffer[j++] = '0' + sh / 10;
                disasm_buffer[j++] = '0' + sh % 10;
                break;
                
            case 'i' :
                sprintf(disasm_buffer + j, "0x%04X", imm);
                j += 6;
                break;
                
            case 'p' :
                sprintf(disasm_buffer + j, "0x%08X", (addr << 2) + (uint32_t)pc);
                j += 10;
                break;
                
            case 'a' :
                sprintf(disasm_buffer + j, "0x%08X", addr);
                j += 10;
                break;
                
            default:
                disasm_buffer[j++] = c;
                break;
        };
    }
    
    return disasm_buffer;
}

enum {
    OP_SPECIAL = 0,
    OP_J = 2,
    OP_JAL = 3,
    OP_CP0 = 0x10,
    OP_CP1 = 0x11,
    OP_CP2 = 0x12,
    OP_CP3 = 0x13,
    OP_SPECIAL2 = 0x1C
};

int mips_universal_decode(MIPS *m)
{
    uint32_t ir = m->hw.ir = m->mem.read_w(&m->mem, m->hw.pc);
    
    printf("%08x:\t%08x\t", (uint32_t)m->hw.pc, ir);
    
    int ret = MIPS_OK;
    const uint32_t op = (ir & OPCODE_MASK) >> OPCODE_SHIFT;
    
    MIPS_Instr i = opcodes[op];
    
    if ( i.decode != NULL )
    {
        if ( i.mnemonic != NULL )
            printf("%s %s", i.mnemonic, mips_disasm(i.args, m->hw.pc, ir));
        
        if ( i.isa & (1 << m->architecture) )
        {
            if ( i.mnemonic != NULL ) printf("\n");
            ret = i.decode(m, ir);
        } else {
            if ( i.mnemonic != NULL ) printf("\t\t[Unsupported in %s]\n", mips_isa_name(m->architecture));
            // error handling...
        }
    } else {
        // error handling...
    }
    
    m->hw.pc += 4;
    
    return ret;
}

int decode_special (MIPS *m, uint32_t ir)
{
    MIPS_Instr i = Rinstr[(ir & FN_MASK)];
    
    if ( i.decode )
    {
        if ( !ir )
            printf("nop");
        else if ( i.mnemonic != NULL )
            printf("%s %s", i.mnemonic, mips_disasm(i.args, m->hw.pc, ir));
        
        if ( i.isa & (1 << m->architecture) )
        {
            printf("\n");
            return i.decode(m, ir);
        } else {
            printf("\t\t[Unsupported in %s]\n", mips_isa_name(m->architecture));
            return 1;
        }
    }
    
    return MIPS_OK;
}

int decode_special2(MIPS *m, uint32_t ir)
{
    MIPS_Instr i = Rinstr2[(ir & FN_MASK)];
    
    if ( i.decode )
    {
        if ( i.mnemonic != NULL )
            printf("%s %s", i.mnemonic, mips_disasm(i.args, m->hw.pc, ir));
        
        if ( i.isa & (1 << m->architecture) )
        {
            printf("\n");
            return i.decode(m, ir);
        } else {
            printf("\t\t[Unsupported in %s]\n", mips_isa_name(m->architecture));
            return 1;
        }
    }
    
    return MIPS_OK;
}

int decode_j       (MIPS *m, uint32_t ir)
{
    const uint32_t addr = (ir & ADDR_MASK);
    //printf("%s 0x%08x\n", ir & 0x04000000 ? "jal" : "j", addr);
    
    return MIPS_OK;
}

int decode_regimm  (MIPS *m, uint32_t ir)
{
    printf("\n");
    return MIPS_OK;
}

int decode_beq     (MIPS *m, uint32_t ir)
{
    return MIPS_OK;
}

int decode_cp0     (MIPS *m, uint32_t ir)
{
    printf("cop 0\n");
    return MIPS_OK;
}

int decode_cp1     (MIPS *m, uint32_t ir)
{
    printf("cop 1\n");
    return MIPS_OK;
}

int decode_cp2     (MIPS *m, uint32_t ir)
{
    printf("cop 2\n");
    return MIPS_OK;
}

int decode_cp3     (MIPS *m, uint32_t ir)
{
    printf("cop 3\n");
    return MIPS_OK;
}
