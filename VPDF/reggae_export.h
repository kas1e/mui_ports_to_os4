#ifndef REGGAEEXPORT_MCC_H
#define REGGAEEXPORT_MCC_H

#include "mcc/classes.h"


#define	MUIM_VPDFExporter_ExportDocument   	    (MUIM_VPDFExporter_Dummy + 1)
#define	MUIM_VPDFExporter_Start			        (MUIM_VPDFExporter_Dummy + 2)
#define MUIM_VPDFExporter_Stop                  (MUIM_VPDFExporter_Dummy + 3)
#define MUIM_VPDFExporter_Done                  (MUIM_VPDFExporter_Dummy + 4)
#define MUIM_VPDFExporter_StatusUpdate          (MUIM_VPDFExporter_Dummy + 5)
#define MUIM_VPDFExporter_Close                 (MUIM_VPDFExporter_Dummy + 6)
#define MUIM_VPDFExporter_Error                 (MUIM_VPDFExporter_Dummy + 7)
#define MUIM_VPDFExporter_EncoderChange 		(MUIM_VPDFExporter_Dummy + 8)

struct MUIP_VPDFExporter_ExportDocument{ULONG MethodID; APTR doc;};
struct MUIP_VPDFExporter_Start{ULONG MethodID;};
struct MUIP_VPDFExporter_Stop{ULONG MethodID;};
struct MUIP_VPDFExporter_Done{ULONG MethodID;};
struct MUIP_VPDFExporter_StatusUpdate{ULONG MethodID; ULONG page_number; ULONG actual_page;};
struct MUIP_VPDFExporter_Close{ULONG MethodID;};
struct MUIP_VPDFExporter_Error{ULONG MethodID;};

#define	VPDFExporterObject   NewObject(getVPDFExporterClass(), NULL

DEFCLASS(VPDFExporter);

#endif