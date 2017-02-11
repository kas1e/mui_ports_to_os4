#ifndef LOCALE_H
#define LOCALE_H

/*
 *   VPDF
 * 
 *   Copyright © 2008 Christian Rosentreter <tokai@binaryriot.org>
 *   Copyright © 2012 Michal Zukowski, Michal Wozniak
 *   All rights reserved.
 *
 *   $Id: locale.h,v 1.3 2012/06/05 13:35:25 tokai Exp $
 */

#define NEW_CATCOMP_ARRAY_IDS
#include "VPDF_strings.h"

void   locale_init(void);
void   locale_cleanup(void);
STRPTR locale_getstr(ULONG);

#define LOCSTR(x) locale_getstr(x##_ID)


#endif /* LOCALE_H */
