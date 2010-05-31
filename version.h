/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#ifndef _MIPSIM_VERSION_H_
#define _MIPSIM_VERSION_H_

#define _VERSION_NUMBER(maj, min, patch) ((maj & 0xFF) << 16) | ((min & 0xFF) << 8) | (patch & 0xFF)
#define _VERSION_STRING(maj, min, patch) ""#maj "."#min "."#patch

#define MIPSIM_VERSION_NUMBER _VERSION_NUMBER(1, 0, 0)
#define MIPSIM_VERSION_STRING _VERSION_STRING(1, 0, 0)

#define MIPSIM_VERSION_MAJOR ((MIPSIM_VERSION_NUMBER >> 16) & 0xFF)
#define MIPSIM_VERSION_MINOR ((MIPSIM_VERSION_NUMBER >>  8) & 0xFF)
#define MIPSIM_VERSION_PATCH ((MIPSIM_VERSION_NUMBER      ) & 0xFF)

#endif
