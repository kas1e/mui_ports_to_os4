/* libvstring/strnequ.c */

#include "libvstring.h"


BOOL StrNEqu(CONST_STRPTR s1, CONST_STRPTR s2, LONG len)
{
	while (len--)
	{
		if (*s1 != *s2) return FALSE;
		if (!(*s1)) return TRUE;
		s1++;
		s2++;
	}

	return TRUE;
}
