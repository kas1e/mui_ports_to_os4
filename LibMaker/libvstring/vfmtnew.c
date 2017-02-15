/* libvstring/vfmtnew.c */

#include "libvstring.h"

#include <proto/exec.h>
#ifndef __amigaos4__
#include <exec/rawfmt.h>
#endif


STRPTR VFmtNew(CONST_STRPTR fmt, va_list args)
{
	ULONG l = 0;
	STRPTR s;
	va_list copy;

	__va_copy(copy, args);
	VNewRawDoFmt(fmt, (APTR(*)(APTR, UBYTE))RAWFMTFUNC_COUNT, (STRPTR)&l, args);

	if (s = internal_alloc(l + 1))
	{
		VNewRawDoFmt(fmt, RAWFMTFUNC_STRING, s, copy);
	}
	return s;
}

