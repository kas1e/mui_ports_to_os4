/* libvstring/vfmtnew.c */

#include "libvstring.h"

#include <proto/exec.h>
#if defined(__MORPHOS__)
#include <exec/rawfmt.h>
#endif

STRPTR VFmtNew(CONST_STRPTR fmt, va_list args)
{
	ULONG l = 0;
	STRPTR s;
	va_list copy;

	va_copy(copy, args);

	VNewRawDoFmt(fmt, RAWFMTFUNC_COUNT, (STRPTR)&l, args);

	if ((s = AllocVecTaskPooled(l + 1)) != NULL)
	{
		VNewRawDoFmt(fmt, RAWFMTFUNC_STRING, s, copy);
	}

	return s;
}
