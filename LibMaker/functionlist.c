/* FunctionListClass code. */


#include "main.h"
#include "functionlist.h"
#include "libvstring/libvstring.h"

#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/locale.h>
#include <clib/alib_protos.h>


struct MUI_CustomClass *FunctionListClass;

DISPATCHERPROTO(FunctionListDispatcher);

struct FunctionListData
{
	char TypeAligned[48];           // used in MUIM_List_Display()
	char *ArgBuf;                   // used in MUIM_List_Display(), dynamically reallocated;
	LONG ArgBufLen;
};


//==============================================================================================
// CreateFunctionListClass()
//==============================================================================================

struct MUI_CustomClass *CreateFunctionListClass(void)
{
	struct MUI_CustomClass *cl;

	cl = MUI_CreateCustomClass(NULL, MUIC_List, NULL, sizeof(struct FunctionListData), ENTRY(FunctionListDispatcher));
	FunctionListClass = cl;
	return cl;
}



//==============================================================================================
// DeleteFunctionListClass()
//==============================================================================================

void DeleteFunctionListClass(void)
{
	MUI_DeleteCustomClass(FunctionListClass);
}


//==============================================================================================
// FunctionListNew()
//==============================================================================================

IPTR FunctionListNew(Class *cl, Object *obj, struct opSet *msg)
{
	obj = DoSuperNewM(cl, obj,
		MUIA_Frame, MUIV_Frame_InputList,
		MUIA_Background, MUII_ListBack,
		MUIA_List_Format, "BAR,BAR,BAR",
		MUIA_List_Title, TRUE,
	TAG_MORE, msg->ops_AttrList);

	return (IPTR)obj;
}



//==============================================================================================
// FunctionListDispose()
//==============================================================================================

IPTR FunctionListDispose(Class *cl, Object *obj, Msg msg)
{
	struct FunctionListData *d = INST_DATA(cl, obj);

	if (d->ArgBuf) FreeTaskPooled(d->ArgBuf, d->ArgBufLen);

	return DoSuperMethodA(cl, obj, msg);
}


//==============================================================================================
// FreeArguments()
//==============================================================================================

static void FreeArguments(struct FunctionArgument *argtable, LONG entries)
{
	LONG i;

	if (argtable)
	{
		for (i = 0; i < entries; i++)
		{
			if (argtable[i].fa_Name) StrFree(argtable[i].fa_Name);
			if (argtable[i].fa_Type) StrFree(argtable[i].fa_Type);
		}

		FreeVecTaskPooled(argtable);
	}
}



//==============================================================================================
// CloneArguments()
//==============================================================================================

static struct FunctionArgument *CloneArguments(struct FunctionArgument *orig, LONG entries)
{
	struct FunctionArgument *clone;
	LONG i;
	BOOL success = FALSE;

	if (clone = AllocVecTaskPooled(sizeof(struct FunctionArgument) * entries))
	{
		/*-----------------*/
		/* Clear it first. */
		/*-----------------*/

		for (i = 0; i < entries; i++)
		{
			clone[i].fa_Name = NULL;
			clone[i].fa_Type = NULL;
		}

		success = TRUE;

		/*--------------------------------------*/
		/* Copy entries. Fail in case of error. */
		/*--------------------------------------*/

		for (i = 0; i < entries; i++)
		{
			clone[i].fa_Name = StrNew(orig[i].fa_Name);
			clone[i].fa_Type = StrNew(orig[i].fa_Type);
			clone[i].fa_Register = orig[i].fa_Register;

			if (!clone[i].fa_Name || !clone[i].fa_Type)
			{
				success = FALSE;
				break;
			}
		}
	}

	if (success) return clone;
	else
	{
		FreeArguments(clone, entries);
		return NULL;
	}
}


//==============================================================================================
// GenerateArgumentString()
//==============================================================================================

char* GenerateArgumentString(struct FunctionListData *d, struct FunctionEntry *fe)
{
	LONG bufsize, i;
	BOOL have_buffer = FALSE;

	/*-----------------------------------------------------------------*/
	/* Calculate buffer size. Elements are as follows:                 */
	/* - Constants: two parentheses, null-terminator: 3 bytes.         */
	/* - Lengths of argument types and formal names.                   */
	/* - A space between each type and formal name: numargs.           */
	/* - A space and comma between each argument: (numargs - 1) * 2.   */
	/* If there are 0 arguments, there will be 7 bytes: "(void)".      */
	/*-----------------------------------------------------------------*/

	if (fe->fe_ArgCount == 0) bufsize = 7;
	else
	{
		bufsize = 3 * fe->fe_ArgCount + 1;

		for (i = 0; i < fe->fe_ArgCount; i++)
		{
			bufsize += StrLen(fe->fe_Arguments[i].fa_Name);
			bufsize += StrLen(fe->fe_Arguments[i].fa_Type);
		}
	}

	/*--------------------------------*/
	/* Buffer reallocation if needed. */
	/*--------------------------------*/

	if (bufsize > d->ArgBufLen)
	{
		char *newbuf;

		if (newbuf = AllocTaskPooled(bufsize))
		{
			FreeTaskPooled(d->ArgBuf, d->ArgBufLen);
			d->ArgBuf = newbuf;
			d->ArgBufLen = bufsize;
			have_buffer = TRUE;
		}
	}
	else have_buffer = TRUE;

	/*-------------*/
	/* Sprintfing. */
	/*-------------*/

	if (have_buffer)
	{
		char *ptr = d->ArgBuf;

		if (fe->fe_ArgCount == 0) StrCopy("(void)", ptr);
		else
		{
			ptr = StrCopy("(", ptr);

			for (i = 0; i < fe->fe_ArgCount; i++)
			{
				ptr = StrCopy(fe->fe_Arguments[i].fa_Type, ptr);
				ptr = StrCopy(" ", ptr);
				ptr = StrCopy(fe->fe_Arguments[i].fa_Name, ptr);
				if (i < fe->fe_ArgCount - 1) ptr = StrCopy(", ", ptr);
			}

			StrCopy(")", ptr);
		}

		return d->ArgBuf;
	}

	return "";      // no memory for buffer, cannot think about anything smarter...
}


//==============================================================================================
// FunctionListListConstruct()
//==============================================================================================

IPTR FunctionListListConstruct(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Construct *msg)
{
	struct FunctionEntry *entry, *input;

	input = (struct FunctionEntry*)msg->entry;

	if (entry = AllocTaskPooled(sizeof(struct FunctionEntry)))
	{
		entry->fe_ArgCount = input->fe_ArgCount;

		if (entry->fe_Name = StrNew(input->fe_Name))
		{
			if (entry->fe_ReturnType = StrNew(input->fe_ReturnType))
			{
				if (input->fe_ArgCount == 0)
				{
					entry->fe_Arguments = NULL;
					return (IPTR)entry;
				}

				if (entry->fe_Arguments = CloneArguments(input->fe_Arguments, input->fe_ArgCount))
				{
					return (IPTR)entry;
				}

				StrFree(entry->fe_ReturnType);
			}

			StrFree(entry->fe_Name);
		}

		FreeTaskPooled(entry, sizeof(struct FunctionEntry));
	}

	return 0;
}


//==============================================================================================
// FunctionListListDestruct()
//==============================================================================================

IPTR FunctionListListDestruct(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Destruct *msg)
{
	struct FunctionEntry *entry = (struct FunctionEntry*)msg->entry;

	if (entry)
	{
		if (entry->fe_Name) StrFree(entry->fe_Name);
		if (entry->fe_ReturnType) StrFree(entry->fe_ReturnType);
		FreeArguments(entry->fe_Arguments, entry->fe_ArgCount);
		FreeTaskPooled(entry, sizeof(struct FunctionEntry));
	}
	return 0;
}


//==============================================================================================
// FunctionListListDisplay()
//==============================================================================================

IPTR FunctionListListDisplay(UNUSED Class *cl, UNUSED Object *obj, struct MUIP_List_Display *msg)
{
	struct FunctionListData *d = INST_DATA(cl, obj);
	struct FunctionEntry *fe = (struct FunctionEntry*)msg->entry;

	// The MUIM_List_Display method of the function list returns pointer to a local
	// variable which contains the string to be displayed. However, this pointer and the array are invalid
	// as soon as the function is left and hence the string might get overwritten by other random data
	// which eventually are displayed by MUI.
	#ifdef __amigaos4__
	static char type_aligned[48];
	#else
	char type_aligned[48];
	#endif


	if (!fe)
	{
		msg->array[0] = LS(MSG_FUNCLIST_HEADER_RESULT_TYPE, "\33cResult Type");
		msg->array[1] = LS(MSG_FUNCLIST_HEADER_NAME, "\33cName");
		msg->array[2] = LS(MSG_FUNCLIST_HEADER_ARGUMENTS, "\33cArguments");
	}
	else
	{
		FmtNPut(type_aligned, "\33r%s", 48, fe->fe_ReturnType);
		msg->array[0] = type_aligned;
		msg->array[1] = fe->fe_Name;
		msg->array[2] = GenerateArgumentString(d, msg->entry);
	}

	return 0;
}


//==============================================================================================
// FunctionListDispatcher()
//==============================================================================================

DISPATCHER(FunctionListDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW:                  return FunctionListNew(cl, obj, (struct opSet*)msg);
		case MUIM_List_Construct:     return FunctionListListConstruct(cl, obj, (struct MUIP_List_Construct*)msg);
		case MUIM_List_Destruct:      return FunctionListListDestruct(cl, obj, (struct MUIP_List_Destruct*)msg);
		case MUIM_List_Display:       return FunctionListListDisplay(cl, obj, (struct MUIP_List_Display*)msg);
		//case MTLM_InsertMethodTable:  return FunctionListInsertMethodTable(cl, obj, (struct MTLP_InsertMethodTable*)msg);
		default:                      return (DoSuperMethodA(cl, obj, msg));
	}
}
