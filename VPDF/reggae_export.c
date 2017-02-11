/// System includes
#define AROS_ALMOST_COMPATIBLE
#include <proto/muimaster.h>
#include <libraries/mui.h>

#include <proto/exec.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <exec/libraries.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <memory.h>

#include <proto/alib.h>
#include <proto/utility.h>
#include <proto/asl.h>

#define USE_INLINE_STDARG
#include <proto/multimedia.h>
#include <classes/multimedia/video.h>
#include <classes/multimedia/metadata.h>

////

#include <private/vapor/vapor.h>
#include "util.h"
#include "reggae_export.h"
#include "pagenumberclass.h"
#include "arrowstring_class.h"
#include "system/functions.h"
#include "locale.h"
#include "settings.h"
#include "poppler.h"
#include "poppler_export.h"

struct Data
{
	Object *strOutput;
	Object *sldFirst;
	Object *sldLast;
	Object *btnSave;
	Object *btnCancel;
	Object *txtStatus;
	Object *grpRight;
	Object *gauge;
	Object *grpPage;
	Object *cycDPI;
	Object *proc;
	Object *save_str;
	Object *savedir_str;
	Object *suffix_txt;
	Object *saver_gui;
	struct exportjob pj ;
	void *doc;
	char status[256];
	char basepath[1024];
	int saving;
	int quit;
	int close;
	int max;
	int start_page;
	int last_page;
};


#define D(x) x
static const char *DPIs[] =
{
      "72",
      "96",
      "150",
      "300",
      "600",
      "1200",
      "2400",
      NULL
};


static const int DPI_values[] =
{
      72,
      96,
      150,
      300,
      600,
      1200,
      2400
};


LONG reggae_save(char *ptr, int w, int h, int dpi, IPTR saver_obj, const char* filename)
{
	LONG ret = FALSE;
	int i=0;
	Object *saver, *output, *memory_stream, *video_filter,  *video_pcm;
	QUAD slen = w*h*4;

	
	if((memory_stream = NewObject(NULL, "memory.stream",
								 MMA_StreamHandle, (IPTR)ptr,
								 MMA_StreamLength, (IPTR)&slen,
								 TAG_END)))
	{
		if((video_filter = NewObject(NULL, "rawvideo.filter",
									MMA_Video_Width, w,
									MMA_Video_Height, h,
									TAG_END)))
		{ 
			MediaSetPort(video_filter, 1, MMA_Port_Format, MMFC_VIDEO_ARGB32);

			if(MediaConnectTagList(memory_stream, 0, video_filter, 0, NULL))
			{		
				Object *saver_obj2 = (Object*)saver_obj;
				//KPrintF("sdfsdfsdf %x %x\n",saver_obj2 ,  saver_obj);
				if((saver = MediaBuildFromGuiTags(saver_obj, video_filter, 1, TAG_END)))
				{		
					IPTR filter;
					double dpi = dpi;
					
					filter = MediaGetPortFwd(saver, 0, MMA_Port_Object);
					DoMethod(filter, MMM_AddMetaItem, MMETA_VideoDpiX, MIMP_SECONDARY, &dpi, 0);
					DoMethod(filter, MMM_AddMetaItem, MMETA_VideoDpiY, MIMP_SECONDARY, &dpi, 0);
					
					if((output = NewObject(NULL, "file.output",
										  MMA_StreamName, filename, TAG_END)))
					{
						if(MediaConnectTagList(saver, 1, output, 0, TAG_END))
						{
					
							DoMethod(output, MMM_SignalAtEnd, (IPTR)FindTask(NULL), SIGBREAKB_CTRL_C);
							DoMethod(output, MMM_Play);
							Wait(SIGBREAKF_CTRL_C);
							ret = TRUE;
						}
						DisposeObject(output);
					}
					DisposeObject(saver);
				}
			}
			DisposeObject(video_filter);
		}
		DisposeObject(memory_stream);
	}				
	return ret;					
}

STRPTR reggae_get_suffix(IPTR saver_obj)
{
	char *pageclassname, *suffix = NULL;

	get(saver_obj, GGRA_PageClassName, &pageclassname);
	if (MediaGetClassAttr(pageclassname, MMA_DefaultExtension, (ULONG*)&suffix))
	{
		return suffix;	
	}
	return NULL;
}

DEFNEW
{

	struct Data data_tmp;
	Object *grpPage, *strOutput,  *btnCancel2;
	Object *right_group, *second_page,  *grpLeft;
	
	Object *free_grp;
	memset(&data_tmp, 0, sizeof(struct Data));

	second_page  = VGroup,
		Child, HVSpace,
		Child, HGroup,
			Child, HSpace(0),
			Child, data_tmp.gauge = GaugeObject,  
				MUIA_Gauge_Horiz, TRUE, 
				MUIA_Frame, MUIV_Frame_Gauge,
				MUIA_Gauge_InfoText, " ",
			End,
			Child, HSpace(0),
		End,
		Child, HGroup,
			Child, HSpace(0),
			Child, data_tmp.btnCancel = SimpleButton( LOCSTR( MSG_PRINTER_ABORT )),
			Child, HSpace(0),
		End,
		Child, HVSpace,
	End;
	
	right_group = VGroup,
	
		Child, ColGroup(2),
			MUIA_Frame, MUIV_Frame_Group,
			MUIA_Background, MUII_GroupBack,
			MUIA_FrameTitle, "Export parameters",
			/*
			Child, Label( "Base name:"),
			Child, data_tmp.save_str =  StringObject,
				StringFrame,
				MUIA_String_MaxLen, 256,
				MUIA_CycleChain, 1,
				MUIA_String_AdvanceOnCR, TRUE,
			End, */
			Child, Label( "Output file:"), 
			Child, HGroup,
				Child, PopaslObject,
					MUIA_Popasl_Type, ASL_FileRequest,
					ASLFR_TitleText, "Select base name",
					//ASLFR_DrawersOnly , TRUE,
					MUIA_Popstring_Button, PopButton( MUII_PopFile ),
					MUIA_Popstring_String,  data_tmp.savedir_str =  StringObject,
						StringFrame,
						MUIA_String_MaxLen, 256,
						MUIA_CycleChain, 1,
						MUIA_String_AdvanceOnCR, TRUE,
					End,
				End,
				Child, TextObject, MUIA_Text_Contents, "_xxx.", MUIA_Text_SetMax, TRUE, MUIA_InnerRight, 0, End,
				Child, data_tmp.suffix_txt = TextObject, MUIA_Text_Contents, "suffix", MUIA_Text_SetMax, TRUE,  MUIA_InnerLeft, 0,  End,
			End,
			Child, Label( "DPI:"), 
			Child, HGroup, 
				Child, data_tmp.cycDPI = CycleObject, MUIA_Cycle_Entries, DPIs, End,  
				Child, HVSpace, 
			End,
			Child, HVSpace,
			Child, TextObject,
                     MUIA_Text_Contents, "Pages",
					 MUIA_Text_PreParse, "\33c",
            End,
			Child, Label( LOCSTR( MSG_PRINTER_FIRSTPAGE )), 
			Child, data_tmp.sldFirst = MUI_NewObject(MUIC_Slider,
				MUIA_Slider_Min, 1,
				MUIA_Slider_Max, 1, // will be corrected when opened
				MUIA_Frame, MUIV_Frame_Slider,
			TAG_END),
			Child,  Label( LOCSTR( MSG_PRINTER_LASTPAGE )), 
			Child, data_tmp.sldLast = SliderObject,
				MUIA_Slider_Min, 1,
				MUIA_Slider_Max, 1, // will be corrected when opened      
				MUIA_Frame, MUIV_Frame_Slider,
			End,
		End,
		Child, 	data_tmp.saver_gui = MediaGetGuiTags(
					MGG_Type, MGG_Type_Muxers,
					MGG_Media, MMT_VIDEO,
					MGG_Selector, MGG_Selector_List,
					MUIA_Frame, MUIV_Frame_Group, TAG_END),
	    End;
	

	obj = DoSuperNew(cl, obj,
						MUIA_Window_Title, LOCSTR(MSG_EXPORT_WINTITLE),
						MUIA_Window_ID, MAKE_ID('E','X','P','T'),
						MUIA_Window_AppWindow, FALSE,
						MUIA_Window_RootObject, data_tmp.grpPage = PageGroup,
					    Child, VGroup,
							Child, right_group,
							Child, HGroup,
								Child, data_tmp.btnSave = SimpleButton("Export"),
								Child, HSpace(0),
								Child, btnCancel2 = SimpleButton(LOCSTR(MSG_PRINTER_CANCEL)),
							End,
						End,	
						Child, second_page,
					End,
					TAG_MORE, INITTAGS);
	if (obj != NULL)
	{
    
		GETDATA;
		*data = data_tmp;
		data->max = 1;
		
		data->proc = MUI_NewObject(MUIC_Process,
					MUIA_Process_SourceClass , cl,
					MUIA_Process_SourceObject, obj,
					MUIA_Process_Name        , "[VPDF]: Bitmap exporting process",
					MUIA_Process_Priority    , -1,
					MUIA_Process_AutoLaunch  , FALSE,
					MUIA_Process_StackSize   , 100000,
					TAG_DONE);

		DoMethod(data->proc, MUIM_Process_Launch);

		DoMethod(data->btnSave,  MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_VPDFExporter_Start);
		DoMethod(data->btnCancel, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_VPDFExporter_Stop);
		DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, MUIV_EveryTime,  obj, 1, MUIM_VPDFExporter_Close);
		DoMethod(btnCancel2, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, MUIM_VPDFExporter_Close);
		DoMethod(data->saver_gui,MUIM_Notify, GGRA_PageDefExtension, MUIV_EveryTime, obj,  1, MUIM_VPDFExporter_EncoderChange	);
	}       

	return (ULONG)obj;
}



DEFDISP
{
	GETDATA;
	
	data->quit = TRUE;
	MUI_DisposeObject(data->proc);
	
	return DOSUPER;
}


DEFMMETHOD(VPDFExporter_ExportDocument)
{
	GETDATA;
	KPrintF("VPDFExporter_ExportDocument\n");
	set(data->sldFirst, MUIA_Slider_Max,   pdfGetPagesNum(msg->doc));
	set(data->sldLast,  MUIA_Slider_Max,   pdfGetPagesNum(msg->doc)); 
	set(data->sldLast,  MUIA_Slider_Level, pdfGetPagesNum(msg->doc));
	
	data->max = pdfGetPagesNum(msg->doc);
	data->doc = msg->doc;
	data->close = FALSE;
	data->quit = FALSE;

	set(obj, MUIA_Window_Open, TRUE);
	
	return TRUE;
}


DEFMMETHOD(VPDFExporter_Start)
{
	GETDATA;
	int dpi_selection;
	STRPTR  *savedir, *save_str;
	
	data->start_page = 1;
	data->last_page = pdfGetPagesNum(data->doc); 
	set (data->gauge, MUIA_Gauge_Max, data->last_page);	 

	
	data->pj.export_gui = data->saver_gui;
	get(data->savedir_str, MUIA_String_Contents, &savedir); 
//	get(data->save_str,    MUIA_String_Contents, &save_str); 

	strcpy(data->basepath, savedir);
	//AddPart(data->basepath, save_str, 1000);
	strcat(data->basepath, "_%04d.");
	strcat(data->basepath, reggae_get_suffix(data->saver_gui));

	get(data->sldFirst,  MUIA_Slider_Level,   &data->pj.first); 
	get(data->sldLast,  MUIA_Slider_Level,  &data->pj.last); 
	get(data->sldLast,  MUIA_Slider_Level,  &data->pj.last); 
	get(data->cycDPI, MUIA_Cycle_Active, &dpi_selection);
	
	data->pj.dpi = DPI_values[dpi_selection];
	
	
	set(data->grpPage,  MUIA_Group_ActivePage, 1);
	// assuming export thread is in ready status siting on Wait()
	DoMethod(data->proc, MUIM_Process_Signal, SIGBREAKF_CTRL_D);
	
	return TRUE;
}

DEFMMETHOD(VPDFExporter_Stop)
{
	GETDATA;
	if (data->saving)
		data->quit = TRUE; // this will signal printing thread which will 'recall' this method later
	else
	{
		data->quit = FALSE;
		
		DoMethod(obj, MUIM_VPDFExporter_Done);	
	}
	return TRUE;
}


DEFMMETHOD(VPDFExporter_Done)
{
	GETDATA;
	
	data->saving = FALSE;
	data->quit = FALSE;

	if (data->close)
		DoMethod(obj, MUIM_VPDFExporter_Close);
		
	set(data->grpPage,  MUIA_Group_ActivePage, 0);
	
	return TRUE;
}

DEFMMETHOD(VPDFExporter_Close)
{
	GETDATA;
	
	if (!data->saving)
	{
		data->close = FALSE;
		set(obj, MUIA_Window_Open, FALSE);
	}
	else
	{	
		data->quit = TRUE;
		data->close = TRUE;
	}
	
	return TRUE;
}




DEFMMETHOD(Process_Process)
{
	GETDATA;
	struct exportjob *pj = &data->pj;
	int i;
	char filename[1024];
	while(!*msg->kill)
	{
		Wait(SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_C);

		if(data->quit == FALSE)
		{
		
			void *pctx;
			
			D(kprintf("started exporting:%d to %d\n", pj->first, pj->last));
			data->saving = TRUE;
			
	    	pctx = pdfExportInit(data->doc, pj->first, pj->last, pj);
			
			if(pctx != NULL)
			{
			
				for (i=pj->first; i<=pj->last;i++)
				{
            	    DoMethod(_app(data->sldFirst), MUIM_Application_PushMethod, obj, 3, MUIM_VPDFExporter_StatusUpdate, i, i);
             	    sprintf(filename, data->basepath, i);
				    pdfExportPage(pctx, i ,filename);
					KPrintF("Exporting %d %s\n", i, filename);
					
					if(data->quit)
						break;
					
				}
			}				
			else
			{
                DoMethod(_app(data->sldFirst), MUIM_Application_PushMethod, obj, 1, MUIM_VPDFExporter_Error);
			}

			pdfExportEnd(pctx);
			D(kprintf("saving done...\n"));
			DoMethod(_app(data->sldFirst), MUIM_Application_PushMethod, obj, 1, MUIM_VPDFExporter_Done);


		}

		data->saving = FALSE;
	}

	return 0;
}

DEFMMETHOD(VPDFExporter_StatusUpdate)
{
	GETDATA;
	
	snprintf(data->status, sizeof(data->status), "Saving page %ld...", msg->page_number);
	D(kprintf("%s\n", data->status));
	set(data->gauge, MUIA_Gauge_Current, msg->actual_page);
	set(data->gauge, MUIA_Gauge_InfoText, data->status);
	
	return TRUE;
}


DEFMMETHOD(VPDFExporter_Error)
{
	GETDATA;
	MUI_Request(_app(data->sldFirst), _window(obj), 0, "VPDF Export Document Error", "OK", "Failed to initialize output device", NULL);
	return TRUE;
}


DEFMMETHOD(VPDFExporter_EncoderChange)
{
	GETDATA;
	//MUI_Request(_app(data->sldFirst), _window(obj), 0, "VPDF Export Document Error", "OK", reggae_get_suffix(data->saver_gui), NULL);
	set(data->suffix_txt,MUIA_Text_Contents, reggae_get_suffix(data->saver_gui));
	return TRUE;
}

BEGINMTABLE
	DECNEW
	DECDISP
	DECMMETHOD(VPDFExporter_ExportDocument)
	DECMMETHOD(VPDFExporter_Start)
	DECMMETHOD(VPDFExporter_Stop)
	DECMMETHOD(VPDFExporter_Done)
	DECMMETHOD(VPDFExporter_Close)
	DECMMETHOD(VPDFExporter_StatusUpdate)
	DECMMETHOD(VPDFExporter_Error)
	DECMMETHOD(VPDFExporter_EncoderChange)
	DECMMETHOD(Process_Process)
ENDMTABLE

DECSUBCLASS_NC(MUIC_Window, VPDFExporterClass)   
