/* MethodEditorClass code. */


#include "main.h"
#include "application.h"
#include "methodeditor.h"
#include "methodlist.h"
#include "classeditor.h"
#include "libvstring/libvstring.h"

#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/locale.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>


struct MUI_CustomClass *MethodEditorClass;

DISPATCHERPROTO(MethodEditorDispatcher);

#if 0
static CONST_STRPTR TypicalTypes[] = {
	"VOID",
	"LONG",
	"STRPTR",
	"ULONG",
	"APTR",
	"BTPR",
	"IPTR",
	"BYTE",
	"UBYTE",
	"WORD",
	"UWORD",
	"FLOAT",
	"DOUBLE",
	"struct TagItem*",
	NULL
};
#endif


struct MEObjs
{
	Object *StrMethodName;
	Object *StrMethodId;
	Object *StrMethodStruct;
	Object *BtnSave;
};

struct MethodEditorData
{
	struct MEObjs Objects;
	char WinTitle[144];
};


//==============================================================================================
// CreateMethodEditorClass()
//==============================================================================================

struct MUI_CustomClass *CreateMethodEditorClass(void)
{
	struct MUI_CustomClass *cl;

	cl = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct MethodEditorData), ENTRY(MethodEditorDispatcher));
	MethodEditorClass = cl;
	return cl;
}



//==============================================================================================
// DeleteMethodEditorClass()
//==============================================================================================

void DeleteMethodEditorClass(void)
{
	MUI_DeleteCustomClass(MethodEditorClass);
}



//==============================================================================================
// CreateUpperGroup()
//==============================================================================================

static Object* CreateUpperGroup(struct MEObjs *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Group_Columns, 2,
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
			MUIA_Text_PreParse, "\33r",
			MUIA_HorizWeight, 1,
			MUIA_Text_Contents, LS(MSG_METHODEDITOR_NAME_LABEL, "Name:"),
		TAG_END),
		MUIA_Group_Child, objs->StrMethodName = MUI_NewObjectM(MUIC_String,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_Background, MUII_StringBack,
			MUIA_String_MaxLen, MAXLEN_ATTRIBUTE_NAME + 1,
			MUIA_String_Accept, IdentifierChars,
			MUIA_CycleChain, TRUE,
			MUIA_String_AdvanceOnCR, TRUE,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
			MUIA_Text_PreParse, "\33r",
			MUIA_HorizWeight, 1,
			MUIA_Text_Contents, LS(MSG_METHODEDITOR_ID_LABEL, "Identifier:"),
		TAG_END),
		MUIA_Group_Child, objs->StrMethodId = MUI_NewObjectM(MUIC_String,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_Background, MUII_StringBack,
			MUIA_String_MaxLen, 9,
			MUIA_String_Accept, HexChars,
			MUIA_CycleChain, TRUE,
			MUIA_String_AdvanceOnCR, TRUE,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
			MUIA_Text_PreParse, "\33r",
			MUIA_HorizWeight, 1,
			MUIA_Text_Contents, LS(MSG_METHODEDITOR_STRCTURE_LABEL, "Message:"),
		TAG_END),
		MUIA_Group_Child, objs->StrMethodStruct = MUI_NewObjectM(MUIC_String,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_Background, MUII_StringBack,
			MUIA_String_MaxLen, MAXLEN_ATTRIBUTE_NAME + 1,
			MUIA_String_Accept, IdentifierChars,
			MUIA_CycleChain, TRUE,
			MUIA_String_AdvanceOnCR, TRUE,
		TAG_END),

	TAG_END);

	return obj;
}



//==============================================================================================
// CreateSaveButtonGroup()
//==============================================================================================

static Object* CreateSaveButtonGroup(struct MEObjs *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Group_Horiz, TRUE,
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
			MUIA_HorizWeight, 25,
		TAG_END),
		MUIA_Group_Child, objs->BtnSave = MUI_NewObjectM(MUIC_Text,
			MUIA_Frame, MUIV_Frame_Button,
			MUIA_Background, MUII_ButtonBack,
			MUIA_Font, MUIV_Font_Button,
			MUIA_Text_PreParse, "\33c",
			MUIA_Text_Contents, LS(MSG_METHODEDITOR_SAVE_BUTTON, "Save"),
			MUIA_InputMode, MUIV_InputMode_RelVerify,
			MUIA_CycleChain, TRUE,
			MUIA_ShortHelp, LS(MSG_METHODEDITOR_SAVE_BUTTON_HELP, "Stores changes in the method list. To cancel changes just close the window."),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
			MUIA_HorizWeight, 25,
		TAG_END),
	TAG_END);

	return obj;
}


//==============================================================================================
// CreateRootGroup()
//==============================================================================================

static Object* CreateRootGroup(struct MEObjs *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Group_Child, CreateUpperGroup(objs),
		MUIA_Group_Child, CreateSaveButtonGroup(objs),
	TAG_END);

	return obj;
}



//==============================================================================================
// MethodEditorNew()
//==============================================================================================

IPTR MethodEditorNew(Class *cl, Object *obj, struct opSet *msg)
{
	Object *newobj = NULL;
	struct MEObjs objs;

	obj = DoSuperNewM(cl, obj,
		MUIA_Window_ID, MAKE_ID('M','E','E','D'),
		MUIA_Window_RootObject, CreateRootGroup(&objs),
	TAG_MORE, msg->ops_AttrList);

	if (obj)
	{
		struct MethodEditorData *d = INST_DATA(cl, obj);

		d->Objects = objs;
		newobj = obj;
	}

	if (!newobj) CoerceMethod(cl, obj, OM_DISPOSE);
	return (IPTR)newobj;
}



//==============================================================================================
// MethodEditorSet()
//==============================================================================================

IPTR MethodEditorSet(Class *cl, Object *obj, struct opSet *msg)
{
	struct MethodEditorData *d = INST_DATA(cl, obj);
	struct TagItem *tag, *tagptr;
	IPTR counter = 0;
	CONST_STRPTR libname = NULL;
	CONST_STRPTR methname = NULL;
	BOOL change_wintitle = FALSE;

	tagptr = msg->ops_AttrList;

	while ((tag = NextTagItem(&tagptr)) != NULL)
	{
		switch (tag->ti_Tag)
		{
		}
	}

	if (change_wintitle)
	{
		FmtNPut(d->WinTitle, LS(MSG_FUNCEDITOR_WINTITILE_PATTERN, "LibMaker: %s/%s()"), 144, libname, methname);
		XSet(obj, MUIA_Window_Title, d->WinTitle);
	}

	counter += DoSuperMethodA(cl, obj, (Msg)msg);
	return counter;
}



//==============================================================================================
// MethodEditorGet()
//==============================================================================================

IPTR MethodEditorGet(Class *cl, Object *obj, struct opGet *msg)
{
//	struct MethodEditorData *d = INST_DATA(cl, obj);
	IPTR result = TRUE;

	switch (msg->opg_AttrID)
	{
		default:
			result = DoSuperMethodA(cl, obj, (Msg)msg);
		break;
	}

	return result;
}


//==============================================================================================
// MethodEditorSetup()
//==============================================================================================

IPTR MethodEditorSetup(Class *cl, Object *obj, struct MEDP_Setup *msg)
{
	struct MethodEditorData *d = INST_DATA(cl, obj);
	struct MethodEntry *me = msg->Entry;
	char hexbuf[9];

	/* Setting up new window title, based on library name and method name. */

	FmtNPut(d->WinTitle, "LibMaker: %s/%s()", 144, msg->LibName, me->me_Name);
	XSet(obj, MUIA_Window_Title, d->WinTitle);

	/* Notification on MethodEditor window close, window is closed, data are not stored. Note  */
	/* that notification method has to be pushed, as the calling object disposes itself in it. */

	DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		MUIV_Notify_Application, 5, MUIM_Application_PushMethod, (IPTR)msg->ClassEditor, 2,
		CEDM_CloseMethodEditor, CEDV_CloseMethodEditor_Discard);

	/* Notification on "Save" button, window is closed, data are stored. Note that notifi-    */
	/* cation method has to be pushed, as the calling object disposes itself in it.           */

	DoMethod(d->Objects.BtnSave, MUIM_Notify, MUIA_Pressed, FALSE,
		MUIV_Notify_Application, 5, MUIM_Application_PushMethod, (IPTR)msg->ClassEditor, 2,
		CEDM_CloseMethodEditor, CEDV_CloseMethodEditor_Store);

	/* Set contents of gadgets. */

	XSet(d->Objects.StrMethodName, MUIA_String_Contents, me->me_Name);
	XSet(d->Objects.StrMethodStruct, MUIA_String_Contents, me->me_Message);
	FmtNPut(hexbuf, "%08lX", 9, me->me_Id);
	XSet(d->Objects.StrMethodId, MUIA_String_Contents, hexbuf);
	return 0;
}



//==============================================================================================
// MethodEditorUpdateEntry()
//==============================================================================================

IPTR MethodEditorUpdateEntry(Class *cl, Object *obj, struct MEDP_UpdateEntry *msg)
{
	struct MethodEditorData *d = INST_DATA(cl, obj);
	struct MethodEntry *me = msg->Entry;
	STRPTR new_name, new_message;

	if ((new_name = StrNew((CONST_STRPTR)XGet(d->Objects.StrMethodName, MUIA_String_Contents))) != NULL)
	{
		if ((new_message = StrNew((CONST_STRPTR)XGet(d->Objects.StrMethodStruct, MUIA_String_Contents))) != NULL)
		{
			if (me->me_Name) StrFree(me->me_Name);
			me->me_Name = new_name;
			if (me->me_Message) StrFree(me->me_Message);
			me->me_Message = new_message;
			me->me_Id = HexStrToULong((CONST_STRPTR)XGet(d->Objects.StrMethodId, MUIA_String_Contents));
			return 0;
		}

		StrFree(new_name);
	}

	return 0;
}



//==============================================================================================
// MethodEditorDispatcher()
//==============================================================================================

DISPATCHER(MethodEditorDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW:                  return MethodEditorNew(cl, obj, (struct opSet*)msg);
		case OM_SET:                  return MethodEditorSet(cl, obj, (struct opSet*)msg);
		case OM_GET:                  return MethodEditorGet(cl, obj, (struct opGet*)msg);
		case MEDM_Setup:              return MethodEditorSetup(cl, obj, (struct MEDP_Setup*)msg);
		case MEDM_UpdateEntry:        return MethodEditorUpdateEntry(cl, obj, (struct MEDP_UpdateEntry*)msg);
		default:                      return (DoSuperMethodA(cl, obj, msg));
	}
}
