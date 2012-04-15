
#include "Common.h"

// Using regular C includes here since this file is mostly std::function_calls.
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>     

void die( const char* const msg, ... )
{
    va_list args;
    va_start( args, msg );

    vfprintf( stderr, msg, args );
    perror("");

    va_end( args );
    exit(1);
}
