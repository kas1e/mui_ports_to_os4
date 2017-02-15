/* libvstring/strlen.c */

#include "libvstring.h"


ULONG StrLen(CONST_STRPTR s)
{
	CONST_STRPTR v = s;

	while (*v) v++;
	return (ULONG)(v - s);  // will fail for strings longer than 4 GB ;-)
}

