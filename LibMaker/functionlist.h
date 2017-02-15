/* FunctionListClass header. */

#include <libraries/mui.h>
#include "functionargument.h"

extern struct MUI_CustomClass *FunctionListClass;

struct MUI_CustomClass *CreateFunctionListClass(void);
void DeleteFunctionListClass(void);

struct FunctionEntry
{
	STRPTR fe_Name;
	STRPTR fe_ReturnType;
	LONG fe_ArgCount;
	struct FunctionArgument *fe_Arguments;       // fe_ArgCount entries
};
