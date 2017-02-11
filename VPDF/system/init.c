#include	<stdlib.h>
#include	<stdio.h>

#include	<proto/exec.h>
#include    <proto/codesets.h>
#include	<datatypes/datatypes.h>
#include	<cybergraphx/cybergraphics.h>
#include	<proto/cybergraphics.h>

#include	"system.h"
#include	"functions.h"
#include	"timer.h"
//os4 #include	"altivec.h"
#include	"memory.h"

#include <proto/dos.h>


struct Library 			*CyberGfxBase = NULL;
struct CyberGfxIFace	*ICyberGfx	  = NULL;

struct Library			*CodesetsBase	= NULL;
struct CodesetsIFace	*ICodesets		= NULL; 

struct Library			*DataTypesBase	= NULL;
struct DataTypesIFace 	*IDataTypes		= NULL;

void ExitSystem(void)
{
	if (CyberGfxBase)
		CloseLibrary(CyberGfxBase);
		
	if (CodesetsBase)
		CloseLibrary(CodesetsBase);

	if (DataTypesBase)
		CloseLibrary(DataTypesBase);

}

void InitSystem( int argc , char *argv[] )
{

    if(!(CyberGfxBase = OpenLibrary("cybergraphics.library",40))) {
		fprintf(stderr, "Failed to open cybergraphics.library.\n");
		exit(0);
    }
    ICyberGfx=(struct CyberGfxIFace*)GetInterface(CyberGfxBase,"main",1,NULL);

 
    if(!(CodesetsBase = OpenLibrary("PROGDIR:libs/codesets.library", CODESETSVER))) {
		fprintf(stderr, "Failed to open PROGDIR:libs/codesets.library.\n");
		exit(0);
	}
	ICodesets = (struct CodesetsIFace *)GetInterface(CodesetsBase, "main", 1, NULL);  

	
	if(!(DataTypesBase = OpenLibrary("datatypes.library", 37))) {
		fprintf(stderr, "Failed to open datatypes.library.\n");
		exit(0);
	}
	IDataTypes = (struct DataTypesIFace *)GetInterface(DataTypesBase, "main", 1, NULL);  
	
	ParamsInit(argc, argv);
	TimerInit();
#ifndef __amigaos4__
	AltivecInit();
#endif
	MemoryInit();
	ConversionInit();

	atexit(&ExitSystem);

}

