/* libvstring/strlen.c */

#include "libvstring.h"


LONG StrLen(CONST_STRPTR s)
{
	CONST_STRPTR v = s;

	while (*v) v++;
	return (LONG)(v - s);  // will fail for strings longer than 2 GB ;-)
}

