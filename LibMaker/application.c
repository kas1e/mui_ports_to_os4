/* ApplicationClass code. */

#include "main.h"
#include "application.h"
#include "functionlist.h"
#include "functioneditor.h"
#include "classeditor.h"
#include "attributelist.h"
#include "libvstring/libvstring.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <proto/locale.h>

#if defined(__MORPHOS__)
#include <proto/lua.h>
#else
#include <lua.h>
#include <lauxlib.h>
#endif

#include <libraries/asl.h>

#include <utility/tagitem.h>
#include <libraries/asl.h>
#include <mui/Aboutbox_mcc.h>

/*
  Format of a project file:
  - The first line: LIBMAKER ver.rev
  - The second line: LIBRARY NAME=name VER=version REV=revision DATE=date COPYRIGHT=copyright
    [BASE=basename] [ALTIVEC] [BOOPSI] [MUI]
  - The third line: GENERATOR TYPE=subdir DEST=path [INSTALL=path] [INCDIR=path] [SFUNCTIONS] [SMETHODS]
  - Function lines: FUNCTION NAME=name RETTYPE=return_type
  - Argument lines: ARGUMENT TYPE=type NAME=name [M68KREG=reg]
  - List of function arguments ends with ENDFUNCTION
  - List of library functions ends with ENDLIBRARY
  NOTE: due to ReadArgs() limitation all '*' characters are replaced with '^'.
*/

struct MUI_CustomClass *ApplicationClass = 0;

struct AppObjects
{
	Object *WndMain;
	Object *WndAbout;
	Object *WndFunctionEditor;
	Object *WndClassEditor;
	Object *BtnGenerateCode;
	Object *StrLibraryName;
	Object *StrDestDir;
	Object *StrLibVersion;
	Object *StrLibRevision;
	Object *StrLibDate;
	Object *StrLibCopyrigt;
	Object *BtnAddFunction;
	Object *BtnDelFunction;
	Object *LstFunctions;
	Object *StrLibBaseName;
	Object *ChkAltiVec;
	Object *ChkBoopsi;
	Object *BtnBoopsi;
	Object *ChkMui;
	Object *BtnMui;
	Object *ChkSeparatedFunctions;
	Object *ChkSeparatedMethods;
	Object *CycSrciptSet;
	Object *StrInstDir;
	Object *StrIncludePath;
	STRPTR *Scripts;
};

struct ObjData
{
	struct AppObjects Objects;
	struct FileRequester *FileReq;
};


DISPATCHERPROTO(ApplicationDispatcher);

#define MENUITEM_ABOUT      1
#define MENUITEM_QUIT       2
#define MENUITEM_SAVE       3
#define MENUITEM_SAVE_AS    4
#define MENUITEM_OPEN       5



static LONG LibraryDefLoader(Object *app, struct ObjData *d, BPTR file);


//==============================================================================================
// CreateApplicationClass()
//==============================================================================================

struct MUI_CustomClass *CreateApplicationClass(void)
{
	struct MUI_CustomClass *cl;

	cl = MUI_CreateCustomClass(NULL, MUIC_Application, NULL, sizeof(struct ObjData), ENTRY(ApplicationDispatcher));
	ApplicationClass = cl;
	return cl;
}


//==============================================================================================
// DeleteApplicationClass()
//==============================================================================================

void DeleteApplicationClass(void)
{
	MUI_DeleteCustomClass(ApplicationClass);
}


//==============================================================================================
// LibMakerMakeDir()
//==============================================================================================

/*
   This function is exported to Lua as libmaker.makedir(). Unlike the original MakeDir command
   it simply passes over if created directory already exists. Also having it avoids calling
   external command, which is slower.
*/

#if defined(__MORPHOS__)
LONG LibMakerMakeDir(LuaState *L)
#else
static int LibMakerMakeDir(lua_State* L)
#endif
{
	CONST_STRPTR path;
	BPTR dirlock;

	path = LuaCheckString(L, 1);

	dirlock = Lock(path, SHARED_LOCK);

	if (!dirlock)
	{
		dirlock = CreateDir(path);
	}

	if (dirlock) UnLock(dirlock);
	else
	{
		char fault[128];

		Fault(IoErr(), (STRPTR)"", fault, sizeof(fault));
		#if defined(__MORPHOS__)
		LuaErrorF(L, &fault[2]);             // Skipping colon added by Fault().
		#else
		lua_error(L);
		#endif
	}

	return 0;
}



//==============================================================================================
// CreateGroup1()
//==============================================================================================

static Object* CreateGroup1(struct AppObjects *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Group_Columns, 2,
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Text_Contents, LS(MSG_LIBRARY_NAME_LABEL, "Library Name:"),
			MUIA_Text_PreParse, "\33r",
			MUIA_HorizWeight, 0,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
		TAG_END),
		MUIA_Group_Child, objs->StrLibraryName = MUI_NewObjectM(MUIC_String,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_Background, MUII_StringBack,
			#ifdef __amigaos4__
			MUIA_String_Contents, "test.library", // as without name (i.e. default) lua script fail.
			#else
			MUIA_String_Contents, ".library",
			#endif
			MUIA_String_Accept, "abcdefghijklmnopqrstuvwxyz0123456789.-_",
			MUIA_String_MaxLen, MAXLEN_LIBRARY_NAME + 1,
			MUIA_ShortHelp, LS(MSG_LIBRARY_NAME_HELP, "Library name. Use small letters of English alphabet, digits, dot, hyphen and underscore."),
			MUIA_CycleChain, TRUE,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Text_Contents, LS(MSG_LIBRARY_NAME_LABEL, "Library Base Name:"),
			MUIA_Text_PreParse, "\33r",
			MUIA_HorizWeight, 0,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
		TAG_END),
		MUIA_Group_Child, objs->StrLibBaseName = MUI_NewObjectM(MUIC_String,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_Background, MUII_StringBack,
			MUIA_String_Accept, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
			MUIA_String_MaxLen, MAXLEN_LIBRARY_NAME + 1,
			MUIA_ShortHelp, LS(MSG_LIBRARY_BASE_NAME_HELP, "Library base name. It is a C language variable, so must follow language rules. "
				"It should be named after the library, capitalized, and end with \"Base\", for example \"FooBarBase\"."),
			MUIA_String_Contents, "Base",
			MUIA_CycleChain, TRUE,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Text_Contents, LS(MSG_LIB_VERSION_LABEL, "Version:"),
			MUIA_Text_PreParse, "\33r",
			MUIA_HorizWeight, 0,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_Group_Child, objs->StrLibVersion = MUI_NewObjectM(MUIC_String,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_Background, MUII_StringBack,
				MUIA_String_Accept, "0123456789",
				MUIA_String_Integer, 1,
				MUIA_String_MaxLen, 6,
				MUIA_HorizWeight, 50,
				MUIA_ShortHelp, LS(MSG_LIBRARY_VERSION_HELP, "Library major version number. Should be a number between 1 and 65535. "
					"Major version should be incremented when new functions are added to the library API."),
				MUIA_CycleChain, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
				MUIA_FixWidthTxt, "M",
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_Contents, LS(MSG_LIB_REVISION_LABEL, "Revision:"),
				MUIA_Text_PreParse, "\33r",
				MUIA_HorizWeight, 0,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_FramePhantomHoriz, TRUE,
			TAG_END),
			MUIA_Group_Child, objs->StrLibRevision = MUI_NewObjectM(MUIC_String,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_Background, MUII_StringBack,
				MUIA_String_Accept, "0123456789",
				MUIA_String_Contents, "0",
				MUIA_String_MaxLen, 6,
				MUIA_HorizWeight, 50,
				MUIA_ShortHelp, LS(MSG_LIBRARY_REVISION_HELP, "Library minor version number. Should be a number between 1 and 65535. "
					"Minor version should be incremented with every release, if major version is not changed."),
				MUIA_CycleChain, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
				MUIA_FixWidthTxt, "M",
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_Contents, LS(MSG_LIB_DATE_LABEL, "Date:"),
				MUIA_Text_PreParse, "\33r",
				MUIA_HorizWeight, 0,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_FramePhantomHoriz, TRUE,
			TAG_END),
			MUIA_Group_Child, objs->StrLibDate = MUI_NewObjectM(MUIC_String,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_Background, MUII_StringBack,
				MUIA_String_Accept, "0123456789.",
				MUIA_String_MaxLen, 11,
				MUIA_ShortHelp, LS(MSG_LIBRARY_DATE_HELP, "Library release date in DD.MM.YYYY format with leading zeros for day and month."),
				MUIA_CycleChain, TRUE,
			TAG_END),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Text_Contents, LS(MSG_LIB_COPYRIGHT_LABEL, "Copyright String:"),
			MUIA_Text_PreParse, "\33r",
			MUIA_HorizWeight, 0,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
		TAG_END),
		MUIA_Group_Child, objs->StrLibCopyrigt = MUI_NewObjectM(MUIC_String,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_Background, MUII_StringBack,
			MUIA_String_MaxLen, MAXLEN_COPYRIGHT_STRING + 1,
			MUIA_ShortHelp, LS(MSG_LIBRARY_COPYRIGHT_HELP, "Library copyright message placed in the version string. Limited to 64 characters."),
			MUIA_CycleChain, TRUE,
		TAG_END),
	TAG_END);

	return obj;
}



//==============================================================================================
// CreateGroup2()
//==============================================================================================

static Object* CreateGroup2(struct AppObjects *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Group_Horiz, TRUE,
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_ShortHelp, LS(MSG_ALTIVEC_CHECKMARK_HELP, "Adds AltiVec compiler flags to the makefile. Adds neccesary include files. "
				"Adds AltiVec detection code to the library initialization and sets 'HaveAltiVec' field in the library base accordingly. "
				"Note that non-AltiVec version of the code should be always provided."),
			MUIA_Group_Child, objs->ChkAltiVec = MUI_NewObjectM(MUIC_Image,
				ImageButtonFrame,
				MUIA_ShowSelState, FALSE,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Image_Spec, "6:15",
				MUIA_Image_FreeHoriz, FALSE,
				MUIA_Image_FreeVert, FALSE,
				MUIA_InputMode, MUIV_InputMode_Toggle,
				MUIA_CycleChain, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_PreParse, "\33l",
				MUIA_Text_Contents, LS(MSG_ALTIVEC_CHECKMARK_LABEL, "Use AltiVec"),
			TAG_END),
		TAG_END),

		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_ShortHelp, LS(MSG_BOOPSI_CHECKMARK_HELP, "Turns the library into an external public BOOPSI class. The class is added"
				" to the system during library initialization. Class methods and attributes may be edited in the class editor, which"
				" is opened with the button."),
			MUIA_Group_Child, objs->ChkBoopsi = MUI_NewObjectM(MUIC_Image,
				ImageButtonFrame,
				MUIA_ShowSelState, FALSE,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Image_Spec, "6:15",
				MUIA_Image_FreeHoriz, FALSE,
				MUIA_Image_FreeVert, FALSE,
				MUIA_InputMode, MUIV_InputMode_Toggle,
				MUIA_CycleChain, TRUE,
				#warning Remove when implemented
				MUIA_Disabled, TRUE,
			TAG_END),
			MUIA_Group_Child, objs->BtnBoopsi = MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_BOOPSI_CHECKMARK_LABEL, "BOOPSI Class"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_Disabled, TRUE,
				MUIA_CycleChain, TRUE,
				MUIA_Text_SetMax, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
			TAG_END),
		TAG_END),

		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_ShortHelp, LS(MSG_MUI_CHECKMARK_HELP, "Turns the library into an external public MUI class. Class methods and "
				"attributes may be edited in the class editor, which is opened with the button."),
			MUIA_Group_Child, objs->ChkMui = MUI_NewObjectM(MUIC_Image,
				ImageButtonFrame,
				MUIA_ShowSelState, FALSE,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Image_Spec, "6:15",
				MUIA_Image_FreeHoriz, FALSE,
				MUIA_Image_FreeVert, FALSE,
				MUIA_InputMode, MUIV_InputMode_Toggle,
				MUIA_CycleChain, TRUE,
				#warning Remove when implemented
				MUIA_Disabled, TRUE,
			TAG_END),
			MUIA_Group_Child, objs->BtnMui = MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_MUI_CHECKMARK_LABEL, "MUI Class"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_Disabled, TRUE,
				MUIA_CycleChain, TRUE,
				MUIA_Text_SetMax, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
			TAG_END),
		TAG_END),
	TAG_END);

	return obj;
}

//==============================================================================================
// CreateFunctionList()
//==============================================================================================

static Object* CreateFunctionList(struct AppObjects *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Frame, MUIV_Frame_Group,
		MUIA_Background, MUII_GroupBack,
		MUIA_FrameTitle, LS(MSG_FUNCLIST_BAR_TITLE, "Library Functions"),
		MUIA_Group_Child, objs->LstFunctions = NewObjectM(FunctionListClass->mcc_Class, NULL,
			MUIA_Frame, MUIV_Frame_InputList,
			MUIA_Background, MUII_ListBack,
			MUIA_CycleChain, TRUE,
			MUIA_List_DragSortable, TRUE,
			MUIA_ShortHelp, LS(MSG_FUNCLIST_HELP, "Doubleclick to edit a function. Change order with drag and drop."),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_Group_Child, objs->BtnAddFunction = MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_FUNCLIST_BUTTON_ADD, "Add"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_FUNCLIST_BUTTON_ADD_HELP, "Adds a new function to the library."),
			TAG_END),
			MUIA_Group_Child, objs->BtnDelFunction = MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_FUNCLIST_BUTTON_DELETE, "Delete"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_FUNCLIST_BUTTON_DELETE_HELP, "Deletes a selected function from the library."),
			TAG_END),
		TAG_END),
	TAG_END);

	return obj;
}

//==============================================================================================
// CreateGeneratorGroup()
//==============================================================================================

static Object* CreateGeneratorGroup(struct AppObjects *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Frame, MUIV_Frame_Group,
		MUIA_Background, MUII_GroupBack,
		MUIA_FrameTitle, LS(MSG_CODE_GENERATOR_GROUP_TITLE, "Code Generator"),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Columns, 2,
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_Contents, LS(MSG_DESTINATION_DIR_LABEL, "Destination Drawer:"),
				MUIA_Text_PreParse, "\33r",
				MUIA_HorizWeight, 0,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_FramePhantomHoriz, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Popasl,
				ASLFR_TitleText, LS(MSG_DESTINATION_DIR_REQTITLE, "Select Destination Drawer"),
				ASLFR_DrawersOnly, TRUE,
				ASLFR_DoSaveMode, TRUE,
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_DESTINATION_DIR_HELP, "Code will be generated to this drawer. No subdrawer will be created."),
				MUIA_Popstring_String, objs->StrDestDir = MUI_NewObjectM(MUIC_String,
					MUIA_Frame, MUIV_Frame_String,
					MUIA_Background, MUII_StringBack,
					MUIA_String_Contents, "RAM:",
				TAG_END),
				MUIA_Popstring_Button, MUI_NewObjectM(MUIC_Image,
					MUIA_Image_Spec, "6:20",
					MUIA_Frame, MUIV_Frame_ImageButton,
					MUIA_Background, MUII_ImageButtonBack,
					MUIA_InputMode, MUIV_InputMode_RelVerify,
					MUIA_Image_FreeVert, TRUE,
				TAG_END),
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_Contents, LS(MSG_INSTALL_DIR_LABEL, "Installation Drawer:"),
				MUIA_Text_PreParse, "\33r",
				MUIA_HorizWeight, 0,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_FramePhantomHoriz, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Popasl,
				ASLFR_TitleText, LS(MSG_INSTALL_DIR_REQTITLE, "Select Installation Drawer"),
				ASLFR_DrawersOnly, TRUE,
				ASLFR_DoSaveMode, TRUE,
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_INSTALL_DIR_HELP, "This drawer is used in makefile generation for \"install\" target. Compiled library "
					"or class will be copied to this directory when \"make install\" is performed, then flushed from memory if possible."),
				MUIA_Popstring_String, objs->StrInstDir = MUI_NewObjectM(MUIC_String,
					MUIA_Frame, MUIV_Frame_String,
					MUIA_Background, MUII_StringBack,
					MUIA_String_Contents, "SYS:Libs/",
				TAG_END),
				MUIA_Popstring_Button, MUI_NewObjectM(MUIC_Image,
					MUIA_Image_Spec, "6:20",
					MUIA_Frame, MUIV_Frame_ImageButton,
					MUIA_Background, MUII_ImageButtonBack,
					MUIA_InputMode, MUIV_InputMode_RelVerify,
					MUIA_Image_FreeVert, TRUE,
				TAG_END),
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_Contents, LS(MSG_INCLUDE_PATH_LABEL, "Include Path:"),
				MUIA_Text_PreParse, "\33r",
				MUIA_HorizWeight, 0,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_FramePhantomHoriz, TRUE,
			TAG_END),
			MUIA_Group_Child, objs->StrIncludePath = MUI_NewObjectM(MUIC_String,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_Background, MUII_StringBack,
				MUIA_String_Contents, "libraries",
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_INCLUDE_PATH_HELP, "Location of the main public library/class header file in the system includes tree. "
					"The file contains definitions of structures, constants, tags, attributes and methods. The location is used to proprely "
					"include the file in code files. For plain libraries usual location is just \"libraries\", for classes it is \"classes\" "
					"or a subdirectory of \"classes\". For MUI classes, the location is \"mui\"."),
			TAG_END),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_ShortHelp, LS(MSG_SINGLE_FUNCTIONS_CHECKMARK_HELP, "When checked, every library function skeleton is generated "
				"into a separate file with name starting from \"f_\" followed by the function name. Makefile is modified accordingly. "
				"If not checked, all the functions are placed in \"library.c\" file."),
			MUIA_Group_Child, objs->ChkSeparatedFunctions = MUI_NewObjectM(MUIC_Image,
				ImageButtonFrame,
				MUIA_ShowSelState, FALSE,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Image_Spec, "6:15",
				MUIA_Image_FreeHoriz, FALSE,
				MUIA_Image_FreeVert, FALSE,
				MUIA_InputMode, MUIV_InputMode_Toggle,
				MUIA_CycleChain, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_PreParse, "\33l",
				MUIA_Text_Contents, LS(MSG_SINGLE_FUNCTIONS_CHECKMARK_LABEL, "Every function in a separate file"),
			TAG_END),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
			MUIA_ShortHelp, LS(MSG_SINGLE_METHODS_CHECKMARK_HELP, "When checked, every class method skeleton (including standard "
				"methods) is generated into a separate file with name starting from \"m_\" followed by the method name. Makefile is "
				"modified accordingly. If not checked, all the functions are placed in \"methods.c\" file. Note that the class dispatcher "
				"is always placed in \"library.c\"."),
			MUIA_Group_Child, objs->ChkSeparatedMethods = MUI_NewObjectM(MUIC_Image,
				ImageButtonFrame,
				MUIA_ShowSelState, FALSE,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Image_Spec, "6:15",
				MUIA_Image_FreeHoriz, FALSE,
				MUIA_Image_FreeVert, FALSE,
				MUIA_InputMode, MUIV_InputMode_Toggle,
				MUIA_CycleChain, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_PreParse, "\33l",
				MUIA_Text_Contents, LS(MSG_SINGLE_METHODS_CHECKMARK_LABEL, "Every class method in a separate file"),
			TAG_END),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
			MUIA_ShortHelp, LS(MSG_SCRIPT_SET_CYCLE_HELP, "Different script sets generate code for different operating systems or "
				"use different coding styles, sets of macros, etc. Every set is placed in a subdirectory of \"scripts\" dir. This "
				"directory is scanned at program startup."),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_FramePhantomHoriz, TRUE,
				MUIA_Text_SetMax, TRUE,
				MUIA_Text_Contents, LS(MSG_SCRIPT_SET_CYCLE_LABEL, "Generator Script Set:"),
			TAG_END),
			MUIA_Group_Child, objs->CycSrciptSet = MUI_NewObjectM(MUIC_Cycle,
				MUIA_CycleChain, TRUE,
				MUIA_Cycle_Entries, objs->Scripts,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
			TAG_END),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
				MUIA_HorizWeight, 25,
			TAG_END),
			MUIA_Group_Child, objs->BtnGenerateCode = MUI_NewObjectM(MUIC_Text,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_BUTTON_GENERATE_CODE, "Generate Code"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_ShortHelp, LS(MSG_OBJ_BTN_GENERATE_CODE_HELP, "Starts the code generator."),
				MUIA_CycleChain, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
				MUIA_HorizWeight, 25,
			TAG_END),
		TAG_END),
	TAG_END);

	return obj;
}


//==============================================================================================
// CreateMainWindow()
//==============================================================================================

static Object* CreateMainWindow(struct AppObjects *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Window,
		MUIA_Window_ID, MAKE_ID('M','A','I','N'),
		MUIA_Window_Title, APP_NAME,
		MUIA_Window_ScreenTitle, APP_NAME " " APP_VER,
		MUIA_Window_RootObject, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Child, CreateGroup1(objs),
			MUIA_Group_Child, CreateGroup2(objs),
			MUIA_Group_Child, CreateFunctionList(objs),
			MUIA_Group_Child, CreateGeneratorGroup(objs),
		TAG_END),
	TAG_END);

	return obj;
}


//==============================================================================================
// CreateAppMenu()
//==============================================================================================

static Object *CreateAppMenu(void)
{
	Object *menu;

	menu = MUI_NewObjectM(MUIC_Menustrip,
		MUIA_Family_Child, MUI_NewObjectM(MUIC_Menu,
			MUIA_Menu_Title, LS(MSG_MENU_PROJECT, "Project"),
			MUIA_Family_Child, MUI_NewObjectM(MUIC_Menuitem,
				MUIA_Menuitem_Title, LS(MSG_MENUITEM_OPEN, "Open..."),
				MUIA_Menuitem_Shortcut, "O",
				MUIA_UserData, MENUITEM_OPEN,
			TAG_END),
			MUIA_Family_Child, MUI_NewObjectM(MUIC_Menuitem,
				MUIA_Menuitem_Title, LS(MSG_MENUITEM_SAVE, "Save"),
				MUIA_Menuitem_Shortcut, "S",
				MUIA_UserData, MENUITEM_SAVE,
			TAG_END),
			MUIA_Family_Child, MUI_NewObjectM(MUIC_Menuitem,
				MUIA_Menuitem_Title, LS(MSG_MENUITEM_SAVE_AS, "Save as..."),
				MUIA_Menuitem_Shortcut, "A",
				MUIA_UserData, MENUITEM_SAVE_AS,
			TAG_END),
			MUIA_Family_Child, MUI_NewObjectM(MUIC_Menuitem,
				MUIA_Menuitem_Title, LS(MSG_MENUITEM_ABOUT, "About..."),
				MUIA_Menuitem_Shortcut, "?",
				MUIA_UserData, MENUITEM_ABOUT,
			TAG_END),
			MUIA_Family_Child, MUI_NewObjectM(MUIC_Menuitem,
				MUIA_Menuitem_Title, -1,
			TAG_END),
			MUIA_Family_Child, MUI_NewObjectM(MUIC_Menuitem,
				MUIA_Menuitem_Title, LS(MSG_MENUITEM_QUIT, "Quit"),
				MUIA_Menuitem_Shortcut, "Q",
				MUIA_UserData, MENUITEM_QUIT,
			TAG_END),
		TAG_END),
	TAG_END);

	return menu;
}


//==============================================================================================
// HandleLoadError()
//==============================================================================================

static void HandleLoadError(LuaState *L, LONG error)
{
	const char *errmsg;

	switch (error)
	{
		case LUA_ERRMEM:
			// have no smart idea what to do here
		break;

		case LUA_ERRSYNTAX:
			errmsg = LuaToString(L, -1);
			Printf("Syntax error at %s.\n", (IPTR)errmsg);    // requester? status bar?
		break;

		case LUA_ERRERR:
			Printf("Script file not found. Check installation.\n");
		break;

		default:
			Printf("Unknown loading error %ld. Please report a bug.\n", error);   // requester? status bar?
		break;
	}
}


//==============================================================================================
// HandleRunError()
//==============================================================================================
static void HandleRunError(LuaState *L, LONG error)
{
	const char *errmsg;

	switch (error)
	{
		case LUA_ERRMEM:
			// have no smart idea what to do here
		break;

		case LUA_ERRRUN:
			errmsg = LuaToString(L, -1);
			Printf("Runtime error at %s.\n", (IPTR)errmsg);          // requester ?
		break;

		default:
			Printf("Unknown runtime error %ld. Please report a bug.\n", error);   // requester ?
		break;
	}
}


//==============================================================================================
// STPutChar()
//==============================================================================================

HOOKPROTONO(STPutChar, void, char c)
{
	char *p;

	p = (char*)hook->h_Data;
	*p++ = c;
	hook->h_Data = (APTR)p;
}


//==============================================================================================
// SetToday()
//==============================================================================================

static void SetToday(Object *datestr)
{
	struct DateStamp ds;
	char dbuf[12];
	MakeHookWithData(STPutCharHook, STPutChar, dbuf);

	DateStamp(&ds);
	FormatDate(Loc, (STRPTR)"%d.%m.%Y", &ds, &STPutCharHook);
	XSet(datestr, MUIA_String_Contents, dbuf);
}



//==============================================================================================
// MergeDrawerWithFile()
//==============================================================================================

static char* MergeDrawerWithFile(char *drawer, char *file)
{
	char *path;
	LONG pathlen;

	pathlen = StrLen(drawer) + StrLen(file) + 8;

	if ((path = AllocVecTaskPooled(pathlen)) != NULL)
	{
		StrCopy(drawer, path);
		if (AddPart(path, file, pathlen)) return path;

		FreeVecTaskPooled(path);
	}

	return NULL;
}



//==============================================================================================
// ScanScripts()
//==============================================================================================

// NOTE: Allocated list nodes are freed in this function. Allocated strings are moved from list
// nodes to the returned array, then strings and the array itself are deallocated in OM_DISPOSE
// of this class with FreeStrArray().

STRPTR* ScanScripts(void)
{
	STRPTR *a = NULL;
	BPTR script_dir;
	struct FileInfoBlock *fib;
	struct List string_list;
	struct Node *n;
	LONG counter = 0, err;

	string_list.lh_Head = (struct Node*)&string_list.lh_Tail;
	string_list.lh_Tail = NULL;
	string_list.lh_TailPred = (struct Node*)&string_list.lh_Head;

	SetIoErr(0);

	if ((fib = AllocDosObjectTags(DOS_FIB, TAG_END)) != NULL)
	{
		if ((script_dir = Lock("PROGDIR:scripts", SHARED_LOCK)) != (BPTR)NULL)
		{
			if (Examine(script_dir, fib))
			{
				while (ExNext(script_dir, fib))
				{
					#ifdef __amigaos4__
					if (fib->fib_DirEntryType > 0)
					#else
					if (fib->fib_EntryType > 0)
					#endif
					{
						if ((n = AllocTaskPooled(sizeof(struct Node))) != NULL)
						{
							if ((n->ln_Name = StrNew(fib->fib_FileName)) != NULL)
							{
								AddTail(&string_list, n);
								counter++;
							}
							else
							{
								SetIoErr(ERROR_NO_FREE_STORE);
								break;
							}
						}
						else
						{
							SetIoErr(ERROR_NO_FREE_STORE);
							break;
						}
					}
				}

				if (IoErr() == ERROR_NO_MORE_ENTRIES) SetIoErr(0);
			}

			UnLock(script_dir);
		}

		FreeDosObject(DOS_FIB, fib);
	}
	else SetIoErr(ERROR_NO_FREE_STORE);

	#warning Display a requester?

	if ((err = IoErr()) != 0)
	{
		PrintFault(err, "LibMaker");
		return NULL;
	}

	if (counter > 0)
	{
		if ((a = AllocVecTaskPooled(sizeof(STRPTR) * (counter + 1))) != NULL)
		{
			LONG i;

			for (i  = 0; i < counter; i++)
			{
				n = RemHead(&string_list);
				a[i] = n->ln_Name;
				FreeTaskPooled(n, sizeof(struct Node));
			}

			a[i] = NULL;
		}
	}

	return a;
}



//==============================================================================================
// ApplicationNew()
//==============================================================================================

IPTR ApplicationNew(Class *cl, Object *obj, struct opSet *msg)
{
	struct AppObjects objs;

	if ((objs.Scripts = ScanScripts()) != NULL)
	{
		if ((obj = DoSuperNewM(cl, obj,
			MUIA_Application_Window, objs.WndMain = CreateMainWindow(&objs),
			MUIA_Application_Window, objs.WndFunctionEditor = NewObjectM(FunctionEditorClass->mcc_Class, NULL,
			TAG_END),
			MUIA_Application_Window, objs.WndClassEditor = NewObjectM(ClassEditorClass->mcc_Class, NULL,
			TAG_END),
			MUIA_Application_Window, objs.WndAbout = MUI_NewObjectM(MUIC_Aboutbox,
				MUIA_Aboutbox_Credits, APP_ABOUT,
				#if defined(__MORPHOS__)
				MUIA_Aboutbox_Build, __SVNVERSION__,
				#else
				MUIA_Aboutbox_Build, APP_VER,
				#endif
			TAG_END),
			MUIA_Application_Menustrip, CreateAppMenu(),
		TAG_MORE, msg->ops_AttrList)) != NULL)
		{
			struct ObjData *d = INST_DATA(cl, obj);

			SetToday(objs.StrLibDate);
			d->Objects = objs;

			if ((d->FileReq = MUI_AllocAslRequestTags(ASL_FileRequest, TAG_END)) != NULL)
			{
				return (IPTR)obj;
			}

			CoerceMethod(cl, obj, OM_DISPOSE);
		}
	}

	return 0;
}



//==============================================================================================
// ApplicationSet()
//==============================================================================================

IPTR ApplicationSet(Class *cl, Object *obj, struct opSet *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	struct TagItem *tag, *tagptr;
	IPTR tagcnt = 0;

	tagptr = msg->ops_AttrList;

	while ((tag = NextTagItem(&tagptr)) != NULL)
	{
		switch (tag->ti_Tag)
		{
			case APPA_ProjectName:
				XSet(d->Objects.StrLibraryName, MUIA_String_Contents, tag->ti_Data);
				tagcnt++;
			break;

			case APPA_ProjectVersion:
				XSet(d->Objects.StrLibVersion, MUIA_String_Integer, tag->ti_Data);
				tagcnt++;
			break;

			case APPA_ProjectRevision:
				XSet(d->Objects.StrLibRevision, MUIA_String_Integer, tag->ti_Data);
				tagcnt++;
			break;

			case APPA_ProjectDate:
				XSet(d->Objects.StrLibDate, MUIA_String_Contents, tag->ti_Data);
				tagcnt++;
			break;

			case APPA_ProjectCopyright:
				XSet(d->Objects.StrLibCopyrigt, MUIA_String_Contents, tag->ti_Data);
				tagcnt++;
			break;

			case APPA_ProjectBaseName:
				XSet(d->Objects.StrLibBaseName, MUIA_String_Contents, tag->ti_Data);
				tagcnt++;
			break;

			case APPA_UseAltiVec:
				XSet(d->Objects.ChkAltiVec, MUIA_Selected, tag->ti_Data);
				tagcnt++;
			break;

			case APPA_BoopsiClass:
				XSet(d->Objects.ChkBoopsi, MUIA_Selected, tag->ti_Data);
				tagcnt++;
			break;

			case APPA_MuiClass:
				XSet(d->Objects.ChkMui, MUIA_Selected, tag->ti_Data);
				tagcnt++;
			break;
		}
	}

	tagcnt += DoSuperMethodA(cl, obj, (Msg)msg);
	return tagcnt;
}



//==============================================================================================
// ApplicationDispose()
//==============================================================================================

IPTR ApplicationDispose(Class *cl, Object *obj, Msg msg)
{
	struct ObjData *d = INST_DATA(cl, obj);

	if (d->Objects.Scripts) FreeStrArray(d->Objects.Scripts);
	if (d->FileReq) MUI_FreeAslRequest(d->FileReq);
	return DoSuperMethodA(cl, obj, msg);
}



//==============================================================================================
// ApplicationGet()
//==============================================================================================

IPTR ApplicationGet(Class *cl, Object *obj, struct opGet *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	BOOL result = TRUE;

	switch (msg->opg_AttrID)
	{
		case APPA_ProjectName:
			*(STRPTR*)msg->opg_Storage = (STRPTR)XGet(d->Objects.StrLibraryName, MUIA_String_Contents);
		break;

		default:
			result = DoSuperMethodA(cl, obj, (Msg)msg);
		break;
	}

	return (IPTR)result;
}



//==============================================================================================
// ApplicationMainLoop()
//==============================================================================================

IPTR ApplicationMainLoop(Class *cl, Object *obj)
{
	struct ObjData *d = INST_DATA(cl, obj);
	ULONG signals;

	set(d->Objects.WndMain, MUIA_Window_Open, TRUE);

	while (DoMethod(obj, MUIM_Application_NewInput, (ULONG)&signals)
	 != (ULONG)MUIV_Application_ReturnID_Quit)
	{
		if (signals)
		{
			signals = Wait(signals | SIGBREAKF_CTRL_C);
			if (signals & SIGBREAKF_CTRL_C) break;
		}
	}

	set(d->Objects.WndMain, MUIA_Window_Open, FALSE);
	return 0;
}


//==============================================================================================
// ApplicationNotifications()
//==============================================================================================

IPTR ApplicationNotifications(Class *cl, Object *obj)
{
	struct ObjData *d = INST_DATA(cl, obj);

	/* Quit application on main window close. */

	DoMethod(d->Objects.WndMain, MUIM_Notify,
		MUIA_Window_CloseRequest, TRUE,
		MUIV_Notify_Application, 2,
		MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	/* Quit application on "Quit" menu item selection. */

	DoMethod(obj, MUIM_Notify,
		MUIA_Application_MenuAction, MENUITEM_QUIT,
		MUIV_Notify_Application, 2,
		MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	/* Display "About" window on "About" menu item selection. */

	DoMethod(obj, MUIM_Notify,
		MUIA_Application_MenuAction, MENUITEM_ABOUT,
		(IPTR)d->Objects.WndAbout, 3,
		MUIM_Set, MUIA_Window_Open, TRUE);

	/* Generate code on click of "Generate Code" button. */

	DoMethod(d->Objects.BtnGenerateCode, MUIM_Notify,
		MUIA_Pressed, FALSE,
		(IPTR)obj, 1,
		APPM_GenerateCode);

	/* Open function editor with existing entry on doubleclick on function list. */

	DoMethod(d->Objects.LstFunctions, MUIM_Notify,
		MUIA_List_DoubleClick, MUIV_EveryTime,
		(IPTR)obj, 2,
		APPM_OpenFunctionEditor, TRUE);

	/* Open function editor with a new entry on click on "Add" button. */

	DoMethod(d->Objects.BtnAddFunction, MUIM_Notify,
		MUIA_Pressed, FALSE,
		(IPTR)obj, 2,
		APPM_OpenFunctionEditor, FALSE);

	/* Delete a function on click on "Delete" button. */

	DoMethod(d->Objects.BtnDelFunction, MUIM_Notify,
		MUIA_Pressed, FALSE,
		(IPTR)d->Objects.LstFunctions, 2,
		MUIM_List_Remove, MUIV_List_Remove_Active);

	/* Save project to the current location on "Save" menu item. */

	DoMethod(obj, MUIM_Notify,
		MUIA_Application_MenuAction, MENUITEM_SAVE,
		MUIV_Notify_Application, 1,
		APPM_SaveProject);

	/* Save project with file requester on "Save As..." menu item. */

	DoMethod(obj, MUIM_Notify,
		MUIA_Application_MenuAction, MENUITEM_SAVE_AS,
		MUIV_Notify_Application, 1,
		APPM_SaveProjectByReq);

	/* Open project with file requester on "Open..." menu item. */

	DoMethod(obj, MUIM_Notify,
		MUIA_Application_MenuAction, MENUITEM_OPEN,
		MUIV_Notify_Application, 1,
		APPM_OpenProjectByReq);

	/* Switch off "BOOPSI Class" checkmark if "MUI Class" is checked. */

	DoMethod(d->Objects.ChkMui, MUIM_Notify,
		MUIA_Selected, TRUE,
		(IPTR)d->Objects.ChkBoopsi, 3,
		MUIM_Set, MUIA_Selected, FALSE);

	/* Switch off "MUI Class" checkmark if "BOOPSI Class" is checked. */

	DoMethod(d->Objects.ChkBoopsi, MUIM_Notify,
		MUIA_Selected, TRUE,
		(IPTR)d->Objects.ChkMui, 3,
		MUIM_Set, MUIA_Selected, FALSE);

	/* Enable "BOOPSI Class" button when its checkmark is checked. */

	DoMethod(d->Objects.ChkBoopsi, MUIM_Notify,
		MUIA_Selected, MUIV_EveryTime,
		(IPTR)d->Objects.BtnBoopsi, 3,
		MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

	/* Open class editor when "BOOPSI Class" button is clicked */

	DoMethod(d->Objects.BtnBoopsi, MUIM_Notify,
		MUIA_Pressed, FALSE,
		MUIV_Notify_Application, 1,
		APPM_OpenClassEditor);

	return 0;
}


//==============================================================================================
// ApplicationGenerateCode()
//==============================================================================================


#define PushStringItem(L, key, val) LuaPushLiteral(L, key); LuaPushString(L, val); LuaSetTable(L, -3)
#define PushIntegerItem(L, key, val) LuaPushLiteral(L, key); LuaPushInteger(L, val); LuaSetTable(L, -3)
#define PushBooleanItem(L, key, val) LuaPushLiteral(L, key); LuaPushBoolean(L, val); LuaSetTable(L, -3)


struct LuaLibReg RegFuncs[2] = {
	{ "makedir", LibMakerMakeDir },
	{ NULL, NULL }
};

#if !defined(__MORPHOS__)

struct LuaMemoryData
{
    char *Buffer;  // set to the memory block containing code
    LONG Length;   // set to the block length in bytes
};

typedef struct LuaMemoryReaderData
{
	char* Block;
	LONG Length;
	LONG Position;
} LuaMemoryReaderData;


typedef struct LuaFileReaderData
{
	BPTR Handle;
    char *Buffer;
} LuaFileReaderData;

const char* memory_reader(UNUSED LuaState *s, APTR data, LONG *size)
{
        struct LuaMemoryReaderData *lmrd = (struct LuaMemoryReaderData*)data;

        *size = lmrd->Length - lmrd->Position;
        lmrd->Position += *size;
        return lmrd->Block;
}

const char* file_reader(UNUSED LuaState *s, APTR data, LONG *size)
{
        struct LuaFileReaderData *lfrd = (struct LuaFileReaderData*)data;

        *size = Read(lfrd->Handle, lfrd->Buffer, CODE_BUFFER_SIZE);
        if (*size < 0) *size = 0;
        return lfrd->Buffer;
}


LONG LuaLoad(LuaState *ls, LuaReader reader, APTR data, const char *name)
{
        LONG result = 0;
		APTR pool;
		pool = CreatePool(MEMF_ANY, 16384, 16384);

        if (reader == LUA_READER_MEMORY)
        {
                struct LuaMemoryData *lmd = (struct LuaMemoryData*)data;
                struct LuaMemoryReaderData lmrd;

                lmrd.Block = (char*)lmd->Buffer;
                lmrd.Length = lmd->Length;
                lmrd.Position = 0;

                result = lua_load(ls, (lua_Reader)memory_reader, &lmrd, name);
        }
        else if (reader == LUA_READER_FILE)
        {
                struct LuaFileReaderData lfrd;
                char *fname = (char*)data;

				if ((lfrd.Buffer = (char*)AllocPooled(pool, CODE_BUFFER_SIZE)) != NULL)
				{
                        BOOL close_input = FALSE;

                        if ((!fname) || (!(*fname))) lfrd.Handle = Input();
                        else
                        {
                                lfrd.Handle = Open((STRPTR)fname, MODE_OLDFILE);
                                close_input = TRUE;
                        }

                        if (lfrd.Handle)
                        {
                                result = lua_load(ls, (lua_Reader)file_reader, &lfrd, name);
                                if (close_input) Close(lfrd.Handle);
                        }
                        else result = LUA_ERRERR;

						FreePooled(pool, lfrd.Buffer, CODE_BUFFER_SIZE);
						DeletePool(pool);
                }
                else result = LUA_ERRMEM;
        }
        else result = lua_load(ls, (lua_Reader)reader, data, name);

        return result;
}
#endif


IPTR ApplicationGenerateCode(Class *cl, Object *obj)
{
	struct ObjData *d = INST_DATA(cl, obj);

	LuaState *L;

	LONG load_result;
	LONG run_result;
	LONG script_set;
	BPTR destdir, olddir;
	STRPTR script_path;

	script_set = XGet(d->Objects.CycSrciptSet, MUIA_Cycle_Active);

	if ((script_path = FmtNew("PROGDIR:scripts/%s/libgen.lua", d->Objects.Scripts[script_set])) != NULL)
	{
		#if defined(__MORPHOS__)
		if ((L = LuaNewState(NULL, NULL)) != NULL)
		#else
		if ((L = LuaNewState()) != NULL)
		#endif
		{
			#if defined(__MORPHOS__)
			LuaRegisterModule(L, "libmaker", RegFuncs);
			#else
			// on morphos LuaNewState() contain luaL_openlibs() for init necessary inbuild modules, doing it manually
			luaL_openlibs(L);
			// equvalent of morphos's LuaRegisterModule()
			luaL_openlib(L,"libmaker", RegFuncs,0);
			#endif

			load_result = LuaLoad(L, LUA_READER_FILE, script_path, "@libgen");

			if (load_result == 0)
			{
				struct FunctionEntry *fe;
				LONG i;

				/* Getting arguments from GUI and pushing them into a Lua table. */

				LuaNewTable(L);

				PushStringItem(L, "scriptdir", d->Objects.Scripts[script_set]);
				PushStringItem(L, "instdir", (char*)XGet(d->Objects.StrInstDir, MUIA_String_Contents));
				PushStringItem(L, "incpath", (char*)XGet(d->Objects.StrIncludePath, MUIA_String_Contents));
				PushBooleanItem(L, "sep_functions", XGet(d->Objects.ChkSeparatedFunctions, MUIA_Selected));
				PushBooleanItem(L, "sep_methods", XGet(d->Objects.ChkSeparatedMethods, MUIA_Selected));
				PushStringItem(L, "name", (char*)XGet(d->Objects.StrLibraryName, MUIA_String_Contents));
				PushIntegerItem(L, "ver", XGet(d->Objects.StrLibVersion, MUIA_String_Integer));
				PushIntegerItem(L, "rev", XGet(d->Objects.StrLibRevision, MUIA_String_Integer));
				PushStringItem(L, "date", (char*)XGet(d->Objects.StrLibDate, MUIA_String_Contents));
				PushStringItem(L, "copyright", (char*)XGet(d->Objects.StrLibCopyrigt, MUIA_String_Contents));
				PushStringItem(L, "basename", (char*)XGet(d->Objects.StrLibBaseName, MUIA_String_Contents));
				PushBooleanItem(L, "altivec", XGet(d->Objects.ChkAltiVec, MUIA_Selected));
				PushBooleanItem(L, "boopsi", XGet(d->Objects.ChkBoopsi, MUIA_Selected));
				PushBooleanItem(L, "mui", XGet(d->Objects.ChkMui, MUIA_Selected));

				/* table of functions, indexes are ordinal numbers */

				LuaPushLiteral(L, "functions");
				LuaNewTable(L);

				for (i = 0;; i++)
				{
					LONG arg;

					DoMethod(d->Objects.LstFunctions, MUIM_List_GetEntry, i, (IPTR)&fe);
					if (!fe) break;
					LuaPushInteger(L, i + 1);
					LuaNewTable(L);
					PushStringItem(L, "name", fe->fe_Name);
					PushStringItem(L, "type", fe->fe_ReturnType);

					/* table of function arguments, indexes are ordinal numbers */

					LuaPushLiteral(L, "args");
					LuaNewTable(L);

					for (arg = 0; arg < fe->fe_ArgCount; arg++)
					{
						LuaPushInteger(L, arg + 1);
						LuaNewTable(L);
						PushStringItem(L, "name", fe->fe_Arguments[arg].fa_Name);
						PushStringItem(L, "type", fe->fe_Arguments[arg].fa_Type);
						PushIntegerItem(L, "m68kreg", fe->fe_Arguments[arg].fa_Register);
						LuaSetTable(L, -3);      // set argument spec table as item of 'args'
					}

					LuaSetTable(L, -3);      // set table 'args' as item of function spec table
					LuaSetTable(L, -3);      // set function spec table as item of 'functions'
				}

				LuaSetTable(L, -3);      // set table 'functions' as item of main table

				/* class table */

				if (XGet(d->Objects.ChkBoopsi, MUIA_Selected) || XGet(d->Objects.ChkMui, MUIA_Selected))
				{
					DoMethod(d->Objects.WndClassEditor, CEDM_PushToLua, (IPTR)L);
				}

				LuaSetGlobal(L, "spec");

				/* Change current directory to the code destination. */

				if ((destdir = Lock((STRPTR)XGet(d->Objects.StrDestDir, MUIA_String_Contents), SHARED_LOCK)) != (BPTR)NULL)
				{
					olddir = CurrentDir(destdir);

					/* Call the stuff. */

					run_result = LuaPCall(L, 0, 1, 0);
					if (run_result) HandleRunError(L, run_result);
					CurrentDir(olddir);
					UnLock(destdir);
				}
				else PrintFault(IoErr(), "Destination directory problem");
			}
			else HandleLoadError(L, load_result);

			LuaClose(L);
		}

		FmtFree(script_path);
	}

	return 0;
}


//==============================================================================================
// ApplicationOpenFunctionEditor()
//==============================================================================================

IPTR ApplicationOpenFunctionEditor(Class *cl, Object *obj, struct APPP_OpenFunctionEditor *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);

	XSet(d->Objects.WndMain, MUIA_Window_Sleep, TRUE);

	if (msg->Doubleclicked)
	{
		struct FunctionEntry *fed;

		DoMethod(d->Objects.LstFunctions, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, (IPTR)&fed);

		SetAttrs(d->Objects.WndFunctionEditor,
			FEDA_FunctionName, (IPTR)fed->fe_Name,
			FEDA_ResultType, (IPTR)fed->fe_ReturnType,
			FEDA_LibraryName, (IPTR)XGet(d->Objects.StrLibraryName, MUIA_String_Contents),
		TAG_END);

		/*-----------------*/
		/* Send arguments. */
		/*-----------------*/

		DoMethod(d->Objects.WndFunctionEditor, FEDM_SetArgumentList, fed->fe_ArgCount, (IPTR)fed->fe_Arguments);
	}
	else                                       // new entry by "Add" button
	{
		SetAttrs(d->Objects.WndFunctionEditor,
			FEDA_FunctionName, (IPTR)"",
			FEDA_ResultType, (IPTR)"",
		TAG_END);

		/*-----------------------------------------------------------------------------------*/
		/* Clear list cursor, so APPM_CloseFunctionEditor() knows it should add a new entry. */
		/*-----------------------------------------------------------------------------------*/

		XSet(d->Objects.LstFunctions, MUIA_List_Active, MUIV_List_Active_Off);

		/*--------------------------*/
		/* Set empty argument list. */
		/*--------------------------*/

		DoMethod(d->Objects.WndFunctionEditor, FEDM_SetArgumentList, 0, NULL);
	}

	XSet(d->Objects.WndFunctionEditor, MUIA_Window_Open, TRUE);

	return 0;
}


//==============================================================================================
// ApplicationCloseFunctionEditor()
//==============================================================================================

IPTR ApplicationCloseFunctionEditor(Class *cl, Object *obj, struct APPP_CloseFunctionEditor *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	LONG active_entry;
	BOOL have_args = FALSE;

	XSet(d->Objects.WndFunctionEditor, MUIA_Window_Open, FALSE);

	if (msg->DoSave == APPV_CloseFunctionEditor_Save)
	{
		struct FunctionEntry fe;
		struct FunctionArgument *fargs = NULL;
		LONG arg_index;

		active_entry = XGet(d->Objects.LstFunctions, MUIA_List_Active);
		fe.fe_Name = (STRPTR)XGet(d->Objects.WndFunctionEditor, FEDA_FunctionName);
		fe.fe_ReturnType = (STRPTR)XGet(d->Objects.WndFunctionEditor, FEDA_ResultType);
		fe.fe_ArgCount = XGet(d->Objects.WndFunctionEditor, FEDA_NumArgs);

		if (fe.fe_ArgCount == 0)
		{
			fe.fe_Arguments = NULL;
			have_args = TRUE;
		}
		else
		{
			if ((fargs = AllocVecTaskPooled(fe.fe_ArgCount * sizeof(struct FunctionArgument))) != NULL)
			{
				/*-------------------------*/
				/* Get function arguments. */
				/*-------------------------*/

				for (arg_index = 0; arg_index < fe.fe_ArgCount; arg_index++)
				{
					fargs[arg_index].fa_Name = (STRPTR)DoMethod(d->Objects.WndFunctionEditor, FEDM_GetArgumentName, arg_index);
					fargs[arg_index].fa_Type = (STRPTR)DoMethod(d->Objects.WndFunctionEditor, FEDM_GetArgumentType, arg_index);
					fargs[arg_index].fa_Register = DoMethod(d->Objects.WndFunctionEditor, FEDM_GetArgumentRegister, arg_index);
				}

				fe.fe_Arguments = fargs;
				have_args = TRUE;
			}
		}

		if (have_args)
		{
			/*--------------*/
			/* Update list. */
			/*--------------*/

			if (active_entry != MUIV_List_Active_Off)          // update existing function
			{
				XSet(d->Objects.LstFunctions, MUIA_List_Quiet, TRUE);
				DoMethod(d->Objects.LstFunctions, MUIM_List_Remove, active_entry);
				DoMethod(d->Objects.LstFunctions, MUIM_List_InsertSingle, (IPTR)&fe, active_entry);
				XSet(d->Objects.LstFunctions, MUIA_List_Quiet, FALSE);
			}
			else                                               // add new function
			{
				DoMethod(d->Objects.LstFunctions, MUIM_List_InsertSingle, (IPTR)&fe, MUIV_List_Insert_Bottom);
			}
		}

		if (fargs) FreeVecTaskPooled(fargs);

	}

	XSet(d->Objects.WndMain, MUIA_Window_Sleep, FALSE);

	return 0;
}



//==============================================================================================
// ApplicationSaveProjectByName()
//==============================================================================================

IPTR ApplicationSaveProjectByName(Class *cl, Object *obj, struct APPP_SaveProjectByName *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	BPTR file;
	LONG function, argument, error;
	BOOL mui, boopsi;
	struct FunctionEntry *fe;

	SetIoErr(0);

	if ((file = Open(msg->Path, MODE_NEWFILE)) != (BPTR)NULL)
	{
		boopsi = XGet(d->Objects.ChkBoopsi, MUIA_Selected);
		mui = XGet(d->Objects.ChkMui, MUIA_Selected);

		FPuts(file, "LIBMAKER " APP_VER "\n");
		FPrintf(file, "LIBRARY NAME=\"%s\" BASE=\"%s\" VER=%ld REV=%ld DATE=\"%s\" COPYRIGHT=\"%s\"",
			XGet(d->Objects.StrLibraryName, MUIA_String_Contents),
			XGet(d->Objects.StrLibBaseName, MUIA_String_Contents),
			XGet(d->Objects.StrLibVersion, MUIA_String_Integer),
			XGet(d->Objects.StrLibRevision, MUIA_String_Integer),
			XGet(d->Objects.StrLibDate, MUIA_String_Contents),
			XGet(d->Objects.StrLibCopyrigt, MUIA_String_Contents));

		if (XGet(d->Objects.ChkAltiVec, MUIA_Selected)) FPuts(file, " ALTIVEC");
		if (boopsi) FPuts(file, " BOOPSI");
		if (mui) FPuts(file, " MUI");

		FPuts(file, "\n");

		/* Code generator settings. */

		FPrintf(file, "GENERATOR TYPE=\"%s\" DEST=\"%s\" INSTALL=\"%s\" INCDIR=\"%s\"",
			(IPTR)d->Objects.Scripts[XGet(d->Objects.CycSrciptSet, MUIA_Cycle_Active)],
			XGet(d->Objects.StrDestDir, MUIA_String_Contents),
			XGet(d->Objects.StrInstDir, MUIA_String_Contents),
			XGet(d->Objects.StrIncludePath, MUIA_String_Contents));
		if (XGet(d->Objects.ChkSeparatedFunctions, MUIA_Selected)) FPuts(file, " SFUNCTIONS");
		if (XGet(d->Objects.ChkSeparatedMethods, MUIA_Selected)) FPuts(file, " SMETHODS");
		FPuts(file, "\n");

		for (function = 0; TRUE; function++)
		{
			DoMethod(d->Objects.LstFunctions, MUIM_List_GetEntry, function, (IPTR)&fe);

			if (fe)
			{
				StrReplace('*', '^', fe->fe_ReturnType);
				FPrintf(file, "FUNCTION NAME=\"%s\" RETTYPE=\"%s\"\n", (IPTR)fe->fe_Name, (IPTR)fe->fe_ReturnType);
				StrReplace('^', '*', fe->fe_ReturnType);

				for (argument = 0; argument < fe->fe_ArgCount; argument++)
				{
					struct FunctionArgument *fa = &fe->fe_Arguments[argument];

					StrReplace('*', '^', fa->fa_Type);
					FPrintf(file, "ARGUMENT TYPE=\"%s\" NAME=\"%s\"", (IPTR)fa->fa_Type, (IPTR)fa->fa_Name);
					StrReplace('^', '*', fa->fa_Type);

					if (fa->fa_Register < 8) FPrintf(file, " M68KREG=\"D%ld\"\n", fa->fa_Register);
					else if (fa->fa_Register < 12) FPrintf(file, " M68KREG=\"A%ld\"\n", fa->fa_Register - 8);
					else FPrintf(file, "\n");

				}

				FPuts(file, "ENDFUNCTION\n");
			}
			else break;
		}

		if (boopsi || mui) DoMethod(d->Objects.WndClassEditor, CEDM_WriteClassSpec, file);
		FPuts(file, "ENDLIBRARY\n");
		Close(file);
	}

	if ((error = IoErr()) != 0)
	{
		char fault[128];

		Fault(error, LS(MSG_SAVING_ERROR, "Project saving error"), fault, 128);
		MUI_Request(obj, d->Objects.WndMain, 0, APP_NAME, "*_OK", "%s.", (IPTR)fault);
	}

	return 0;
}



//==============================================================================================
// ApplicationSaveProjectByReq()
//==============================================================================================

IPTR ApplicationSaveProjectByReq(Class *cl, Object *obj)
{
	struct ObjData *d = INST_DATA(cl, obj);
	char *generated_filename = NULL;

	/*---------------------------------------------------------------*/
	/* Create a default filename if requester has no filename given. */
	/*---------------------------------------------------------------*/

	if (*d->FileReq->fr_File == 0x00)
	{
		generated_filename = FmtNew("%s.libmaker", XGet(d->Objects.StrLibraryName, MUIA_String_Contents));
	}

	/*--------------------------*/
	/* Ask for file and drawer. */
	/*--------------------------*/

	if (MUI_AslRequestTags(d->FileReq,
		ASLFR_Window, (IPTR)_window(d->Objects.WndMain),
		ASLFR_TitleText, (IPTR)LS(MSG_SAVEREQ_TITLE, "Save Project"),
		ASLFR_PositiveText, (IPTR)LS(MSG_SAVEREQ_SAVE_BUTTON, "_Save"),
		(generated_filename ? ASLFR_InitialFile : TAG_IGNORE), (IPTR)generated_filename,
		ASLFR_DoSaveMode, TRUE,
	TAG_END))
	{
		char *full_path;

		if ((full_path = MergeDrawerWithFile(d->FileReq->fr_Drawer, d->FileReq->fr_File)) != NULL)
		{
			DoMethod(obj, APPM_SaveProjectByName, (IPTR)full_path);
			FreeVecTaskPooled(full_path);
		}
	}

	if (generated_filename) FmtFree(generated_filename);
	return 0;
}


//==============================================================================================
// ApplicationSaveProject()
//==============================================================================================

IPTR ApplicationSaveProject(Class *cl, Object *obj)
{
	struct ObjData *d = INST_DATA(cl, obj);

	/*------------------------------------------------------------------------------------*/
	/* If requester file is not empty, use requester name and drawer and save silently.   */
	/* If there is no file name in the requester, pop up the requester and save from it.  */
	/*------------------------------------------------------------------------------------*/

	if (*d->FileReq->fr_File == 0x00) DoMethod(obj, APPM_SaveProjectByReq);
	else
	{
		char *full_path;

		if ((full_path = MergeDrawerWithFile(d->FileReq->fr_Drawer, d->FileReq->fr_File)) != NULL)
		{
			DoMethod(obj, APPM_SaveProjectByName, (IPTR)full_path);
			FreeVecTaskPooled(full_path);
		}
	}

	return 0;
}



//==============================================================================================
// ApplicationOpenProjectByReq()
//==============================================================================================

IPTR ApplicationOpenProjectByReq(Class *cl, Object *obj)
{
	struct ObjData *d = INST_DATA(cl, obj);

	/*--------------------------*/
	/* Ask for file and drawer. */
	/*--------------------------*/

	if (MUI_AslRequestTags(d->FileReq,
		ASLFR_Window, (IPTR)_window(d->Objects.WndMain),
		ASLFR_TitleText, (IPTR)LS(MSG_OPENREQ_TITLE, "Open Project"),
		ASLFR_PositiveText, (IPTR)LS(MSG_OPENREQ_OPEN_BUTTON, "_Open"),
	TAG_END))
	{
		char *full_path;

		if ((full_path = MergeDrawerWithFile(d->FileReq->fr_Drawer, d->FileReq->fr_File)) != NULL)
		{
			DoMethod(obj, APPM_OpenProjectByName, (IPTR)full_path);
			FreeVecTaskPooled(full_path);
		}
	}

	return 0;
}



//==============================================================================================
// ApplicationOpenProjectByName()
//==============================================================================================

IPTR ApplicationOpenProjectByName(Class *cl, Object *obj, struct APPP_OpenProjectByName *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);
	BPTR file;
	LONG error;

	SetIoErr(0);

	if ((file = Open(msg->Path, MODE_OLDFILE)) != (BPTR)NULL)
	{
		LibraryDefLoader(obj, d, file);
		Close(file);
	}

	if ((error = IoErr()) != 0)
	{
		if (error > 100)
		{
			char fault[128];

			Fault(error, LS(MSG_OPENING_ERROR, "Project opening error"), fault, 128);
			MUI_Request(obj, d->Objects.WndMain, 0, APP_NAME, "*_OK", "%s.", (IPTR)fault);
		}
		else
		{
			const char *message = "?";

			switch (error)
			{
				case LDLERR_SPECIAL_CHARS_IN_PATH:
					message = LS(MSG_ERROR_SPECIAL_CHARS_IN_PATH, "Code directory path contains special DOS characters.");
				break;

				case LDLERR_WRONG_SCRIPT_TYPE:
					message = LS(MSG_ERROR_NO_CODE_GENERATOR, "Specified code generator not found.");
				break;

				case LDLERR_VALUE_TOO_LONG:
					message = LS(MSG_ERROR_TEXT_STRING_TOO_LONG, "Error: text string too long.");
				break;

				case LDLERR_SYNTAX_ERROR:
					message = LS(MSG_ERROR_PROJECT_SYNTAX, "Syntax error in project file.");
				break;
			}

			MUI_Request(obj, d->Objects.WndMain, 0, APP_NAME, "*_OK", message);
		}
	}

	return 0;
}



//==============================================================================================
// ApplicationAddFunction()
//==============================================================================================

IPTR ApplicationAddFunction(Class *cl, Object *obj, struct APPP_AddFunction *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);

	DoMethod(d->Objects.LstFunctions, MUIM_List_InsertSingle, (IPTR)msg->Entry, MUIV_List_Insert_Bottom);
	return 0;
}



//==============================================================================================
// ApplicationOpenClassEditor()
//==============================================================================================

IPTR ApplicationOpenClassEditor(Class *cl, Object *obj)
{
	struct ObjData *d = INST_DATA(cl, obj);

	XSet(d->Objects.WndClassEditor, CEDA_LibraryName, (IPTR)XGet(d->Objects.StrLibraryName, MUIA_String_Contents));
	XSet(d->Objects.WndMain, MUIA_Window_Sleep, TRUE);
	XSet(d->Objects.WndClassEditor, MUIA_Window_Open, TRUE);
	return 0;
}



//==============================================================================================
// ApplicationCloseClassEditor()
//==============================================================================================

IPTR ApplicationCloseClassEditor(Class *cl, Object *obj, UNUSED struct APPP_CloseClassEditor *msg)
{
	struct ObjData *d = INST_DATA(cl, obj);

	XSet(d->Objects.WndClassEditor, MUIA_Window_Open, FALSE);
	XSet(d->Objects.WndMain, MUIA_Window_Sleep, FALSE);
	return 0;
}



//==============================================================================================
// ApplicationDispatcher()
//==============================================================================================

DISPATCHER(ApplicationDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW:                    return ApplicationNew(cl, obj, (struct opSet*)msg);
		case OM_SET:                    return ApplicationSet(cl, obj, (struct opSet*)msg);
		case OM_GET:                    return ApplicationGet(cl, obj, (struct opGet*)msg);
		case OM_DISPOSE:                return ApplicationDispose(cl, obj, msg);
		case APPM_Notifications:        return ApplicationNotifications(cl, obj);
		case APPM_MainLoop:             return ApplicationMainLoop(cl, obj);
		case APPM_GenerateCode:         return ApplicationGenerateCode(cl, obj);
		case APPM_OpenFunctionEditor:   return ApplicationOpenFunctionEditor(cl, obj, (struct APPP_OpenFunctionEditor*)msg);
		case APPM_CloseFunctionEditor:  return ApplicationCloseFunctionEditor(cl, obj, (struct APPP_CloseFunctionEditor*)msg);
		case APPM_SaveProjectByName:    return ApplicationSaveProjectByName(cl, obj, (struct APPP_SaveProjectByName*)msg);
		case APPM_SaveProjectByReq:     return ApplicationSaveProjectByReq(cl, obj);
		case APPM_SaveProject:          return ApplicationSaveProject(cl, obj);
		case APPM_OpenProjectByReq:     return ApplicationOpenProjectByReq(cl, obj);
		case APPM_OpenProjectByName:    return ApplicationOpenProjectByName(cl, obj, (struct APPP_OpenProjectByName*)msg);
		case APPM_AddFunction:          return ApplicationAddFunction(cl, obj, (struct APPP_AddFunction*)msg);
		case APPM_OpenClassEditor:      return ApplicationOpenClassEditor(cl, obj);
		case APPM_CloseClassEditor:     return ApplicationCloseClassEditor(cl, obj, (struct APPP_CloseClassEditor*)msg);
		default:                        return DoSuperMethodA(cl, obj, msg);
	}
}



//==============================================================================================
// ClearFunction()
//==============================================================================================

/*
   Clears all the fields of FunctionEntry and attached array of ArgumentEntries.
*/

static void ClearFunction(struct FunctionEntry *fe)
{
	LONG i;

	fe->fe_Name = NULL;
	fe->fe_ReturnType = NULL;
	fe->fe_ArgCount = 0;

	for (i = 0; i < MAX_ARGUMENT_COUNT; i++)
	{
		fe->fe_Arguments[i].fa_Name = NULL;
		fe->fe_Arguments[i].fa_Type = NULL;
	}
}



//==============================================================================================
// FlushFunction()
//==============================================================================================

/*
   Frees all the strings from FunctionEntry and attached array of ArgumentEntries.
   ClearFunction() should be called on FunctionEntry before loading data.
*/

static void FlushFunction(struct FunctionEntry *fe)
{
	LONG i;

	for (i = 0; i < MAX_ARGUMENT_COUNT; i++)
	{
		struct FunctionArgument *fa = &fe->fe_Arguments[i];

		if (fa->fa_Name) StrFree(fa->fa_Name);
		if (fa->fa_Type) StrFree(fa->fa_Type);
	}

	if (fe->fe_Name) StrFree(fe->fe_Name);
	if (fe->fe_ReturnType) StrFree(fe->fe_ReturnType);
}


//==============================================================================================
// FindScriptIndex()
//==============================================================================================

static LONG FindScriptIndex(STRPTR name, STRPTR *scripts)
{
	LONG i = 0;
	STRPTR script;

	while ((script = *scripts++) != NULL)
	{
		if (StrEqu(name, script)) return i;
		i++;
	}

	return -1;

}


//==============================================================================================
// NoSpecialChars()
//==============================================================================================

static BOOL NoSpecialChars(CONST_STRPTR path)
{
	const char *specials = "?#()|~%'[]";
	char c, d;
	CONST_STRPTR p;
	const char *s;

	p = path;

	while ((c = *p++) != '\0')
	{
		s = specials;

		while ((d = *s++) != '\0')
		{
			if (c == d) return FALSE;
		}
	}

	return TRUE;
}


//==============================================================================================
// M68kRegNameToNumber()
//==============================================================================================

static ULONG M68kRegNameToNumber(STRPTR regname)
{
	ULONG regnumber = M68K_REG_UNDEFINED;
	UBYTE c;

	if (regname)
	{
		if ((regname[0] == 'a') || (regname[0] == 'A'))
		{
			c = regname[1];
			if ((c >= '0') && (c <= '9')) regnumber = c - '0' + 8;
		}
		else if ((regname[0] == 'd') || (regname[0] == 'D'))
		{
			c = regname[1];
			if ((c >= '0') && (c <= '9')) regnumber = c - '0';
		}
	}

	return regnumber;
}


//==============================================================================================
// LoadArguments()
//==============================================================================================

static void LoadArguments(BPTR file, STRPTR line, struct FunctionEntry *fe)
{
	struct RDArgs *args, srcargs;

	while (!IoErr())
	{
		if (ReadLine(file, line))
		{
			LONG params[4] = {0, 0, 0, 0};

			if (StrEqu(line, "ENDFUNCTION\n")) break;

			if ((args = ParseLine(line, "ARGUMENT/S/A,TYPE/K/A,NAME/K/A,M68KREG/K", params, &srcargs)) != NULL)
			{
				STRPTR argtype, argname;

				argtype = (STRPTR)params[1];
				argname = (STRPTR)params[2];
				StrReplace('^', '*', argtype);
				fe->fe_Arguments[fe->fe_ArgCount].fa_Register = M68kRegNameToNumber((STRPTR)params[3]);

				if (StrLen(argtype) <= MAXLEN_TYPE_SPEC)
				{
					if (StrLen(argname) <= MAXLEN_ARGUMENT_NAME)
					{
						if ((fe->fe_Arguments[fe->fe_ArgCount].fa_Type = StrNew(argtype)) != NULL)
						{
							if ((fe->fe_Arguments[fe->fe_ArgCount].fa_Name = StrNew(argname)) != NULL)
							{
								fe->fe_ArgCount++;
							}
							else SetIoErr(ERROR_NO_FREE_STORE);
						}
						else SetIoErr(ERROR_NO_FREE_STORE);
					}
					else SetIoErr(LDLERR_VALUE_TOO_LONG);
				}
				else SetIoErr(LDLERR_VALUE_TOO_LONG);

				FreeArgs(args);
			}
		}
	}
}



//==============================================================================================
// LoadFunctions()
//==============================================================================================

static void LoadFunctions(Object *app, struct ObjData *d, BPTR file, STRPTR line, BOOL loadclass)
{
	struct FunctionEntry fe;
	struct FunctionArgument fa[MAX_ARGUMENT_COUNT];
	struct RDArgs *args, srcargs;
	BOOL have_generator_config = FALSE;

	fe.fe_Arguments = fa;

	while (!IoErr())
	{
		if (ReadLine(file, line))
		{
			LONG params[7] = { 0, 0, 0, 0, 0, 0, 0 };

			if (StrEqu(line, "ENDLIBRARY\n")) break;

			if (loadclass && StrNEqu(line, "CLASS ", 6))
			{
				DoMethod(d->Objects.WndClassEditor, CEDM_ReadClassSpec, (IPTR)file, (IPTR)line);
				continue;
			}

			if ((args = ParseLine(line, "GENERATOR/S/A,TYPE/K/A,DEST/K/A,INSTALL/K,INCDIR/K,SFUNCTIONS/S,SMETHODS/S", params, &srcargs)) != NULL)
			{
				if (!have_generator_config)
				{
					if ((StrLen((STRPTR)params[1]) <= 64)
					 && (StrLen((STRPTR)params[2]) <= 128)
					 && (StrLen((STRPTR)params[3]) <= 128)
					 && (StrLen((STRPTR)params[4]) <= 128))
					{
						LONG script_index;

						script_index = FindScriptIndex((STRPTR)params[1], d->Objects.Scripts);

						if (script_index >= 0)
						{
							if (NoSpecialChars((STRPTR)params[1]) && NoSpecialChars((STRPTR)params[2]) && NoSpecialChars((STRPTR)params[3]))
							{
								XSet(d->Objects.StrDestDir, MUIA_String_Contents, params[2]);
								XSet(d->Objects.CycSrciptSet, MUIA_Cycle_Active, script_index);
								XSet(d->Objects.StrInstDir, MUIA_String_Contents, params[3]);
								XSet(d->Objects.StrIncludePath, MUIA_String_Contents, params[4]);
								XSet(d->Objects.ChkSeparatedFunctions, MUIA_Selected, params[5] ? TRUE : FALSE);
								XSet(d->Objects.ChkSeparatedMethods, MUIA_Selected, params[6] ? TRUE : FALSE);
								have_generator_config = TRUE;
								continue;
							}
							else SetIoErr(LDLERR_SPECIAL_CHARS_IN_PATH);
						}
						else SetIoErr(LDLERR_WRONG_SCRIPT_TYPE);
					}
					else SetIoErr(LDLERR_VALUE_TOO_LONG);
				}
				else SetIoErr(LDLERR_SYNTAX_ERROR);
			}
			else SetIoErr(0);

			if ((args = ParseLine(line, "FUNCTION/S/A,NAME/K/A,RETTYPE/K/A", params, &srcargs)) != NULL)
			{
				STRPTR funcname, functype;

				funcname = (STRPTR)params[1];
				functype = (STRPTR)params[2];
				StrReplace('^', '*', functype);
				ClearFunction(&fe);

				if (StrLen(funcname) <= MAXLEN_FUNCTION_NAME)
				{
					if (StrLen(functype) <= MAXLEN_TYPE_SPEC)
					{
						if ((fe.fe_Name = StrNew(funcname)) != NULL)
						{
							if ((fe.fe_ReturnType = StrNew(functype)) != NULL)
							{
								LoadArguments(file, line, &fe);
								if (!IoErr()) DoMethod(app, APPM_AddFunction, (IPTR)&fe);
							}
							else SetIoErr(ERROR_NO_FREE_STORE);
						}
						else SetIoErr(ERROR_NO_FREE_STORE);
					}
					else SetIoErr(LDLERR_VALUE_TOO_LONG);
				}
				else SetIoErr(LDLERR_VALUE_TOO_LONG);

				FlushFunction(&fe);
				FreeArgs(args);
			}
		}
	}
}



//==============================================================================================
// LoadLibrary()
//==============================================================================================

static void LoadLibrary(Object *app, struct ObjData *d, BPTR file, STRPTR line)
{
	struct RDArgs *args, srcargs;
	CONST_STRPTR defbase = "Base";
	LONG params[10] = {0, 0, 0, 0, 0, 0, (LONG)defbase, 0, 0, 0};
	BOOL loadclass = FALSE;

	DoMethod(d->Objects.LstFunctions, MUIM_List_Clear);
	DoMethod(d->Objects.WndClassEditor, CEDM_Clear);

	if (ReadLine(file, line))
	{
		if ((args = ParseLine(line, "LIBRARY/S/A,NAME/K/A,VER/K/N/A,REV/K/N/A,DATE/K/A,COPYRIGHT/K/A,BASE/K,ALTIVEC/S,BOOPSI/S,MUI/S",
			params, &srcargs)) != NULL)
		{
			if ((StrLen((STRPTR)params[1]) <= MAXLEN_LIBRARY_NAME)
			 && (StrLen((STRPTR)params[4]) <= 10)
			 && (StrLen((STRPTR)params[5]) <= MAXLEN_COPYRIGHT_STRING)
			 && (StrLen((STRPTR)params[6]) <= MAXLEN_LIBRARY_NAME))
			{
				LONG ver = *(LONG*)params[2];
				LONG rev = *(LONG*)params[3];

				if ((ver >= 0) && (rev >= 0) && (ver < 65536) && (rev < 65536))
				{
					SetAttrs(app,
						APPA_ProjectName, params[1],
						APPA_ProjectVersion, ver,
						APPA_ProjectRevision, rev,
						APPA_ProjectDate, params[4],
						APPA_ProjectCopyright, params[5],
						APPA_ProjectBaseName, params[6],
						APPA_UseAltiVec, params[7],
						APPA_BoopsiClass, params[8],
						APPA_MuiClass, params[9],
					TAG_END);

					if (params[8] || params[9]) loadclass = TRUE;
					LoadFunctions(app, d, file, line, loadclass);
				}
				else SetIoErr(ERROR_BAD_NUMBER);
			}
			else SetIoErr(LDLERR_VALUE_TOO_LONG);

			FreeArgs(args);
		}
	}
}


//==============================================================================================
// LibraryDefLoader()
//==============================================================================================


static LONG LibraryDefLoader(Object *app, struct ObjData *d, BPTR file)
{
	char line[LINEBUFSIZE];

	if (ReadLine(file, line))
	{
		if (StrNEqu(line, "LIBMAKER ", 9)) LoadLibrary(app, d, file, line);
		else SetIoErr(LDLERR_SYNTAX_ERROR);
	}

	return IoErr();
}
