/* libvstring/clonestrarray.c */

#include "libvstring.h"


STRPTR* CloneStrArray(CONST_STRPTR *array)
{
	CONST_STRPTR *p0;
	STRPTR *p1;
	STRPTR *copy;

	/* Allocate array. */

	if ((copy = AllocVecTaskPooled((StrArrayLen(array) + 1) * sizeof(STRPTR*))) != NULL)
	{
		/* Copy entries. */

		for (p0 = array, p1 = copy; *p0; p0++, p1++)
		{
			if ((*p1 = StrNew(*p0)) == NULL)
			{
				FreeStrArray(copy);
				copy = NULL;
				break;
			}
		}

		*p1 = NULL;
	}

	return copy;
}

