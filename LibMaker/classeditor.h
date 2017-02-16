/* ClassEditorClass header. */

#include <libraries/mui.h>
#if defined(__MORPHOS__)
#include <libraries/lua.h>
#else
#include <lua.h>
#endif

extern struct MUI_CustomClass *ClassEditorClass;

struct MUI_CustomClass *CreateClassEditorClass(void);
void DeleteClassEditorClass(void);

#define CEDA_LibraryName               0x6EDA7200      // [.S.]
//#define CEDA_SuperClassName            0x6EDA7201      // [.SG]


#define CEDM_NewAttribute              0x6EDA9100      // no message structure
#define CEDM_UpdateAttrFromGadgets     0x6EDA9101      // no message structure
#define CEDM_UpdateGadgetsFromAttr     0x6EDA9102      // no message structure
#define CEDM_WriteClassSpec            0x6EDA9103
#define CEDM_Clear                     0x6EDA9105
#define CEDM_PushToLua                 0x6EDA9106
#define CEDM_OpenMethodEditor          0x6EDA9107
#define CEDM_CloseMethodEditor         0x6EDA9108
#define CEDM_ReadClassSpec             0x6EDA9109


struct CEDP_UpdateGadgetsFromAttr
{
	ULONG MethodID;
	IPTR Position;
};


struct CEDP_WriteClassSpec
{
	ULONG MethodID;
	BPTR File;
};


struct CEDP_PushToLua
{
	ULONG MethodID;
	#ifdef __amigaos4__
	lua_State *L;
	#else
	LuaState *L;
	#endif
};


struct CEDP_OpenMethodEditor
{
	ULONG MethodID;
	IPTR Action;
};

#define CEDV_OpenMethodEditor_New         0
#define CEDV_OpenMethodEditor_Existing    1


struct CEDP_CloseMethodEditor
{
	ULONG MethodID;
	IPTR StoreData;
};

#define CEDV_CloseMethodEditor_Store      1
#define CEDV_CloseMethodEditor_Discard    0


struct CEDP_ReadClassSpec
{
	ULONG MethodID;
	BPTR FHandle;
	STRPTR TxtLine;
};
