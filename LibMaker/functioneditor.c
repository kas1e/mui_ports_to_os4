/* FunctionEditorClass code. */


#include "main.h"
#include "functioneditor.h"
#include "functionlist.h"
#include "application.h"
#include "argumentlist.h"
#include "libvstring/libvstring.h"

#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/locale.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>


struct MUI_CustomClass *FunctionEditorClass;

DISPATCHERPROTO(FunctionEditorDispatcher);

CONST_STRPTR TypicalTypes[] = {
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

CONST_STRPTR M68kRegs[] = { "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "A0", "A1", "A2", "A3", NULL };

struct FEObjs
{
	Object *StrFunctionName;
	Object *StrResultType;
	Object *LstArguments;
	Object *StrArgType;
	Object *StrArgName;
	Object *BtnAddArgument;
	Object *BtnDeleteArgument;
	Object *BtnSave;
	Object *Grp68KRegsContainer;
	Object *Cyc68KRegs;
};

struct FunctionEditorData
{
	struct FEObjs Objects;
	char WinTitle[144];
};


//==============================================================================================
// CreateFunctionEditorClass()
//==============================================================================================

struct MUI_CustomClass *CreateFunctionEditorClass(void)
{
	struct MUI_CustomClass *cl;

	cl = MUI_CreateCustomClass(NULL, MUIC_Window, NULL, sizeof(struct FunctionEditorData), ENTRY(FunctionEditorDispatcher));
	FunctionEditorClass = cl;
	return cl;
}



//==============================================================================================
// DeleteFunctionEditorClass()
//==============================================================================================

void DeleteFunctionEditorClass(void)
{
	MUI_DeleteCustomClass(FunctionEditorClass);
}


//==============================================================================================
// CreateUpperTableGroup()
//==============================================================================================

static Object* CreateUpperTableGroup(struct FEObjs *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Group_Columns, 2,
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
			MUIA_Text_PreParse, "\33r",
			MUIA_Text_Contents, LS(MSG_FUNCEDITOR_FUNCTION_NAME_LABEL, "Function Name:"),
			MUIA_HorizWeight, 0,
		TAG_END),
		MUIA_Group_Child, objs->StrFunctionName = MUI_NewObjectM(MUIC_String,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_Background, MUII_StringBack,
			MUIA_String_MaxLen, 40,
			MUIA_String_Accept, IdentifierChars,
			MUIA_CycleChain, TRUE,
			MUIA_String_AdvanceOnCR, TRUE,
			MUIA_ShortHelp, LS(MSG_FUNCEDITOR_FUNCTION_NAME_HELP, "Function name should follow C language rules. Usually MorphOS API functions "
				"have capitalized first letter of each word. Adding a short prefix to all the functions of a library helps to avoid name "
				"conflicts."),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
			MUIA_Frame, MUIV_Frame_String,
			MUIA_FramePhantomHoriz, TRUE,
			MUIA_Text_PreParse, "\33r",
			MUIA_Text_Contents, LS(MSG_FUNCEDITOR_RESULT_TYPE_LABEL, "Result Type:"),
			MUIA_HorizWeight, 0,
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Poplist,
			MUIA_Poplist_Array, TypicalTypes,
			MUIA_Popstring_String, objs->StrResultType = MUI_NewObjectM(MUIC_String,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_Background, MUII_StringBack,
				MUIA_String_MaxLen, 40,
				MUIA_String_Accept, TypeChars,
				MUIA_CycleChain, TRUE,
				MUIA_String_AdvanceOnCR, TRUE,
				MUIA_ShortHelp, LS(MSG_FUNCEDITOR_RESULT_TYPE_HELP, "Library functions should return either primitive types or pointers. "
					"MorphOS types in the dropdown list are preferred over standard C types. Internal library structures should not be "
					"exposed without a good reason, but returned as APTR (void pointer)."),
			TAG_END),
			MUIA_Popstring_Button, MUI_NewObjectM(MUIC_Image,
				MUIA_Frame, MUIV_Frame_ImageButton,
				MUIA_Background, MUII_ImageButtonBack,
				MUIA_Image_Spec, "6:18",
				MUIA_Image_FreeVert, TRUE,
				MUIA_InputMode, MUIV_InputMode_RelVerify,
			TAG_END),
		TAG_END),
	TAG_END);

	return obj;
}


//==============================================================================================
// CreateArgumentListGroup()
//==============================================================================================

static Object* CreateArgumentListGroup(struct FEObjs *objs)
{
	Object *obj;

	obj = MUI_NewObjectM(MUIC_Group,
		MUIA_Frame, MUIV_Frame_Group,
		MUIA_FrameTitle, LS(MSG_FUNCEDITOR_ARGUMENTS_BAR, "Function Arguments"),
		MUIA_Background, MUII_GroupBack,
		MUIA_Group_Child, objs->LstArguments = NewObjectM(ArgumentListClass->mcc_Class, NULL,
			MUIA_ShortHelp, LS(MSG_FUNCEDITOR_ARGLIST_HELP, "Arguments order may be changed with drag and drop. Using more than 10 "
				"arguments in MorphOS API is not recommended. Consider taglists for passing more arguments, also for optional ones."),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Horiz, TRUE,
			MUIA_Group_Child, objs->BtnAddArgument = MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_FUNCEDITOR_ADDARG_BUTTON, "Add"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_FUNCEDITOR_ADDARG_BUTTON_HELP, "Adds a new argument at the end of the argument list."),
			TAG_END),
			MUIA_Group_Child, objs->BtnDeleteArgument = MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_Button,
				MUIA_Background, MUII_ButtonBack,
				MUIA_Font, MUIV_Font_Button,
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_Contents, LS(MSG_FUNCEDITOR_DELARG_BUTTON, "Delete"),
				MUIA_InputMode, MUIV_InputMode_RelVerify,
				MUIA_CycleChain, TRUE,
				MUIA_ShortHelp, LS(MSG_FUNCEDITOR_DELARG_BUTTON_HELP, "Deletes selected argument."),
			TAG_END),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Columns, 2,
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_FramePhantomHoriz, TRUE,
				MUIA_Text_PreParse, "\33r",
				MUIA_Text_Contents, LS(MSG_FUNCEDITOR_ARGTYPE_LABEL, "Argument Type:"),
				MUIA_HorizWeight, 0,
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
				MUIA_Group_Horiz, TRUE,
				MUIA_Group_Child, MUI_NewObjectM(MUIC_Poplist,
					MUIA_Poplist_Array, TypicalTypes,
					MUIA_Popstring_String, objs->StrArgType = MUI_NewObjectM(MUIC_String,
						MUIA_Frame, MUIV_Frame_String,
						MUIA_Background, MUII_StringBack,
						MUIA_String_MaxLen, 40,
						MUIA_String_Accept, TypeChars,
						MUIA_CycleChain, TRUE,
						MUIA_String_AdvanceOnCR, TRUE,
						MUIA_ShortHelp, LS(MSG_FUNCEDITOR_ARGTYPE_HELP, "Library functions should take either primitive types or pointers "
							"as arguments. MorphOS types in the dropdown list are preferred over standard C types."),
					TAG_END),
					MUIA_Popstring_Button, MUI_NewObjectM(MUIC_Image,
						MUIA_Frame, MUIV_Frame_ImageButton,
						MUIA_Background, MUII_ImageButtonBack,
						MUIA_Image_Spec, "6:18",
						MUIA_Image_FreeVert, TRUE,
						MUIA_InputMode, MUIV_InputMode_RelVerify,
					TAG_END),
				TAG_END),
				MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
					MUIA_FixWidthTxt, "i",
				TAG_END),
				MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
					MUIA_Frame, MUIV_Frame_String,
					MUIA_FramePhantomHoriz, TRUE,
					MUIA_Text_PreParse, "\33r",
					MUIA_Text_Contents, LS(MSG_FUNCEDITOR_M68KREG_LABEL, "M68k Register:"),
					MUIA_HorizWeight, 0,
				TAG_END),
				MUIA_Group_Child, objs->Grp68KRegsContainer = MUI_NewObjectM(MUIC_Group,
					MUIA_Group_Child, objs->Cyc68KRegs = MUI_NewObjectM(MUIC_Cycle,
						MUIA_Cycle_Entries, M68kRegs,
					TAG_END),
					MUIA_HorizWeight, 2,
					MUIA_ShortHelp, LS(MSG_FUNCEDITOR_M68KREG_HELP, "Libraries for AmigaOS 3, or libraries for AmigaOS 4 and MorphOS callable "
						"from M68k code, have processor registers assigned to function arguments. Use data registers for numbers, address "
						"registers for pointers."),
				TAG_END),
			TAG_END),
			MUIA_Group_Child, MUI_NewObjectM(MUIC_Text,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_FramePhantomHoriz, TRUE,
				MUIA_Text_PreParse, "\33r",
				MUIA_Text_Contents, LS(MSG_FUNCEDITOR_ARGNAME_LABEL, "Formal Name:"),
				MUIA_HorizWeight, 0,
			TAG_END),
			MUIA_Group_Child, objs->StrArgName = MUI_NewObjectM(MUIC_String,
				MUIA_Frame, MUIV_Frame_String,
				MUIA_Background, MUII_StringBack,
				MUIA_String_MaxLen, 40,
				MUIA_String_Accept, IdentifierChars,
				MUIA_CycleChain, TRUE,
				MUIA_String_AdvanceOnCR, TRUE,
				MUIA_ShortHelp, LS(MSG_FUNCEDITOR_ARGNAME_HELP, "Formal name for the argument used in header files and function code template. "
					"The name must follow C rules for naming variables and should be lowercase."),
			TAG_END),
		TAG_END),
	TAG_END);

	return obj;
}


//==============================================================================================
// CreateSaveButtonGroup()
//==============================================================================================

static Object* CreateSaveButtonGroup(struct FEObjs *objs)
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
			MUIA_Text_Contents, LS(MSG_FUNCEDITOR_SAVE_BUTTON, "Save"),
			MUIA_InputMode, MUIV_InputMode_RelVerify,
			MUIA_CycleChain, TRUE,
			MUIA_ShortHelp, LS(MSG_FUNCEDITOR_SAVE_BUTTON_HELP, "Stores changes in the function list. To cancel changes just close the window."),
		TAG_END),
		MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
			MUIA_HorizWeight, 25,
		TAG_END),
	TAG_END);

	return obj;
}


//==============================================================================================
// FunctionEditorNew()
//==============================================================================================

IPTR FunctionEditorNew(Class *cl, Object *obj, struct opSet *msg)
{
	struct FEObjs objs;

	obj = DoSuperNewM(cl, obj,
		MUIA_Window_ID, MAKE_ID('F','N','E','D'),
		MUIA_Window_RootObject, MUI_NewObjectM(MUIC_Group,
			MUIA_Group_Child, CreateUpperTableGroup(&objs),
			MUIA_Group_Child, CreateArgumentListGroup(&objs),
			MUIA_Group_Child, CreateSaveButtonGroup(&objs),
		TAG_END),
	TAG_MORE, msg->ops_AttrList);

	if (obj)
	{
		struct FunctionEditorData *d = INST_DATA(cl, obj);

		d->Objects = objs;

		/* When window is closed, close it without saving changes. */

		DoMethod(obj, MUIM_Notify,
			MUIA_Window_CloseRequest, TRUE,
			MUIV_Notify_Application, 2,
			APPM_CloseFunctionEditor, APPV_CloseFunctionEditor_DoNotSave);

		/* When "Save" button is clicked, close the window with saving changes. */

		DoMethod(objs.BtnSave, MUIM_Notify,
			MUIA_Pressed, FALSE,
			MUIV_Notify_Application, 2,
			APPM_CloseFunctionEditor, APPV_CloseFunctionEditor_Save);

		/* "Add" argument button action. */

		DoMethod(objs.BtnAddArgument, MUIM_Notify,
			MUIA_Pressed, FALSE,
			(IPTR)obj, 1,
			FEDM_AddArgument);

		/* Display current argument entry after it is selected. */

		DoMethod(objs.LstArguments, MUIM_Notify,
			MUIA_List_Active, MUIV_EveryTime,
			(IPTR)obj, 2,
			FEDM_DisplayArgument, MUIV_TriggerValue);

		/* When "Delete" button is used, delete the active argument entry. */

		DoMethod(objs.BtnDeleteArgument, MUIM_Notify,
			MUIA_Pressed, FALSE,
			(IPTR)objs.LstArguments, 2,
			MUIM_List_Remove, MUIV_List_Remove_Active);

		/* Update argument entry, when name is changed. */

		DoMethod(objs.StrArgName, MUIM_Notify,
			MUIA_String_Contents, MUIV_EveryTime,
			(IPTR)obj, 1,
			FEDM_UpdateArgument);

		/* Update argument entry, when type is changed. */

		DoMethod(objs.StrArgType, MUIM_Notify,
			MUIA_String_Contents, MUIV_EveryTime,
			(IPTR)obj, 1,
			FEDM_UpdateArgument);

		/* Update argument entry, when M68k register is changed. */

		DoMethod(objs.Cyc68KRegs, MUIM_Notify,
			MUIA_Cycle_Active, MUIV_EveryTime,
			(IPTR)obj, 1,
			FEDM_UpdateArgument);
	}

	return (IPTR)obj;
}


//==============================================================================================
// FunctionEditorSet()
//==============================================================================================

IPTR FunctionEditorSet(Class *cl, Object *obj, struct opSet *msg)
{
	struct FunctionEditorData *d = INST_DATA(cl, obj);
	struct TagItem *tag, *tagptr;
	IPTR counter = 0;
	CONST_STRPTR libname = NULL;
	CONST_STRPTR funcname = NULL;
	BOOL change_wintitle = FALSE;

	tagptr = msg->ops_AttrList;

	while ((tag = NextTagItem(&tagptr)) != NULL)
	{
		switch (tag->ti_Tag)
		{
			case FEDA_FunctionName:
				XSet(d->Objects.StrFunctionName, MUIA_String_Contents, tag->ti_Data);
				funcname = (CONST_STRPTR)tag->ti_Data;
				change_wintitle = TRUE;
				counter++;
			break;

			case FEDA_ResultType:
				XSet(d->Objects.StrResultType, MUIA_String_Contents, tag->ti_Data);
				counter++;
			break;

			case FEDA_LibraryName:
				libname = (CONST_STRPTR)tag->ti_Data;
				change_wintitle = TRUE;
				counter++;
			break;
		}
	}

	if (change_wintitle)
	{
		FmtNPut(d->WinTitle, LS(MSG_FUNCEDITOR_WINTITILE_PATTERN, "LibMaker: %s/%s()"), 144, libname, funcname);
		XSet(obj, MUIA_Window_Title, d->WinTitle);
	}

	counter += DoSuperMethodA(cl, obj, (Msg)msg);
	return counter;
}


//==============================================================================================
// FunctionEditorGet()
//==============================================================================================

IPTR FunctionEditorGet(Class *cl, Object *obj, struct opGet *msg)
{
	struct FunctionEditorData *d = INST_DATA(cl, obj);
	IPTR result = TRUE;

	switch (msg->opg_AttrID)
	{
		case FEDA_FunctionName:
			*(CONST_STRPTR*)msg->opg_Storage = (CONST_STRPTR)XGet(d->Objects.StrFunctionName, MUIA_String_Contents);
		break;

		case FEDA_ResultType:
			*(CONST_STRPTR*)msg->opg_Storage = (CONST_STRPTR)XGet(d->Objects.StrResultType, MUIA_String_Contents);
		break;

		case FEDA_NumArgs:
			*msg->opg_Storage = XGet(d->Objects.LstArguments, MUIA_List_Entries);
		break;

		default:
			result = DoSuperMethodA(cl, obj, (Msg)msg);
		break;
	}

	return result;
}


//==============================================================================================
// FunctionEditorAddArgument()
//==============================================================================================

IPTR FunctionEditorAddArgument(Class *cl, Object *obj)
{
	struct FunctionEditorData *d = INST_DATA(cl, obj);
	struct ArgumentEntry ae;

	ae.ae_Name = (STRPTR)LS(MSG_FUNCEDITOR_ARGNAME_NOT_SPECIFIED, "not specified");
	ae.ae_Type = (STRPTR)LS(MSG_FUNCEDITOR_ARGTYPE_NOT_SPECIFIED, "not specified");
	ae.ae_Register = M68K_REG_UNDEFINED;

	DoMethod(d->Objects.LstArguments, MUIM_List_InsertSingle, (IPTR)&ae, MUIV_List_Insert_Bottom);
	XSet(d->Objects.LstArguments, MUIA_List_Active, MUIV_List_Active_Bottom);

	return 0;
}


//==============================================================================================
// FunctionEditorUpdateArgument()
//==============================================================================================

IPTR FunctionEditorUpdateArgument(Class *cl, Object *obj)
{
	struct FunctionEditorData *d = INST_DATA(cl, obj);
	struct ArgumentEntry ae;
	LONG position;

	position = XGet(d->Objects.LstArguments, MUIA_List_Active);

	if (position != MUIV_List_Active_Off)
	{
		XSet(d->Objects.LstArguments, MUIA_List_Quiet, TRUE);
		XSet(d->Objects.LstArguments, MUIA_List_Active, MUIV_List_Active_Off);
		DoMethod(d->Objects.LstArguments, MUIM_List_Remove, position);
		ae.ae_Name = (STRPTR)XGet(d->Objects.StrArgName, MUIA_String_Contents);
		ae.ae_Type = (STRPTR)XGet(d->Objects.StrArgType, MUIA_String_Contents);
		ae.ae_Register = XGet(d->Objects.Cyc68KRegs, MUIA_Cycle_Active);
		DoMethod(d->Objects.LstArguments, MUIM_List_InsertSingle, (IPTR)&ae, position);
		XSet(d->Objects.LstArguments, MUIA_List_Active, position);
		XSet(d->Objects.LstArguments, MUIA_List_Quiet, FALSE);
	}

	return 0;
}


//==============================================================================================
// FunctionEditorDisplayArgument()
//==============================================================================================

IPTR FunctionEditorDisplayArgument(Class *cl, Object *obj, struct FEDP_DisplayArgument *msg)
{
	struct FunctionEditorData *d = INST_DATA(cl, obj);
	struct ArgumentEntry *ae;

	DoMethod(d->Objects.LstArguments, MUIM_List_GetEntry, msg->ArgumentNumber, (IPTR)&ae);

	if (ae)
	{
		XNSet(d->Objects.StrArgName, MUIA_String_Contents, ae->ae_Name);
		XNSet(d->Objects.StrArgType, MUIA_String_Contents, ae->ae_Type);
		if (ae->ae_Register < 12) XNSet(d->Objects.Cyc68KRegs, MUIA_Cycle_Active, ae->ae_Register);
	}

	return 0;
}



//==============================================================================================
// FunctionEditorGetArgumentType()
//==============================================================================================

IPTR FunctionEditorGetArgumentType(Class *cl, Object *obj, struct FEDP_GetArgumentType *msg)
{
	struct FunctionEditorData *d = INST_DATA(cl, obj);
	struct ArgumentEntry *ae;

	DoMethod(d->Objects.LstArguments, MUIM_List_GetEntry, msg->ArgumentNumber, (IPTR)&ae);

	if (ae) return (IPTR)ae->ae_Type;

	return 0;
}



//==============================================================================================
// FunctionEditorGetArgumentName()
//==============================================================================================

IPTR FunctionEditorGetArgumentName(Class *cl, Object *obj, struct FEDP_GetArgumentName *msg)
{
	struct FunctionEditorData *d = INST_DATA(cl, obj);
	struct ArgumentEntry *ae;

	DoMethod(d->Objects.LstArguments, MUIM_List_GetEntry, msg->ArgumentNumber, (IPTR)&ae);

	if (ae) return (IPTR)ae->ae_Name;

	return 0;
}



//==============================================================================================
// FunctionEditorSetArgumentList()
//==============================================================================================

IPTR FunctionEditorSetArgumentList(Class *cl, Object *obj, struct FEDP_SetArgumentList *msg)
{
	struct FunctionEditorData *d = INST_DATA(cl, obj);

	XSet(d->Objects.LstArguments, MUIA_List_Quiet, TRUE);
	DoMethod(d->Objects.LstArguments, MUIM_List_Clear);

	if (msg->Args && (msg->NumArgs > 0))
	{
		LONG arg;
		struct ArgumentEntry ae;

		for (arg = 0; arg < (LONG)msg->NumArgs; arg++)
		{
			ae.ae_Name = msg->Args[arg].fa_Name;
			ae.ae_Type = msg->Args[arg].fa_Type;
			ae.ae_Register = msg->Args[arg].fa_Register;
			DoMethod(d->Objects.LstArguments, MUIM_List_InsertSingle, (IPTR)&ae, MUIV_List_Insert_Bottom);
		}
	}

	XSet(d->Objects.LstArguments, MUIA_List_Quiet, FALSE);
	return 0;
}



//==============================================================================================
// FunctionEditorGetArgumentRegister()
//==============================================================================================

IPTR FunctionEditorGetArgumentRegister(Class *cl, Object *obj, struct FEDP_GetArgumentRegister *msg)
{
	struct FunctionEditorData *d = INST_DATA(cl, obj);
	struct ArgumentEntry *ae;

	DoMethod(d->Objects.LstArguments, MUIM_List_GetEntry, msg->ArgumentNumber, (IPTR)&ae);

	if (ae) return ae->ae_Register;

	return 0;
}



//==============================================================================================
// FunctionEditorDispatcher()
//==============================================================================================

DISPATCHER(FunctionEditorDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW:                     return FunctionEditorNew(cl, obj, (struct opSet*)msg);
		case OM_SET:                     return FunctionEditorSet(cl, obj, (struct opSet*)msg);
		case OM_GET:                     return FunctionEditorGet(cl, obj, (struct opGet*)msg);
		case FEDM_AddArgument:           return FunctionEditorAddArgument(cl, obj);
		case FEDM_DisplayArgument:       return FunctionEditorDisplayArgument(cl, obj, (struct FEDP_DisplayArgument*)msg);
		case FEDM_UpdateArgument:        return FunctionEditorUpdateArgument(cl, obj);
		case FEDM_GetArgumentName:       return FunctionEditorGetArgumentName(cl, obj, (struct FEDP_GetArgumentName*)msg);
		case FEDM_GetArgumentType:       return FunctionEditorGetArgumentType(cl, obj, (struct FEDP_GetArgumentType*)msg);
		case FEDM_SetArgumentList:       return FunctionEditorSetArgumentList(cl, obj, (struct FEDP_SetArgumentList*)msg);
		case FEDM_GetArgumentRegister:   return FunctionEditorGetArgumentRegister(cl, obj, (struct FEDP_GetArgumentRegister*)msg);
		default:                         return (DoSuperMethodA(cl, obj, msg));
	}
}
