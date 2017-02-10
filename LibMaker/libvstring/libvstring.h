/* libvstring.h */

#ifndef __NOLIBBASE__
#define __NOLIBBASE__
#endif

#ifndef STRPTR_TYPEDEF
typedef char* STRPTR;
typedef const char* CONST_STRPTR;
#define STRPTR_TYPEDEF
#endif

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/locale.h>
#include <stdarg.h>

extern struct Library *SysBase;
extern struct Library *LocaleBase;


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



void FmtPut(STRPTR dest, CONST_STRPTR fmt, ...);
LONG FmtNPut(STRPTR dest, CONST_STRPTR fmt, LONG maxlen, ...);

LONG FmtLen(CONST_STRPTR fmt, ...);
STRPTR VFmtNew(CONST_STRPTR fmt, va_list args);
STRPTR FmtNew(CONST_STRPTR fmt, ...);
LONG StrLen(CONST_STRPTR str);
STRPTR StrCopy(CONST_STRPTR src, STRPTR dest);
STRPTR StrNCopy(CONST_STRPTR src, STRPTR dest, LONG maxlen);
STRPTR StrNew(CONST_STRPTR str);
STRPTR* CloneStrArray(STRPTR *array);
VOID FreeStrArray(STRPTR *array);
LONG StrArrayLen(STRPTR *array);
void StrFree(STRPTR str);
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
