
#include "Object.h"

#ifndef __amigaos4__
#include <proto/charsets.h>
#endif
#ifdef __amigaos4__
typedef unsigned long IPTR;
#endif

#define AROS_ALMOST_COMPATIBLE
#define _NO_PPCINLINE
#include <proto/keymap.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <dos/stdio.h>
#include <clib/debug_protos.h>
#include <exec/semaphores.h>
#include <exec/lists.h>
//#include <constructor.h>

#define USE_FLOAT 1
#include <poppler-config.h>
#include <config.h>

#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <time.h>

#include "PDFDoc.h"
#include "Outline.h"
#include "Link.h"
#include "GlobalParams.h"
#include "PDFDocEncoding.h"
#include "goo/GooList.h"

//#define USE_SPLASH
#ifdef USE_SPLASH
#include "splash/SplashBitmap.h"
#include "splash/Splash.h"
#include "SplashOutputDev.h"
#include "TextOutputDev.h"
#else
#include "CairoOutputDev.h"
#endif

#include "poppler.h"
#include "poppler_io.h"
#include "poppler_device.h"
#include "poppler_export.h"

extern struct Library *LocaleBase;
#define LOCALE_BASE_NAME LocaleBase
#define D(x) x
#warning TODO: make the semaphore per-document
extern struct SignalSemaphore semaphore;

extern struct Library *CairoBase;



#if 1
#define ENTER_SECTION \
{ ObtainSemaphore(&semaphore);}
#else // time delay needed to enter critical section
#define ENTER_SECTION \
{ clock_t t0 = clock(); ObtainSemaphore(&semaphore); t0 = clock() - t0; kprintf("section enter:%f:%s\n", (float)t0/CLOCKS_PER_SEC, __FUNCTION__);}
#endif

//#define ENTER_SECTION
#define LEAVE_SECTION ReleaseSemaphore(&semaphore);
//#define LEAVE_SECTION


#define OUTPUTBUFFERSIZE 262144
extern "C"
{
LONG reggae_save(char *ptr, int w, int h, int dpi, IPTR saver_obj, const char* filename);
}

struct exportcontext
{
	ULONG format;
	PDFDoc *doc;
	IPTR export_gui;
	cairo_surface_t *surface;
	cairo_t *cairo;
	CairoOutputDev *cairo_dev;
	int last_page;
	int page_size;
	int dpi;
	char buffer[OUTPUTBUFFERSIZE];
};


void *pdfExportInit(void *_ctx,  int first, int last, struct exportjob *pj)
{
	struct devicecontext *ctx = (struct devicecontext*)_ctx;
	struct exportcontext *pctx = (struct exportcontext*)calloc(1, sizeof(*pctx));

	if(pctx != NULL)
	{
		// Cairo device init 
		pctx->doc = ctx->doc;
		pctx->dpi = pj->dpi;
		pctx->export_gui = (IPTR)pj->export_gui;
		pctx->cairo_dev = new CairoOutputDev();
		pctx->cairo_dev->setPrinting(false);
		pctx->surface = NULL;
		pctx->last_page = -1; // indicate that we haven't printed anything yet               
		pctx->cairo = NULL;
		pctx->cairo_dev->startDoc(pctx->doc);
		return pctx;
	}
	return pctx;
}



int pdfExportPage(void *_pctx, int page, const char *path)
{
	struct exportcontext *pctx = (struct exportcontext*)_pctx;
	Page *pdfpage = pctx->doc->getCatalog()->getPage(page);

	ENTER_SECTION

	try
	{    
		char not_changed = FALSE;
		cairo_surface_t *surface_back=NULL;
		cairo_t *cairo_back = NULL;
		int width_mm  = round((pdfpage->getMediaWidth()/72.0) *25.4);
		int height_mm = round((pdfpage->getMediaHeight()/72.0)*25.4); 
		int width  = width_mm *pctx->dpi/25.4;
		int height = height_mm*pctx->dpi/25.4;
		
		struct pdfBitmap bm;
		// check if we had to create new surface
		if(pctx->surface != NULL)
		{
			if(cairo_image_surface_get_width(pctx->surface) != width || cairo_image_surface_get_height(pctx->surface) != height)
			{
				cairo_surface_destroy(pctx->surface);
				pctx->surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
				D(kprintf(" wrong sizes - new surface ok\n"));
			}
			else
			{
				not_changed = TRUE;
				D(kprintf(" not changed ok\n"));
			}
		}
		else
		{
			pctx->surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
			D(kprintf("new surface ok\n"));
		}
		
		D(kprintf("pdfExportPage()\n"));
		if(pctx->surface == NULL)
		{
			LEAVE_SECTION
			return FALSE;
		}
	
		// check if we print the same page 2nd time
		if (pctx->last_page != page)
		{
			pctx->cairo_dev->setCairo(NULL);
			if (pctx->cairo)                    // if any context exist - delete context
				cairo_destroy(pctx->cairo);     
			pctx->cairo = cairo_create(pctx->surface); // create new cairo context from surface
			
			cairo_save(pctx->cairo);                   // save context on stack
			
			cairo_set_source_rgb(pctx->cairo,  1., 1., 1);  // change color to white
			cairo_paint(pctx->cairo);                  // paint all surface
			pctx->cairo_dev->setCairo(pctx->cairo);  
			D(kprintf("setCairo ok\n"));	
			D(kprintf("Display page: %d\n", page));
			
			pctx->doc->displayPage(pctx->cairo_dev, page, pctx->dpi, pctx->dpi, 0, gTrue, gFalse, gTrue);
				
		   
			D(kprintf(" displayPage ok\n"));
		}
		
		#ifdef _RZK_TEST
		cairo_surface_write_to_png(surface_back, "ram:image.png");  // only for testing
		cairo_surface_write_to_png(pctx->surface , "ram:image_back.png");  // only for testing
		#endif
		
		
		if (pctx->last_page != page)  // if we 
		{
			int i, j;
			LONG pixel, *ARGB_ptr;
			char *tmp2;

			ARGB_ptr = (LONG *)cairo_image_surface_get_data(pctx->surface);
		/*	tmp2 = (char *)ARGB_ptr; 
		
			// RGBA to RGB inline conversion
			for(j = 0; j < height; j++)
			{
				for(i = 0; i < width; i++)
				{
					pixel = *ARGB_ptr++;
					*tmp2++ = (pixel & 0xFF0000) >> 16;
					*tmp2++ = (pixel & 0xFF00) >> 8;
					*tmp2++ = (pixel & 0xFF);
				}
			}
			D(kprintf(" conversion run ok\n"));*/
			KPrintF("run reggae save, export gui obj %x\n",pctx->export_gui );
			#ifndef __amigaos4__
			reggae_save((char*)ARGB_ptr, width, height, pctx->dpi, pctx->export_gui, path);
			#endif
			
		}			
		
	}
	catch(...)
	{
		LEAVE_SECTION
		return 0;
	}
	LEAVE_SECTION
	return 1;
}

void pdfExportEnd(void *_pctx)
{
	struct exportcontext *pctx = (struct exportcontext*)_pctx;
	if(pctx != NULL)
	{
		if (pctx->cairo)
		{
			cairo_destroy(pctx->cairo);
			pctx->cairo = NULL;	
		}
		if(pctx->surface)
		{
			cairo_surface_destroy(pctx->surface);
		}
		if(pctx->cairo_dev)
		{
			pctx->cairo_dev->setCairo(NULL);
			delete pctx->cairo_dev;
		}
		free(pctx);
	}
}
