#ifndef AOS4FUNCS_API_H
#define AOS4FUNCS_API_H

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#ifdef __cplusplus
extern "C" {
#endif

Object * VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...);
int stccpy(char *p, const char *q, int n);

#ifdef __cplusplus
}
#endif

#endif


