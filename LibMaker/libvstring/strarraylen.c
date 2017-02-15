/* libvstring/strarraylen.c */

#include "libvstring.h"


LONG StrArrayLen(STRPTR *array)
{
	STRPTR *p;
	LONG len;

	for (p = array, len = 0; *p; p++) len++;

	return len;
}

