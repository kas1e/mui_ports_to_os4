/* AttributeListClass header. */

#include <libraries/mui.h>

extern struct MUI_CustomClass *AttributeListClass;

struct MUI_CustomClass *CreateAttributeListClass(void);
void DeleteAttributeListClass(void);

struct AttributeEntry
{
	STRPTR ae_Name;
	IPTR   ae_Id;
	IPTR   ae_Usage;
};

#define ATTR_USAGE_INIT     1
#define ATTR_USAGE_SET      2
#define ATTR_USAGE_GET      4
