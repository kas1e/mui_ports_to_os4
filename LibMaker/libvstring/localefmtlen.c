/* libvstring/localefmtlen.c */

#include "libvstring.h"

#include <proto/locale.h>

/****** libvstring/LocaleFmtLen() *******************************************
*
* NAME
*   LocaleFmtLen() -- Counts characters of string formatted with
*  	locale.library/FormatString().
*
* SYNOPSIS
*   LONG LocaleVFmtLen(struct Locale *locale, CONST_STRPTR fmt, va_list args)
*   LONG LocaleFmtLen(struct Locale *locale, CONST_STRPTR fmt, ...)
*
* FUNCTION
*   Returns the number of bytes taken by a string formatted according to
*   'fmt' using locale.library/FormatString() function. This number of bytes
*   includes null termination. Note that LocaleFmtLen() inherits FormatString()
*   limitations and features, namely it supports argument reordering, and
*   localized digit grouping, but does not support floating point numbers.
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
*   fmt - string formatting pattern as specified in FormatString()
*  	  documentation.
*   ... - variable arguments used to fill format string placeholders.
*
* RESULT
*   The length of resulting string in bytes including null terminator.
*
* SEE ALSO
*   FmtLen(), LocaleFmtPut(), LocaleFmtNPut(), LocaleFmtNew(),
*  	locale.library/FormatString(), locale.library/OpenLocale()
*
*****************************************************************************
*
*/

#include "libvstring.h"

#include <SDI_hook.h>

extern LONG cph(CONST_STRPTR s);


HOOKPROTONONP(ctfunc, void)
{
	LONG x = (LONG)hook->h_Data;

	hook->h_Data = (APTR)(++x);
}


LONG LocaleVFmtLen(struct Locale *locale, CONST_STRPTR fmt, va_list args)
{
	LONG ph, result;

	ph = cph(fmt);

	{
		LONG argstream[ph];
		LONG i;
		LONG length;
		MakeHookWithData(h, ctfunc, &length);

		for (i = 0; i < ph; i++)
		{
			argstream[i] = va_arg(args, LONG);
		}

		FormatString(locale, (STRPTR)fmt, argstream, &h);
		result = length;
	}

	return result;
}


LONG LocaleFmtLen(struct Locale *locale, CONST_STRPTR fmt, ...)
{
	va_list args;
	LONG result;

	va_start(args, fmt);
	result = LocaleVFmtLen(locale, fmt, args);
	va_end(args);
	return result;
}

