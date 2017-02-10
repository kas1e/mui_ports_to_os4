/* libvstring/localefmtput.c */

#include "libvstring.h"

#include <proto/locale.h>

/****** libvstring/LocaleFmtPut() *******************************************
*
* NAME
*   LocaleFmtPut() -- Stores a string formatted with
*  	locale.library/FormatString() to a buffer.
*
* SYNOPSIS
*   void LocaleVFmtPut(struct Locale *locale, STRPTR dest, CONST_STRPTR fmt,
*     va_list args)
*   void LocaleFmtPut(struct Locale *locale, STRPTR dest, CONST_STRPTR fmt,
*     ...)
*
* FUNCTION
*   Stores a string formatted according to 'fmt' in 'dest' buffer. Formatting
*   is done with locale.library/FormatString() function. Buffer length is not
*   checked so it must be long enough to hold the whole string, or overflow
*   will occur. LFmtLen() inherits FormatString() limitations and features,
*   namely it supports argument reordering, and localized digit grouping, but
*   does not support floating point numbers.
*
*   The function formats all variable arguments into a table of 32-bit values
*   so the 'l' specifier *must* be used for numeric placeholders %lc, %ld,
*   %lD, %lu, %lU, %lx, %lX. Also when a 64-bit integer is passed, it must be
*   split into two 32-bit values using QSPLIT() macro.
*
* INPUTS
*   locale - the locale used for parsing formatting pattern. Note that NULL
*     here cannot be used with meaning "system default locale", the same as
*     for FormatString(). Pass the result of OpenLocale(NULL) here for system
*     default.
*   dest - pointer to the destination buffer.
*   fmt - string formatting pattern as specified in FormatString()
*     documentation.
*   ... - variable arguments used to fill format string placeholders.
*
* RESULT
*   None.
*
* SEE ALSO
*   FmtPut(), LocaleFmtLen(), LocaleFmtNPut(), LocaleFmtNew(),
*  	locale.library/FormatString(), locale.library/OpenLocale()
*
*****************************************************************************
*
*/

extern LONG cph(CONST_STRPTR s);

#ifdef __amigaos4__
static void pufunc(struct Hook *h, APTR a, char c)
#else
static void pufunc(void)
#endif
{


	#ifndef __amigaos4__
	struct Hook *h = (struct Hook*)REG_A0;
	char c = (char)REG_A1;
	#endif
	
	STRPTR p = (STRPTR)h->h_Data;

	*p++ = c;
	h->h_Data = (APTR)p;
}

#ifndef __amigaos4__
static const struct EmulLibEntry pugate = { TRAP_LIB, 0, (void(*)(void))pufunc };
#endif

void LocaleVFmtPut(struct Locale *locale, STRPTR dest, CONST_STRPTR fmt, va_list args)
{
	LONG ph;

	ph = cph(fmt);

	{
		LONG argstream[ph];
		LONG i;
		struct Hook h;

		for (i = 0; i < ph; i++)
		{
			argstream[i] = va_arg(args, LONG);
		}
		#ifdef __amigaos4__
		h.h_Entry = (HOOKFUNC)&pufunc;
		#else
		h.h_Entry = (HOOKFUNC)&pugate;
		#endif
		h.h_Data = dest;

		FormatString(locale, (STRPTR)fmt, argstream, &h);
	}
}


void LocaleFmtPut(struct Locale *locale, STRPTR dest, CONST_STRPTR fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	LocaleVFmtPut(locale, dest, fmt, args);
	va_end(args);
}

