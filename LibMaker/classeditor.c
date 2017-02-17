/* ClassEditorClass code. */


#include "main.h"
#include "classeditor.h"
#include "functionlist.h"
#include "application.h"
#include "attributelist.h"
#include "methodlist.h"
#include "methodeditor.h"
#include "libvstring/libvstring.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <proto/locale.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>


struct MUI_CustomClass *ClassEditorClass;

DISPATCHERPROTO(ClassEditorDispatcher);

struct CEObjs
{
	Object *BtnSave;
	Object *StrSuper;
	Object *LstAttributes;
	Object *BtnAttrAdd;
	Object *BtnAttrDelete;
	Object *LstMethods;
	Object *BtnMethodAdd;
	Object *BtnMethodDelete;
	Object *StrAttrName;
	Object *StrAttrId;
	Object *ChkAttrUsageInit;
	Object *ChkAttrUsageSet;
	Object *ChkAttrUsageGet;
	Object *WndMethodEditor;          // dynamically created
};

struct ClassEditorData
{
	struct CEObjs Objects;
	char WinTitle[80];
};


//==============================================================================================
// CreateClassEditorClass()
//==============================================================================================

struct MUI_CustomClass *CreateClassEditorClass(void)
{
	struct MUI_CustomClass *cl;

	cl = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct ClassEditorData), ENTRY(ClassEditorDispatcher));
	ClassEditorClass = cl;
	return cl;
}



//==============================================================================================
// DeleteClassEditorClass()
//==============================================================================================

void DeleteClassEditorClass(void)
{
	MUI_DeleteCustomClass(ClassEditorClass);
}



//==============================================================================================
// CreateSaveButtonGroup()
//==============================================================================================

static Object* CreateSaveButtonGroup(struct CEObjs *objs)
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
			MUIA_Text_Contents, LS(MSG_CLASSEDITOR_SAVE_BUTTON, "Save"),
			MUIA_InputMode, MUIV_InputMode_RelVerify,
			MUIA_CycleChain, TRUE,
			MUIA_ShortHelp, LS(MSG_CLASSEDITOR_SAVE_BUTTON_HELP, "Stores changes in the class. To cancel changes just close the window."),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
			MUIA_HorizWeight, 25,
		TAG_END),
	TAG_END);

	return obj;
}



//==============================================================================================
// CreateGroup1()
//==============================================================================================

static Object* CreateGroup1(struct CEObjs *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_FramePhantomHoriz, TRUE,
				MUIA_Text_Contents, LS(MSG_CLASSEDITOR_SUPERCLASS_LABEL, "Superclass:"),
				MUIA_Text_SetMax, TRUE,
			TAG_END),
			MUIA_Group_Child, objs->StrSuper = MUI_NewObjectM(MUIC_String,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_Background, MUII_StringBack,
				MUIA_String_MaxLen, MAXLEN_LIBRARY_NAME + 1,
				MUIA_String_Accept, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.",
				MUIA_CycleChain, TRUE,
			TAG_END),
		TAG_END),
	TAG_END);

	return obj;
}



//==============================================================================================
// CreateAttributesGroup()
//==============================================================================================

Object* CreateAttrPropertiesGroup(struct CEObjs *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Group_Columns, 2,
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
			MUIA_Text_PreParse, "\33r",
			MUIA_HorizWeight, 1,
			MUIA_Text_Contents, LS(MSG_CLASSEDITOR_ATTRNAME_LABEL, "Name:"),
		TAG_END),
		MUIA_Group_Child, objs->StrAttrName = MUI_NewObjectM(MUIC_String,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_Background, MUII_StringBack,
			MUIA_String_MaxLen, MAXLEN_ATTRIBUTE_NAME + 1,
			MUIA_String_Accept, IdentifierChars,
			MUIA_CycleChain, TRUE,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
			MUIA_Text_PreParse, "\33r",
			MUIA_HorizWeight, 1,
			MUIA_Text_Contents, LS(MSG_CLASSEDITOR_ATTRID_LABEL, "Identifier:"),
		TAG_END),
		MUIA_Group_Child, objs->StrAttrId = MUI_NewObjectM(MUIC_String,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_Background, MUII_StringBack,
			MUIA_String_MaxLen, 9,
			MUIA_String_Accept, HexChars,
			MUIA_CycleChain, TRUE,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Frame, MUIV_Frame_Group,
			MUIA_FramePhantomHoriz, TRUE,
			MUIA_Text_PreParse, "\33r",
			MUIA_HorizWeight, 1,
			MUIA_Text_Contents, LS(MSG_CLASSEDITOR_ATTRUSAGE_LABEL, "Usage:"),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Frame, MUIV_Frame_Group,
			MUIA_Background, MUII_GroupBack,
			MUIA_Group_Horiz, TRUE,
			MUIA_Group_Child, objs->ChkAttrUsageInit = MUI_NewObjectM(MUIC_Image,
				MUIA_Image_Spec, "6:15",
				MUIA_ShowSelState, TRUE,
				MUIA_InputMode, MUIV_InputMode_Toggle,
				MUIA_CycleChain, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_PreParse, "\33l",
				MUIA_Text_Contents, LS(MSG_CLASSEDITOR_ATTRUSAGE_INIT, "Init"),
			TAG_END),
			MUIA_Group_Child, objs->ChkAttrUsageSet = MUI_NewObjectM(MUIC_Image,
				MUIA_Image_Spec, "6:15",
				MUIA_ShowSelState, TRUE,
				MUIA_InputMode, MUIV_InputMode_Toggle,
				MUIA_CycleChain, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_PreParse, "\33l",
				MUIA_Text_Contents, LS(MSG_CLASSEDITOR_ATTRUSAGE_SET, "Set"),
			TAG_END),
			MUIA_Group_Child, objs->ChkAttrUsageGet = MUI_NewObjectM(MUIC_Image,
				MUIA_Image_Spec, "6:15",
				MUIA_ShowSelState, TRUE,
				MUIA_InputMode, MUIV_InputMode_Toggle,
				MUIA_CycleChain, TRUE,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Text_PreParse, "\33l",
				MUIA_Text_Contents, LS(MSG_CLASSEDITOR_ATTRUSAGE_GET, "Get"),
			TAG_END),
		TAG_END),
	TAG_END);

	return obj;
}


//==============================================================================================
// CreateAttributesGroup()
//==============================================================================================

static Object* CreateAttributesGroup(struct CEObjs *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Frame, MUIV_Frame_Group,
		MUIA_Background, MUII_GroupBack,
		MUIA_FrameTitle, LS(MSG_CLASSEDITOR_ATTRIBUTES_FRAMETITLE, "Class Attributes"),
		MUIA_Group_Child, objs->LstAttributes = NewObjectM(AttributeListClass->mcc_Class, NULL,
			MUIA_Frame, MUIV_Frame_InputList,
			MUIA_Background, MUII_ListBack,
			MUIA_CycleChain, TRUE,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_Group_Child, objs->BtnAttrAdd = MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_CLASSEDITOR_ADDATTRIBUTE_BUTTON, "Add"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_CLASSEDITOR_ADDATTRIBUTE_BUTTON_HELP, "Adds a new attribute to the class."),
			TAG_END),
			MUIA_Group_Child, objs->BtnAttrDelete = MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_CLASSEDITOR_DELATTRIBUTE_BUTTON, "Delete"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_CLASSEDITOR_DELATTRIBUTE_BUTTON_HELP, "Deletes selected attribute from the class."),
			TAG_END),
		TAG_END),
		MUIA_Group_Child, CreateAttrPropertiesGroup(objs),
	TAG_END);

	return obj;
}



//==============================================================================================
// CreateMethodsGroup()
//==============================================================================================

static Object* CreateMethodsGroup(struct CEObjs *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Frame, MUIV_Frame_Group,
		MUIA_Background, MUII_GroupBack,
		MUIA_FrameTitle, LS(MSG_CLASSEDITOR_METHODS_FRAMETITLE, "Class Methods"),
		MUIA_Group_Child, objs->LstMethods = NewObjectM(MethodListClass->mcc_Class, NULL,
			MUIA_Frame, MUIV_Frame_InputList,
			MUIA_Background, MUII_ListBack,
			MUIA_CycleChain, TRUE,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_Group_Child, objs->BtnMethodAdd = MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_CLASSEDITOR_ADDATTRIBUTE_BUTTON, "Add"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_CLASSEDITOR_ADDATTRIBUTE_BUTTON_HELP, "Adds a new attribute to the class, with properties taken from the gadgets below."),
			TAG_END),
			MUIA_Group_Child, objs->BtnMethodDelete = MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_CLASSEDITOR_DELATTRIBUTE_BUTTON, "Delete"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_CLASSEDITOR_DELATTRIBUTE_BUTTON_HELP, "Deletes selected attribute from the class."),
			TAG_END),
		TAG_END),
	TAG_END);

	return obj;
}



//==============================================================================================
// ClassEditorNew()
//==============================================================================================

IPTR ClassEditorNew(Class *cl, Object *obj, struct opSet *msg)
{
	struct CEObjs objs;

	obj = DoSuperNewM(cl, obj,
		MUIA_Window_ID, MAKE_ID('C','L','E','D'),
		MUIA_Window_RootObject, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Child, CreateGroup1(&objs),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
				MUIA_Group_Horiz, TRUE,
				MUIA_Group_Child, CreateAttributesGroup(&objs),
				MUIA_Group_Child, CreateMethodsGroup(&objs),
			TAG_END),
			MUIA_Group_Child, CreateSaveButtonGroup(&objs),
		TAG_END),
	TAG_MORE, msg->ops_AttrList);

	if (obj)
	{
		struct ClassEditorData *d = INST_DATA(cl, obj);

		d->Objects = objs;

		/* When window is closed, close it without saving changes. */

		DoMethod(obj, MUIM_Notify,
			MUIA_Window_CloseRequest, TRUE,
			MUIV_Notify_Application, 2,
			APPM_CloseClassEditor, APPV_CloseClassEditor_DoNotSave);

		/* When "Save" button is clicked, close the window with saving changes. */

		DoMethod(objs.BtnSave, MUIM_Notify,
			MUIA_Pressed, FALSE,
			MUIV_Notify_Application, 2,
			APPM_CloseClassEditor, APPV_CloseClassEditor_Save);

		/* Add fresh attribute when attribute "Add" button is clicked. */

		DoMethod(objs.BtnAttrAdd, MUIM_Notify,
			MUIA_Pressed, FALSE,
			(IPTR)obj, 1,
			CEDM_NewAttribute);

		/* Delete attribute when attribute "Delete" button is clicked. */

		DoMethod(objs.BtnAttrDelete, MUIM_Notify,
			MUIA_Pressed, FALSE,
			(IPTR)objs.LstAttributes, 2,
			MUIM_List_Remove, MUIV_List_Remove_Active);

		/* Update attribute properties gadgets on attribute selection change. */

		DoMethod(objs.LstAttributes, MUIM_Notify,
			MUIA_List_Active, MUIV_EveryTime,
			(IPTR)obj, 2,
			CEDM_UpdateGadgetsFromAttr, MUIV_TriggerValue);

		/* Update active attribute list entry on change of attribute name. */

		DoMethod(objs.StrAttrName, MUIM_Notify,
			MUIA_String_Contents, MUIV_EveryTime,
			(IPTR)obj, 1,
			CEDM_UpdateAttrFromGadgets);

		/* Update active attribute list entry on change of attribute identifier. */

		DoMethod(objs.StrAttrId, MUIM_Notify,
			MUIA_String_Contents, MUIV_EveryTime,
			(IPTR)obj, 1,
			CEDM_UpdateAttrFromGadgets);

		/* Update active attribute list entry on change of usage "Init" change. */

		DoMethod(objs.ChkAttrUsageInit, MUIM_Notify,
			MUIA_Selected, MUIV_EveryTime,
			(IPTR)obj, 1,
			CEDM_UpdateAttrFromGadgets);

		/* Update active attribute list entry on change of usage "Set" change. */

		DoMethod(objs.ChkAttrUsageSet, MUIM_Notify,
			MUIA_Selected, MUIV_EveryTime,
			(IPTR)obj, 1,
			CEDM_UpdateAttrFromGadgets);

		/* Update active attribute list entry on change of usage "Get" change. */

		DoMethod(objs.ChkAttrUsageGet, MUIM_Notify,
			MUIA_Selected, MUIV_EveryTime,
			(IPTR)obj, 1,
			CEDM_UpdateAttrFromGadgets);

		/* Open empty method editor when "Add" method button is clicked. */

		DoMethod(objs.BtnMethodAdd, MUIM_Notify,
			MUIA_Pressed, FALSE,
			(IPTR)obj, 2,
			CEDM_OpenMethodEditor, CEDV_OpenMethodEditor_New);

		/* Open method editor with a doubleclicked method. */

		DoMethod(objs.LstMethods, MUIM_Notify,
			MUIA_List_DoubleClick, MUIV_EveryTime,
			(IPTR)obj, 2,
			CEDM_OpenMethodEditor, CEDV_OpenMethodEditor_Existing);

	}

	return (IPTR)obj;
}


//==============================================================================================
// ClassEditorSet()
//==============================================================================================

IPTR ClassEditorSet(Class *cl, Object *obj, struct opSet *msg)
{
	struct ClassEditorData *d = INST_DATA(cl, obj);
	struct TagItem *tag, *tagptr;
	IPTR counter = 0;
	CONST_STRPTR libname = NULL;
	BOOL change_wintitle = FALSE;

	tagptr = msg->ops_AttrList;

	while ((tag = NextTagItem(&tagptr)) != NULL)
	{
		switch (tag->ti_Tag)
		{
			case CEDA_LibraryName:
				libname = (CONST_STRPTR)tag->ti_Data;
				change_wintitle = TRUE;
				counter++;
			break;

//			case CEDA_SuperClassName:
//				XSet(d->Objects.StrSuper, MUIA_String_Contents, tag->ti_Data);
//				counter++;
			break;
		}
	}

	if (change_wintitle)
	{
		FmtNPut(d->WinTitle, LS(MSG_CLASSEDITOR_WINTITILE_PATTERN, "LibMaker: %s"), 80, libname);
		XSet(obj, MUIA_Window_Title, d->WinTitle);
	}

	counter += DoSuperMethodA(cl, obj, (Msg)msg);
	return counter;
}


//==============================================================================================
// ClassEditorGet()
//==============================================================================================

IPTR ClassEditorGet(Class *cl, Object *obj, struct opGet *msg)
{
//	struct ClassEditorData *d = INST_DATA(cl, obj);
	IPTR result = TRUE;

	switch (msg->opg_AttrID)
	{
//		case CEDA_SuperClassName:
//			*msg->opg_Storage = XGet(d->Objects.StrSuper, MUIA_String_Contents);
//		break;

		default:
			result = DoSuperMethodA(cl, obj, (Msg)msg);
		break;
	}

	return result;
}



//==============================================================================================
// ClassEditorNewAttribute()
//==============================================================================================

IPTR ClassEditorNewAttribute(Class *cl, Object *obj)
{
	struct ClassEditorData *d = INST_DATA(cl, obj);
	struct AttributeEntry ae;

	ae.ae_Name = (STRPTR)LS(MSG_CLASSEDITOR_ATTRIBUTE_NOT_SPECIFIED, "not specified");
	ae.ae_Id = 0;
	ae.ae_Usage = 0;
	DoMethod(d->Objects.LstAttributes, MUIM_List_InsertSingle, (IPTR)&ae, MUIV_List_Insert_Bottom);
	XSet(d->Objects.LstAttributes, MUIA_List_Active, MUIV_List_Active_Bottom);
	return 0;
}



//==============================================================================================
// ClassEditorUpdateAttrFromGadgets()
//==============================================================================================

IPTR ClassEditorUpdateAttrFromGadgets(Class *cl, Object *obj)
{
	struct ClassEditorData *d = INST_DATA(cl, obj);
	struct AttributeEntry *ae = NULL;
	STRPTR new_name = NULL;

	if ((new_name = StrNew((STRPTR)XGet(d->Objects.StrAttrName, MUIA_String_Contents))) != NULL)
	{
		DoMethod(d->Objects.LstAttributes, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, (IPTR)&ae);

		if (ae)
		{
			if (ae->ae_Name) StrFree(ae->ae_Name);
			ae->ae_Name = new_name;
			ae->ae_Id = HexStrToULong((CONST_STRPTR)XGet(d->Objects.StrAttrId, MUIA_String_Contents));
			ae->ae_Usage = 0;
			if (XGet(d->Objects.ChkAttrUsageInit, MUIA_Selected)) ae->ae_Usage |= ATTR_USAGE_INIT;
			if (XGet(d->Objects.ChkAttrUsageSet, MUIA_Selected)) ae->ae_Usage |= ATTR_USAGE_SET;
			if (XGet(d->Objects.ChkAttrUsageGet, MUIA_Selected)) ae->ae_Usage |= ATTR_USAGE_GET;
			DoMethod(d->Objects.LstAttributes, MUIM_List_Redraw, MUIV_List_Redraw_Entry, (IPTR)ae);
		}
	}

	return 0;
}



//==============================================================================================
// ClassEditorUpdateGadgetsFromAttr()
//==============================================================================================

IPTR ClassEditorUpdateGadgetsFromAttr(Class *cl, Object *obj, struct CEDP_UpdateGadgetsFromAttr *msg)
{
	struct ClassEditorData *d = INST_DATA(cl, obj);
	struct AttributeEntry *ae = NULL;
	char hexbuf[9];

	DoMethod(d->Objects.LstAttributes, MUIM_List_GetEntry, msg->Position, (IPTR)&ae);

	if (ae)
	{
		FmtNPut(hexbuf, "%08lX", 9, ae->ae_Id);
		XNSet(d->Objects.StrAttrName, MUIA_String_Contents, ae->ae_Name);
		XNSet(d->Objects.StrAttrId, MUIA_String_Contents, hexbuf);
		XNSet(d->Objects.ChkAttrUsageInit, MUIA_Selected, (ae->ae_Usage & ATTR_USAGE_INIT) ? TRUE : FALSE);
		XNSet(d->Objects.ChkAttrUsageSet, MUIA_Selected, (ae->ae_Usage & ATTR_USAGE_SET) ? TRUE : FALSE);
		XNSet(d->Objects.ChkAttrUsageGet, MUIA_Selected, (ae->ae_Usage & ATTR_USAGE_GET) ? TRUE : FALSE);
	}

	return 0;
}



//==============================================================================================
// ClassEditorWriteClassSpec()
//==============================================================================================

IPTR ClassEditorWriteClassSpec(Class *cl, Object *obj, struct CEDP_WriteClassSpec *msg)
{
	struct ClassEditorData *d = INST_DATA(cl, obj);
	struct AttributeEntry *ae = NULL;
	struct MethodEntry *me = NULL;
	LONG i;

	FPrintf(msg->File, "CLASS SUPERCLASS=\"%s\"\n", XGet(d->Objects.StrSuper, MUIA_String_Contents));

	for (i = 0; ; i++)
	{
		DoMethod(d->Objects.LstAttributes, MUIM_List_GetEntry, i, (IPTR)&ae);
		if (!ae) break;
		FPrintf(msg->File, "ATTRIBUTE NAME=\"%s\" ID=\"%08lx\" USAGE=%ld\n", (IPTR)ae->ae_Name, ae->ae_Id, ae->ae_Usage);
	}

	for (i = 0; ; i++)
	{
		DoMethod(d->Objects.LstMethods, MUIM_List_GetEntry, i, (IPTR)&me);
		if (!me) break;
		FPrintf(msg->File, "METHOD NAME=\"%s\" ID=\"%08lx\" MESSAGE=\"%s\"\n", (IPTR)me->name, me->id, (IPTR)me->message);

		/* method arguments here */

		FPrintf(msg->File, "ENDMETHOD\n");
	}


	FPuts(msg->File, "ENDCLASS\n");
	return 0;
}



//==============================================================================================
// ReadMethodContents()
//==============================================================================================

static void ReadMethodContents(UNUSED Object *obj, UNUSED struct ClassEditorData *d, BPTR file, STRPTR line, struct MethodEntry *me)
{
//	struct RDArgs *args, srcargs;
	BYTE i;

	while (!IoErr())
	{
		if (ReadLine(file, line))
		{
			//LONG params[3] = { 0, 0, 0 };

			if (StrEqu(line, "ENDMETHOD\n")) break;
			else SetIoErr(LDLERR_SYNTAX_ERROR);
		}
	}

	/* clear unused argument fields, for proper freeing */

	for (i = me->argCount; i < MAX_ARGS_IN_METHOD; i++)
	{
		me->args[i].name = NULL;
		me->args[i].type = NULL;
	}
}



//==============================================================================================
// ReadMethod()
//==============================================================================================

static void ReadMethod(Object *obj, struct ClassEditorData *d, BPTR file, STRPTR line)
{
	struct RDArgs *args, srcargs;
	LONG params[4] = { 0, 0, 0, 0 };

	if ((args = ParseLine(line, "METHOD/S/A,NAME/K/A,ID/K/A,MESSAGE/K/A", params, &srcargs)) != NULL)
	{
		if ((StrLen((CONST_STRPTR)params[1]) <= MAXLEN_METHOD_NAME)
		 && (StrLen((CONST_STRPTR)params[2]) <= 8)
		 && (StrLen((CONST_STRPTR)params[3]) <= MAXLEN_METHOD_NAME))
		{
			struct MethodEntry me;

			me.name = (STRPTR)params[1];
			me.message = (STRPTR)params[3];
			me.id = HexStrToULong((CONST_STRPTR)params[2]);
			me.argCount = 0;
			ReadMethodContents(obj, d, file, line, &me);
			if (!IoErr()) DoMethod(d->Objects.LstMethods, MUIM_List_InsertSingle, (IPTR)&me, MUIV_List_Insert_Bottom);
		}
		else SetIoErr(LDLERR_VALUE_TOO_LONG);

		FreeArgs(args);
	}
}



//==============================================================================================
// ReadClassContents()
//==============================================================================================

static void ReadClassContents(Object *obj, struct ClassEditorData *d, BPTR file, STRPTR line)
{
	struct RDArgs *args, srcargs;

	while (!IoErr())
	{
		if (ReadLine(file, line))
		{
			LONG params[4] = { 0, 0, 0, 0 };

			if (StrEqu(line, "ENDCLASS\n")) break;

			if (StrNEqu(line, "ATTRIBUTE ", 10))
			{
				if ((args = ParseLine(line, "ATTRIBUTE/S/A,NAME/K/A,ID/K/A,USAGE/K/N/A", params, &srcargs)) != NULL)
				{
					struct AttributeEntry ae;

					if ((StrLen((CONST_STRPTR)params[1]) <= MAXLEN_ATTRIBUTE_NAME)
					 && (StrLen((CONST_STRPTR)params[2]) <= 8))
					{
						ae.ae_Name = (STRPTR)params[1];
						ae.ae_Id = HexStrToULong((CONST_STRPTR)params[2]);
						ae.ae_Usage = *(ULONG*)params[3];
						DoMethod(d->Objects.LstAttributes, MUIM_List_InsertSingle, (IPTR)&ae, MUIV_List_Insert_Bottom);
					}
					else SetIoErr(LDLERR_VALUE_TOO_LONG);

					FreeArgs(args);
				}
			}
			else if (StrNEqu(line, "METHOD ", 7)) ReadMethod(obj, d, file, line);
			else SetIoErr(LDLERR_SYNTAX_ERROR);
		}
	}
}



//==============================================================================================
// ClassEditorReadClassSpec()
//==============================================================================================

IPTR ClassEditorReadClassSpec(Class *cl, Object *obj, struct CEDP_ReadClassSpec *msg)
{
	struct ClassEditorData *d = INST_DATA(cl, obj);
	struct RDArgs *args, srcargs;
	LONG params[2] = { 0, 0 };

	if ((args = ParseLine(msg->TxtLine, "CLASS/S/A,SUPERCLASS/K/A", params, &srcargs)) != NULL)
	{
		if (StrLen((CONST_STRPTR)params[1]) <= MAXLEN_LIBRARY_NAME)
		{
			XSet(d->Objects.StrSuper, MUIA_String_Contents, params[1]);
			ReadClassContents(obj, d, msg->FHandle, msg->TxtLine);
		}
		else SetIoErr(LDLERR_VALUE_TOO_LONG);

		FreeArgs(args);
	}

	return 0;
}



//==============================================================================================
// ClassEditorClear()
//==============================================================================================

IPTR ClassEditorClear(Class *cl, Object *obj)
{
	struct ClassEditorData *d = INST_DATA(cl, obj);

	DoMethod(d->Objects.LstAttributes, MUIM_List_Clear);
	return 0;
}



//==============================================================================================
// ClassEditorPushToLua()
//==============================================================================================

IPTR ClassEditorPushToLua(Class *cl, Object *obj, struct CEDP_PushToLua *msg)
{
	struct ClassEditorData *d = INST_DATA(cl, obj);
	LONG i;
	struct AttributeEntry *ae;
	struct MethodEntry *me;
	char hexbuf[9];

	LuaNewTable(msg->L);
	LuaPushString(msg->L, (CONST_STRPTR)XGet(d->Objects.StrSuper, MUIA_String_Contents));
	LuaSetField(msg->L, -2, "super");

	/* attributes */

	LuaNewTable(msg->L);

	for (i = 0; ; i++)
	{
		DoMethod(d->Objects.LstAttributes, MUIM_List_GetEntry, i, (IPTR)&ae);
		if (!ae) break;
		LuaPushInteger(msg->L, i + 1);
		LuaNewTable(msg->L);
		LuaPushString(msg->L, ae->ae_Name);
		LuaSetField(msg->L, -2, "name");
		FmtNPut(hexbuf, "%08lX", 9, ae->ae_Id);
		LuaPushLString(msg->L, hexbuf, 8);
		LuaSetField(msg->L, -2, "id");
		LuaPushBoolean(msg->L, ae->ae_Usage & ATTR_USAGE_INIT);
		LuaSetField(msg->L, -2, "init");
		LuaPushBoolean(msg->L, ae->ae_Usage & ATTR_USAGE_SET);
		LuaSetField(msg->L, -2, "set");
		LuaPushBoolean(msg->L, ae->ae_Usage & ATTR_USAGE_GET);
		LuaSetField(msg->L, -2, "get");
		LuaSetTable(msg->L, -3);
	}

	LuaSetField(msg->L, -2, "attributes");

	/* methods */

	LuaNewTable(msg->L);

	for (i = 0; ; i++)
	{
		DoMethod(d->Objects.LstMethods, MUIM_List_GetEntry, i, (IPTR)&me);
		if (!me) break;
		LuaPushInteger(msg->L, i + 1);
		LuaNewTable(msg->L);
		LuaPushString(msg->L, me->name);
		LuaSetField(msg->L, -2, "name");
		LuaPushString(msg->L, me->message);
		LuaSetField(msg->L, -2, "message");
		FmtNPut(hexbuf, "%08lX", 9, me->id);
		LuaPushLString(msg->L, hexbuf, 8);
		LuaSetField(msg->L, -2, "id");
		LuaSetTable(msg->L, -3);
	}

	LuaSetField(msg->L, -2, "methods");


	LuaSetField(msg->L, -2, "class");
	return 0;
}



//==============================================================================================
// ClassEditorOpenMethodEditor()
//==============================================================================================

IPTR ClassEditorOpenMethodEditor(Class *cl, Object *obj, struct CEDP_OpenMethodEditor *msg)
{
	struct ClassEditorData *d = INST_DATA(cl, obj);

	if ((d->Objects.WndMethodEditor = NewObjectM(MethodEditorClass->mcc_Class, NULL, TAG_END)) != NULL)
	{
		struct MethodEntry *me;

		if (msg->Action == CEDV_OpenMethodEditor_New)
		{
			struct MethodEntry me2;
			BYTE i;

			me2.name = (STRPTR)LS(MSG_METHODEDITOR_NEW_METHOD, "new method");
			me2.id = 0;
			me2.message = (STRPTR)"";
			me2.argCount = 0;

			for (i = 0; i < MAX_ARGS_IN_METHOD; i++)
			{
				me2.args[i].name = NULL;
				me2.args[i].type = NULL;
			}

			DoMethod(d->Objects.LstMethods, MUIM_List_InsertSingle, (IPTR)&me2, MUIV_List_Insert_Bottom);
			XSet(d->Objects.LstMethods, MUIA_List_Active, MUIV_List_Active_Bottom);
		}

		DoMethod(d->Objects.LstMethods, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, (IPTR)&me);
		XSet(_app(obj), MUIA_Application_Sleep, TRUE);
		DoMethod(d->Objects.WndMethodEditor, MEDM_Setup, (IPTR)XGet(_app(obj), APPA_ProjectName), (IPTR)me, (IPTR)obj);
		DoMethod(_app(obj), OM_ADDMEMBER, (IPTR)d->Objects.WndMethodEditor);
		XSet(d->Objects.WndMethodEditor, MUIA_Window_Open, TRUE);
	}

	return 0;
}



//==============================================================================================
// ClassEditorCloseMethodEditor()
//==============================================================================================

IPTR ClassEditorCloseMethodEditor(Class *cl, Object *obj, struct CEDP_CloseMethodEditor *msg)
{
	struct ClassEditorData *d = INST_DATA(cl, obj);

	/* This method is only called by MethodEditor object, so I can assume it always exists. */

	if (msg->StoreData == CEDV_CloseMethodEditor_Store)
	{
		struct MethodEntry *me;

		DoMethod(d->Objects.LstMethods, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, (IPTR)&me);

		if (me)
		{
			DoMethod(d->Objects.WndMethodEditor, MEDM_UpdateEntry, (IPTR)me);
			DoMethod(d->Objects.LstMethods, MUIM_List_Redraw, MUIV_List_Redraw_Active);
		}
	}

	XSet(d->Objects.WndMethodEditor, MUIA_Window_Open, FALSE);
	DoMethod(_app(obj), OM_REMMEMBER, (IPTR)d->Objects.WndMethodEditor);
	MUI_DisposeObject(d->Objects.WndMethodEditor);
	d->Objects.WndMethodEditor = NULL;
	XSet(_app(obj), MUIA_Application_Sleep, FALSE);
	return 0;
}



//==============================================================================================
// ClassEditorDispatcher()
//==============================================================================================

DISPATCHER(ClassEditorDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW:                        return ClassEditorNew(cl, obj, (struct opSet*)msg);
		case OM_SET:                        return ClassEditorSet(cl, obj, (struct opSet*)msg);
		case OM_GET:                        return ClassEditorGet(cl, obj, (struct opGet*)msg);
		case CEDM_NewAttribute:             return ClassEditorNewAttribute(cl, obj);
		case CEDM_UpdateAttrFromGadgets:    return ClassEditorUpdateAttrFromGadgets(cl, obj);
		case CEDM_UpdateGadgetsFromAttr:    return ClassEditorUpdateGadgetsFromAttr(cl, obj, (struct CEDP_UpdateGadgetsFromAttr*)msg);
		case CEDM_Clear:                    return ClassEditorClear(cl, obj);
		case CEDM_PushToLua:                return ClassEditorPushToLua(cl, obj, (struct CEDP_PushToLua*)msg);
		case CEDM_OpenMethodEditor:         return ClassEditorOpenMethodEditor(cl, obj, (struct CEDP_OpenMethodEditor*)msg);
		case CEDM_CloseMethodEditor:        return ClassEditorCloseMethodEditor(cl, obj, (struct CEDP_CloseMethodEditor*)msg);
		case CEDM_WriteClassSpec:           return ClassEditorWriteClassSpec(cl, obj, (struct CEDP_WriteClassSpec*)msg);
		case CEDM_ReadClassSpec:            return ClassEditorReadClassSpec(cl, obj, (struct CEDP_ReadClassSpec*)msg);
		default:                            return (DoSuperMethodA(cl, obj, msg));
	}
}
