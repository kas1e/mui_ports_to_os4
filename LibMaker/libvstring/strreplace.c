/* libvstring/strreplace.c */

#include "libvstring.h"


VOID StrReplace(char c, char r, STRPTR array)
{
	char k;

	for (;;)
	{
		k = *array;
		if (k == 0x00) return;
		if (k == c) *array = r;
		array++;
	}
}
