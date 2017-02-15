/* ArgumentListClass header. */

#include <libraries/mui.h>

extern struct MUI_CustomClass *ArgumentListClass;

struct MUI_CustomClass *CreateArgumentListClass(void);
void DeleteArgumentListClass(void);

struct ArgumentEntry
{
	STRPTR ae_Name;
	STRPTR ae_Type;
	ULONG ae_Register;    // 0 to 7 for D0 to D7, 8 to 11 for A0 to A3.
};
