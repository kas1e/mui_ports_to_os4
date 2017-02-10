/* LibMaker */

#ifndef LIBMAKER_FUNCTIONARGUMENT_H
#define LIBMAKER_FUNCTIONARGUMENT_H

#include <exec/types.h>

struct FunctionArgument
{
	STRPTR fa_Type;
	STRPTR fa_Name;
	ULONG fa_Register;    // 0 to 7 for D0 to D7, 8 to 11 for A0 to A3.
};

#define M68K_REG_UNDEFINED   (~0)


#endif     /* LIBMAKER_FUNCTIONARGUMENT_H */
