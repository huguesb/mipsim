/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "decode.h"

#include "io.h"

int decode_unknown(MIPS *m, uint32_t ir);

int decode_special (MIPS *m, uint32_t ir);
int decode_regimm  (MIPS *m, uint32_t ir);
int decode_j       (MIPS *m, uint32_t ir);
int decode_beq     (MIPS *m, uint32_t ir);
int decode_blez    (MIPS *m, uint32_t ir);
int decode_bltz    (MIPS *m, uint32_t ir);
int decode_cp0     (MIPS *m, uint32_t ir);
int decode_cp1     (MIPS *m, uint32_t ir);
int decode_cp2     (MIPS *m, uint32_t ir);
int decode_cp3     (MIPS *m, uint32_t ir);
int decode_special2(MIPS *m, uint32_t ir);

int decode_shift   (MIPS *m, uint32_t ir);
int decode_jr      (MIPS *m, uint32_t ir);
int decode_movhilo (MIPS *m, uint32_t ir);
int decode_add     (MIPS *m, uint32_t ir);
int decode_sub     (MIPS *m, uint32_t ir);
int decode_mult    (MIPS *m, uint32_t ir);
int decode_multu   (MIPS *m, uint32_t ir);
int decode_div     (MIPS *m, uint32_t ir);
int decode_divu    (MIPS *m, uint32_t ir);
int decode_and     (MIPS *m, uint32_t ir);
int decode_or      (MIPS *m, uint32_t ir);
int decode_xor     (MIPS *m, uint32_t ir);
int decode_nor     (MIPS *m, uint32_t ir);
int decode_slt     (MIPS *m, uint32_t ir);
int decode_sltu    (MIPS *m, uint32_t ir);

int decode_movcond (MIPS *m, uint32_t ir);

int decode_addi    (MIPS *m, uint32_t ir);
int decode_slti    (MIPS *m, uint32_t ir);
int decode_sltiu   (MIPS *m, uint32_t ir);
int decode_andi    (MIPS *m, uint32_t ir);
int decode_ori     (MIPS *m, uint32_t ir);
int decode_xori    (MIPS *m, uint32_t ir);
int decode_lui     (MIPS *m, uint32_t ir);

int decode_lb      (MIPS *m, uint32_t ir);
int decode_lbu     (MIPS *m, uint32_t ir);
int decode_lh      (MIPS *m, uint32_t ir);
int decode_lhu     (MIPS *m, uint32_t ir);
int decode_lw      (MIPS *m, uint32_t ir);
int decode_lwu     (MIPS *m, uint32_t ir);
int decode_lwl     (MIPS *m, uint32_t ir);
int decode_lwr     (MIPS *m, uint32_t ir);
int decode_ld      (MIPS *m, uint32_t ir);
int decode_ldl     (MIPS *m, uint32_t ir);
int decode_ldr     (MIPS *m, uint32_t ir);

int decode_sb      (MIPS *m, uint32_t ir);
int decode_sh      (MIPS *m, uint32_t ir);
int decode_sw      (MIPS *m, uint32_t ir);
int decode_swl     (MIPS *m, uint32_t ir);
int decode_swr     (MIPS *m, uint32_t ir);
int decode_sdi     (MIPS *m, uint32_t ir);
int decode_sdl     (MIPS *m, uint32_t ir);
int decode_sdr     (MIPS *m, uint32_t ir);

int decode_break   (MIPS *m, uint32_t ir);
int decode_trap    (MIPS *m, uint32_t ir);

const MIPS_Instr Rinstr[64] = {
    {"sll",     "d, t, <", decode_shift,    ISA_from_1}, // 000000
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"srl",     "d, t, <", decode_shift,    ISA_from_1}, // 000010
    {"sra",     "d, t, <", decode_shift,    ISA_from_1},
    {"sllv",    "d, t, s", decode_shift,    ISA_from_1}, // 000100
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"srlv",    "d, t, s", decode_shift,    ISA_from_1},
    {"srav",    "d, t, s", decode_shift,    ISA_from_1},
    {"jr",      "s",       decode_jr,       ISA_from_1}, // 001000
    {"jalr",    "d, s",    decode_jr,       ISA_from_1},
    {"movz",    "d, s, t", decode_movcond,  ISA_from_4 | ISA_32},
    {"movn",    "d, s, t", decode_movcond,  ISA_from_4 | ISA_32},
    {"syscall", "",        decode_unknown,  ISA_from_1},
    {"break",   "",        decode_break,    ISA_from_1},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"mfhi",    "d",       decode_movhilo,  ISA_from_1}, // 010000
    {"mthi",    "s",       decode_movhilo,  ISA_from_1},
    {"mflo",    "d",       decode_movhilo,  ISA_from_1},
    {"mtlo",    "s",       decode_movhilo,  ISA_from_1},
    {"dsllv",   "d, t, s", decode_unknown,  ISA_from_3},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"dsrlv",   "d, t, s", decode_unknown,  ISA_from_3},
    {"dsrav",   "d, t, s", decode_unknown,  ISA_from_3},
    {"mult",    "s, t",    decode_mult,     ISA_from_1},
    {"multu",   "s, t",    decode_multu,    ISA_from_1},
    {"div",     "s, t",    decode_div,      ISA_from_1},
    {"divu",    "s, t",    decode_divu,     ISA_from_1},
    {"dmult",   "s, t",    decode_unknown,  ISA_from_3},
    {"dmultu",  "s, t",    decode_unknown,  ISA_from_3},
    {"ddiv",    "s, t",    decode_unknown,  ISA_from_3},
    {"ddivu",   "s, t",    decode_unknown,  ISA_from_3},
    {"add",     "d, s, t", decode_add,      ISA_from_1}, // 100000
    {"addu",    "d, s, t", decode_add,      ISA_from_1},
    {"sub",     "d, s, t", decode_sub,      ISA_from_1},
    {"subu",    "d, s, t", decode_sub,      ISA_from_1},
    {"and",     "d, s, t", decode_and,      ISA_from_1},
    {"or",      "d, s, t", decode_or,       ISA_from_1},
    {"xor",     "d, s, t", decode_xor,      ISA_from_1},
    {"nor",     "d, s, t", decode_nor,      ISA_from_1},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"slt",     "d, s, t", decode_slt,      ISA_from_1},
    {"sltu",    "d, s, t", decode_sltu,     ISA_from_1},
    {"dadd",    "d, s, t", decode_unknown,  ISA_from_3},
    {"daddu",   "d, s, t", decode_unknown,  ISA_from_3},
    {"dsub",    "d, s, t", decode_unknown,  ISA_from_3},
    {"dsubu",   "d, s, t", decode_unknown,  ISA_from_3},
    {"tge",     "s, t",    decode_trap,     ISA_from_2},
    {"tgeu",    "s, t",    decode_trap,     ISA_from_2},
    {"tlt",     "s, t",    decode_trap,     ISA_from_2},
    {"tltu",    "s, t",    decode_trap,     ISA_from_2},
    {"teq",     "s, t",    decode_trap,     ISA_from_2},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"tne",     "s, t",    decode_trap,     ISA_from_2},
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
    {"madd",    "s, t",    decode_unknown,  ISA_from_32}, // 000000
    {"maddu",   "s, t",    decode_unknown,  ISA_from_32},
    {"mul",     "d, s, t", decode_unknown,  ISA_from_32},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"msub",    "s, t",    decode_unknown,  ISA_from_32},
    {"musbu",   "s, t",    decode_unknown,  ISA_from_32},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE}, // 010000
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"clz",     "d, s",    decode_unknown,  ISA_from_32}, // 100000
    {"clo",     "d, s",    decode_unknown,  ISA_from_32},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {NULL,      NULL,      NULL          ,  ISA_NONE},
    {"dclz",    "d, s",    decode_unknown,  ISA_from_32},
    {"dclo",    "d, s",    decode_unknown,  ISA_from_32},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE}
};

const MIPS_Instr Iinstr[32] = {
    {"bltz",    "s, p",    decode_bltz,     ISA_from_1}, // 000000
    {"bgez",    "s, p",    decode_bltz,     ISA_from_1},
    {"bltzl",   "s, p",    decode_bltz,     ISA_from_2},
    {"bgezl",   "s, p",    decode_bltz,     ISA_from_2},
    {"",    "",    decode_unknown,  ISA_NONE},
    {"",    "",    decode_unknown,  ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"bltzal",  "s, p",    decode_bltz,     ISA_from_1},
    {"bgezal",  "s, p",    decode_bltz,     ISA_from_1},
    {"bltzall", "s, p",    decode_bltz,     ISA_from_2},
    {"bgezall", "s, p",    decode_bltz,     ISA_from_2},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE}
};

const MIPS_Instr opcodes[64] = {
    {NULL,      NULL,      decode_special,  ISA_from_1},
    {NULL,      NULL,      decode_regimm,   ISA_from_1},
    {"j",       "a",       decode_j,        ISA_from_1},
    {"jal",     "a",       decode_j,        ISA_from_1},
    {"beq",     "s, t, p", decode_beq,      ISA_from_1}, // 000100
    {"bne",     "s, t, p", decode_beq,      ISA_from_1},
    {"blez",    "s, p",    decode_blez,     ISA_from_1},
    {"bgtz",    "s, p",    decode_blez,     ISA_from_1},
    {"addi",    "t, s, i", decode_addi,     ISA_from_1}, // 001000
    {"addiu",   "t, s, i", decode_addi,     ISA_from_1},
    {"slti",    "t, s, i", decode_slti,     ISA_from_1},
    {"sltiu",   "t, s, i", decode_sltiu,    ISA_from_1},
    {"andi",    "t, s, i", decode_andi,     ISA_from_1},
    {"ori",     "t, s, i", decode_ori,      ISA_from_1},
    {"xori",    "t, s, i", decode_xori,     ISA_from_1},
    {"lui",     "t, i",    decode_lui,      ISA_from_1},
    {NULL,      NULL,      decode_cp0,      ISA_from_1}, // 010000
    {NULL,      NULL,      decode_cp1,      ISA_from_1},
    {NULL,      NULL,      decode_cp2,      ISA_from_1},
    {NULL,      NULL,      decode_cp3,      ISA_from_1},
    {"beql",    "s, t, p", decode_beq,      ISA_from_2},
    {"bnel",    "s, t, p", decode_beq,      ISA_from_2},
    {"blezl",   "s, p",    decode_blez,     ISA_from_2},
    {"bgtzl",   "s, p",    decode_blez,     ISA_from_2},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"ldl",     "t, i(s)", decode_ldl,      ISA_from_3},
    {"ldr",     "t, i(s)", decode_ldr,      ISA_from_3},
    {NULL,      NULL,      decode_special2, ISA_from_3},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"lb",      "t, i(s)", decode_lb,       ISA_from_1}, // 100000
    {"lh",      "t, i(s)", decode_lh,       ISA_from_1},
    {"lwl",     "t, i(s)", decode_lwl,      ISA_from_1},
    {"lw",      "t, i(s)", decode_lw,       ISA_from_1},
    {"lbu",     "t, i(s)", decode_lbu,      ISA_from_1},
    {"lhu",     "t, i(s)", decode_lhu,      ISA_from_1},
    {"lwr",     "t, i(s)", decode_lwr,      ISA_from_1},
    {"lwu",     "t, i(s)", decode_lwu,      ISA_from_1},
    {"sb",      "t, i(s)", decode_sb,       ISA_from_1},
    {"sh",      "t, i(s)", decode_sh,       ISA_from_1},
    {"swl",     "t, i(s)", decode_swl,      ISA_from_1},
    {"sw",      "t, i(s)", decode_sw,       ISA_from_1},
    {"sdl",     "t, i(s)", decode_sdl,      ISA_from_3},
    {"sdr",     "t, i(s)", decode_sdr,      ISA_from_3},
    {"swr",     "t, i(s)", decode_swr,      ISA_from_1},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"ll",      "t, i(s)", decode_unknown,  ISA_from_2}, // 110000
    {"lwc1",    "t, i(s)", decode_unknown,  ISA_from_1},
    {"lwc2",    "t, i(s)", decode_unknown,  ISA_from_1},
    {"pref",    "h, i(s)", decode_unknown,  ISA_from_4 | ISA_32},
    {"lld",     "t, i(s)", decode_unknown,  ISA_from_3},
    {"ldc1",    "t, i(s)", decode_unknown,  ISA_from_2},
    {"ldc2",    "t, i(s)", decode_unknown,  ISA_from_32},
    {"ld",      "t, i(s)", decode_ld,       ISA_from_3},
    {"sc",      "t, i(s)", decode_unknown,  ISA_from_2}, // 111000
    {"swc1",    "t, i(s)", decode_unknown,  ISA_from_1}, 
    {"swc2",    "t, i(s)", decode_unknown,  ISA_from_1},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"scd",     "t, i(s)", decode_unknown,  ISA_from_3},
    {"sdc1",    "t, i(s)", decode_unknown,  ISA_from_2},
    {"sdc2",    "t, i(s)", decode_unknown,  ISA_from_32},
    {"sdi",     "t, i(s)", decode_sdi,      ISA_from_3}
};

#define DISASM_BUFFER_SIZE 128

int print_reg(char *s, int i, int maxsz, int reg)
{
    const char *rn = mips_reg_name(reg);
    
    while ( (i < maxsz) && (*rn != '\0') )
        s[i++] = *rn++;
    
    return i;
}

const char* mips_disasm(const char *args, MIPS_Addr pc, uint32_t ir)
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
                j = print_reg(disasm_buffer, j, DISASM_BUFFER_SIZE, rs);
                break;
                
            case 't' :
                j = print_reg(disasm_buffer, j, DISASM_BUFFER_SIZE, rt);
                break;
                
            case 'd' :
                j = print_reg(disasm_buffer, j, DISASM_BUFFER_SIZE, rd);
                break;
                
            case '<' :
                if ( sh >= 10 ) disasm_buffer[j++] = '0' + sh / 10;
                disasm_buffer[j++] = '0' + sh % 10;
                break;
                
            case 'i' :
                sprintf(disasm_buffer + j, "0x%04x", imm);
                j += 6;
                break;
                
            case 'p' :
                sprintf(disasm_buffer + j, "0x%08x", ((int16_t)imm << 2) + (uint32_t)pc);
                j += 10;
                break;
                
            case 'a' :
                sprintf(disasm_buffer + j, "0x%08x", ((pc + 4) & (-1 << 28)) | (addr << 2));
                j += 10;
                break;
                
            default:
                disasm_buffer[j++] = c;
                break;
        };
    }
    
    return disasm_buffer;
}

int mips_universal_decode(MIPS *m)
{
    MIPS_Native pc = m->hw.get_pc(&m->hw);
    
    if ( (pc & 0xFFFFF003) == 0xBFC00000 )
    {
        // monitor (I/O) entry points
        
        // execute requested operation (out of emulation)
        mips_monitor(m, (pc >> 2) & 0x1FF);
        
        // go back to emulation (j r31)
        m->hw.set_pc(&m->hw, m->hw.get_reg(&m->hw, 31));
        
        return MIPS_OK;
    }
    
    int stat;
    uint32_t ir = mips_read_w(m, pc, &stat);
    
    if ( stat != MEM_OK )
    {
        mipsim_printf(IO_TRACE, "SEGFAULT\n");
        mips_stop(m, MIPS_ERROR);
        return MIPS_ERROR;
    }
    
    mipsim_printf(IO_TRACE, "%08x:\t%08x\t", (uint32_t)pc, ir);
    
    pc += 4;
    m->hw.set_pc(&m->hw, pc);
    
    int ret = MIPS_OK;
    const uint32_t op = (ir & OPCODE_MASK) >> OPCODE_SHIFT;
    
    MIPS_Instr i = opcodes[op];
    
    if ( i.decode != NULL )
    {
        if ( i.mnemonic != NULL )
            mipsim_printf(IO_TRACE, "%s %s", i.mnemonic, mips_disasm(i.args, pc, ir));
        
        if ( i.isa & (1 << m->architecture) )
        {
            if ( i.mnemonic != NULL ) mipsim_printf(IO_TRACE, "\n");
            
            ret = i.decode(m, ir);
        } else {
            mipsim_printf(IO_TRACE, "\t\t[Unsupported in %s]\n", mips_isa_name(m->architecture));
            
            // error handling...
        }
    } else {
        // error handling...
        mipsim_printf(IO_TRACE, "???\n");
    }
    
    return ret;
}

int decode_unknown(MIPS *m, uint32_t ir)
{
    //printf("MIPS: Unrecognized instruction.\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int delay_slot(MIPS *m)
{
    const MIPS_Native pc = m->hw.get_pc(&m->hw) + 4;
    
    // execute delay slot
    int ret = mips_universal_decode(m);
    
    if ( m->hw.get_pc(&m->hw) != pc )
    {
        mipsim_printf(IO_WARNING, "Warning : delay slot must not alter the PC [0x%08lx]\n", pc - 4);
        
        mips_stop(m, MIPS_UNPREDICTABLE);
        return -1;
    }
    
    return ret;
}

int decode_special (MIPS *m, uint32_t ir)
{
    MIPS_Instr i = Rinstr[(ir & FN_MASK)];
    
    if ( i.decode )
    {
        if ( !ir )
            mipsim_printf(IO_TRACE, "nop");
        else if ( i.mnemonic != NULL )
            mipsim_printf(IO_TRACE, "%s %s", i.mnemonic, mips_disasm(i.args, m->hw.get_pc(&m->hw), ir));
        
        if ( i.isa & (1 << m->architecture) )
        {
            if ( i.mnemonic != NULL ) mipsim_printf(IO_TRACE, "\n");
            return i.decode(m, ir);
        } else {
            mipsim_printf(IO_TRACE, "\t\t[Unsupported in %s]\n", mips_isa_name(m->architecture));
            return 1;
        }
    } else {
        mipsim_printf(IO_TRACE, "???\n");
    }
    
    return MIPS_OK;
}

int decode_special2(MIPS *m, uint32_t ir)
{
    MIPS_Instr i = Rinstr2[(ir & FN_MASK)];
    
    if ( i.decode )
    {
        if ( i.mnemonic != NULL )
            mipsim_printf(IO_TRACE, "%s %s", i.mnemonic, mips_disasm(i.args, m->hw.get_pc(&m->hw), ir));
        
        if ( i.isa & (1 << m->architecture) )
        {
            if ( i.mnemonic != NULL ) mipsim_printf(IO_TRACE, "\n");
            return i.decode(m, ir);
        } else {
            mipsim_printf(IO_TRACE, "\t\t[Unsupported in %s]\n", mips_isa_name(m->architecture));
            return 1;
        }
    } else {
        mipsim_printf(IO_TRACE, "???\n");
    }
    
    return MIPS_OK;
}

int decode_j       (MIPS *m, uint32_t ir)
{
    // delay slot
    int ret = delay_slot(m);
    MIPS_Native pc = m->hw.get_pc(&m->hw);
    
    if ( ret == MIPS_OK )
    {
        // link
        if ( ir & 0x04000000 )
            m->hw.set_reg(&m->hw, 31, pc);
        
        // jump
        m->hw.set_pc(&m->hw, (pc & (-1 << 28)) | ((ir & ADDR_MASK) << 2));
    } else {
        mipsim_printf(IO_WARNING, "problem in delay slot @ 0x%08lx : %d\n", pc - 4, ret);
    }
    
    return ret;
}

int decode_regimm  (MIPS *m, uint32_t ir)
{
    MIPS_Instr i = Iinstr[(ir & RT_MASK) >> RT_SHIFT];
    
    if ( i.decode )
    {
        if ( i.mnemonic != NULL )
            mipsim_printf(IO_TRACE, "%s %s", i.mnemonic, mips_disasm(i.args, m->hw.get_pc(&m->hw), ir));
        
        if ( i.isa & (1 << m->architecture) )
        {
            if ( i.mnemonic != NULL ) mipsim_printf(IO_TRACE, "\n");
            return i.decode(m, ir);
        } else {
            mipsim_printf(IO_TRACE, "\t\t[Unsupported in %s]\n", mips_isa_name(m->architecture));
            return 1;
        }
    } else {
        mipsim_printf(IO_TRACE, "???\n");
    }
    
    return MIPS_OK;
}

int decode_beq     (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    int cond = (rs == rt ? 0x04000000 : 0) ^ (ir & 0x04000000);
    
    // delay slot
    if ( !(ir & 0x40000000) || cond )
    {
        int ret = delay_slot(m);
        
        if ( ret != MIPS_OK )
        {
            mipsim_printf(IO_WARNING, "problem in delay slot @ 0x%08lx : %d\n", m->hw.get_pc(&m->hw) - 4, ret);
            return ret;
        }
    } else {
         m->hw.set_pc(&m->hw, m->hw.get_pc(&m->hw) + 4);
    }
    
    // jump if condition verified
    if ( cond )
        m->hw.set_pc(&m->hw, m->hw.get_pc(&m->hw) + (((int16_t)(ir & IMM_MASK)) << 2) - 4);
    
    return MIPS_OK;
}

int decode_blez     (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    
    int cond = (rs <= 0 ? 0x04000000 : 0) ^ (ir & 0x04000000);
    
    // delay slot
    if ( !(ir & 0x40000000) || cond )
    {
        int ret = delay_slot(m);
        
        if ( ret != MIPS_OK )
        {
            mipsim_printf(IO_WARNING, "problem in delay slot @ 0x%08lx : %d\n", m->hw.get_pc(&m->hw) - 4, ret);
            return ret;
        }
    } else {
         m->hw.set_pc(&m->hw, m->hw.get_pc(&m->hw) + 4);
    }
    
    // jump if condition verified
    if ( cond )
        m->hw.set_pc(&m->hw, m->hw.get_pc(&m->hw) + (((int16_t)(ir & IMM_MASK)) << 2) - 4);
    
    return MIPS_OK;
}

int decode_bltz     (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    
    int cond = (rs < 0 ? 0x00010000 : 0) ^ (ir & 0x00010000);
    
    // delay slot
    if ( !(ir & 0x00020000) || cond )
    {
        int ret = delay_slot(m);
        
        if ( ret != MIPS_OK )
        {
            mipsim_printf(IO_WARNING, "problem in delay slot @ 0x%08lx : %d\n", m->hw.get_pc(&m->hw) - 4, ret);
            return ret;
        }
    } else {
         m->hw.set_pc(&m->hw, m->hw.get_pc(&m->hw) + 4);
    }
    
    // jump if condition verified
    if ( cond )
    {
        MIPS_Native pc = m->hw.get_pc(&m->hw);
        
        // link
        if ( ir & 0x00100000 )
            m->hw.set_reg(&m->hw, 31, pc);
        
        // jump
        m->hw.set_pc(&m->hw, pc + (((int16_t)(ir & IMM_MASK)) << 2) - 4);
    }
    
    return MIPS_OK;
}

int decode_shift   (MIPS *m, uint32_t ir)
{
    uint32_t rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    int sa = ir & 4 ? (m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) & 0x1F) : (ir & SH_MASK) >> SH_SHIFT;
    
    if ( ir & 2 )
        if ( ir & 1 )
            rt = ((int32_t)rt) >> sa;
        else
            rt >>= sa;
    else
        rt <<= sa;
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, rt);
    
    return MIPS_OK;
}

int decode_jr      (MIPS *m, uint32_t ir)
{
    // delay slot
    int ret = delay_slot(m);
    
    if ( ret == MIPS_OK )
    {
        // link
        if ( ir & 0x00000001 )
            m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, m->hw.get_pc(&m->hw));
        
        // jump
        m->hw.set_pc(&m->hw, m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) & (-1 << 2));
    } else {
        mipsim_printf(IO_WARNING, "problem in delay slot @ 0x%08lx : %d\n", m->hw.get_pc(&m->hw) - 4, ret);
    }
    
    return ret;
}

int decode_movhilo (MIPS *m, uint32_t ir)
{
    if ( ir & 0x00000001 )
    {
        MIPS_Native v = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
        
        if ( ir & 2 )
            m->hw.set_lo(&m->hw, v);
        else
            m->hw.set_hi(&m->hw, v);
        
    } else {
        m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, ir & 0x00000002 ? m->hw.get_lo(&m->hw) : m->hw.get_hi(&m->hw));
    }
    
    return MIPS_OK;
}

int decode_add     (MIPS *m, uint32_t ir)
{
    int32_t rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    int32_t rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    int32_t sum = rs + rt;
    
    if ( !(ir & 0x00000001) && (sum < rs || sum < rt) )
    {
        // integer overflow exception...
        mipsim_printf(IO_WARNING, "overflow...\n");
        return MIPS_EXCEPTION;
    }
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, sum);
    
    return MIPS_OK;
}

int decode_sub     (MIPS *m, uint32_t ir)
{
    int32_t rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    int32_t rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    int32_t diff = rs - rt;
    
    if ( !(ir & 0x00000001) && (diff > rs) )
    {
        // integer overflow exception...
        mipsim_printf(IO_WARNING, "overflow...\n");
        return MIPS_EXCEPTION;
    }
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, diff);
    
    return MIPS_OK;
}

int decode_mult    (MIPS *m, uint32_t ir)
{
    int32_t rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    int32_t rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    int64_t res = rs * rt;
    
    m->hw.set_hi(&m->hw, res >> 32);
    m->hw.set_lo(&m->hw, res & 0x00000000FFFFFFFFL);
    
    return MIPS_OK;
}

int decode_multu   (MIPS *m, uint32_t ir)
{
    uint32_t rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    uint32_t rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    uint64_t res = rs * rt;
    
    m->hw.set_hi(&m->hw, res >> 32);
    m->hw.set_lo(&m->hw, res & 0x00000000FFFFFFFFL);
    
    return MIPS_OK;
}

int decode_div     (MIPS *m, uint32_t ir)
{
    int32_t rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    int32_t rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_lo(&m->hw, rs / rt);
    m->hw.set_hi(&m->hw, rs % rt);
    
    return MIPS_OK;
}

int decode_divu    (MIPS *m, uint32_t ir)
{
    uint32_t rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    uint32_t rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_lo(&m->hw, rs / rt);
    m->hw.set_hi(&m->hw, rs % rt);
    
    return MIPS_OK;
}

int decode_and     (MIPS *m, uint32_t ir)
{
    MIPS_NativeU rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_NativeU rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, rs & rt);
    
    return MIPS_OK;
}

int decode_or      (MIPS *m, uint32_t ir)
{
    MIPS_NativeU rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_NativeU rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, rs | rt);
    
    return MIPS_OK;
}

int decode_xor     (MIPS *m, uint32_t ir)
{
    MIPS_NativeU rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_NativeU rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, rs ^ rt);
    
    return MIPS_OK;
}

int decode_nor     (MIPS *m, uint32_t ir)
{
    MIPS_NativeU rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_NativeU rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, ~(rs | rt));

    return MIPS_OK;
}

int decode_slt     (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, (rs < rt) ? 1 : 0);
    
    return MIPS_OK;
}

int decode_sltu    (MIPS *m, uint32_t ir)
{
    MIPS_NativeU rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_NativeU rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, (rs < rt) ? 1 : 0);
    
    return MIPS_OK;
}

int decode_movcond (MIPS *m, uint32_t ir)
{
    MIPS_NativeU rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    if ( (rt && !(ir & 1)) || (!rt && (ir & 1)) )
        m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT));
    
    return MIPS_OK;
}

int decode_addi    (MIPS *m, uint32_t ir)
{
    int32_t rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    int32_t rt = (int16_t)(ir & IMM_MASK);
    int32_t sum = rs + rt;
    
    if ( !(ir & 0x04000000) && (sum < rs || sum < rt) )
    {
        // integer overflow exception...
        mipsim_printf(IO_WARNING, "overflow...\n");
        return MIPS_EXCEPTION;
    }
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, sum);
    
    return MIPS_OK;
}

int decode_slti    (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = (int16_t)(ir & IMM_MASK);
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, (rs < rt) ? 1 : 0);
    
    return MIPS_OK;
}

int decode_sltiu   (MIPS *m, uint32_t ir)
{
    MIPS_NativeU rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_NativeU rt = (int16_t)(ir & IMM_MASK);
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, (rs < rt) ? 1 : 0);
    
    return MIPS_OK;
}

int decode_andi    (MIPS *m, uint32_t ir)
{
    MIPS_NativeU rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_NativeU rt = (ir & IMM_MASK);
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, rs & rt);
    
    return MIPS_OK;
}

int decode_ori     (MIPS *m, uint32_t ir)
{
    MIPS_NativeU rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_NativeU rt = (ir & IMM_MASK);
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, rs | rt);
    
    return MIPS_OK;
}

int decode_xori    (MIPS *m, uint32_t ir)
{
    MIPS_NativeU rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_NativeU rt = (ir & IMM_MASK);
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, rs ^ rt);
    
    return MIPS_OK;
}

int decode_lui     (MIPS *m, uint32_t ir)
{
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, ((int16_t)(ir & IMM_MASK)) << 16);
    return MIPS_OK;
}

int decode_lb      (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, (int8_t)mips_read_b(m, a, &stat));
    
    return MIPS_OK;
}

int decode_lbu     (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, mips_read_b(m, a, &stat));
    
    return MIPS_OK;
}

int decode_lh      (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, (int16_t)mips_read_h(m, a, &stat));
    
    return MIPS_OK;
}

int decode_lhu     (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, mips_read_h(m, a, &stat));
    
    return MIPS_OK;
}

int decode_lw      (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, (int32_t)mips_read_w(m, a, &stat));
    
    return MIPS_OK;
}

int decode_lwu     (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, mips_read_w(m, a, &stat));
    
    return MIPS_OK;
}

int decode_lwl     (MIPS *m, uint32_t ir)
{
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_lwr     (MIPS *m, uint32_t ir)
{
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_ld      (MIPS *m, uint32_t ir)
{
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_ldl     (MIPS *m, uint32_t ir)
{
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_ldr     (MIPS *m, uint32_t ir)
{
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}


int decode_sb      (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    mips_write_b(m, a, m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT) & 0xFF, &stat);
    
    return MIPS_OK;
}

int decode_sh      (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    mips_write_h(m, a, m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT) & 0xFFFF, &stat);
    
    return MIPS_OK;
}

int decode_sw      (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    mips_write_w(m, a, m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT) & 0xFFFFFFFF, &stat);
    
    return MIPS_OK;
}

int decode_swl     (MIPS *m, uint32_t ir)
{
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_OK;
}

int decode_swr     (MIPS *m, uint32_t ir)
{
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_OK;
}

int decode_sdi     (MIPS *m, uint32_t ir)
{
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_sdl     (MIPS *m, uint32_t ir)
{
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_sdr     (MIPS *m, uint32_t ir)
{
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_break   (MIPS *m, uint32_t ir)
{
    mips_stop(m, MIPS_BREAK);
    return MIPS_BREAK;
}

int decode_trap    (MIPS *m, uint32_t ir)
{
    mips_stop(m, MIPS_TRAP);
    return MIPS_TRAP;
}

int decode_cp0     (MIPS *m, uint32_t ir)
{
    mipsim_printf(IO_TRACE, "cop 0\n");
    return MIPS_OK;
}

int decode_cp1     (MIPS *m, uint32_t ir)
{
    mipsim_printf(IO_TRACE, "cop 1\n");
    return MIPS_OK;
}

int decode_cp2     (MIPS *m, uint32_t ir)
{
    mipsim_printf(IO_TRACE, "cop 2\n");
    return MIPS_OK;
}

int decode_cp3     (MIPS *m, uint32_t ir)
{
    mipsim_printf(IO_TRACE, "cop 3\n");
    return MIPS_OK;
}
