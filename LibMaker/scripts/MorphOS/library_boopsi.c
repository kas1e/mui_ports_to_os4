/*
   ${LIBNAME}, library skeleton
   Generated with ${GENERATOR}.
*/

$${BACKGROUNDAUTODOC}

typedef char* STRPTR;
typedef const char* CONST_STRPTR;
#define STRPTR_TYPEDEF

#define __NOLIBBASE__

#include <proto/exec.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/system.h>
#include <libraries/query.h>
#include <clib/alib_protos.h>
$${EXTRALIBINCLUDES}

#define UNUSED __attribute__((unused))

#include "lib_version.h"
#include "library.h"

const char LibName[] = LIBNAME;
extern const char VTag[];

struct Library *SysBase;
$${EXTRALIBBASES}


struct Library *LibInit(struct Library *unused, APTR seglist, struct Library *sysb);
struct MyLibBase *lib_init(struct MyLibBase *base, APTR seglist, struct Library *SysBase);
APTR lib_expunge(struct MyLibBase *base);
struct Library *LibOpen(void);
ULONG LibClose(void);
APTR LibExpunge(void);
ULONG LibReserved(void);
BOOL InitResources(struct MyLibBase *base);
BOOL FreeResources(struct MyLibBase *base);
Class* InitClass(struct MyLibBase *base);
Class *GetClass(void);
static IPTR ClassDispatcher(void);

$${LIBFUNCPROTOS}


$${AVCHECKROUTINE}

BOOL InitResources(struct MyLibBase *base)
{
	$${OPENEXTRALIBS}

	if (InitClass(base))
	{
		$${INITALTIVECCHECK}
		return TRUE;
	}

	return FALSE;
}


BOOL FreeResources(struct MyLibBase *base)
{
	if (base->BoopsiClass)
	{
		if (!FreeClass(base->BoopsiClass))
		{
			AddClass(base->BoopsiClass);
			return FALSE;
		}
	}

	$${CLOSEEXTRALIBS}
	return TRUE;
}


const struct TagItem RTags[] =
{
	{ QUERYINFOATTR_NAME, (IPTR)LibName },
	{ QUERYINFOATTR_IDSTRING, (IPTR)&VTag[1] },
	{ QUERYINFOATTR_COPYRIGHT, (IPTR)"${LIBCOPYRIGHT}" },
	{ QUERYINFOATTR_DATE, (IPTR)DATE },
	{ QUERYINFOATTR_VERSION, VERSION },
	{ QUERYINFOATTR_REVISION, REVISION },
	{ QUERYINFOATTR_SUBTYPE, QUERYSUBTYPE_LIBRARY },
	{ TAG_END,  0 }
};

struct Resident ROMTag =
{
	RTC_MATCHWORD,
	&ROMTag,
	&ROMTag + 1,
	RTF_EXTENDED | RTF_PPC,
	VERSION,
	NT_LIBRARY,
	0,
	(char*)LibName,
	VSTRING,
	(APTR)LibInit,
	REVISION,
	(struct TagItem*)RTags
};

APTR JumpTable[] =
{
	(APTR)FUNCARRAY_BEGIN,
	(APTR)FUNCARRAY_32BIT_NATIVE,
	(APTR)LibOpen,
	(APTR)LibClose,
	(APTR)LibExpunge,
	(APTR)LibReserved,
	(APTR)GetClass,
	(APTR)0xFFFFFFFF,
	$${JUMPTABLE}
	(APTR)FUNCARRAY_END
};


static const struct EmulLibEntry ClassDispatcher_gate =
{
	TRAP_LIB,
	0,
	(void(*)(void))ClassDispatcher
};


Class* InitClass(struct MyLibBase *base)
{
	Class *cl = NULL;

	if ((cl = MakeClass(LibName, "${SUPERCLASS}", NULL, sizeof(struct ObjData), 0L)))
	{
		cl->cl_Dispatcher.h_Entry = (HOOKFUNC)&ClassDispatcher_gate;
		cl->cl_UserData = (ULONG)base;
		AddClass(cl);
	}

	base->BoopsiClass = cl;
	return cl;
}



struct MyLibBase* lib_init(struct MyLibBase *base, APTR seglist, UNUSED struct Library *sysbase)
{
	InitSemaphore(&base->BaseLock);
	base->Seglist = seglist;
	return base;
}

struct TagItem LibTags[] = {
	{ LIBTAG_FUNCTIONINIT, (IPTR)JumpTable },
	{ LIBTAG_LIBRARYINIT,  (IPTR)lib_init },
	{ LIBTAG_MACHINE,      MACHINE_PPC },
	{ LIBTAG_BASESIZE,     sizeof(struct MyLibBase) },
	{ LIBTAG_SEGLIST,      0 },
	{ LIBTAG_TYPE,         NT_LIBRARY },
	{ LIBTAG_NAME,         0 },
	{ LIBTAG_IDSTRING,     0 },
	{ LIBTAG_FLAGS,        LIBF_CHANGED | LIBF_SUMUSED },
	{ LIBTAG_VERSION,      VERSION },
	{ LIBTAG_REVISION,     REVISION },
	{ LIBTAG_PUBLIC,       TRUE },
	{ TAG_END,             0 }
};

struct Library* LibInit(UNUSED struct Library *unused, APTR seglist, struct Library *sysbase)
{
	SysBase = sysbase;

	LibTags[4].ti_Data = (IPTR)seglist;
	LibTags[6].ti_Data = (IPTR)ROMTag.rt_Name;
	LibTags[7].ti_Data = (IPTR)ROMTag.rt_IdString;

	return (NewCreateLibrary(LibTags));
}


struct Library* LibOpen(void)
{
	struct MyLibBase *base = (struct MyLibBase*)REG_A6;
	struct Library *lib = (struct Library*)base;

	ObtainSemaphore(&base->BaseLock);

	if (!base->InitFlag)
	{
		if (InitResources(base)) base->InitFlag = TRUE;
		else
		{
			FreeResources(base);
			lib = NULL;
		}
	}

	if (lib)
	{
		base->LibNode.lib_Flags &= ~LIBF_DELEXP;
		base->LibNode.lib_OpenCnt++;
	}

	ReleaseSemaphore(&base->BaseLock);
	if (!lib) lib_expunge(base);
	return lib;
}


ULONG LibClose(void)
{
	struct MyLibBase *base = (struct MyLibBase*)REG_A6;
	ULONG ret = 0;

	ObtainSemaphore(&base->BaseLock);

	if (--base->LibNode.lib_OpenCnt == 0)
	{
		if (base->LibNode.lib_Flags & LIBF_DELEXP) ret = (ULONG)lib_expunge(base);
	}

	if (ret == 0) ReleaseSemaphore(&base->BaseLock);
	return ret;
}


APTR LibExpunge(void)
{
	struct MyLibBase *base = (struct MyLibBase*)REG_A6;

	return(lib_expunge(base));
}


APTR lib_expunge(struct MyLibBase *base)
{
	APTR seglist = NULL;

	ObtainSemaphore(&base->BaseLock);

	if (base->LibNode.lib_OpenCnt == 0)
	{
		if (FreeResources(base))
		{
			Forbid();
			Remove((struct Node*)base);
			Permit();
			seglist = base->Seglist;
			FreeMem((UBYTE*)base - base->LibNode.lib_NegSize, base->LibNode.lib_NegSize + base->LibNode.lib_PosSize);
			base = NULL;    /* freed memory, no more valid */
		}
	}
	else base->LibNode.lib_Flags |= LIBF_DELEXP;

	if (base) ReleaseSemaphore(&base->BaseLock);
	return seglist;
}


ULONG LibReserved(void)
{
	return 0;
}


Class *GetClass(void)
{
	struct MyLibBase *base = (struct MyLibBase*)REG_A6;

	return base->BoopsiClass;
}


