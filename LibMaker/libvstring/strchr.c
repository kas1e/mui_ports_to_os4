/* libvstring/strchr.c */

#include "libvstring.h"


STRPTR StrChr(char c, CONST_STRPTR array)
{
	char k;

	for (;;)
	{
		k = *array;
		if (c == k) return (STRPTR)array;
		if (k == 0x00) return NULL;
		array++;
	}
}
