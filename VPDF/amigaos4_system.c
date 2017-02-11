// This implementation is for emulating the same "ProcessPixelArray"
// functionality on AmigaOS4 like CyberGraphX on MorphOS provides.

#include <cybergraphx/cybergraphics.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <proto/exec.h>
#include <proto/utility.h>
#include <stdarg.h>
#include <strings.h>

#include "amigaos4_system.h"



struct Library *CyberGfxBase;
struct CyberGfxIFace *ICyberGfx;


ULONG ProcessPixelArrayA(struct RastPort *rp,
												 LONG left, LONG top,
												 LONG width, LONG height,
												 struct DrawInfo *dri,
												 struct TagItem *taglist)
{
   ULONG success = TRUE, done = FALSE;
   ULONG gradient, gradstart, gradend, makegrey, blur, impose;
   LONG brighten, darken;
   ULONG *array = NULL, size = 0UL, bytesperrow, line, *curr, *start, *stop;
   ULONG first = TRUE;
   struct RastPort trp;
   struct BitMap *tbm = NULL;
   struct TagItem *ti, *tptr = taglist;

   bytesperrow = width * sizeof(ULONG);
   size = bytesperrow * height;

   while (!done)
   {
      gradient = ~0UL;
      gradstart = gradend = impose = 0UL;
      brighten = darken = 0L;
      makegrey = blur = FALSE;

      while (((ti = NextTagItem(&tptr)) != NULL) && (ti->ti_Tag != PPA_Reset))
      {
         switch (ti->ti_Tag)
         {
            case PPA_Gradient:
               gradient = ti->ti_Data;
               break;

            case PPA_GradientStart:
               gradstart = ti->ti_Data;
               break;

            case PPA_GradientEnd:
               gradend = ti->ti_Data;
               break;

            case PPA_Brighten:
               brighten = (LONG)ti->ti_Data;
               break;

            case PPA_Darken:
               darken = (LONG)ti->ti_Data;
               break;

            case PPA_MakeGrey:
               makegrey = ti->ti_Data;
               break;

            case PPA_Blur:
               blur = ti->ti_Data;
               break;

            case PPA_Impose:
               impose = ti->ti_Data;
               break;
         }
      }

      if (ti == NULL) done = TRUE;

      if ((gradient != ~0UL) || brighten || darken || makegrey || blur || impose)
      {
         if (array == NULL)
         {
            array = AllocVec(size + size,MEMF_ANY);
            if (array == NULL) success = FALSE;
         }

         if (array != NULL)
         {
            ULONG alpha = 0UL, alpha_inv = 0UL;

            if (impose)
            {
               alpha = ((impose >> 24) & 0xFF) + 1;
               alpha_inv = 257 - alpha;
            }

            if (gradient != ~0UL)
            {
               if (tbm == NULL)
               {
                  tbm = AllocBitMap(width,height,GetBitMapAttr(rp->BitMap,BMA_DEPTH),0UL,rp->BitMap);
                  InitRastPort(&trp);
                  trp.BitMap = tbm;
               }

               if (tbm != NULL)
               {
                  struct GradientSpec gs;

                  memset(&gs,0,sizeof(struct GradientSpec));

                  gs.Mode = GRADMODE_COLOR;
                  gs.Direction = DirectionVector(gradient);
                  gs.Specs.Abs.RGBStart[0] = (gradstart >> 16) & 0xFF;
                  gs.Specs.Abs.RGBStart[1] = (gradstart >>  8) & 0xFF;
                  gs.Specs.Abs.RGBStart[2] = (gradstart      ) & 0xFF;
                  gs.Specs.Abs.RGBEnd[0] = (gradend >> 16) & 0xFF;
                  gs.Specs.Abs.RGBEnd[1] = (gradend >>  8) & 0xFF;
                  gs.Specs.Abs.RGBEnd[2] = (gradend      ) & 0xFF;

                  /* ClipBlit(rp,left,top,&trp,0,0,width,height,0xC0); */  /* Not needed for now */
                  DrawGradient(&trp,0,0,width,height,NULL,0UL,&gs,dri);
                  ReadPixelArray(array,0,0,bytesperrow,&trp,0,0,width,height,RECTFMT_ARGB);
               }
               else
               {
                  gradient = ~0UL;
                  success = FALSE;
               }
            }

            if ((gradient == ~0UL) && first)
            {
               ReadPixelArray(array,0,0,bytesperrow,rp,left,top,width,height,RECTFMT_ARGB);
            }

            first = FALSE;

            if (blur) CopyMem(array,(UBYTE *)array + size,size);

            curr = array;
            line = 0;

            while (line < height)
            {
               start = curr;
               stop = curr + width;

               while (curr < stop)
               {
                  ULONG pc = *curr;

                  if (blur)
                  {
                     ULONG pa, pb, pl, pr;
                     LONG x, y;

                     x = left + (curr - start);
                     y = top + line;

                     pa = (line > 0)? *((ULONG *)((UBYTE *)curr + size) - width): pc;
                     pb = ((line + 1) < height)? *((ULONG *)((UBYTE *)curr + size) + width): pc;
                     pl = (curr > start)? *((ULONG *)((UBYTE *)curr + size) - 1): pc;
                     pr = ((curr + 1) < stop)? *((ULONG *)((UBYTE *)curr + size) + 1): pc;

                     pc = ((((pa & 0xFF0000) + (pb & 0xFF0000) + (pl & 0xFF0000) + (pr & 0xFF0000)) >> 2) & 0xFF0000) |
                          ((((pa & 0x00FF00) + (pb & 0x00FF00) + (pl & 0x00FF00) + (pr & 0x00FF00)) >> 2) & 0x00FF00) |
                          ((((pa & 0x0000FF) + (pb & 0x0000FF) + (pl & 0x0000FF) + (pr & 0x0000FF)) >> 2) & 0x0000FF);
                  }

                  if (brighten > 0)
                  {
                     ULONG i, shift;
                     LONG v;

                     for (i = 0, shift = 0; i < 3; i++, shift += 8)
                     {
                        if ((v = ((LONG)((pc >> shift) & 0xFF) + brighten)) > 255) v = 255;
                        pc = (pc & ~(0xFF << shift)) | ((ULONG)v << shift);
                     }
                  }

                  if (darken > 0)
                  {
                     ULONG i, shift;
                     LONG v;

                     for (i = 0, shift = 0; i < 3; i++, shift += 8)
                     {
                        if ((v = ((LONG)((pc >> shift) & 0xFF) - darken)) < 0) v = 0;
                        pc = (pc & ~(0xFF << shift)) | ((ULONG)v << shift);
                     }
                  }

                  if (makegrey)
                  {
                     pc = (((((pc >> 16) & 0xFF) << 1) + (((pc >> 8) & 0xFF) << 2) + (pc & 0xFF)) / 7) * 0x00010101;
                  }

                  if (impose)
                  {
                     pc = (((((pc >> 16) & 0xFF) * alpha_inv + ((impose >> 16) & 0xFF) * alpha) >> 8) << 16) |
                          (((((pc >>  8) & 0xFF) * alpha_inv + ((impose >>  8) & 0xFF) * alpha) >> 8) <<  8) |
                          (((((pc      ) & 0xFF) * alpha_inv + ((impose      ) & 0xFF) * alpha) >> 8)      );
                  }

                  *(curr++) = pc;
               }

               line += 1;
            }
         }
      }
   }

   if (array != NULL)
   {
      WritePixelArray(array,0,0,bytesperrow,rp,left,top,width,height,RECTFMT_ARGB);
      FreeVec(array);
   }

   if (tbm != NULL)
   {
      WaitBlit();
      FreeBitMap(tbm);
   }

   return (success);
}

ULONG VARARGS68K ProcessPixelArray(struct RastPort *rp, LONG left, LONG top, LONG width, LONG height, struct DrawInfo *dri, ...)
{
    va_list ap;
    ULONG success;

    va_startlinear(ap,dri);
		success = ProcessPixelArrayA(rp,left,top,width,height,dri,(struct TagItem *)va_getlinearva(ap,struct TagItem *));
    va_end(ap);

    return (success);
}
