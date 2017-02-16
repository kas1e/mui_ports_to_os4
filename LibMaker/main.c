/* LibMaker */

#include "main.h"
#include "application.h"
#include "functionlist.h"
#include "functioneditor.h"
#include "argumentlist.h"
#include "classeditor.h"
#include "attributelist.h"
#include "methodlist.h"
#include "methodeditor.h"
#include "libvstring/libvstring.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/locale.h>
#include <dos/dos.h>
#include <workbench/startup.h>
#include <libraries/asl.h>
#include <workbench/workbench.h>
#include <proto/icon.h>
#include <SDI_stdarg.h>

struct DiskObject *disk_object = NULL;

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
#elif defined(__MORPHOS__)
struct Library *UtilityBase;
struct IntuitionBase *IntuitionBase;
struct Library *MUIMasterBase;
struct Library *LocaleBase;
struct Library *IconBase;
#else
struct UtilityBase *UtilityBase;
struct IntuitionBase *IntuitionBase;
struct Library *MUIMasterBase;
struct LocaleBase *LocaleBase;
struct Library *IconBase;
#endif

#if defined(__MORPHOS__)
struct Library *LuaBase;
#endif

#ifdef __amigaos4__
static const char *  __attribute__((used)) stackcookie = "$STACK: 100000";
#endif

struct Locale *Loc;
struct Catalog *Cat;

STRPTR UsedClasses[] = {0};


void InitStrings(void);


CONST_STRPTR HexChars = "0123456789ABCDEFabcdef";
CONST_STRPTR TypeChars = " *()0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
CONST_STRPTR IdentifierChars;

//==============================================================================================
// GetResources()
//==============================================================================================

BOOL GetResources(void)
{
	if (!(IntuitionBase = (APTR)OpenLibrary("intuition.library", 37))) return FALSE;
	if (!(MUIMasterBase = (APTR)OpenLibrary("muimaster.library", 20))) return FALSE;
	if (!(UtilityBase = (APTR)OpenLibrary("utility.library", 37))) return FALSE;
	if (!(LocaleBase = (APTR)OpenLibrary("locale.library", 37))) return FALSE;
	if (!(IconBase = (APTR)OpenLibrary("icon.library", 37))) return FALSE;
#if defined(__MORPHOS__)
	if (!(LuaBase = (APTR)OpenLibrary("lua.library", 51))) return FALSE;
#endif

#if defined(__amigaos4__)
	IIntuition = (struct IntuitionIFace *)GetInterface(IntuitionBase, "main", 1, NULL);
	IMUIMaster = (struct MUIMasterIFace *)GetInterface(MUIMasterBase, "main", 1, NULL);
	IUtility = (struct UtilityIFace *)GetInterface(UtilityBase, "main", 1, NULL);
	ILocale = (struct LocaleIFace *)GetInterface(LocaleBase, "main", 1, NULL);
	IIcon = (struct IconIFace *)GetInterface(IconBase, "main", 1, NULL);
#endif

	if (!CreateApplicationClass()) return FALSE;
	if (!CreateFunctionListClass()) return FALSE;
	if (!CreateFunctionEditorClass()) return FALSE;
	if (!CreateArgumentListClass()) return FALSE;
	if (!CreateClassEditorClass()) return FALSE;
	if (!CreateAttributeListClass()) return FALSE;
	if (!CreateMethodListClass()) return FALSE;
	if (!CreateMethodEditorClass()) return FALSE;
	Loc = OpenLocale(NULL);
	Cat = OpenCatalog(Loc, (STRPTR)(APP_NAME ".catalog"), TAG_END);
	return TRUE;
}


//==============================================================================================
// FreeResources()
//==============================================================================================

void FreeResources(void)
{
	CloseCatalog(Cat);
	CloseLocale(Loc);
	if (MethodEditorClass) DeleteMethodEditorClass();
	if (MethodListClass) DeleteMethodListClass();
	if (AttributeListClass) DeleteAttributeListClass();
	if (ClassEditorClass) DeleteClassEditorClass();
	if (ArgumentListClass) DeleteArgumentListClass();
	if (FunctionEditorClass) DeleteFunctionEditorClass();
	if (FunctionListClass) DeleteFunctionListClass();
	if (ApplicationClass) DeleteApplicationClass();
	if (disk_object) FreeDiskObject(disk_object);

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

#if defined(__MORPHOS__)
	if (LuaBase) CloseLibrary(LuaBase);
#endif
	if (IconBase) CloseLibrary((struct Library *)IconBase);
	if (LocaleBase) CloseLibrary((struct Library *)LocaleBase);
	if (UtilityBase) CloseLibrary((struct Library *)UtilityBase);
	if (MUIMasterBase) CloseLibrary((struct Library *)MUIMasterBase);
	if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
	return;
}


//==============================================================================================
// BuildGui()
//==============================================================================================

Object *BuildGui(void)
{
	Object *application;

	disk_object = GetDiskObject((STRPTR)"PROGDIR:LibMaker");

	application = NewObjectM(ApplicationClass->mcc_Class, 0,
		MUIA_Application_Author, APP_AUTHOR,
		MUIA_Application_Base, APP_BASE,
		MUIA_Application_Copyright, "(c) " APP_CYEARS " " APP_AUTHOR,
		MUIA_Application_Description, LS(MSG_APPLICATION_DESCRIPTION, APP_DESC),
		MUIA_Application_Title, APP_NAME,
		MUIA_Application_UsedClasses, UsedClasses,
		MUIA_Application_Version, "$VER: " APP_NAME " " APP_VER " (" APP_DATE ")",
		MUIA_Application_DiskObject,    disk_object,
	TAG_END);

	return application;
}


//==============================================================================================
// Main()
//==============================================================================================

int main(UNUSED int argc, UNUSED char *argv[])
{
	ULONG result = RETURN_OK;
	Object *application = 0;

	if (GetResources())
	{
		InitStrings();

		if ((application = BuildGui()) != NULL)
		{
			DoMethod(application, APPM_Notifications);
			DoMethod(application, APPM_MainLoop);
			MUI_DisposeObject(application);
		}
	}
	else result = RETURN_FAIL;

	FreeResources();
	return result;
}


//==============================================================================================
// DoSuperNewM()
//==============================================================================================

#if defined(__MORPHOS__)
Object* DoSuperNewM(Class *cl, Object *obj, ...)
{
	va_list args, args2;
	LONG argc = 0;
	ULONG tag;
	IPTR val;
	Object *result = NULL;

	__va_copy(args2, args);

	va_start(args, obj);

	do
	{
		tag = va_arg(args, ULONG);
		val = va_arg(args, IPTR);
		argc++;
	}
	while (tag != TAG_MORE);

	va_end(args);

	{
		struct TagItem tags[argc];
		LONG i;

		va_start(args2, obj);

		for (i = 0; i < argc; i++)
		{
			tags[i].ti_Tag = va_arg(args2, ULONG);
			tags[i].ti_Data = va_arg(args2, IPTR);
		}

		va_end(args2);

		result = (Object*)DoSuperMethod(cl, obj, OM_NEW, (IPTR)tags);
	}
	return result;
}
#else
Object * VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...)
{
  Object *rc;
  VA_LIST args;

  VA_START(args, obj);
  #if defined(__AROS__)
  rc = (Object *)DoSuperNewTagList(cl, obj, NULL, (struct TagItem *)VA_ARG(args, IPTR));
  #else
  rc = (Object *)DoSuperMethod(cl, obj, OM_NEW, VA_ARG(args, ULONG), NULL);
  #endif
  VA_END(args);

  return rc;
}
#endif


//==============================================================================================
// MUI_NewObjectM()
//==============================================================================================

#if defined(__MORPHOS__)
Object* MUI_NewObjectM(const char *classname, ...)
{
	va_list args, args2;
	LONG argc = 0;
	ULONG tag;
	IPTR val;
	Object *result = NULL;

	__va_copy(args2, args);

	va_start(args, classname);

	while ((tag = va_arg(args, ULONG)) != TAG_END)
	{
		val = va_arg(args, IPTR);
		argc++;
	}

	va_end(args);

	{
		struct TagItem tags[argc + 1];  // one for {TAG_END, 0}
		LONG i;

		va_start(args2, classname);

		for (i = 0; i < argc; i++)
		{
			tags[i].ti_Tag = va_arg(args2, ULONG);
			tags[i].ti_Data = va_arg(args2, IPTR);
		}

		tags[argc].ti_Tag = TAG_END;
		tags[argc].ti_Data = 0;

		va_end(args2);

		result = (Object*)MUI_NewObjectA(classname, tags);
	}
	return result;
}
#endif


//==============================================================================================
// NewObjectM()
//==============================================================================================

#if defined(__MORPHOS__)
Object* NewObjectM(Class *cl, const char *classname, ...)
{
	va_list args, args2;
	LONG argc = 0;
	ULONG tag;
	IPTR val;
	Object *result = NULL;

	__va_copy(args2, args);

	va_start(args, classname);

	while ((tag = va_arg(args, ULONG)) != TAG_END)
	{
		val = va_arg(args, IPTR);
		argc++;
	}

	va_end(args);

	{
		struct TagItem tags[argc + 1];  // one for {TAG_END, 0}
		LONG i;

		va_start(args2, classname);

		for (i = 0; i < argc; i++)
		{
			tags[i].ti_Tag = va_arg(args2, ULONG);
			tags[i].ti_Data = va_arg(args2, IPTR);
		}

		tags[argc].ti_Tag = TAG_END;
		tags[argc].ti_Data = 0;

		va_end(args2);

		result = NewObjectA(cl, (STRPTR)classname, tags);
	}
	return result;
}
#endif


//==============================================================================================
// InitStrings()
//==============================================================================================

void InitStrings(void)
{
	IdentifierChars = &TypeChars[4];
}


//==============================================================================================
// XGet()
//==============================================================================================

ULONG XGet(Object *obj, ULONG attr)
{
	ULONG val;

	GetAttr(attr, obj, &val);
	return val;
}



//==============================================================================================
// HexStrToULong()
//==============================================================================================

ULONG HexStrToULong(CONST_STRPTR s)
{
	ULONG v = 0, d;
	BYTE i;

	for (i = 0; i < 8; i++)
	{
		d = (ULONG)*s++;

		if ((d >= '0') && (d <= '9')) d -= 48;
		else if ((d >= 'A') && (d <= 'F')) d -= 55;
		else if ((d >= 'a') && (d <= 'f')) d -= 87;
		else break;

		v <<= 4;
		v |= d;
	}

	return v;
}


//==============================================================================================
// ParseLine()
//==============================================================================================

/*
   This helper function makes it possible to use dos.library/ReadArgs() on a string instead of
   commandline. Note that RDArgs structure returned by this function has to be freed later with
   dos.library/FreeArgs().
*/

struct RDArgs* ParseLine(char *line, const char *templ, LONG *params, struct RDArgs *srcargs)
{
	srcargs->RDA_Source.CS_Buffer = line;
	srcargs->RDA_Source.CS_Length = StrLen(line);
	srcargs->RDA_Source.CS_CurChr = 0;
	srcargs->RDA_DAList = 0;
	srcargs->RDA_Buffer = NULL;
	srcargs->RDA_BufSiz = 0;
	srcargs->RDA_ExtHelp = NULL;
	srcargs->RDA_Flags = RDAF_NOPROMPT;
	return ReadArgs(templ, params, srcargs);
}


//==============================================================================================
// ReadLine()
//==============================================================================================

/*
   This function reads a line of limited length from a filehandle. As dos.library/FGets() always
   NULL-terminates and always loads LF if it fits, I clear the next to last character to 0x00.
   Then, after reading a line it may be:
   - zero, it means that the line (with LF) is at least two characters shorter than the buffer.
   - LF, it means that the line is one character shorter.
   - any other character, it means the line has been cut and has no LF at the end.
   When line has been cut, the function sets IoErr to ERROR_LINE_TOO_LONG.
*/

BOOL ReadLine(BPTR file, char *buffer)
{
	BOOL result = FALSE;

	buffer[LINEBUFSIZE - 2] = 0x00;

	if (FGets(file, buffer, LINEBUFSIZE))
	{
		if ((buffer[LINEBUFSIZE - 2] == 0x00) || (buffer[LINEBUFSIZE - 2] == 0x0A))
		{
			result = TRUE;
		}
		else SetIoErr(ERROR_LINE_TOO_LONG);
	}

	return result;
}
