/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#ifndef _MIPS_ELF_H_
#define _MIPS_ELF_H_

/*!
    \file mipself.h
    \brief Bridge between ELF manipulation and MIPS simulator
    \author Hugues Bruant
*/

#include "mips.h"
#include "elffile.h"

int mips_load_elf(MIPS *m, ELF_File *f);

#endif
