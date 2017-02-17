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
#include <clib/alib_protos.h>
$${EXTRALIBINCLUDES}

#define UNUSED __attribute__((unused))

#include "lib_version.h"
#include "library.h"

const char LibName[] = LIBNAME;
extern const char VTag[];

struct Library *SysBase;
$${EXTRALIBBASES}


__saveds struct Library *LibInit(APTR seglist __asm("a0"), struct Library *sysbase __asm("a6"));
__saveds struct Library *LibOpen(struct MyLibBase *base __asm("a6"));
__saveds ULONG LibClose(struct MyLibBase *base __asm("a6"));
__saveds APTR LibExpunge(struct MyLibBase *base __asm("a6"));
__saveds ULONG LibReserved(void);


$${LIBFUNCPROTOS}



BOOL InitResources(struct MyLibBase *base)
{
	$${OPENEXTRALIBS}
	return TRUE;
}


VOID FreeResources(struct MyLibBase *base)
{
	$${CLOSEEXTRALIBS}
}


struct Resident ROMTag =
{
	RTC_MATCHWORD,
	&ROMTag,
	&ROMTag + 1,
	0,
	VERSION,
	NT_LIBRARY,
	0,
	(char*)LibName,
	VSTRING,
	(APTR)LibInit
};


APTR JumpTable[] =
{
	(APTR)LibOpen,
	(APTR)LibClose,
	(APTR)LibExpunge,
	(APTR)LibReserved,
	$${JUMPTABLE}
	(APTR)0xFFFFFFFF,
};


__saveds struct Library* LibInit(APTR seglist __asm("a0"), struct Library *sysbase __asm("a6"))
{
	struct MyLibBase *base = NULL;

	SysBase = sysbase;

	if ((base = (struct MyLibBase*)MakeLibrary(JumpTable, NULL, NULL, sizeof(struct MyLibBase), 0)))
	{
		base->LibNode.lib_Node.ln_Type = NT_LIBRARY;
		base->LibNode.lib_Node.ln_Name = ROMTag.rt_Name;
		base->LibNode.lib_Flags = LIBF_CHANGED | LIBF_SUMUSED;
		base->LibNode.lib_Version = VERSION;
		base->LibNode.lib_Revision = REVISION;
		base->LibNode.lib_IdString = ROMTag.rt_IdString;
		base->LibNode.lib_OpenCnt = 0;
		base->Seglist = seglist;
		InitSemaphore(&base->BaseLock);
		AddLibrary((struct Library*)base);
	}

	return (struct Library*)base;
}


__saveds struct Library* LibOpen(struct MyLibBase* base __asm("a6"))
{
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
	if (!lib) LibExpunge(base);
	return lib;
}


__saveds ULONG LibClose(struct MyLibBase *base __asm("a6"))
{
	ULONG ret = 0;

	ObtainSemaphore(&base->BaseLock);

	if (--base->LibNode.lib_OpenCnt == 0)
	{
		if (base->LibNode.lib_Flags & LIBF_DELEXP) ret = (ULONG)LibExpunge(base);
	}

	if (ret == 0) ReleaseSemaphore(&base->BaseLock);
	return ret;
}


__saveds APTR LibExpunge(struct MyLibBase *base __asm("a6"))
{
	APTR seglist = NULL;

	ObtainSemaphore(&base->BaseLock);

	if (base->LibNode.lib_OpenCnt == 0)
	{
		FreeResources(base);
		Forbid();
		Remove((struct Node*)base);
		Permit();
		seglist = base->Seglist;
		FreeMem((UBYTE*)base - base->LibNode.lib_NegSize, base->LibNode.lib_NegSize + base->LibNode.lib_PosSize);
		base = NULL;    /* freed memory, no more valid */
	}
	else base->LibNode.lib_Flags |= LIBF_DELEXP;

	if (base) ReleaseSemaphore(&base->BaseLock);
	return seglist;
}


__saveds ULONG LibReserved(void)
{
	return 0;
}
