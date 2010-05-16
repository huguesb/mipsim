/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file.
****************************************************************************/

#include "version.h"

#include "config.h"
#include "io.h"
#include "cli.h"
#include "mipself.h"

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
    
    /*
        Set default config option
    */
    MIPSIM_Config *cfg = mipsim_config();
    cfg->io_mask = 0;
    cfg->trace_log = NULL;
    cfg->debug_log = NULL;
    
    /*
        Create ELF file loading structure
    */
    ELF_File *f = elf_file_create();
    
    if ( f == NULL )
    {
        printf("MIPSim: Failed to allocate memory for ELF file\n");
        return 2;
    }
    
    /*
        Load ELF file
    */
    if ( elf_file_load(f, argv[1]) )
    {
        printf("MIPSim : Unable to load ELF file\n");
        return 2;
    }
    
    /*
        Create simulator structures
    */
    MIPS *m = mips_create(arch);
    
    if ( m == NULL )
    {
        printf("MIPSim: Failed to allocate memory for MIPS machine\n");
        return 2;
    }
    
    /*
        Map ELF file content to emulated machine memory
    */
    mips_load_elf(m, f);
    
    /*
        Run program in simulator
    */
    mipsim_cli(m);
    
    /*
        always destroy emulated machine before ELF file
    */
    mips_destroy(m);
    elf_file_destroy(f);
    
    return 0;
}
