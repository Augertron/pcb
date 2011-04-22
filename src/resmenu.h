/* $Id: resmenu.h,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $ */

#ifndef __RESMENU_INCLUDED_
#define __RESMENU_INCLUDED_ 1

#include <stdio.h>

#include "resource.h"

#ifdef __cplusplus
extern "C" {
#endif

Widget MenuCreateFromResource(Widget menu, Resource *res,
			      Widget top, Widget left, int chain);

void MenuSetFlag(char *flag, int value);

void MenuSetAccelerators(Widget w);

#ifdef __cplusplus
}
#endif

#endif
