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

int decode_unknown(MIPS *m, uint32_t ir)
{
    //printf("MIPS: Unrecognized instruction.\n");
    return MIPS_OK;
}
