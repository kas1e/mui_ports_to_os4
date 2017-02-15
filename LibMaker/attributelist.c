/* AttributeListClass code. */


#include "main.h"
#include "attributelist.h"
#include "libvstring/libvstring.h"

#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/locale.h>
#include <clib/alib_protos.h>


struct MUI_CustomClass *AttributeListClass;

#ifdef __amigaos4__
LONG AttributeListDispatcher(Class *cl,Object * obj,	Msg msg);
#else
LONG AttributeListDispatcher(void);
const struct EmulLibEntry AttributeListGate = {TRAP_LIB, 0, (void(*)(void))AttributeListDispatcher};
#endif

struct AttributeListData
{
	char IdBuf[12];
	char UsageBuf[4];
};


//==============================================================================================
// CreateAttributeListClass()
//==============================================================================================

struct MUI_CustomClass *CreateAttributeListClass(void)
{
	struct MUI_CustomClass *cl;

	#ifdef __amigaos4__
	cl = MUI_CreateCustomClass(NULL, MUIC_List, NULL, sizeof(struct AttributeListData), (APTR)&AttributeListDispatcher);
	#else
	cl = MUI_CreateCustomClass(NULL, MUIC_List, NULL, sizeof(struct AttributeListData), (APTR)&AttributeListGate);
	#endif
	AttributeListClass = cl;
	return cl;
}



//==============================================================================================
// DeleteAttributeListClass()
//==============================================================================================

void DeleteAttributeListClass(void)
{
	MUI_DeleteCustomClass(AttributeListClass);
}


//==============================================================================================
// AttributeListNew()
//==============================================================================================

IPTR AttributeListNew(Class *cl, Object *obj, struct opSet *msg)
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
// AttributeListListConstruct()
//==============================================================================================

IPTR AttributeListListConstruct(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Construct *msg)
{
	struct AttributeEntry *entry, *input;

	input = (struct AttributeEntry*)msg->entry;

	if (entry = AllocTaskPooled(sizeof(struct AttributeEntry)))
	{
		if (entry->ae_Name = StrNew(input->ae_Name))
		{
			entry->ae_Id = input->ae_Id;
			entry->ae_Usage = input->ae_Usage;
			return (IPTR)entry;
		}

		FreeTaskPooled(entry, sizeof(struct AttributeEntry));
	}
	return 0;
}



//==============================================================================================
// AttributeListListDestruct()
//==============================================================================================

IPTR AttributeListListDestruct(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Destruct *msg)
{
	struct AttributeEntry *entry = (struct AttributeEntry*)msg->entry;

	if (entry)
	{
		if (entry->ae_Name) StrFree(entry->ae_Name);
		FreeTaskPooled(entry, sizeof(struct AttributeEntry));
	}
	return 0;
}



//==============================================================================================
// AttributeListListDisplay()
//==============================================================================================

IPTR AttributeListListDisplay(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Display *msg)
{
	struct AttributeEntry *ae = (struct AttributeEntry*)msg->entry;
	struct AttributeListData *d = INST_DATA(cl, obj);

	if (!ae)
	{
		msg->array[0] = LS(MSG_ATTRLIST_HEADER_TYPE, "\33lName");
		msg->array[1] = LS(MSG_ATTRLIST_HEADER_ID, "\33lIdentifier");
		msg->array[2] = LS(MSG_ATTRLIST_HEADER_USAGE, "\33lUsage");
	}
	else
	{
		FmtNPut(d->IdBuf, "$%08lX", 12, ae->ae_Id);
		FmtNPut(d->UsageBuf, "%s%s%s", 4, (ae->ae_Usage & ATTR_USAGE_INIT) ? "I" : "", (ae->ae_Usage & ATTR_USAGE_SET) ? "S" : "",
			(ae->ae_Usage & ATTR_USAGE_GET) ? "G" : "");
		msg->array[0] = ae->ae_Name;
		msg->array[1] = d->IdBuf;
		msg->array[2] = d->UsageBuf;
	}

	return 0;
}



//==============================================================================================
// AttributeListDispatcher()
//==============================================================================================

#ifdef __amigaos4__
LONG AttributeListDispatcher(Class *cl,Object * obj,	Msg msg)
#else
LONG AttributeListDispatcher(void)
{
	Class *cl = (Class*)REG_A0;
	Object *obj = (Object*)REG_A2;
	Msg msg = (Msg)REG_A1;
#endif
{
	switch (msg->MethodID)
	{
		case OM_NEW:                  return AttributeListNew(cl, obj, (struct opSet*)msg);
		case MUIM_List_Construct:     return AttributeListListConstruct(cl, obj, (struct MUIP_List_Construct*)msg);
		case MUIM_List_Destruct:      return AttributeListListDestruct(cl, obj, (struct MUIP_List_Destruct*)msg);
		case MUIM_List_Display:       return AttributeListListDisplay(cl, obj, (struct MUIP_List_Display*)msg);
		default:                      return (DoSuperMethodA(cl, obj, msg));
	}
}
