/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "mipsim.h"

#include "mipself.h"

#include <stdio.h>

void version()
{
    printf(
        "MIPS Simulator %s\n"
        "Copyright (c) 2010, Hugues Bruant. All rights reserved.\n"
        "This is free software; see the source for copying conditions.  There is NO\n"
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
        "\n",
        MIPSIM_VERSION_STRING
    );
}

void usage()
{
    printf(
        "mipsim [options] program\n"
        "\n"
        "Options : \n"
        "  -f freq       : set CPU frequency [default : unconstrained]\n"
        "  -d            : enable debugging\n"
        "  -c file       : read extra options from a config file\n"
        "\n"
    );
}

int main(int argc, char **argv)
{
    int arch = MIPS_I;
    
    if ( argc <= 1 )
    {
        usage();
        return 1;
    }
    
    ELF_File *f = elf_file_create();
    
    if ( f == NULL )
    {
        printf("MIPSim: Failed to allocate memory for ELF file");
        return 2;
    }
    
    elf_file_load(f, argv[1]);
    
    /*
    
    */
    MIPS *m = mips_create(arch);
    
    if ( m == NULL )
    {
        printf("MIPSim: Failed to allocate memory for MIPS machine");
        return 2;
    }
    
    /*
        Map ELF file content to emulated machine memory
    */
    mips_load_elf(m, f);
    
    /*
        Run program in simulator
    */
    int err_code = mips_exec(m, 100);
    
    /*
        always destroy emulated machine before ELF file
    */
    mips_destroy(m);
    elf_file_destroy(f);
    
    return 0;
}
