/*
   Stubs for the variable argument functions of the shared libraries used by
   MUI. Please note that these stubs should only be used if the compiler
   suite/SDK doesn't come with own stubs/inline functions.

   Also note that these stubs are only safe on m68k machines as it
   requires a linear stack layout!
*/

#if defined(__VBCC__) || defined(NO_INLINE_STDARG)
#if !defined(__PPC__)

#include <exec/types.h>

/* FIX V45 breakage... */
#if INCLUDE_VERSION < 45
#define MY_CONST_STRPTR CONST_STRPTR
#else
#define MY_CONST_STRPTR CONST STRPTR
#endif

#include <proto/intuition.h>
ULONG SetAttrs( APTR object, ULONG tag1, ... )
{ return SetAttrsA(object, (struct TagItem *)&tag1); }
APTR NewObject( struct IClass *classPtr, CONST_STRPTR classID, Tag tag1, ... )
{ return NewObjectA(classPtr, classID, (struct TagItem *)&tag1); }

#include <proto/dos.h>
#undef FPrintf
LONG FPrintf( BPTR fh, CONST_STRPTR format, ... )
{ return VFPrintf(fh, format, (CONST APTR)(&format+1)); }
#undef Printf
LONG Printf( CONST_STRPTR format, ... )
{ return VPrintf(format, (CONST APTR)(&format+1)); }
#undef AllocDosObjectTags
APTR AllocDosObjectTags( ULONG type, Tag tag1, ...)
{ return AllocDosObject(type, (struct TagItem *)&tag1); }

#include <proto/locale.h>
struct Catalog *OpenCatalog( struct Locale *locale, STRPTR name, Tag tag1, ... )
{ return OpenCatalogA(locale, name, (struct TagItem *)&tag1); }

#else
  #error "VARGS stubs are only save on m68k systems!"
#endif // !defined(__PPC__)

#endif // defined(__VBCC__) || defined(NO_INLINE_STDARG)
