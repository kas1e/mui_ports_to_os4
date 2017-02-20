/* libvstring/localefmtnew.c */

#include "libvstring.h"

#include <proto/exec.h>


/****** libvstring/LocaleFmtNew() *******************************************
*
* NAME
*   LocaleFmtNew() -- Creates a new string formatted with
*  	locale.library/FormatString().
*
* SYNOPSIS
*   void LocaleVFmtNew(struct Locale *locale, CONST_STRPTR fmt, va_list
*     args);
*   void LocaleFmtNew(struct Locale *locale, CONST_STRPTR fmt, ...)
*
* FUNCTION
*   Creates a new null terminated buffer containing a string formatted
*   according to 'fmt' in 'dest' buffer. Formatting is done with
*   locale.library/FormatString() function. LocaleFmtNew() inherits
*   FormatString() limitations and features, namely it supports argument
*   reordering, and	localized digit grouping, but does not support floating
*   point numbers. The buffer returned must be later freed
*   with FmtFree() function, unless the memory allocator is a pooled one.
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
*   A new buffer containing the formatted string or NULL in case of no free
*   memory.
*
* SEE ALSO
*   FmtPut(), LocaleFmtLen(), LocaleFmtNPut(), LocaleFmtPut(),
*  	locale.library/FormatString(), locale.library/OpenLocale()
*
*****************************************************************************
*
*/

#if !defined(va_copy) && defined(__va_copy)
	#define va_copy __va_copy
#endif

STRPTR LocaleVFmtNew(struct Locale *loc, CONST_STRPTR fmt, va_list args)
{
	LONG l = 0;
	STRPTR s;
	va_list copy;

	__va_copy(copy, args);
	l = LocaleVFmtLen(loc, fmt, args);

	if ((s = internal_alloc(l)) != NULL)
	{
		LocaleVFmtPut(loc, s, fmt, copy);
	}
	return s;
}



STRPTR LocaleFmtNew(struct Locale *loc, CONST_STRPTR fmt, ...)
{
	STRPTR s;
	va_list args;

	va_start(args, fmt);
	s = LocaleVFmtNew(loc, fmt, args);
	va_end(args);
	return s;
}
