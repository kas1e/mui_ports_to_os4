#ifndef MAIN_H
#define MAIN_H 1

#include <exec/types.h>
#include <proto/intuition.h>
#include <dos/dos.h>

#include <SDI_compiler.h>
#include <SDI_hook.h>

#if !defined(__MORPHOS__)

#include <stdarg.h>
#include <strings.h>
#include <lua.h>

Object * VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...);

#define AllocTaskPooled(size) AllocMem(size, MEMF_ANY)
#define FreeTaskPooled(mem, size) FreeMem(mem, size)

APTR AllocVecTaskPooled(ULONG byteSize);
void FreeVecTaskPooled(APTR memory);

// morphos lua.library calls to static lua in os4:
#define LuaState lua_State
#define LuaNewState luaL_newstate

typedef const char* (*LuaReader)(LuaState*, APTR, LONG*);

#define LUA_READER_MEMORY   (LuaReader)(-1)
#define LUA_READER_FILE     (LuaReader)(-2)

#define CODE_BUFFER_SIZE 16384

#define LuaLibReg luaL_Reg



#define LuaCheckString luaL_checkstring
#define LuaPushLiteral lua_pushliteral
#define LuaPushBoolean lua_pushboolean
#define LuaPushInteger lua_pushinteger
#define LuaPushString lua_pushstring
#define LuaSetTable lua_settable

#define LuaClose lua_close
#define LuaSetField lua_setfield
#define LuaPushLString lua_pushlstring
#define LuaNewTable lua_newtable
#define LuaToString lua_tostring
#define LuaSetGlobal lua_setglobal
#define LuaPCall lua_pcall
#define LuaRegisterModule lua_register

#endif

#define APP_VER      "0.11"
#define APP_DATE     "19.10.2014"
#define APP_AUTHOR   "Grzegorz Kraszewski"
#define APP_NAME     "LibMaker"
#define APP_CYEARS   "2014"
#define APP_BASE     "LIBMAKER"
#define APP_DESC     "Library code generator"
#define APP_ABOUT    "\33b%p\33n\n\t" APP_AUTHOR "\n"

#define FINDOBJ(parent, id) (Object*)DoMethod(parent, MUIM_FindUData, id)
#define UNUSED __attribute__((unused))
#define XSet(obj, attr, val) SetAttrs(obj, attr, (IPTR)val, TAG_END)
#define XNSet(obj, attr, val) DoMethod(obj, MUIM_NoNotifySet, attr, (IPTR)val)


/* maximum lengths of various strings */

#define MAXLEN_LIBRARY_NAME            64
#define MAXLEN_COPYRIGHT_STRING        64
#define MAXLEN_FUNCTION_NAME           40
#define MAXLEN_TYPE_SPEC               40
#define MAXLEN_ARGUMENT_NAME           40
#define MAXLEN_ATTRIBUTE_NAME          64
#define MAXLEN_METHOD_NAME             64
#define MAX_ARGUMENT_COUNT             10
#define LINEBUFSIZE                   200

/* extra errors from project loader */

#define LDLERR_SYNTAX_ERROR            99
#define LDLERR_VALUE_TOO_LONG          98
#define LDLERR_WRONG_SCRIPT_TYPE       97
#define LDLERR_SPECIAL_CHARS_IN_PATH   96


Object* DoSuperNewM(Class *cl, Object *obj, ...);
Object* MUI_NewObjectM(const char *classname, ...);
Object* NewObjectM(Class *cl, const char *classname, ...);
ULONG XGet(Object *obj, ULONG attr);
ULONG HexStrToULong(CONST_STRPTR s);
struct RDArgs* ParseLine(char *line, char *templ, LONG *params, struct RDArgs *srcargs);
BOOL ReadLine(BPTR file, char *buffer);


extern CONST_STRPTR IdentifierChars;
extern CONST_STRPTR TypeChars;
extern CONST_STRPTR HexChars;

/* LOCALE */

extern struct Locale *Loc;
extern struct Catalog *Cat;

#define LS(id, str) GetCatalogStr(Cat, id, str)

#define MSG_APPLICATION_DESCRIPTION                    0
#define MSG_MENUITEM_ABOUT                             1
#define MSG_MENUITEM_QUIT                              2
#define MSG_BUTTON_GENERATE_CODE                       3
#define MSG_LIBRARY_NAME_LABEL                         4
#define MSG_DESTINATION_DIR_LABEL                      5
#define MSG_DESTINATION_DIR_REQTITLE                   6
#define MSG_LIBRARY_NAME_HELP                          7
#define MSG_DESTINATION_DIR_HELP                       8
#define MSG_OBJ_BTN_GENERATE_CODE_HELP                 9
#define MSG_LIB_VERSION_LABEL                         10
#define MSG_LIBRARY_VERSION_HELP                      11
#define MSG_LIB_REVISION_LABEL                        12
#define MSG_LIBRARY_REVISION_HELP                     13
#define MSG_LIB_DATE_LABEL                            14
#define MSG_LIBRARY_DATE_HELP                         15
#define MSG_LIB_COPYRIGHT_LABEL                       16
#define MSG_LIBRARY_COPYRIGHT_HELP                    17
#define MSG_FUNCLIST_BAR_TITLE                        18
#define MSG_FUNCLIST_HEADER_RESULT_TYPE               19
#define MSG_FUNCLIST_HEADER_NAME                      20
#define MSG_FUNCLIST_HEADER_ARGUMENTS                 21
#define MSG_FUNCLIST_BUTTON_ADD                       22
#define MSG_FUNCLIST_BUTTON_DELETE                    23
#define MSG_FUNCLIST_BUTTON_ADD_HELP                  24
#define MSG_FUNCLIST_BUTTON_DELETE_HELP               25
#define MSG_FUNCLIST_HELP                             26
#define MSG_FUNCEDITOR_FUNCTION_NAME_LABEL            27
#define MSG_FUNCEDITOR_FUNCTION_NAME_HELP             28
#define MSG_FUNCEDITOR_RESULT_TYPE_LABEL              29
#define MSG_FUNCEDITOR_RESULT_TYPE_HELP               30
#define MSG_FUNCEDITOR_SAVE_BUTTON                    31
#define MSG_FUNCEDITOR_SAVE_BUTTON_HELP               32
#define MSG_FUNCEDITOR_ARGUMENTS_BAR                  33
#define MSG_FUNCEDITOR_WINTITILE_PATTERN              34
#define MSG_ARGLIST_HEADER_TYPE                       35
#define MSG_ARGLIST_HEADER_NAME                       36
#define MSG_FUNCEDITOR_ADDARG_BUTTON                  37
#define MSG_FUNCEDITOR_ADDARG_BUTTON_HELP             38
#define MSG_FUNCEDITOR_DELARG_BUTTON                  37
#define MSG_FUNCEDITOR_DELARG_BUTTON_HELP             38
#define MSG_FUNCEDITOR_ARGTYPE_LABEL                  39
#define MSG_FUNCEDITOR_ARGTYPE_HELP                   40
#define MSG_FUNCEDITOR_ARGNAME_LABEL                  41
#define MSG_FUNCEDITOR_ARGNAME_HELP                   42
#define MSG_FUNCEDITOR_ARGLIST_HELP                   43
#define MSG_FUNCEDITOR_ARGNAME_NOT_SPECIFIED          44
#define MSG_FUNCEDITOR_ARGTYPE_NOT_SPECIFIED          45
#define MSG_MENU_PROJECT                              46
#define MSG_MENUITEM_OPEN                             47
#define MSG_MENUITEM_SAVE                             48
#define MSG_MENUITEM_SAVE_AS                          49
#define MSG_SAVEREQ_TITLE                             50
#define MSG_SAVEREQ_SAVE_BUTTON                       51
#define MSG_SAVING_ERROR                              52
#define MSG_OPENREQ_TITLE                             53
#define MSG_OPENREQ_OPEN_BUTTON                       54
#define MSG_ERROR_PROJECT_SYNTAX                      55
#define MSG_ERROR_TEXT_STRING_TOO_LONG                56
#define MSG_OPENING_ERROR                             57
#define MSG_LIBRARY_BASE_NAME_HELP                    58
#define MSG_ALTIVEC_CHECKMARK_LABEL                   59
#define MSG_ALTIVEC_CHECKMARK_HELP                    60
#define MSG_BOOPSI_CHECKMARK_LABEL                    61
#define MSG_BOOPSI_CHECKMARK_HELP                     62
#define MSG_MUI_CHECKMARK_LABEL                       63
#define MSG_MUI_CHECKMARK_HELP                        64
#define MSG_CLASSEDITOR_SAVE_BUTTON                   65
#define MSG_CLASSEDITOR_SAVE_BUTTON_HELP              66
#define MSG_CLASSEDITOR_WINTITILE_PATTERN             67
#define MSG_CLASSEDITOR_SUPERCLASS_LABEL              68
#define MSG_CLASSEDITOR_ATTRIBUTES_FRAMETITLE         69
#define MSG_CLASSEDITOR_ADDATTRIBUTE_BUTTON           70
#define MSG_CLASSEDITOR_ADDATTRIBUTE_BUTTON_HELP      71
#define MSG_CLASSEDITOR_DELATTRIBUTE_BUTTON           72
#define MSG_CLASSEDITOR_DELATTRIBUTE_BUTTON_HELP      73
#define MSG_CLASSEDITOR_METHODS_FRAMETITLE            74
#define MSG_ATTRLIST_HEADER_TYPE                      75
#define MSG_ATTRLIST_HEADER_ID                        76
#define MSG_ATTRLIST_HEADER_USAGE                     77
#define MSG_METHLIST_HEADER_NAME                      78
#define MSG_METHLIST_HEADER_IDENT                     79
#define MSG_METHLIST_HEADER_MESSAGE                   80
#define MSG_CLASSEDITOR_ATTRNAME_LABEL                81
#define MSG_CLASSEDITOR_ATTRID_LABEL                  82
#define MSG_CLASSEDITOR_ATTRUSAGE_LABEL               83
#define MSG_CLASSEDITOR_ATTRUSAGE_INIT                84
#define MSG_CLASSEDITOR_ATTRUSAGE_SET                 85
#define MSG_CLASSEDITOR_ATTRUSAGE_GET                 86
#define MSG_CLASSEDITOR_ATTRIBUTE_NOT_SPECIFIED       87
#define MSG_METHODEDITOR_NAME_LABEL                   88
#define MSG_METHODEDITOR_ID_LABEL                     89
#define MSG_METHODEDITOR_NEW_METHOD                   90
#define MSG_METHODEDITOR_SAVE_BUTTON                  91
#define MSG_METHODEDITOR_SAVE_BUTTON_HELP             92
#define MSG_METHODEDITOR_STRCTURE_LABEL               93
#define MSG_CODE_GENERATOR_GROUP_TITLE                94
#define MSG_SINGLE_FUNCTIONS_CHECKMARK_LABEL          95
#define MSG_SINGLE_FUNCTIONS_CHECKMARK_HELP           96
#define MSG_SINGLE_METHODS_CHECKMARK_LABEL            97
#define MSG_SINGLE_METHODS_CHECKMARK_HELP             98
#define MSG_SCRIPT_SET_CYCLE_LABEL                    99
#define MSG_SCRIPT_SET_CYCLE_HELP                    100
#define MSG_ERROR_SPECIAL_CHARS_IN_PATH              101
#define MSG_ERROR_NO_CODE_GENERATOR                  102
#define MSG_INSTALL_DIR_LABEL                        103
#define MSG_INSTALL_DIR_REQTITLE                     104
#define MSG_INSTALL_DIR_HELP                         105
#define MSG_INCLUDE_PATH_LABEL                       106
#define MSG_INCLUDE_PATH_HELP                        107
#define MSG_FUNCEDITOR_M68KREG_LABEL                 108
#define MSG_ARGLIST_HEADER_REGISTER                  109
#define MSG_FUNCEDITOR_M68KREG_HELP                  110

#endif /* MAIN_H */
