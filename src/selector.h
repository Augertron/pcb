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
 *  RCS: $Id: selector.h,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $
 */

/* prototypes for selectors
 */

#ifndef	__SELECTOR_INCLUDED__
#define	__SELECTOR_INCLUDED__

#include "global.h"

/* ---------------------------------------------------------------------------
 * some types
 */
typedef struct
{
	String		Text;
	XtPointer	ClientData;
} SelectorEntryType, *SelectorEntryTypePtr;

typedef struct				/* a select box */
{
	String		Name;		/* the name of the list widget */
	Widget		ViewportW,	/* the listwidget and its manager */
			ListW;
	XtCallbackProc	Callback;	/* notify callback */
	XtPointer	ClientData;
	int		Number,		/* number of entries */
			MaxNumber;	/* maximum number */
	SelectorEntryTypePtr	Entries;	/* per entry data */
	String		*StringList;
} SelectorType, *SelectorTypePtr;

/* ---------------------------------------------------------------------------
 * the prototypes
 */
void	UpdateSelector(SelectorTypePtr);
void	AddEntryToSelector(char *, XtPointer, SelectorTypePtr);
void	FreeSelectorEntries(SelectorTypePtr);
Widget	CreateSelector(Widget, Widget, Widget, SelectorTypePtr, Cardinal);

#endif
