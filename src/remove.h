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
 *  RCS: $Id: remove.h,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $
 */

/* prototypes for remove routines
 */

#ifndef	__REMOVE_INCLUDED__
#define	__REMOVE_INCLUDED__

#include "global.h"

/* ---------------------------------------------------------------------------
 * some constants
 */
#define REMOVE_TYPES            \
	(VIA_TYPE | LINEPOINT_TYPE | LINE_TYPE | TEXT_TYPE | ELEMENT_TYPE |	\
	POLYGONPOINT_TYPE | POLYGON_TYPE | RATLINE_TYPE | ARC_TYPE)

void	*RemoveLine(LayerTypePtr, LineTypePtr);
void	*RemoveArc(LayerTypePtr, ArcTypePtr);
void	*RemovePolygon(LayerTypePtr, PolygonTypePtr);
void	*RemoveText(LayerTypePtr, TextTypePtr);
void	*RemoveElement(ElementTypePtr);
void	ClearRemoveList(void);
void	RemovePCB(PCBTypePtr);
Boolean	RemoveSelected(void);
Boolean DeleteRats(Boolean);
void	*RemoveObject(int, void *, void *, void *);
void	*DestroyObject(DataTypePtr, int, void *, void *, void *);
 
#endif
