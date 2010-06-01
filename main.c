/****************************************************************************
**  MIPSim
**   
**  Copyright (c) 2010, Hugues Bruant
**  All rights reserved.
**  
**  This file may be used under the terms of the BSD license.
**  Refer to the accompanying COPYING file for legalese.
****************************************************************************/

#include "version.h"

/*!
    \file main.c
    \brief main
    \author Hugues Bruant
*/

#include "config.h"
#include "shell.h"

void version()
{
    printf(
        "MIPSim %s\n"
        "Copyright (c) 2010, Hugues Bruant. All rights reserved.\n"
        "This is free software; see the source for copying conditions.  There is NO\n"
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
        "\n",
        MIPSIM_VERSION_STRING
    );
}

int main(int argc, char **argv)
{
    /*
        Set default config option from CLI args
    */
    if ( mipsim_config_init(argc, argv) )
    {
        /*
            Display info and exit if requested on CLI
        */
        version();
        return 0;
    }
    
    /*
        Launch shell
    */
    mipsim_shell(argc, argv);
    
    /*
        Close any open log file
    */
    mipsim_config_fini();
    
    return 0;
}
