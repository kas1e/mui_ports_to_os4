/* MethodListClass code. */


#include "main.h"
#include "methodlist.h"
#include "libvstring/libvstring.h"

#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/locale.h>
#include <clib/alib_protos.h>


struct MUI_CustomClass *MethodListClass;

#ifdef __amigaos4__
LONG MethodListDispatcher(Class *cl,Object * obj,	Msg msg);
#else
LONG MethodListDispatcher(void);
const struct EmulLibEntry MethodListGate = {TRAP_LIB, 0, (void(*)(void))MethodListDispatcher};
#endif

struct MethodListData
{
	char IdBuf[12];
};



//==============================================================================================
// CreateMethodListClass()
//==============================================================================================

struct MUI_CustomClass *CreateMethodListClass(void)
{
	struct MUI_CustomClass *cl;
	
	#ifdef __amigaos4__
	cl = MUI_CreateCustomClass(NULL, MUIC_List, NULL, sizeof(struct MethodListData), (APTR)&MethodListDispatcher);
	#else
	cl = MUI_CreateCustomClass(NULL, MUIC_List, NULL, sizeof(struct MethodListData), (APTR)&MethodListGate);
	#endif
	
	MethodListClass = cl;
	return cl;
}



//==============================================================================================
// DeleteMethodListClass()
//==============================================================================================

void DeleteMethodListClass(void)
{
	MUI_DeleteCustomClass(MethodListClass);
}



//==============================================================================================
// MethodListNew()
//==============================================================================================

IPTR MethodListNew(Class *cl, Object *obj, struct opSet *msg)
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
// MethodListListConstruct()
//==============================================================================================

IPTR MethodListListConstruct(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Construct *msg)
{
	struct MethodEntry *entry, *input;

	input = (struct MethodEntry*)msg->entry;

	if (entry = AllocTaskPooled(sizeof(struct MethodEntry)))
	{
		if (entry->me_Name = StrNew(input->me_Name))
		{
			if (entry->me_Message = StrNew(input->me_Message))
			{
				entry->me_Id = input->me_Id;
				entry->me_ArgCount = input->me_ArgCount;
				return (IPTR)entry;
			}

			StrFree(entry->me_Name);
		}

		FreeTaskPooled(entry, sizeof(struct MethodEntry));
	}
	return 0;
}



//==============================================================================================
// MethodListListDestruct()
//==============================================================================================

IPTR MethodListListDestruct(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Destruct *msg)
{
	struct MethodEntry *entry = (struct MethodEntry*)msg->entry;

	if (entry)
	{
		if (entry->me_Name) StrFree(entry->me_Name);
		if (entry->me_Message) StrFree(entry->me_Message);
		FreeTaskPooled(entry, sizeof(struct MethodEntry));
	}
	return 0;
}



//==============================================================================================
// MethodListListDisplay()
//==============================================================================================

IPTR MethodListListDisplay(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Display *msg)
{
	struct MethodListData *d = INST_DATA(cl, obj);
	struct MethodEntry *me = (struct MethodEntry*)msg->entry;

	if (!me)
	{
		msg->array[0] = LS(MSG_METHLIST_HEADER_NAME, "\33lName");
		msg->array[1] = LS(MSG_METHLIST_HEADER_IDENT, "\33lIdentifier");
		msg->array[2] = LS(MSG_METHLIST_HEADER_MESSAGE, "\33lMessage");
	}
	else
	{
		FmtNPut(d->IdBuf, "$%08lX", 12, me->me_Id);
		msg->array[0] = me->me_Name;
		msg->array[1] = d->IdBuf;
		msg->array[2] = me->me_Message;
	}

	return 0;
}



//==============================================================================================
// MethodListDispatcher()
//==============================================================================================

#ifdef __amigaos4__
LONG MethodListDispatcher(Class *cl,Object * obj,	Msg msg)
#else
LONG MethodListDispatcher(void)
{
	Class *cl = (Class*)REG_A0;
	Object *obj = (Object*)REG_A2;
	Msg msg = (Msg)REG_A1;
#endif
{

	switch (msg->MethodID)
	{
		case OM_NEW:                  return MethodListNew(cl, obj, (struct opSet*)msg);
		case MUIM_List_Construct:     return MethodListListConstruct(cl, obj, (struct MUIP_List_Construct*)msg);
		case MUIM_List_Destruct:      return MethodListListDestruct(cl, obj, (struct MUIP_List_Destruct*)msg);
		case MUIM_List_Display:       return MethodListListDisplay(cl, obj, (struct MUIP_List_Display*)msg);
		//case MTLM_InsertMethodTable:  return MethodListInsertMethodTable(cl, obj, (struct MTLP_InsertMethodTable*)msg);
		default:                      return (DoSuperMethodA(cl, obj, msg));
	}
}
