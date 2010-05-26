/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "decode.h"

#include "io.h"
#include "util.h"
#include "monitor.h"

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

int decode_syscall (MIPS *m, uint32_t ir);
int decode_break   (MIPS *m, uint32_t ir);
int decode_trap    (MIPS *m, uint32_t ir);

int decode_lwc     (MIPS *m, uint32_t ir);
int decode_swc     (MIPS *m, uint32_t ir);

int decode_mfc     (MIPS *m, uint32_t ir);
int decode_mtc     (MIPS *m, uint32_t ir);
int decode_cfc     (MIPS *m, uint32_t ir);
int decode_ctc     (MIPS *m, uint32_t ir);
int decode_bc1     (MIPS *m, uint32_t ir);
int decode_fpu     (MIPS *m, uint32_t ir);

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
    {"syscall", "",        decode_syscall,  ISA_from_1},
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

const MIPS_Instr cp0[32] = {
    {"mfc0",    "t, S",    decode_mfc,      ISA_from_1}, // 000000
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"mtc0",    "t, S",    decode_mtc,      ISA_from_1},
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
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE}
};

const MIPS_Instr cp1[32] = {
    {"mfc1",    "t, S",    decode_mfc,      ISA_from_1}, // 000000
    {"dmfc1",   "t, S",    NULL,            ISA_from_3},
    {"cfc1",    "t, S",    decode_cfc,      ISA_from_1},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"mtc1",    "t, S",    decode_mtc,      ISA_from_1},
    {"dmtc1",   "t, S",    NULL,            ISA_from_3},
    {"ctc1",    "t, S",    decode_ctc,      ISA_from_1},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      decode_bc1,      ISA_from_1},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      decode_fpu,      ISA_from_1}, // 10000 : fmt = S (float 32)
    {NULL,      NULL,      decode_fpu,      ISA_from_1}, // 10001 : fmt = D (float 64)
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {NULL,      NULL,      decode_fpu,      ISA_from_1}, // 10100 : fmt = W (fixed 32)
    {NULL,      NULL,      decode_fpu,      ISA_from_1}, // 10101 : fmt = L (fixed 64)
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

const MIPS_Instr cp2[32] = {
    {"mfc2",    "t, S",    decode_mfc,      ISA_from_1}, // 000000
    {"dmfc2",   "t, S",    NULL,            ISA_from_3},
    {"cfc2",    "t, S",    decode_cfc,      ISA_from_1},
    {NULL,      NULL,      NULL,            ISA_NONE},
    {"mtc2",    "t, S",    decode_mtc,      ISA_from_1},
    {"dmtc2",   "t, S",    NULL,            ISA_from_3},
    {"ctc2",    "t, S",    decode_ctc,      ISA_from_1},
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
    {"lwc1",    "t, i(s)", decode_lwc,      ISA_from_1},
    {"lwc2",    "t, i(s)", decode_lwc,      ISA_from_1},
    {"pref",    "h, i(s)", decode_unknown,  ISA_from_4 | ISA_32},
    {"lld",     "t, i(s)", decode_unknown,  ISA_from_3},
    {"ldc1",    "t, i(s)", decode_unknown,  ISA_from_2},
    {"ldc2",    "t, i(s)", decode_unknown,  ISA_from_32},
    {"ld",      "t, i(s)", decode_ld,       ISA_from_3},
    {"sc",      "t, i(s)", decode_unknown,  ISA_from_2}, // 111000
    {"swc1",    "t, i(s)", decode_swc,      ISA_from_1}, 
    {"swc2",    "t, i(s)", decode_swc,      ISA_from_1},
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
    const uint8_t fd = (ir & FD_MASK) >> FD_SHIFT;
    const uint8_t fs = (ir & FS_MASK) >> FS_SHIFT;
    
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
                
            case 'S' :
                j = print_reg(disasm_buffer, j, DISASM_BUFFER_SIZE, fs | CP1);
                break;
                
            case 'T' :
                j = print_reg(disasm_buffer, j, DISASM_BUFFER_SIZE, rt | CP1);
                break;
                
            case 'D' :
                j = print_reg(disasm_buffer, j, DISASM_BUFFER_SIZE, fd | CP1);
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

/*!
    \brief Disassemble four bytes of memory
    \param m simulated machine
    \param a address of memory to disassemble
    \param sym_name symbol naming callback
    \param sym_data symbol naming data (to be fed to callback)
    \return Mnemonic
    
    A null pointer will be returned if the memory is unreachable.
    
    The caller is responsible for freeing the returned string (if non-NULL)
*/
const char* mips_disassemble(MIPS *m, MIPS_Addr a, symbol_name sym_name, void *sym_data)
{
    int stat;
    uint32_t w = m->mem.read_w(&m->mem, a, &stat);
    
    if ( stat == MEM_UNMAPPED )
        return NULL;
    
    char *s = NULL;
    int cp = 0;
    
    if ( stat & MEM_NOEXEC )
    {
        s = malloc(17 * sizeof(char));
        s[0] = '.';
        s[1] = 'w';
        s[2] = 'o';
        s[3] = 'r';
        s[4] = 'd';
        s[5] = ' ';
        s[6] = '0';
        s[7] = 'x';
        
        cat_num(w, 16, s + 8, 8);
        
        s[16] = '\0';
    } else {
        MIPS_Instr i = opcodes[(w & OPCODE_MASK) >> OPCODE_SHIFT];
        
        if ( i.mnemonic == NULL && i.decode != NULL )
        {
            if ( i.decode == decode_special )
                i = Rinstr[w & FN_MASK];
            else if ( i.decode == decode_special2 )
                i = Rinstr2[w & FN_MASK];
            else if ( i.decode == decode_regimm )
                i = Iinstr[(w & RT_MASK) >> RT_SHIFT];
            else if ( i.decode == decode_cp0 ) {
                cp = CP0;
                i = cp0[(w & FMT_MASK) >> FMT_SHIFT];
            } else if ( i.decode == decode_cp1 ) {
                cp = CP1;
                i = cp1[(w & FMT_MASK) >> FMT_SHIFT];
                // TODO : extra step for actual functions...
            } else if ( i.decode == decode_cp2 ) {
                cp = CP2;
                i = cp2[(w & FMT_MASK) >> FMT_SHIFT];
            }
        }
        
        if ( i.mnemonic != NULL && i.args != NULL )
        {
            const uint8_t sh = (w & SH_MASK) >> SH_SHIFT;
            const uint8_t rd = (w & RD_MASK) >> RD_SHIFT;
            const uint8_t rt = (w & RT_MASK) >> RT_SHIFT;
            const uint8_t rs = (w & RS_MASK) >> RS_SHIFT;
            const uint16_t imm = (w & IMM_MASK);
            const uint32_t addr = (w & ADDR_MASK);
            const uint8_t fd = (w & FD_MASK) >> FD_SHIFT;
            const uint8_t fs = (w & FS_MASK) >> FS_SHIFT;
    
            //i.mnemonic, mips_disasm(i.args, pc, ir)
            int sz = strlen(i.mnemonic), alloc = sz + 8 * strlen(i.args);
            s = malloc(alloc * sizeof(char));
            
            strcpy(s, i.mnemonic);
            s[sz] = '\t';
            s[++sz] = '\0';
            
            const char *args = i.args;
            
            while ( *args )
            {
                const char *cn = NULL;
                char *dn = NULL;
                
                if ( *args == 's' )
                {
                    cn = mips_reg_name(rs);
                } else if ( *args == 't' ) {
                    cn = mips_reg_name(rt);
                } else if ( *args == 'd' ) {
                    cn = mips_reg_name(rd);
                } else if ( *args == 'S' ) {
                    cn = mips_reg_name(fs | cp);
                } else if ( *args == 'T' ) {
                    cn = mips_reg_name(rt | cp);
                } else if ( *args == 'D' ) {
                    cn = mips_reg_name(fd | cp);
                } else if ( *args == '<' ) {
                    dn = num_to_str(sh, 10);
                } else if ( *args == 'i' ) {
                    dn = num_to_str(imm, 16 | C_PREFIX);
                } else if ( *args == 'p' ) {
                    MIPS_Addr tg = ((int16_t)imm << 2) + (uint32_t)a;
                    
                    if ( sym_name != NULL )
                        cn = sym_name(a, tg, sym_data);
                    
                    if ( cn == NULL )
                        dn = num_to_str(tg, 16 | C_PREFIX);
                } else if ( *args == 'a' ) {
                    MIPS_Addr tg = ((a + 4) & (-1 << 28)) | (addr << 2);
                    
                    if ( sym_name != NULL )
                        cn = sym_name(a, tg, sym_data);
                    
                    if ( cn == NULL )
                        dn = num_to_str(tg, 16 | C_PREFIX);
                } else {
                    if ( sz + 1 == alloc )
                    {
                        alloc *= 2;
                        s = realloc(s, alloc);
                    }
                    s[sz++] = *args;
                    s[sz] = '\0';
                }
                
                if ( dn != NULL )
                {
                    sz += strlen(dn);
                    if ( sz >= alloc )
                    {
                        alloc *= 2;
                        s = realloc(s, alloc);
                    }
                    strcat(s, dn);
                    free(dn);
                } else if ( cn != NULL ) {
                    sz += strlen(cn);
                    if ( sz >= alloc )
                    {
                        alloc *= 2;
                        s = realloc(s, alloc);
                    }
                    strcat(s, cn);
                }
                
                ++args;
            }
        }
    }
    
    return s;
}

/*!
    \internal
    \brief core of instr decode/execution
    \param m simulated machine
    \return stop reason
    
    Fetch the instruction at PC, increase PC and execute the instruction
*/
int mips_universal_decode(MIPS *m)
{
    MIPS_Native pc = m->hw.get_pc(&m->hw);
    
    if ( pc & 3 )
    {
        mipsim_printf(IO_WARNING, "Segfault : Unaligned PC\n");
        mips_stop(m, MIPS_EXCEPTION);
        return MIPS_EXCEPTION;
    }
    
    // TODO Check for MEM_READ and MEM_EXEC breakpoints here
    
    int stat;
    uint32_t ir = m->hw.fetch(&m->hw, &stat);
    
    if ( stat == MEM_FWMON )
        return MIPS_OK;
    
    if ( stat & MEM_UNMAPPED )
    {
        mipsim_printf(IO_TRACE, "Segfault : PC out of mapped memory\n");
        mips_stop(m, MIPS_ERROR);
        return MIPS_ERROR;
    } else  if ( stat & MEM_NOEXEC ) {
        mipsim_printf(IO_TRACE, "Segfault : PC out of executable memory\n");
        mips_stop(m, MIPS_ERROR);
        return MIPS_ERROR;
    }
    
    // TODO check for OPCODE breakpoints here
    
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
    (void)m; (void)ir;
    
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
    int32_t rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    int sa = ir & 4 ? (m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) & 0x1F) : (ir & SH_MASK) >> SH_SHIFT;
    
    if ( ir & 2 )
        if ( ir & 1 )
            rt >>= sa;
        else
            rt = s32_to_u32(rt) >> sa;
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
    int32_t rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    int32_t rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    uint64_t res = s32_to_u32(rs) * s32_to_u32(rt);
    
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
    int32_t rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    int32_t rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_lo(&m->hw, s32_to_u32(rs) / s32_to_u32(rt));
    m->hw.set_hi(&m->hw, s32_to_u32(rs) % s32_to_u32(rt));
    
    return MIPS_OK;
}

int decode_and     (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, rs & rt);
    
    return MIPS_OK;
}

int decode_or      (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, rs | rt);
    
    return MIPS_OK;
}

int decode_xor     (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, rs ^ rt);
    
    return MIPS_OK;
}

int decode_nor     (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
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
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
    m->hw.set_reg(&m->hw, (ir & RD_MASK) >> RD_SHIFT, (s32_to_u32(rs) < s32_to_u32(rt)) ? 1 : 0);
    
    return MIPS_OK;
}

int decode_movcond (MIPS *m, uint32_t ir)
{
    MIPS_Native rt = m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT);
    
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
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = (int16_t)(ir & IMM_MASK);
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, (s32_to_u32(rs) < s32_to_u32(rt)) ? 1 : 0);
    
    return MIPS_OK;
}

int decode_andi    (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = (ir & IMM_MASK);
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, rs & rt);
    
    return MIPS_OK;
}

int decode_ori     (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = (ir & IMM_MASK);
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, rs | rt);
    
    return MIPS_OK;
}

int decode_xori    (MIPS *m, uint32_t ir)
{
    MIPS_Native rs = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT);
    MIPS_Native rt = (ir & IMM_MASK);
    
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
    
    if ( a & 1 )
    {
        mips_stop(m, MIPS_EXCEPTION);
        return MIPS_EXCEPTION;
    }
    
    int stat;
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, (int16_t)mips_read_h(m, a, &stat));
    
    return MIPS_OK;
}

int decode_lhu     (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    if ( a & 1 )
    {
        mips_stop(m, MIPS_EXCEPTION);
        return MIPS_EXCEPTION;
    }
    
    int stat;
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, mips_read_h(m, a, &stat));
    
    return MIPS_OK;
}

int decode_lw      (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    if ( a & 3 )
    {
        mips_stop(m, MIPS_EXCEPTION);
        return MIPS_EXCEPTION;
    }
    
    int stat;
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, (int32_t)mips_read_w(m, a, &stat));
    
    return MIPS_OK;
}

int decode_lwu     (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    if ( a & 3 )
    {
        mips_stop(m, MIPS_EXCEPTION);
        return MIPS_EXCEPTION;
    }
    
    int stat;
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, mips_read_w(m, a, &stat));
    
    return MIPS_OK;
}

int decode_lwl     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_lwr     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_ld      (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_ldl     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_ldr     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
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
    
    if ( a & 1 )
    {
        mips_stop(m, MIPS_EXCEPTION);
        return MIPS_EXCEPTION;
    }
    
    int stat;
    mips_write_h(m, a, m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT) & 0xFFFF, &stat);
    
    return MIPS_OK;
}

int decode_sw      (MIPS *m, uint32_t ir)
{
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    if ( a & 3 )
    {
        mips_stop(m, MIPS_EXCEPTION);
        return MIPS_EXCEPTION;
    }
    
    int stat;
    mips_write_w(m, a, m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT) & 0xFFFFFFFF, &stat);
    
    return MIPS_OK;
}

int decode_swl     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_OK;
}

int decode_swr     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_OK;
}

int decode_sdi     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_sdl     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_sdr     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mipsim_printf(IO_WARNING, "sorry : 64bit CPU unsupported...\n");
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_syscall (MIPS *m, uint32_t ir)
{
    return mips_syscall(m, m->hw.get_reg(&m->hw, V0));
}

int decode_break   (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mips_stop(m, MIPS_BREAK);
    return MIPS_BREAK;
}

int decode_trap    (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mips_stop(m, MIPS_TRAP);
    return MIPS_TRAP;
}

int decode_cp0     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    /*
    MIPS_Instr i = cp0[(ir & FMT_MASK) >> FMT_SHIFT];
    
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
        mipsim_printf(IO_TRACE, "cp0 ???\n");
    }
    */
    
    printf("cp0\n");
    
    return MIPS_OK;
}

int decode_cp1     (MIPS *m, uint32_t ir)
{
    MIPS_Instr i = cp1[(ir & FMT_MASK) >> FMT_SHIFT];
    
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
        mipsim_printf(IO_TRACE, "cp1 ???\n");
    }
    
    return MIPS_OK;
}

int decode_cp2     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mipsim_printf(IO_TRACE, "cop 2\n");
    return MIPS_OK;
}

int decode_cp3     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mipsim_printf(IO_TRACE, "cop 3\n");
    return MIPS_OK;
}

int decode_lwc      (MIPS *m, uint32_t ir)
{
    MIPS_Coprocessor *cp = &m->cp[(ir >> OPCODE_SHIFT) & 3];
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    cp->set_reg(cp, (ir & RT_MASK) >> RT_SHIFT, mips_read_w(m, a, &stat));
    
    return MIPS_OK;
}

int decode_swc      (MIPS *m, uint32_t ir)
{
    MIPS_Coprocessor *cp = &m->cp[(ir >> OPCODE_SHIFT) & 3];
    MIPS_Addr a = m->hw.get_reg(&m->hw, (ir & RS_MASK) >> RS_SHIFT) + (int16_t)(ir & IMM_MASK);
    
    int stat;
    mips_write_w(m, a, cp->get_reg(cp, (ir & RT_MASK) >> RT_SHIFT) & 0xFFFFFFFF, &stat);
    
    return MIPS_OK;
}

int decode_mfc     (MIPS *m, uint32_t ir)
{
    MIPS_Coprocessor *cp = &m->cp[(ir >> OPCODE_SHIFT) & 3];
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, cp->get_reg(cp, (ir & FS_MASK) >> FS_SHIFT));
    return MIPS_OK;
}

int decode_mtc     (MIPS *m, uint32_t ir)
{
    MIPS_Coprocessor *cp = &m->cp[(ir >> OPCODE_SHIFT) & 3];
    
    cp->set_reg(cp, (ir & FS_MASK) >> FS_SHIFT, m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT));
    return MIPS_OK;
}

int decode_cfc     (MIPS *m, uint32_t ir)
{
    MIPS_Coprocessor *cp = &m->cp[(ir >> OPCODE_SHIFT) & 3];
    
    m->hw.set_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT, cp->get_ctrl(cp, (ir & FS_MASK) >> FS_SHIFT));
    return MIPS_OK;
}

int decode_ctc     (MIPS *m, uint32_t ir)
{
    MIPS_Coprocessor *cp = &m->cp[(ir >> OPCODE_SHIFT) & 3];
    
    cp->set_ctrl(cp, (ir & FS_MASK) >> FS_SHIFT, m->hw.get_reg(&m->hw, (ir & RT_MASK) >> RT_SHIFT));
    return MIPS_OK;
}

int decode_bc1     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}

int decode_fpu     (MIPS *m, uint32_t ir)
{
    (void)m; (void)ir;
    mips_stop(m, MIPS_UNSUPPORTED);
    return MIPS_UNSUPPORTED;
}
