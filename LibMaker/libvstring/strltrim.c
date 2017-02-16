/* libvstring/strltrim.c */

#include "libvstring.h"

void StrLTrim(STRPTR s, CONST_STRPTR array)
{
	STRPTR k = s;
	char a;
	BOOL lead = TRUE;

	while ((a = *s) != '\0')
	{
		if ((!lead) || (!StrChr(a, array)))
		{
			if (k < s) *k = a;
			k++;
			lead = FALSE;
		}

		s++;
	}

	if (k < s) *k = 0x00;
}

