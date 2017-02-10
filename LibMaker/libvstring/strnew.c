/* libvstring/strnew.c */

#include "libvstring.h"

#include <proto/exec.h>


STRPTR StrNew(CONST_STRPTR str)
{
	STRPTR n = NULL;

	if (n = internal_alloc(StrLen(str) + 1))
	{
		StrCopy(str, n);
	}

	return n;
}

