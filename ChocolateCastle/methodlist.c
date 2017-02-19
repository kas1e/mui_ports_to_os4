/*------------------------------------------*/
/* Code generated with ChocolateCastle 0.1  */
/* written by Grzegorz "Krashan" Kraszewski */
/* <krashan@teleinfo.pb.bialystok.pl>       */
/*------------------------------------------*/


/* MethodListClass code. */

#include <libvstring.h>

#include "methodlist.h"
#include "support.h"
#include "locale.h"

#include <SDI_hook.h>

struct MUI_CustomClass *MethodListClass;

DISPATCHERPROTO(MethodListDispatcher);

/// MethodListData

struct MethodListData
{
	char IdBuf[10];
};


///
/// CreateMethodListClass()

struct MUI_CustomClass *CreateMethodListClass(void)
{
	struct MUI_CustomClass *cl;

	cl = MUI_CreateCustomClass(NULL, (STRPTR)MUIC_List, NULL, sizeof(struct MethodListData), ENTRY(MethodListDispatcher));
	MethodListClass = cl;
	return cl;
}


///
/// DeleteMethodListClass()

void DeleteMethodListClass(void)
{
	MUI_DeleteCustomClass(MethodListClass);
}


///

/// MethodListNew()

IPTR MethodListNew(Class *cl, Object *obj, struct opSet *msg)
{
	obj = DoSuperNewM(cl, obj,
		MUIA_Frame, MUIV_Frame_InputList,
		MUIA_List_Format, "BAR,BAR,BAR,BAR",
		MUIA_List_Title, TRUE,
	TAG_MORE, msg->ops_AttrList);

	return (IPTR)obj;
}


///
/// MethodListListConstruct()

IPTR MethodListListConstruct(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Construct *msg)
{
	struct MethodEntry *ent, *ine;

	ine = (struct MethodEntry*)msg->entry;

	if ((ent = AllocPooled(msg->pool, sizeof(struct MethodEntry))) != NULL)
	{
		if ((ent->Name = AllocVecPooled(msg->pool, strln(ine->Name) + 1)) != NULL)
		{
			if ((ent->Structure = AllocVecPooled(msg->pool, strln(ine->Structure) + 1)) != NULL)
			{
				if ((ent->Function = AllocVecPooled(msg->pool, strln(ine->Function) + 1)) != NULL)
				{
					if ((ent->Identifier = AllocVecPooled(msg->pool, strln(ine->Identifier) + 1)) != NULL)
					{
						StrCopy(ine->Name, ent->Name);
						StrCopy(ine->Structure, ent->Structure);
						StrCopy(ine->Function, ent->Function);
						StrCopy(ine->Identifier, ent->Identifier);
						ent->Standard = ine->Standard;
						return (IPTR)ent;
					}
					FreeVecPooled(msg->pool, ent->Identifier);
				}
				FreeVecPooled(msg->pool, ent->Structure);
			}
			FreeVecPooled(msg->pool, ent->Name);
		}
		FreePooled(msg->pool, ent, sizeof(struct MethodEntry));
	}
	return 0;
}


///
/// MethodListListDestruct()

IPTR MethodListListDestruct(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Destruct *msg)
{
	struct MethodEntry *ent = (struct MethodEntry*)msg->entry;

	if (ent)
	{
		if (ent->Function) FreeVecPooled(msg->pool, ent->Function);
		if (ent->Name) FreeVecPooled(msg->pool, ent->Name);
		if (ent->Structure) FreeVecPooled(msg->pool, ent->Structure);
		if (ent->Identifier) FreeVecPooled(msg->pool, ent->Identifier);
		FreePooled(msg->pool, ent, sizeof(struct MethodEntry));
	}
	return 0;
}


///
/// MethodListListDisplay()

IPTR MethodListListDisplay(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Display *msg)
{
	struct MethodEntry *ent = (struct MethodEntry*)msg->entry;

	if (!ent)
	{
		msg->array[0] = (STRPTR)LS(MSG_METHOD_LIST_HEADER_NAME, "\33cMethod Name");
		msg->array[1] = (STRPTR)LS(MSG_METHOD_LIST_HEADER_IDENTIFIER, "\33cIdentifier");
		msg->array[2] = (STRPTR)LS(MSG_METHOD_LIST_HEADER_FUNCTION, "\33cFunction");
		msg->array[3] = (STRPTR)LS(MSG_METHOD_LIST_HEADER_STRUCTURE, "\33cMessage Structure");
	}
	else
	{
		msg->array[0] = ent->Name;
		msg->array[1] = ent->Identifier;
		msg->array[2] = ent->Function;
		msg->array[3] = ent->Structure;
	}

	return 0;
}


///
/// MethodListInsertMethodTable()

IPTR MethodListInsertMethodTable(UNUSED Class *cl, Object *obj, struct MTLP_InsertMethodTable *msg)
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


///
/// MethodListDispatcher()

DISPATCHER(MethodListDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW:                  return MethodListNew(cl, obj, (struct opSet*)msg);
		case MUIM_List_Construct:     return MethodListListConstruct(cl, obj, (struct MUIP_List_Construct*)msg);
		case MUIM_List_Destruct:      return MethodListListDestruct(cl, obj, (struct MUIP_List_Destruct*)msg);
		case MUIM_List_Display:       return MethodListListDisplay(cl, obj, (struct MUIP_List_Display*)msg);
		case MTLM_InsertMethodTable:  return MethodListInsertMethodTable(cl, obj, (struct MTLP_InsertMethodTable*)msg);
		default:                      return (DoSuperMethodA(cl, obj, msg));
	}
}

///

