/* MethodListClass header. */

#include <libraries/mui.h>

extern struct MUI_CustomClass *MethodListClass;

struct MUI_CustomClass *CreateMethodListClass(void);
void DeleteMethodListClass(void);

#define MAX_ARGS_IN_METHOD       20

struct MethodArg
{
	STRPTR type;
	STRPTR name;
};

struct MethodEntry
{
	STRPTR name;
	STRPTR message;
	IPTR id;
	IPTR argCount;
	struct MethodArg args[MAX_ARGS_IN_METHOD];
};
