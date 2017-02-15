/* libvstring/strarraylen.c */

#include "libvstring.h"


ULONG StrArrayLen(CONST_STRPTR *array)
{
	CONST_STRPTR *p;
	ULONG len;

	for (p = array, len = 0; *p; p++) len++;

	return len;
}

