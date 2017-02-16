#ifndef LIBVSTRING_H
#define LIBVSTRING_H 1

/* vstring.h */

//#ifndef __NOLIBBASE__
//#define __NOLIBBASE__
//#endif

#include <exec/types.h>
#include <libraries/locale.h>
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
BOOL StrEqu(CONST_STRPTR s1, CONST_STRPTR s2);
BOOL StrNEqu(CONST_STRPTR s1, CONST_STRPTR s2, LONG len);
LONG LocaleVFmtLen(struct Locale *l, CONST_STRPTR fmt, va_list args);
LONG LocaleFmtLen(struct Locale *l, CONST_STRPTR fmt, ...);
void LocaleVFmtPut(struct Locale *l, STRPTR dest, CONST_STRPTR fmt, va_list args);
void LocaleFmtPut(struct Locale *l, STRPTR dest, CONST_STRPTR fmt, ...);
STRPTR LocaleVFmtNew(struct Locale *l, CONST_STRPTR fmt, va_list args);
STRPTR LocaleFmtNew(struct Locale *l, CONST_STRPTR fmt, ...);
STRPTR StrChr(char chr, CONST_STRPTR str);
void StrLTrim(STRPTR str, CONST_STRPTR array);
void StrRTrim(STRPTR str, CONST_STRPTR array);
void StrTrim(STRPTR str, CONST_STRPTR array);
void StrReplace(char chr, char repl, STRPTR array);

/* memory alloc and free functions must be a matched pair */

#define StrFree(s) FreeVecTaskPooled(s)
#define FmtFree(s) FreeVecTaskPooled(s)
#define internal_alloc(l) AllocVecTaskPooled(l)

/* Pass 64-bit elements into LocaleXxxx() */

#define QSPLIT(a)  (ULONG)((a) >> 32), (ULONG)((a) & 0xFFFFFFFF)

#endif /* LIBVSTRING_H */
