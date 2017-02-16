#ifndef LIBVSTRING_H
#define LIBVSTRING_H 1

/* vstring.h */

//#ifndef __NOLIBBASE__
//#define __NOLIBBASE__
//#endif

#include <exec/types.h>
#include <stdarg.h>

#ifndef __typedef_FMT_FUNC
#define __typedef_FMT_FUNC
typedef APTR (*FMT_FUNC)(APTR, UBYTE);
#endif


#if !defined(__MORPHOS__)
#define RAWFMTFUNC_STRING  (FMT_FUNC)0
#define	RAWFMTFUNC_COUNT   (FMT_FUNC)2
STRPTR VNewRawDoFmt(CONST_STRPTR FormatString, FMT_FUNC PutChProc, APTR PutChData, va_list DataStream);
STRPTR NewRawDoFmt(CONST_STRPTR FormatString, FMT_FUNC PutChProc, APTR PutChData, ... );
APTR AllocVecTaskPooled(ULONG byteSize);
void FreeVecTaskPooled(APTR memory);
#endif


ULONG FmtLen(CONST_STRPTR fmt, ...);
void FmtPut(STRPTR dest, CONST_STRPTR fmt, ...);
STRPTR FmtNew(CONST_STRPTR fmt, ...);
STRPTR VFmtNew(CONST_STRPTR fmt, va_list args);
LONG FmtNPut(STRPTR dest, CONST_STRPTR fmt, LONG maxlen, ...);
ULONG StrLen(CONST_STRPTR str);
STRPTR StrCopy(CONST_STRPTR src, STRPTR dest);
STRPTR StrNCopy(CONST_STRPTR src, STRPTR dest, LONG maxlen);
STRPTR StrNew(CONST_STRPTR str);
STRPTR* CloneStrArray(CONST_STRPTR *array);
VOID FreeStrArray(STRPTR *array);
ULONG StrArrayLen(CONST_STRPTR *array);

#define StrFree(s) FreeVecTaskPooled(s)

#endif /* LIBVSTRING_H */
