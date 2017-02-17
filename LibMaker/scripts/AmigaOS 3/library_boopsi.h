/*
   ${LIBNAME} internal library definitions.
   Generated with ${GENERATOR}.
*/

#ifndef ${DEFINENAME}_LIBRARY_H
#define ${DEFINENAME}_LIBRARY_H

#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <intuition/classes.h>
#include ${LIBINCLUDE}

struct MyLibBase
{
	struct Library          LibNode;
	APTR                    Seglist;
	struct SignalSemaphore  BaseLock;
	BOOL                    InitFlag;
	Class*                  BoopsiClass;
	$${EXTRABASEFIELDS}
};


struct ObjData
{
};


#endif      /* ${DEFINENAME}_LIBRARY_H */
