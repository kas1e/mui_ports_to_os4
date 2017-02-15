/* libvstring/freestrarray.c */

#include "libvstring.h"

#include <proto/exec.h>


void FreeStrArray(STRPTR *array)
{
	STRPTR *p0;

	for (p0 = array; *p0; p0++) StrFree(*p0);
	StrFree(array);
}

