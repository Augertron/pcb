/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 1994,1995,1996 Thomas Nau
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
 *  Thomas Nau, Schlehenweg 15, 88471 Baustetten, Germany
 *  Thomas.Nau@rz.uni-ulm.de
 *
 *  RCS: $Id: select.h,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $
 */

/* prototypes for select routines
 */

#ifndef	__SELECT_INCLUDED__
#define	__SELECT_INCLUDED__

#include "global.h"

#define SELECT_TYPES	\
	(VIA_TYPE | LINE_TYPE | TEXT_TYPE | POLYGON_TYPE | ELEMENT_TYPE |	\
	 PIN_TYPE | PAD_TYPE | ELEMENTNAME_TYPE | RATLINE_TYPE | ARC_TYPE)

Boolean		SelectObject(void);
Boolean		SelectBlock(BoxTypePtr, Boolean);
Boolean		SelectedOperation(ObjectFunctionTypePtr, Boolean, int);
void		*ObjectOperation(ObjectFunctionTypePtr, int,
				void *, void *, void *);
Boolean		SelectConnection(Boolean);

#if defined(HAVE_REGCOMP) || defined(HAVE_RE_COMP)
Boolean		SelectObjectByName(int, char *);
#endif

void		SelectedObjOperation (void);
#endif
