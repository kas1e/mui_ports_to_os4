/*
 *  Multiview 3
 *
 *  Copyright © 2007-2011 Ilkka Lehtoranta <ilkleht@yahoo.com>
 *  All rights reserved.
 *  
 *  $Id: clipboard.c,v 1.1 2012/11/22 22:45:25 kiero Exp $
 */

#warning "!!!!!!!REPLACE THIS WITH SOME SYSTEM SOLUTION!!!!!!!"
#define USE_INLINE_STDARG 1

#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <devices/clipboard.h>

#ifndef __amigaos4__
#include <libraries/charsets.h>
#endif

#include <libraries/locale.h>
#include <proto/alib.h>

#ifndef __amigaos4__
#include <proto/charsets.h>
#endif
#ifdef __amigaos4__
//#include <proto/codesets.h>
#endif 

#include <proto/datatypes.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>

#define ID_FTXT	MAKE_ID('F','T','X','T')
#define ID_CHRS	MAKE_ID('C','H','R','S')
#define ID_UTF8	MAKE_ID('U','T','F','8')

#ifdef __amigaos4__
typedef unsigned long IPTR;
#endif

#ifdef __amigaos4__
STATIC APTR clipboard_open(void)
{
	struct MsgPort *mp = (struct MsgPort *)AllocSysObjectTags(ASOT_PORT, TAG_END);

if (mp != NULL)
    {
    struct IOStdReq *io = (struct IOStdReq *)AllocSysObjectTags(ASOT_IOREQUEST,
        ASOIOR_Size, sizeof(struct IOClipReq),
        ASOIOR_ReplyPort, mp,
        TAG_END);
        
    if (io != NULL)
        {
        if (!(OpenDevice("clipboard.device", 0, (struct IORequest *)io, 0)))
            {
            return((struct IOClipReq *)io);
            }
        FreeSysObject(ASOT_IOREQUEST, io);
        }
    FreeSysObject(ASOT_PORT, mp);
    }

	return(NULL);
}
#else
STATIC APTR clipboard_open(void)
{
	struct IOClipReq *io;
	struct MsgPort *mp;

	mp = CreateMsgPort();

	if ((io = (struct IOClipReq *)CreateIORequest(mp,sizeof(struct IOClipReq))))
	{
		if (!(OpenDevice("clipboard.device", 0, (struct IORequest *)io, 0)))
		{
			return io;
		}
		DeleteIORequest((struct IORequest *)io);
	}

	DeleteMsgPort(mp);
	return NULL;
}
#endif


#ifdef __amigaos4__
STATIC void clipboard_close(struct IOClipReq *io)
{
	struct MsgPort *mp = io->io_Message.mn_ReplyPort;

	CloseDevice((struct IORequest *)io);
	FreeSysObject(ASOT_IOREQUEST, io);
	FreeSysObject(ASOT_PORT, mp);
} 

#else
STATIC void clipboard_close(struct IOClipReq *io)
{
	if (io)
	{
		struct MsgPort *mp;

		mp = io->io_Message.mn_ReplyPort;

		CloseDevice((struct IORequest *)io);
		DeleteIORequest((struct IORequest *)io);
		DeleteMsgPort(mp);
	}
}
#endif

STATIC ULONG clipboard_write_data(struct IOClipReq *io, CONST_APTR data, ULONG len)
{
	LONG rc;

	io->io_Command = CMD_WRITE;
	io->io_Data    = (APTR)data;
	io->io_Length  = len;
	DoIO( (struct IORequest *)io);

	if (io->io_Actual != len)
	{
		io->io_Error = 1;
	}

	rc = io->io_Error ? FALSE : TRUE;
	return rc;
}


STATIC VOID clipboard_pad_text(struct IOClipReq *io, ULONG textlen)
{
	if (textlen & 1) clipboard_write_data(io, "", 1);
}


STATIC ULONG clipboard_write_header_and_text(struct IOClipReq *io, CONST_STRPTR string, ULONG slen, ULONG ulen)
{
	ULONG rc;

	struct
	{
		ULONG form;
		ULONG totalsize;
		ULONG ftxt;
		ULONG type;
		ULONG strlen;
	} iffheader;

	io->io_Offset = 0;
	io->io_Error  = 0;
//	io->io_ClipID = 0;

	/* FIXME: For correct operation we should also store font name. Used font
	 * is relevant with guides written in Japanese for example.
	 */

	iffheader.form      = ID_FORM;
	iffheader.totalsize = (slen & 1 ? slen + 1 : slen) + (ulen & 1 ? ulen + 1 : ulen) + 12 + 8;
	iffheader.ftxt      = ID_FTXT;
	iffheader.type      = ID_CHRS;
	iffheader.strlen    = slen;

	rc = FALSE;

	if (clipboard_write_data(io, &iffheader, sizeof(iffheader)))
	{
		if (clipboard_write_data(io, string, slen))
		{
			clipboard_pad_text(io, slen);
			rc = TRUE;
		}
	}

	return rc;
}


STATIC ULONG clipboard_write_utf8(struct IOClipReq *io, CONST_STRPTR utext, ULONG ulen)
{
	ULONG rc;

	struct
	{
		ULONG type;
		ULONG strlen;
	} utf8_header;

	/* FIXME: For correct operation we should also store font name. Used font
	 * is relevant with guides written in Japanese for example.
	 */

	utf8_header.type   = ID_UTF8;
	utf8_header.strlen = ulen;

	rc = FALSE;

	if (clipboard_write_data(io, &utf8_header, sizeof(utf8_header)))
	{
		if (clipboard_write_data(io, utext, ulen))
		{
			clipboard_pad_text(io, ulen);
			rc = TRUE;
		}
	}

	return rc;
}

STATIC VOID clipboard_finalize(struct IOClipReq *io)
{
	io->io_Command = CMD_UPDATE;
	DoIO((struct IORequest *)io);
}

static void clips_write(CONST_STRPTR stext, LONG slen, CONST_STRPTR utext, LONG ulen)
{
	APTR ctx;
	
	if ((ctx = clipboard_open()))
	{
		if (clipboard_write_header_and_text(ctx, stext, slen, ulen))
		{
			if (clipboard_write_utf8(ctx, utext, ulen))
			{
			}
		}
		
		clipboard_finalize(ctx);
		clipboard_close(ctx);
	}
}

//==============================================================================
// This function is fully automatic and is used for writing a single
// null-terminated buffer of text.
//
// codeset is either CODESET_LATIN1 or CODESET_UTF8


#include <proto/codesets.h>

#define AllocTaskPooled(size) AllocMem(size, MEMF_ANY)
#define FreeTaskPooled(mem, size) FreeMem(mem, size)

// from mos's locale.h 
#define CODESET_UTF8    1 

// to redefine from mos's charset.library to codesets.library
#define MIBENUM_SYSTEM CS_MIBENUM_SYSTEM
#define MIBENUM_UTF_8 CS_MIBENUM_UTF_8

// from mos's charset.h
#define CST_GetDestBytes         (TAG_USER + 1) 

VOID clipboard_write_text(CONST_STRPTR string, ULONG codeset)
{

 if (string)
    {
        struct Library *CodesetsBase = OpenLibrary("PROGDIR:libs/codesets.library", CODESETSVER);
        ICodesets = (struct CodesetsIFace *)GetInterface(CodesetsBase, "main", 1, NULL);

        if (ICodesets) //test on interface instead
        {
            CONST_STRPTR stext, utext;
            LONG slen, ulen, buflen;
            char *cstr;
            struct codeset *utf8CodeSet = CodesetsFindA("UTF-8", NULL);

            buflen = strlen(string);

            if (codeset == CODESET_UTF8) //that means from UTF8 to system
            {
                ulen   = buflen;
                utext  = string;
                cstr = CodesetsConvertStr(CSA_SourceCodeset, utf8CodeSet,
                                          CSA_Source, (unsigned char *)string,
                                          CSA_SourceLen, buflen,
                                          CSA_DestLenPtr, &slen,
                                          TAG_DONE);
                stext = cstr;
            }
            else    // Some system default maybe, who knows (from system to UTF8)
            {
                slen   = buflen;
                stext  = string;
                cstr = CodesetsConvertStr(CSA_DestCodeset, utf8CodeSet,
                                          CSA_Source, (unsigned char *)string,
                                          CSA_SourceLen, buflen,
                                          CSA_DestLenPtr, &ulen,
                                          TAG_DONE);
                utext = cstr;
            }

            if (cstr != NULL)
            {
                clips_write(stext, slen, utext, ulen);
                CodesetsFreeA(cstr, NULL);
            }

            CloseLibrary(CodesetsBase); 
        }
    }
}