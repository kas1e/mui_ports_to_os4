/* libvstring/vfmtnew.c */

#include "libvstring.h"

#include <proto/exec.h>
#if defined(__MORPHOS__)
#include <exec/rawfmt.h>
#endif

#include <SDI_stdarg.h>

STRPTR VFmtNew(CONST_STRPTR fmt, va_list args)
{
	ULONG l = 0;
	STRPTR s;
	VA_LIST copy;

	VA_COPY(copy, args);

	VNewRawDoFmt(fmt, RAWFMTFUNC_COUNT, (STRPTR)&l, args);

	if ((s = AllocVecTaskPooled(l + 1)) != NULL)
	{
		VNewRawDoFmt(fmt, RAWFMTFUNC_STRING, s, copy);
	}

	return s;
}
