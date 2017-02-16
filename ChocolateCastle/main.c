/* ChocolateCastle */
#include <proto/exec.h>
#if defined(__MORPHOS__)
#include <exec/rawfmt.h>
#endif
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <clib/alib_protos.h>
#include <mui/Rawimage_mcc.h>
#include <stdint.h>

#include "support.h"
#include "generator.h"
#include "castleapp.h"
#include "muigenerator.h"
#include "reggenerator.h"
#include "methodlist.h"
#include "editor.h"
#include "methodeditor.h"
#include "logo.h"
#include "locale.h"

#if defined(__amigaos4__)
struct Library *UtilityBase;
struct Library *IntuitionBase;
struct Library *MUIMasterBase;
struct Library *LocaleBase;
struct Library *IconBase;

struct UtilityIFace  	*IUtility = NULL;
struct IntuitionIFace 	*IIntuition = NULL;
struct MUIMasterIFace	*IMUIMaster	= NULL;
struct LocaleIFace   	*ILocale = NULL;
struct IconIFace        *IIcon = NULL;
#else
struct UtilityBase *UtilityBase;
struct IntuitionBase *IntuitionBase;
struct Library *MUIMasterBase;
struct LocaleBase *LocaleBase;
struct Library *IconBase;
#endif

#ifdef __amigaos4__
static const char *  __attribute__((used)) stackcookie = "$STACK: 100000";
#endif



APTR MPool;
Object *App, *InWin, *MuiBtn, *RegBtn, *GenWin;
struct Catalog *Cat;

/// get_classes()

BOOL get_classes(void)
{
	if (!(GeneratorClass = CreateGeneratorClass())) return FALSE;
	if (!(CastleAppClass = CreateCastleAppClass())) return FALSE;
	if (!(MuiGeneratorClass = CreateMuiGeneratorClass())) return FALSE;
	if (!(RegGeneratorClass = CreateRegGeneratorClass())) return FALSE;
	if (!(MethodListClass = CreateMethodListClass())) return FALSE;
	if (!(EditorClass = CreateEditorClass())) return FALSE;
	if (!(MethodEditorClass = CreateMethodEditorClass())) return FALSE;
	return TRUE;
}

///
/// free_classes

void free_classes(void)
{
	DeleteMethodEditorClass();
	DeleteEditorClass();
	DeleteMethodListClass();
	DeleteRegGeneratorClass();
	DeleteMuiGeneratorClass();
	DeleteCastleAppClass();
	DeleteGeneratorClass();
}

///
/// get_resources()



BOOL get_resources(void)
{
	#if defined(__amigaos4__)
	if ((MPool = AllocSysObjectTags(ASOT_MEMPOOL,
      ASOPOOL_MFlags,    MEMF_SHARED,
      ASOPOOL_Puddle,    16384,
      ASOPOOL_Threshold, 16384,
      ASOPOOL_Name,      (ULONG)"ChocolateCastle shared pool",
      ASOPOOL_LockMem,   FALSE,
      TAG_DONE)) != NULL)
	#else
	if ((MPool = CreatePool(MEMF_ANY, 16384, 16384)) != NULL)
	#endif
	{
		if ((UtilityBase = (APTR)OpenLibrary((STRPTR)"utility.library", 39)) != NULL)
		{
			#if defined(__amigaos4__)
			IUtility = (struct UtilityIFace *)GetInterface(UtilityBase, "main", 1, NULL);
			#endif

			if ((IntuitionBase = (APTR)OpenLibrary((STRPTR)"intuition.library", 39)) != NULL)
			{
				#if defined(__amigaos4__)
				IIntuition = (struct IntuitionIFace *)GetInterface(IntuitionBase, "main", 1, NULL);
				#endif

				if ((MUIMasterBase = (APTR)OpenLibrary((STRPTR)"muimaster.library", 19)) != NULL)
				{
					#if defined(__amigaos4__)
					IMUIMaster = (struct MUIMasterIFace *)GetInterface(MUIMasterBase, "main", 1, NULL);
					#endif

					if ((LocaleBase = (APTR)OpenLibrary((STRPTR)"locale.library", 50)) != NULL)
					{
						#if defined(__amigaos4__)
						ILocale = (struct LocaleIFace *)GetInterface(LocaleBase, "main", 1, NULL);
						#endif

						if ((IconBase = (APTR)OpenLibrary((STRPTR)"icon.library", 39)) != NULL)
						{
							#if defined(__amigaos4__)
							IIcon = (struct IconIFace *)GetInterface(IconBase, "main", 1, NULL);
							#endif

							Cat = OpenCatalog(NULL, (STRPTR)"ChocolateCastle.catalog", TAG_END);
							if (get_classes()) return TRUE;
						}
					}
				}
			}
		}
	}
	return FALSE;
}

///
/// free_resources()

void free_resources(void)
{
	free_classes();
	CloseCatalog(Cat);
	#if defined(__amigaos4__)
	if(IIcon != NULL)
		DropInterface((struct Interface *)IIcon);

	if(ILocale != NULL)
		DropInterface((struct Interface *)ILocale);

	if(IMUIMaster != NULL)
		DropInterface((struct Interface *)IMUIMaster);

	if(IIntuition != NULL)
		DropInterface((struct Interface *)IIntuition);

	if(IUtility != NULL)
		DropInterface((struct Interface *)IUtility);
	#endif

	if (IconBase) CloseLibrary((struct Library *)IconBase);
	if (LocaleBase) CloseLibrary((struct Library *)LocaleBase);
	if (MUIMasterBase) CloseLibrary((struct Library *)MUIMasterBase);
	if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
	if (UtilityBase) CloseLibrary((struct Library *)UtilityBase);
	if (MPool)
	{
		#if defined(__amigaos4__)
		FreeSysObject(ASOT_MEMPOOL, MPool);
		#else
		DeletePool(MPool);
		#endif
	}
	return;
}

///
/// build_gui()

BOOL build_gui(void)
{
	App = NewObjectM(CastleAppClass->mcc_Class, NULL,
		MUIA_Application_Window, InWin = MUI_NewObjectM(MUIC_Window,
			MUIA_Window_Title, LS(MSG_MAIN_WINDOW_TITLE, "ChocolateCastle - Project Type Selection"),
			MUIA_Window_ScreenTitle, (IPTR)ScreenTitle,
			MUIA_Window_ID, 0x494E4954,
			MUIA_Window_RootObject, MUI_NewObjectM(MUIC_Group,
				MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
					MUIA_Group_Horiz, TRUE,
					MUIA_Group_Child, MUI_NewObjectM(MUIC_Rawimage,
						MUIA_Rawimage_Data, logo2,
					TAG_END),
					MUIA_Group_Child, MUI_NewObjectM(MUIC_Rawimage,
						MUIA_Rawimage_Data, logo1,
					TAG_END),
					MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
					TAG_END),
				TAG_END),
				MUIA_Group_Child, MUI_NewObjectM(MUIC_Group,
					MUIA_Group_Horiz, TRUE,
					MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
					TAG_END),
					MUIA_Group_Child, MuiBtn = MUI_NewObjectM(MUIC_Text,
						MUIA_CycleChain, TRUE,
						MUIA_Frame, MUIV_Frame_Button,
						MUIA_Background, MUII_ButtonBack,
						MUIA_Font, MUIV_Font_Button,
						MUIA_ControlChar, LS(MSG_BUTTON_MUI_CLASS_HOTKEY, "m")[0],
						MUIA_Text_HiChar, LS(MSG_BUTTON_MUI_CLASS_HOTKEY, "m")[0],
						MUIA_Text_Contents, LS(MSG_PROJECT_SELECTOR_BUTTON_MUI_CLASS, "MUI Class"),
						MUIA_Text_PreParse, "\33c",
						MUIA_InputMode, MUIV_InputMode_RelVerify,
					TAG_END),

					// no Reggae for os4
					#ifndef __amigaos4__
					MUIA_Group_Child, RegBtn = MUI_NewObjectM(MUIC_Text,
						MUIA_CycleChain, TRUE,
						MUIA_Frame, MUIV_Frame_Button,
						MUIA_Background, MUII_ButtonBack,
						MUIA_Font, MUIV_Font_Button,
						MUIA_ControlChar, LS(MSG_BUTTON_REGGAE_CLASS_HOTKEY, "r")[0],
						MUIA_Text_HiChar, LS(MSG_BUTTON_REGGAE_CLASS_HOTKEY, "r")[0],
						MUIA_Text_Contents, LS(MSG_PROJECT_SELECTOR_BUTTON_REGGAE_CLASS, "Reggae Class"),
						MUIA_Text_PreParse, "\33c",
						MUIA_InputMode, MUIV_InputMode_RelVerify,
					TAG_END),
					#endif

					MUIA_Group_Child, MUI_NewObjectM(MUIC_Rectangle,
					TAG_END),
				TAG_END),
			TAG_END),
		TAG_END),
	TAG_END);

	if (App) return TRUE;
	else return FALSE;
}

///
/// main_loop()

void main_loop(void)
{
	ULONG signals;
	LONG return_id;
	BOOL running = TRUE;

	SetAttrs (InWin, MUIA_Window_Open, TRUE, TAG_END);

	while (running)
	{
		return_id = DoMethod(App, MUIM_Application_NewInput, (intptr_t)&signals);

    if (signals)
    {
      signals = Wait (signals | SIGBREAKF_CTRL_C);
      if (signals & SIGBREAKF_CTRL_C) break;
    }

		if (return_id == MUIV_Application_ReturnID_Quit) running = FALSE;

		if (return_id == MUIV_Application_ReturnID_RemGen)
		{
			DoMethod(App, CAAM_RemoveGenerator, (IPTR)GenWin);
		}
	}

	SetAttrs (InWin, MUIA_Window_Open, FALSE, TAG_END);
    return;
}

///
/// notifications()

void notifications (void)
{
	DoMethod(InWin, MUIM_Notify, MUIA_Window_CloseRequest, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
	DoMethod(MuiBtn, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, CAAM_Generate, PROJECT_TYPE_MUI);
	DoMethod(RegBtn, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, CAAM_Generate, PROJECT_TYPE_REGGAE);
}

///
/// main()

int main(void)
{
	if (get_resources())
	{
		InitializeStrings();
		if (build_gui())
		{
			notifications();
			main_loop();
			MUI_DisposeObject(App);
		}
	}
	free_resources();
	return 0;
}

///
