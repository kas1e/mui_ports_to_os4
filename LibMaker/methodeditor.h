/* MethodEditorClass header. */

#include <libraries/mui.h>
#include "functionargument.h"

extern struct MUI_CustomClass *MethodEditorClass;

struct MUI_CustomClass *CreateMethodEditorClass(void);
void DeleteMethodEditorClass(void);


#define MEDM_Setup                     0x6EDAA865
#define MEDM_UpdateEntry               0x6EDAA866

struct MEDP_Setup
{
	ULONG MethodID;
	STRPTR LibName;
	APTR Entry;
	Object *ClassEditor;
};


struct MEDP_UpdateEntry
{
	ULONG MethodID;
	APTR Entry;
};
