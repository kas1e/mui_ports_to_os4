#include "main.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <workbench/startup.h>

struct Library *SysBase = 0;
struct Library *DOSBase = 0;

extern ULONG Main(struct WBStartup *wbmessage);

ULONG __abox__ = 1;

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

ULONG Start(void)
{
	struct Process *myproc = 0;
	struct Message *wbmessage = 0;
	BOOL have_shell = FALSE;
	ULONG return_code = RETURN_OK;

	SysBase = *(struct Library**)4L;
	myproc = (struct Process*)FindTask(0);
	if (myproc->pr_CLI) have_shell = TRUE;

	if (!have_shell)
	{
		WaitPort(&myproc->pr_MsgPort);
		wbmessage = GetMsg(&myproc->pr_MsgPort);
	}

	if (DOSBase = OpenLibrary((STRPTR)"dos.library", 0))
	{
		return_code = Main((struct WBStartup*)wbmessage);
		CloseLibrary(DOSBase);
	}
	else return_code = RETURN_FAIL;

	if (wbmessage)
	{
		Forbid();
		ReplyMsg(wbmessage);
	}
	return return_code;
}

__attribute__ ((section(".text"))) CONST UBYTE VString[] = "$VER: " APP_NAME " " APP_VER " (" APP_DATE ") (c) " APP_CYEARS " " APP_AUTHOR "\r\n";

