/****************** AROS IMPLEMENTATION OF NewRawDoFmt and VNewRawDoFmt *************************
TODO:

1. Implement RAWFMTFUNC_SERIAL

2. Rewrite following part of "Call the PutCharProc function with the given parameters"

        AROS_UFC2NR(void, PutChProc,        	\
	    AROS_UFCA(UBYTE, (ch), D0),       		\
	    AROS_UFCA(APTR , PutChData, A3)); 		\
		
	currently just commented out

*************************************************************************************************/


#include <dos/dos.h>
#include <proto/exec.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#define NO_INLINE_STDARG


#define is_va_list(ap) ap
#define null_va_list(ap) void *ap = NULL

/* An unsigned integer which can store a pointer */
#ifndef __typedef_IPTR
#   define __typedef_IPTR
#   ifdef AROS_IPTR_TYPE
	typedef AROS_IPTR_TYPE			IPTR;
#   else
	typedef unsigned long			IPTR;
#   endif
#endif

/* A signed type that can store a pointer */
#ifndef __typedef_SIPTR
#   define __typedef_SIPTR
#   ifdef AROS_SIPTR_TYPE
	typedef AROS_SIPTR_TYPE			SIPTR;
#   else
	typedef long				SIPTR;
#   endif
#endif


#ifndef __typedef_VOID_FUNC
#define __typedef_VOID_FUNC
typedef void (*VOID_FUNC)(void);
#endif


#define RAWFMTFUNC_STRING (VOID_FUNC)0 /* Output to string given in PutChData	        */
#define RAWFMTFUNC_SERIAL (VOID_FUNC)1 /* Output to debug log (usually serial port)     */
#define RAWFMTFUNC_COUNT  (VOID_FUNC)2 /* Just count characters, PutChData is a pointer */


#define AROS_BSTR_ADDR(s)        (((STRPTR)BADDR(s))+1)
#define AROS_BSTR_strlen(s)      (AROS_BSTR_ADDR(s)[-1]) 




/* Fetch the data from a va_list.

   Variables are allocated in the va_list using the default argument
   promotion rule of the C standard, which states that:

       "types char and short int are promoted to int, and float is promoted
        to double" (http://www.eskimo.com/~scs/C-faq/q15.2.html)

   That rule directly translates into relations on types sizes, rather
   than the types themselves, since sizeof(char) and sizeof(short) is always
   less than or equal to sizeof(int), and sizeof(float) is always less than
   or equal to sizeof(double). In addition, we also handle the case of
   sizeof(long), whilst the sizeof(double) case is not handled for two reasons:

       1) RawDoFmt() doesn't handle floating point values.
       2) Given (1), sizeof(type) > sizeof(long) would hold true if
          and only if type were a 64 bit pointer and long's and pointers
	  had different sizes (quite unusual).  */

#define fetch_va_arg(type)                              \
({                                                      \
    type res;                                           \
                                                        \
    if (sizeof(type) <= sizeof(int))                    \
	res = (type)(IPTR)va_arg(VaListStream, int);    \
    else                                                \
    if (sizeof(type) == sizeof(long))                   \
	res = (type)(IPTR)va_arg(VaListStream, long);   \
    else                                                \
	res = (type)(IPTR)va_arg(VaListStream, void *); \
                                                        \
    res;                                                \
})

/* Fetch an argument from memory.

   We can be sure data is always aligned to a WORD boundary,
   which is what we need.

   However, on some architectures some kind of data needs to
   have a certain alignment greater than 2 bytes, and this
   code will miserably fail if DataStream is not properly
   aligned; in such cases data should be fetched in pieces,
   taking into account the endianess of the machine.

   Currently it is assumed that ULONG values are stored
   in an array of IPTRs. This results in good portability for
   both 32- and 64-bit systems. */

#define fetch_mem_arg(type)              	\
({                                       	\
    IPTR res;					\
    						\
    if (sizeof(type) <= sizeof(UWORD))		\
    {						\
    	res = *(UWORD *)DataStream	;	\
    	DataStream = (UWORD *)DataStream + 1;	\
    }						\
    else if (sizeof(type) <= sizeof(ULONG))	\
    {						\
    	res = *(ULONG *)DataStream;		\
    	DataStream = (ULONG *)DataStream + 1;	\
    }						\
    else if (sizeof(type) <= sizeof(IPTR))	\
    {						\
    	res = *(IPTR *)DataStream;		\
    	DataStream = (IPTR *)DataStream + 1;	\
    }						\
    (type)res;					\
})

/* Fetch the data either from memory or from the va_list, depending
   on the value of VaListStream.  */
#define fetch_arg(type) \
    (is_va_list(VaListStream) ? fetch_va_arg(type) : fetch_mem_arg(type))

/*
 * Fetch a number from the stream.
 *
 * size - one of 'h', 'l', 'i'
 * sign - <0 or >= 0.
 *
 * EXPERIMENTAL: 'i' is used to represent full IPTR value on 64-bit systems
 */
#define fetch_number(size, sign)                                                               \
    (sign >= 0                                                                                 \
     ? (size == 'i' ? fetch_arg(IPTR)  : (size == 'l' ? fetch_arg(ULONG) : fetch_arg(UWORD)))   \
     : (size == 'i' ? fetch_arg(SIPTR) : (size == 'l' ? fetch_arg(LONG)  : fetch_arg(WORD))))

/* Call the PutCharProc function with the given parameters.  */
#define PutCh(ch)                         \
do                                        \
{                                         \
    switch ((IPTR)PutChProc)              \
    {                                     \
    case (IPTR)RAWFMTFUNC_STRING:	  \
	*(PutChData++) = ch;               \
	break;				  \
    case (IPTR)RAWFMTFUNC_SERIAL:	  \
	break;				  \
    case (IPTR)RAWFMTFUNC_COUNT:	  \
	(*((ULONG *)PutChData))++;	  \
	break;				  \
    default:				  \
	if (is_va_list(VaListStream))			\
	{						\
	    APTR (*proc)(APTR, UBYTE) = (APTR)PutChProc;\
	    PutChData = proc((APTR)PutChData, ch);	\
	}						\
	else						\
	{						\
	}						\
    }                                     \
} while (0)




//		AROS_UFC2NR(void, PutChProc,        		\
//		AROS_UFCA(UBYTE, (ch), D0),       		\
//	    AROS_UFCA(APTR , PutChData, A3)); 		\


/*
 * DataStream == NULL can't be used to select between new or old style PutChProc() because
 * RawDoFmt(<string without parameters>, NULL, PutChProc, PutChData); is valid and used by
 * m68k programs.
 * In order to get around we use specially formed va_list with NULL value.
 */

APTR InternalRawDoFmt(CONST_STRPTR FormatString, APTR DataStream, VOID_FUNC PutChProc, APTR inPutChData, va_list VaListStream)
{
    UBYTE *PutChData = inPutChData;

    /* As long as there is something to format left */
    while (*FormatString)
    {
	/* Check for '%' sign */
	if (*FormatString == '%')
	{
	    /*
		left	 - left align flag
		fill	 - pad character
		minus	 - 1: number is negative
		minwidth - minimum width
		maxwidth - maximum width
		size	 - one of 'h', 'l', 'i'.
		width	 - width of printable string
		buf	 - pointer to printable string
	    */
	    int left  = 0;
	    int fill  = ' ';
	    int minus = 0;
	    int size  = 'h';
	    ULONG minwidth = 0;
	    ULONG maxwidth = ~0;
	    ULONG width    = 0;
	    UBYTE *buf;

            /* Number of decimal places required to convert a unsigned long to
               ascii. The formula is: ceil(number_of_bits*log10(2)).
	       Since I can't do this here I use .302 instead of log10(2) and
	       +1 instead of ceil() which most often leads to exactly the
	       same result (and never becomes smaller).

	       Note that when the buffer is large enough for decimal it's
	       large enough for hexadecimal as well.  */

	    #define CBUFSIZE (sizeof(IPTR)*8*302/1000+1)
	    /* The buffer for converting long to ascii.  */
	    UBYTE cbuf[CBUFSIZE];
	    ULONG i;

	    /* Skip over '%' character */
	    FormatString++;

	    /* '-' modifier? (left align) */
	    if (*FormatString == '-')
		left = *FormatString++;

	    /* '0' modifer? (pad with zeros) */
	    if (*FormatString == '0')
		fill = *FormatString++;

	    /* Get minimal width */
	    while (*FormatString >= '0' && *FormatString <= '9')
	    {
	        minwidth = minwidth * 10 + (*FormatString++ - '0');
	    }

	    /* Dot following width modifier? */
	    if(*FormatString == '.')
	    {
		FormatString++;
		/* Get maximum width */

		if(*FormatString >= '0' && *FormatString <= '9')
		{
		    maxwidth = 0;
		    do
			maxwidth = maxwidth *10 + (*FormatString++ - '0');
		    while (*FormatString >= '0' && *FormatString <= '9');
		}
	    }

	    /* size modifiers */
	    switch (*FormatString)
	    {
	    case 'l':
	    case 'i':
	    	size = *FormatString++;
		break;
	    }

	    /* Switch over possible format characters. Sets minus, width and buf. */
	    switch(*FormatString)
	    {
		/* BCPL string */
		case 'b':
                {
                    BSTR s = fetch_arg(BSTR);
                    
                    if (s)
                    {
		    	buf = AROS_BSTR_ADDR(s);
		    	width = AROS_BSTR_strlen(s);
		    }
		    else
		    {
		    	buf = "";
		    	width = 0;
		    }

		    break;
                }

		/* C string */
		case 's':
  		    buf = fetch_arg(UBYTE *);

                    if (!buf)
                        buf = "";
		    width = strlen(buf);

		    break;
		{
		    IPTR number = 0; int base;
		    static const char digits[] = "0123456789ABCDEF";

		    case 'p':
		    case 'P':
			fill = '0';
			minwidth = sizeof(APTR)*2;
			size = 'i';
		    case 'x':
		    case 'X':
		        base   = 16;
			number = fetch_number(size, 1);

                        goto do_number;

		    case 'd':
		    case 'D':
		        base   = 10;
  		        number = fetch_number(size, -1);
			minus  = (SIPTR)number < 0;

			if (minus) number = -number;

			goto do_number;

		    case 'u':
		    case 'U':
		        base = 10;
  		        number = fetch_number(size, 1);

		    do_number:

		        buf = &cbuf[CBUFSIZE];
			do
			{
  		            *--buf = digits[number % base];
			    number /= base;
		            width++;
			} while (number);

		    break;
		}


		/* single character */
		case 'c':
		    /* Some space for the result */
		    buf   = cbuf;
		    width = 1;

		    *buf = fetch_number(size, 1);

		    break;

		/* '%' before '\0'? */
		case '\0':
		    /*
			This is nonsense - but do something useful:
			Instead of reading over the '\0' reuse the '\0'.
		    */
		    FormatString--;
		    /* Get compiler happy */
		    buf = NULL;
		    break;

		/* Convert '%unknown' to 'unknown'. This includes '%%' to '%'. */
		default:
		    buf   = (UBYTE *)FormatString;
		    width = 1;
		    break;
	    }

	    if (width > maxwidth) width = maxwidth;

	    /* Skip the format character */
	    FormatString++;

	    /*
		Now everything I need is known:
		buf	 - contains the string to be printed
		width	 - the size of the string
		minus	 - is 1 if there is a '-' to print
		fill	 - is the pad character
		left	 - is 1 if the string should be left aligned
		minwidth - is the minimal width of the field
		(maxwidth is already part of width)

		So just print it.
	    */

	    /* Print '-' (if there is one and the pad character is no space) */
	    if (minus && fill != ' ')
	        PutCh('-');

	    /* Pad left if not left aligned */
	    if (!left)
		for (i = width + minus; i < minwidth; i++)
		    PutCh(fill);

	    /* Print '-' (if there is one and the pad character is a space) */
	    if(minus && fill == ' ')
                PutCh('-');

	    /* Print body upto width */
	    for(i=0; i<width; i++) {
	        PutCh(*buf);
		buf++;
	    }

	    /* Pad right if left aligned */
	    if(left)
		for(i = width + minus; i<minwidth; i++)
		    PutCh(fill);
	}
	else
	{
	    /* No '%' sign? Put the formatstring out */
	    PutCh(*FormatString);
	    FormatString++;
	}
    }
    /* All done. Put the terminator out. */
    PutCh('\0');

    /* Return the rest of the DataStream or buffer. */
    return is_va_list(VaListStream) ? (APTR)PutChData : DataStream;
}



STRPTR VNewRawDoFmt(CONST_STRPTR FormatString, VOID_FUNC PutChProc, APTR PutChData, va_list DataStream)
{
    return InternalRawDoFmt(FormatString, NULL, PutChProc, PutChData, DataStream);
} /* VNewRawDoFmt */


STRPTR NewRawDoFmt(CONST_STRPTR FormatString, VOID_FUNC PutChProc, APTR PutChData, ... )
{
    STRPTR retval;
    va_list args;
    
    va_start(args, PutChData);
    retval = VNewRawDoFmt(FormatString, PutChProc, PutChData, args);
    va_end(args);
    return retval;

} 
 
 
 





APTR AllocVecTaskPooled(ULONG byteSize)
{
    return calloc(1, byteSize);
}

void FreeVecTaskPooled(APTR memory)
{
    free(memory);
}

