/* ApplicationClass header. */

#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/muimaster.h>
#include <clib/alib_protos.h>

extern struct MUI_CustomClass *ApplicationClass;
struct MUI_CustomClass *CreateApplicationClass(void);
void DeleteApplicationClass(void);


/* attributes */

#define APPA_ProjectName               0x6EDA3400      // [.SG] STRPTR
#define APPA_ProjectVersion            0x6EDA3401      // [.S.] LONG
#define APPA_ProjectRevision           0x6EDA3402      // [.S.] LONG
#define APPA_ProjectDate               0x6EDA3403      // [.S.] STRPTR
#define APPA_ProjectCopyright          0x6EDA3404      // [.S.] STRPTR
#define APPA_ProjectBaseName           0x6EDA3405      // [.S.] STRPTR
#define APPA_UseAltiVec                0x6EDA3406      // [.S.] BOOL
#define APPA_BoopsiClass               0x6EDA3407      // [.S.] BOOL
#define APPA_MuiClass                  0x6EDA3408      // [.S.] BOOL


/* methods */

#define APPM_Notifications             0x6EDA3601
#define APPM_MainLoop                  0x6EDA3602
#define APPM_GenerateCode              0x6EDA3603
#define APPM_OpenFunctionEditor        0x6EDA3604

struct APPP_OpenFunctionEditor
{
	ULONG MethodID;
	IPTR Doubleclicked;
};

#define APPM_CloseFunctionEditor       0x6EDA3605

struct APPP_CloseFunctionEditor
{
	ULONG MethodID;
	IPTR DoSave;
};

#define APPV_CloseFunctionEditor_Save           1
#define APPV_CloseFunctionEditor_DoNotSave      0

#define APPM_SaveProjectByName         0x6EDA3606

struct APPP_SaveProjectByName
{
	ULONG MethodID;
	STRPTR Path;
};

#define APPM_SaveProjectByReq          0x6EDA3607     // no message
#define APPM_SaveProject               0x6EDA3608     // no message

#define APPM_OpenProjectByName         0x6EDA3609

struct APPP_OpenProjectByName
{
	ULONG MethodID;
	STRPTR Path;
};

#define APPM_OpenProjectByReq          0x6EDA360A     // no message

#define APPM_AddFunction               0x6EDA360B

struct APPP_AddFunction
{
	ULONG MethodID;
	APTR Entry;
};

#define APPM_OpenClassEditor           0x6EDA630C     // no message

#define APPM_CloseClassEditor          0x6EDA630D

struct APPP_CloseClassEditor
{
	ULONG MethodID;
	IPTR DoSave;
};

#define APPV_CloseClassEditor_Save           1
#define APPV_CloseClassEditor_DoNotSave      0
