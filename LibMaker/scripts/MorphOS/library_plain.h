/*
   ${LIBNAME} internal library definitions.
   Generated with ${GENERATOR}.
*/

#ifndef ${DEFINENAME}_LIBRARY_H
#define ${DEFINENAME}_LIBRARY_H

#include <exec/libraries.h>
#include <exec/semaphores.h>


struct MyLibBase
{
	struct Library          LibNode;
	APTR                    Seglist;
	struct SignalSemaphore  BaseLock;
	BOOL                    InitFlag;
	$${EXTRABASEFIELDS}
};

#endif      /* ${DEFINENAME}_LIBRARY_H */
