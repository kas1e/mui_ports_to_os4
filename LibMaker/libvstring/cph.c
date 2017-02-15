/* libvstring/cph.c */

#include <exec/types.h>

// Counts placeholders in a format string, used to count how many variable
// arguments will be needed. It searches for a '%', then searches for either
// another '%', which means an escaped percent sign (not a placeholder), or
// parses through (0123456789$-.). Then it checks for (b, lc, s, ld, lD, lu,
// lU, lx, lX), if one found, counter of placeholders is incremented.

// The parser is implemented as a state machine, with the following states:
// 0 - searching for '%'
// 1 - searching for the first char of placeholder type
// 2 - searching for the second char of placeholder type

LONG cph(CONST_STRPTR s)
{
	LONG ph = 0;
	LONG st = 0;
	BOOL quad = FALSE;
	char c;

	while (c = *s++)
	{
		switch (st)
		{
			case 0:
				if (c == '%') st = 1;
			break;

			case 1:
				if (c == '%') st = 0;
				else if ((c >= '0') && (c <= '9'));
				else if ((c == '$') || (c == '.') || (c == '-'));
				else if (c == 'l') { quad = FALSE; st = 2; }
				else if (c == 'L') { quad = TRUE; st = 2; }
				else if ((c == 'b') || (c == 's'))
				{
					ph++;
					st = 0;
				}
				else st = 0;
			break;

			case 2:
				switch (c)
				{
					case 'c':
					case 'd':
					case 'D':
					case 'u':
					case 'U':
					case 'x':
					case 'X':
						ph++;
						if (quad) ph++;
						st = 0;
					break;

					case '%':
						st = 1;
					break;

					default:
						st = 0;
					break;
				}
			break;
		}
	}

	return ph;
}

