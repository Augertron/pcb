/* $Id: djopt.h,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $ */

/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 2003 DJ Delorie
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Contact addresses for paper mail and Email:
 *  DJ Delorie, 334 North Road, Deerfield NH 03037-1110, USA
 *  dj@delorie.com
 *
 */

#ifndef	__DJOPT_INCLUDED__
#define	__DJOPT_INCLUDED__

#include "global.h"

/*
void debumpify(void);
void unjaggy(void);
void simple_optimizations(void);
void vianudge(void);
void viatrim(void);
void orthopull(void);
void miter(void);
void automagic(void);
*/
extern int autorouted_only;
/* route options */
extern int route_debumpify;
extern int route_orthopull;
extern int route_unjaggy;
extern int route_miter;
extern int route_simple;
extern int route_via_nudge;
extern int route_via_trim;
extern int route_auto;


#ifdef TCL_PCB
void ActionDJopt(char **);
#else
void ActionDJopt(Widget, XEvent *, String *, Cardinal *);
#endif

void RouteOptimize();

#endif
