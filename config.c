/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "config.h"

MIPSIM_Config* mipsim_config()
{
    static MIPSIM_Config cfg;
    return &cfg;
}
