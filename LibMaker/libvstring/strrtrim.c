/* libvstring/strrtrim.c */

#include "libvstring.h"

void StrRTrim(STRPTR s, CONST_STRPTR array)
{
	STRPTR k;
	char a;

	for (k = s; *k; k++);      // go to end of string

	while (k >= s)
	{
		a = *k;
		if (!(StrChr(a, array))) return;
		else *k = 0x00;
		k--;
	}
}
