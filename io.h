/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#ifndef _MIPSIM_IO_H_
#define _MIPSIM_IO_H_

enum MIPSIM_IO_Context {
    IO_NULL,
    IO_WARNING  = 1,
    IO_DEBUG    = 2,
    IO_TRACE    = 4,
    IO_MONITOR  = 8
};

int mipsim_open (int cxt, const char *path, int flags);
int mipsim_read (int cxt, int file, char *d, int len);
int mipsim_write(int cxt, int file, char *d, int len);
int mipsim_close(int cxt, int file);

char mipsim_inbyte (int cxt);
void mipsim_outbyte(int cxt, char c);

int mipsim_printf(int cxt, const char *fmt, ...);

#endif
