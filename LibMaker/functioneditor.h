/* FunctionEditorClass header. */

#include <libraries/mui.h>
#include "functionargument.h"

extern struct MUI_CustomClass *FunctionEditorClass;

struct MUI_CustomClass *CreateFunctionEditorClass(void);
void DeleteFunctionEditorClass(void);


#define FEDA_FunctionName              0x6EDA3742  // [.SG] CONST_STRPTR
#define FEDA_ResultType                0x6EDA3743  // [.SG] CONST_STRPTR
#define FEDA_LibraryName               0x6EDA3744  // [.S.] CONST_STRPTR
#define FEDA_NumArgs                   0x6EDA3745  // [..G] LONG


#define FEDM_GetArgumentType           0x6EDA3791  // returns CONST_STRPTR

struct FEDP_GetArgumentType
{
	ULONG MethodID;
	IPTR ArgumentNumber;
};


#define FEDM_GetArgumentName           0x6EDA3792  // returns CONST_STRPTR

struct FEDP_GetArgumentName
{
	ULONG MethodID;
	IPTR ArgumentNumber;
};


#define FEDM_AddArgument               0x6EDA3793  // returns void, no parameter message

#define FEDM_DisplayArgument           0x6EDA3794  // returns void

struct FEDP_DisplayArgument
{
	ULONG MethodID;
	IPTR ArgumentNumber;
};


#define FEDM_UpdateArgument            0x6EDA3795  // returns void, no parameter message

#define FEDM_SetArgumentList           0x6EDA3796  // returns void

struct FEDP_SetArgumentList
{
	ULONG MethodID;
	IPTR NumArgs;
	struct FunctionArgument *Args;
};


#define FEDM_GetArgumentRegister       0x6EDA3797  // returns ULONG, 0 to 7 = D0 to D7, 8 to 11 = A0 to A3

struct FEDP_GetArgumentRegister
{
	ULONG MethodID;
	IPTR ArgumentNumber;
};
