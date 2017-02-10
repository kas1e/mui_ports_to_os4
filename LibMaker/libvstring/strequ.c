/* libvstring/strequ.c */

#include "libvstring.h"


BOOL StrEqu(CONST_STRPTR s1, CONST_STRPTR s2)
{
	while (*s1 && (*s1 == *s2)) { s1++; s2++; }
	if (*s1 == *s2) return TRUE;
	else return FALSE;
}

