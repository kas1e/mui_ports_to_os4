/* CastleApp class body */

/* Code generated with ChocolateCastle 0.1 */
/* written by Grzegorz "Krashan" Kraszewski <krashan@teleinfo.pb.bialystok.pl> */

#include <proto/intuition.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>
#include <proto/muimaster.h>
#include <proto/icon.h>

#include "support.h"
#include "castleapp.h"
#include "generator.h"
#include "muigenerator.h"
#include "reggenerator.h"
#include "locale.h"

#include <SDI_hook.h>

struct MUI_CustomClass *CastleAppClass;

struct DiskObject *disk_object = NULL;


DISPATCHERPROTO(CastleAppDispatcher);

struct MUI_CustomClass *CreateCastleAppClass(void)
{
	struct MUI_CustomClass *cl;

	cl = MUI_CreateCustomClass(NULL, (STRPTR)MUIC_Application, NULL, sizeof(struct CastleAppData), ENTRY(CastleAppDispatcher));
	CastleAppClass = cl;
	return cl;
}


void DeleteCastleAppClass(void)
{
	 MUI_DeleteCustomClass(CastleAppClass);
}


intptr_t CastleAppNew(Class *cl, Object *obj, struct opSet *msg)
{
	Object *newobj = NULL;

	disk_object = GetDiskObject((STRPTR)"PROGDIR:ChocolateCastle");

	obj = DoSuperNewM(cl, obj,
		MUIA_Application_Author,        "Grzegorz Kraszewski",
		MUIA_Application_Base,          "CHOCOCASTLE",
		MUIA_Application_Copyright,     "2005-2010 Grzegorz Kraszewski",
		MUIA_Application_Description,   LS(MSG_APPLICATION_DESCRIPTION, "Code template generator"),
		MUIA_Application_Title,         "ChocolateCastle",
		MUIA_Application_Version,       "$VER: ChocolateCastle " CHC_VERSION " (" CHC_DATE ")",
		MUIA_Application_DiskObject,    disk_object,
	TAG_MORE, (ULONG)msg->ops_AttrList);

	if (obj)
	{
		newobj = obj;

		if (!newobj) CoerceMethod(cl, obj, OM_DISPOSE);
	}
	return (intptr_t)newobj;
}

intptr_t CastleAppDispose(Class *cl, Object *obj, Msg msg)
{
	intptr_t rc = DoSuperMethodA(cl, obj, msg);

	// dispose the icon after the application definitely ceased to exist
	if (disk_object) FreeDiskObject(disk_object);

	return rc;
}


intptr_t CastleAppGenerate(Class *cl, Object *obj, struct opCAA_Generate *msg)
{
	struct MUI_CustomClass *gen_class = NULL;

	cl = cl;

	switch (msg->ProjectType)
	{
		case PROJECT_TYPE_MUI:      gen_class = MuiGeneratorClass;  break;
		case PROJECT_TYPE_REGGAE:   gen_class = RegGeneratorClass;  break;
	}

	GenWin = NewObject(gen_class->mcc_Class, NULL, TAG_END);

	// added one more NULL pointer check.
	if (GenWin == NULL) {
		printf("damn! GenWin sucked!\n");
	}

	if (GenWin)
	{
		DoMethod(obj, OM_ADDMEMBER, (intptr_t)GenWin);

		DoMethod(GenWin, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 2,
		 MUIM_Application_ReturnID, MUIV_Application_ReturnID_RemGen);

	    xset(GenWin, MUIA_Window_Open, TRUE);

		/*------------------------------------------------------------------------*/
		/* Added 22.01.2008: Disable main window while generator window is        */
		/* opened.                                                                */
		/*------------------------------------------------------------------------*/

		xset(InWin, MUIA_Window_Sleep, TRUE);
	}

	return 0;
}


intptr_t CastleAppRemoveGenerator(UNUSED Class *cl, Object *obj, struct CAAP_RemoveGenerator *msg)
{
	xset(msg->Generator, MUIA_Window_Open, FALSE);
	DoMethod(obj, OM_REMMEMBER, (intptr_t)msg->Generator);
	MUI_DisposeObject(msg->Generator);

	/*------------------------------------------------------------------------*/
	/* Added 22.01.2008: Enable main window after generator window is closed. */
	/*------------------------------------------------------------------------*/

	xset(InWin, MUIA_Window_Sleep, FALSE);

  return 0;
}

DISPATCHER(CastleAppDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW:                return (CastleAppNew(cl, obj, (struct opSet*)msg));
		case OM_DISPOSE:            return (CastleAppDispose(cl, obj, msg));
		case CAAM_Generate:         return(CastleAppGenerate(cl, obj, (struct opCAA_Generate*)msg));
		case CAAM_RemoveGenerator:  return(CastleAppRemoveGenerator(cl, obj, (struct CAAP_RemoveGenerator*)msg));
		default:                    return (DoSuperMethodA(cl, obj, msg));
	}
}

