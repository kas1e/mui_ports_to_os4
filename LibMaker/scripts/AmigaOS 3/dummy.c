/*
   ${LIBNAME} dummy function and version string file.
   Generated with ${GENERATOR}.
*/

#include <exec/types.h>
#include "lib_version.h"

LONG dummy_function(void)
{
	return -1;
}

__attribute__ ((section(".text"))) const char VTag[] = VERSTAG;
