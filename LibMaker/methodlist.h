/* MethodListClass header. */

#include <libraries/mui.h>

extern struct MUI_CustomClass *MethodListClass;

struct MUI_CustomClass *CreateMethodListClass(void);
void DeleteMethodListClass(void);

#define MAX_ARGS_IN_METHOD       20

struct MethodArg
{
	STRPTR ma_Type;
	STRPTR ma_Name;
};

struct MethodEntry
{
	STRPTR me_Name;
	STRPTR me_Message;
	IPTR me_Id;
	IPTR me_ArgCount;
	struct MethodArg me_Args[MAX_ARGS_IN_METHOD];
};
