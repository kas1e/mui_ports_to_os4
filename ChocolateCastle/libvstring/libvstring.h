#ifndef LIBVSTRING_H
#define LIBVSTRING_H 1

/* vstring.h */

//#ifndef __NOLIBBASE__
//#define __NOLIBBASE__
//#endif

#include <exec/types.h>
#include <stdarg.h>

#if defined(__amigaos4__)
extern struct Library *SysBase;
#else
extern struct ExecBase *SysBase;
#endif

#ifndef __typedef_VOID_FUNC
#define __typedef_VOID_FUNC
typedef void (*VOID_FUNC)(void);
#endif


#ifdef __amigaos4__
#define RAWFMTFUNC_STRING  0
#define	RAWFMTFUNC_COUNT   2
STRPTR VNewRawDoFmt(CONST_STRPTR FormatString, VOID_FUNC PutChProc, APTR PutChData, va_list DataStream);
STRPTR NewRawDoFmt(CONST_STRPTR FormatString, VOID_FUNC PutChProc, APTR PutChData, ... );



APTR AllocVecTaskPooled(ULONG byteSize);
void FreeVecTaskPooled(APTR memory);
#endif


ULONG FmtLen(STRPTR fmt, ...);
void FmtPut(STRPTR dest, STRPTR fmt, ...);
STRPTR FmtNew(STRPTR fmt, ...);
STRPTR VFmtNew(STRPTR fmt, va_list args);
LONG FmtNPut(STRPTR dest, STRPTR fmt, LONG maxlen, ...);
ULONG StrLen(STRPTR str);
STRPTR StrCopy(STRPTR src, STRPTR dest);
STRPTR StrNCopy(STRPTR src, STRPTR dest, LONG maxlen);
STRPTR StrNew(STRPTR str);
STRPTR* CloneStrArray(STRPTR *array);
VOID FreeStrArray(STRPTR *array);
ULONG StrArrayLen(STRPTR *array);

#define StrFree(s) FreeVecTaskPooled(s)

#endif /* LIBVSTRING_H */
