/* libvstring/strnew.c */

#include "libvstring.h"

#include <proto/exec.h>


STRPTR StrNew(CONST_STRPTR str)
{
	STRPTR n = NULL;

	if ((n = AllocVecTaskPooled(StrLen(str) + 1)) != NULL)
	{
		StrCopy(str, n);
	}

	return n;
}

