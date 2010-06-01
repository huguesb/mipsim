/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#ifndef _MIPS_MONITOR_H_
#define _MIPS_MONITOR_H_

/*!
    \file monitor.h
    \brief Monitor / syscall entry points for use by simulator
    \author Hugues Bruant
*/

#include "mips.h"

int mips_syscall(MIPS *m, int code);
int mips_monitor(MIPS *m, int entry);

#endif
