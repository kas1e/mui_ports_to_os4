/* ArgumentListClass code. */


#include "main.h"
#include "argumentlist.h"
#include "libvstring/libvstring.h"

#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/locale.h>
#include <clib/alib_protos.h>


struct MUI_CustomClass *ArgumentListClass;

#ifdef __amigaos4__
LONG ArgumentListDispatcher(Class *cl, Object *obj,	Msg msg);
#else
LONG ArgumentListDispatcher(void);
const struct EmulLibEntry ArgumentListGate = {TRAP_LIB, 0, (void(*)(void))ArgumentListDispatcher};
#endif

struct ArgumentListData
{
	char M68kRegBuf[3];
};	


//==============================================================================================
// CreateArgumentListClass()
//==============================================================================================

struct MUI_CustomClass *CreateArgumentListClass(void)
{
	struct MUI_CustomClass *cl;
	#ifdef __amigaos4__
	cl = MUI_CreateCustomClass(NULL, MUIC_List, NULL, sizeof(struct ArgumentListData), (APTR)&ArgumentListDispatcher);
	#else
	cl = MUI_CreateCustomClass(NULL, MUIC_List, NULL, sizeof(struct ArgumentListData), (APTR)&ArgumentListGate);
	#endif
	ArgumentListClass = cl;
	return cl;
}



//==============================================================================================
// DeleteArgumentListClass()
//==============================================================================================

void DeleteArgumentListClass(void)
{
	MUI_DeleteCustomClass(ArgumentListClass);
}


//==============================================================================================
// ArgumentListNew()
//==============================================================================================

IPTR ArgumentListNew(Class *cl, Object *obj, struct opSet *msg)
{
	obj = DoSuperNewM(cl, obj,
		MUIA_Frame, MUIV_Frame_InputList,
		MUIA_Background, MUII_ListBack,
		MUIA_List_Format, "BAR,BAR,BAR",
		MUIA_List_Title, TRUE,
		MUIA_List_DragSortable, TRUE,
		MUIA_List_DragType, MUIV_List_DragType_Immediate,
	TAG_MORE, msg->ops_AttrList);

	return (IPTR)obj;
}


//==============================================================================================
// ArgumentListListConstruct()
//==============================================================================================

IPTR ArgumentListListConstruct(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Construct *msg)
{
	struct ArgumentEntry *entry, *input;

	input = (struct ArgumentEntry*)msg->entry;

	if (entry = AllocTaskPooled(sizeof(struct ArgumentEntry)))
	{			
		if (entry->ae_Name = StrNew(input->ae_Name))
		{			
			
			if (entry->ae_Type = StrNew(input->ae_Type))
			{
				entry->ae_Register = input->ae_Register;
				return (IPTR)entry;
			}

			StrFree(input->ae_Name);
		}

		FreeTaskPooled(entry, sizeof(struct ArgumentEntry));
	}
	return 0;
}


//==============================================================================================
// ArgumentListListDestruct()
//==============================================================================================

IPTR ArgumentListListDestruct(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Destruct *msg)
{
	struct ArgumentEntry *entry = (struct ArgumentEntry*)msg->entry;

	if (entry)
	{
		if (entry->ae_Name) StrFree(entry->ae_Name);
		if (entry->ae_Type) StrFree(entry->ae_Type);
		FreeTaskPooled(entry, sizeof(struct ArgumentEntry));
	}
	return 0;
}


//==============================================================================================
// ArgumentListListDisplay()
//==============================================================================================

IPTR ArgumentListListDisplay(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Display *msg)
{
	struct ArgumentListData *d = (struct ArgumentListData*)INST_DATA(cl, obj);
	struct ArgumentEntry *ae = (struct ArgumentEntry*)msg->entry;
	
	// The MUIM_List_Display method of the function list returns pointer to a local 
	// variable which contains the string to be displayed. However, this pointer and the array are invalid 
	// as soon as the function is left and hence the string might get overwritten by other random data 
	// which eventually are displayed by MUI.
	#ifdef __amigaos4__
	static char type_aligned[48];
	#else
	char type_aligned[48];
	#endif
	

	if (!ae)
	{
		msg->array[0] = LS(MSG_ARGLIST_HEADER_TYPE, "\33rType");
		msg->array[1] = LS(MSG_ARGLIST_HEADER_NAME, "\33lFormal Name");
		msg->array[2] = LS(MSG_ARGLIST_HEADER_REGISTER, "\33lReg.");
	}
	else
	{
		if (ae->ae_Register < 8)
		{
			d->M68kRegBuf[0] = 'D';
			d->M68kRegBuf[1] = '0' + ae->ae_Register;
		}
		else if (ae->ae_Register < 12)
		{
			d->M68kRegBuf[0] = 'A';
			d->M68kRegBuf[1] = '0' + ae->ae_Register - 8;
		}
		else
		{
			d->M68kRegBuf[0] = '-';
			d->M68kRegBuf[1] = '-';
		}

		d->M68kRegBuf[2] = 0x00;
		
		FmtNPut(type_aligned, "\33r%s", 48, ae->ae_Type);		
		
		msg->array[0] = type_aligned;
		msg->array[1] = ae->ae_Name;
		msg->array[2] = d->M68kRegBuf;
	}

	return 0;
}


//==============================================================================================
// ArgumentListInsertMethodTable()
//==============================================================================================

#if 0
IPTR ArgumentListInsertMethodTable(UNUSED Class *cl, Object *obj, struct MTLP_InsertMethodTable *msg)
{
	struct MethodEntry *me = msg->Table;

	xset(obj, MUIA_List_Quiet, TRUE);

	while (me->Name)
	{
		DoMethod(obj, MUIM_List_InsertSingle, (IPTR)me++, MUIV_List_Insert_Bottom);
	}

	xset(obj, MUIA_List_Quiet, FALSE);
	return 0;
}
#endif

//==============================================================================================
// ArgumentListDispatcher()
//==============================================================================================

#ifdef __amigaos4__
LONG ArgumentListDispatcher(Class *cl, Object *obj,	Msg msg)
#else
LONG ArgumentListDispatcher(void)
{
	Class *cl = (Class*)REG_A0;
	Object *obj = (Object*)REG_A2;
	Msg msg = (Msg)REG_A1;
#endif
{

	switch (msg->MethodID)
	{
		case OM_NEW:                  return ArgumentListNew(cl, obj, (struct opSet*)msg);
		case MUIM_List_Construct:     return ArgumentListListConstruct(cl, obj, (struct MUIP_List_Construct*)msg);
		case MUIM_List_Destruct:      return ArgumentListListDestruct(cl, obj, (struct MUIP_List_Destruct*)msg);
		case MUIM_List_Display:       return ArgumentListListDisplay(cl, obj, (struct MUIP_List_Display*)msg);
		//case MTLM_InsertMethodTable:  return ArgumentListInsertMethodTable(cl, obj, (struct MTLP_InsertMethodTable*)msg);
		default:                      return (DoSuperMethodA(cl, obj, msg));
	}
}
