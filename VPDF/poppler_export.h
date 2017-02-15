#ifndef POPPLER_PRINTER_H


#include <exec/types.h>
#include <exec/lists.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Exporting pages to bitmaps */

struct exportjob
{
	int  first;
	int  last;
	int  total_pages;
	int  dpi;
	char output[1024];
	char basename[1024];
	Object *export_gui;
};

void *pdfExportInit(void *_ctx,  int first, int last, struct exportjob *pj);
int pdfExportPage(void *_pctx, int page, const char *path);
void pdfExportEnd(void *_pctx);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif