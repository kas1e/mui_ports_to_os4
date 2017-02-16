/* libvstring/strtrim.c */

#include "libvstring.h"

void StrTrim(STRPTR s, CONST_STRPTR array)
{
	STRPTR k = s, r = s;
	char a;
	BOOL lead = TRUE;

	/* left trim phase */

	while ((a = *r) != '\0')
	{
		if ((!lead) || (!StrChr(a, array)))
		{
			if (k < r) *k = a;
			k++;
			lead = FALSE;
		}

		r++;
	}

	if (k < r) *k = 0x00;

	/* now 'k' points to the string terminator, so right trim phase may follow immediately */

	while (k >= s)
	{
		a = *k;
		if (!(StrChr(a, array))) return;
		else *k = 0x00;
		k--;
	}
}
