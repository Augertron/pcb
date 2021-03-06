/* $Id: report.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $ */

/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 1994,1995,1996,1997,1998,1999 Thomas Nau
 *
 *  This module, report.c, was written and is Copyright (C) 1997 harry eaton
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
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rtree.h"
#include "report.h"
#include "crosshair.h"
#include "data.h"
#include "dialog.h"
#include "drill.h"
#include "error.h"
#include "gui.h"
#include "search.h"
#include "misc.h"
#include "mymem.h"


#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Toggle.h>

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

RCSID("$Id: report.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $");

/*
 * some local prototypes
 */
#ifdef TCL_PCB
#include "tcltk/tclpcb.h"
extern void tcl_printf(FILE *, const char *, ...);
//extern Tcl_Interp *pcbinterp;
//char *button[] = {"Ok", 0 };
extern Boolean isSummary;
extern SelObjects *SelObjectEntry;
extern char* reportString;
#else
static void CB_OK (Widget, XtPointer, XtPointer);
static long int ReturnCode;	/* filled in by dialog */
static DialogButtonType button =
       { "defaultButton", "  OK  ", CB_OK, (XtPointer) OK_BUTTON, NULL };

/* ---------------------------------------------------------------------------
 * callback for standard dialog
 * just copies the button-code which is passed as ClientData to a
 * public identifier
 */
static void
CB_OK (Widget W, XtPointer ClientData, XtPointer CallData)
{
  ReturnCode = (long int) ClientData;
}

#endif

void
ReportDrills (void)
{
  DrillInfoTypePtr AllDrills;
  Cardinal n;
  char *stringlist, *thestring;
  int total_drills = 0;

  AllDrills = GetDrillInfo (PCB->Data);

  for (n = 0; n < AllDrills->DrillN; n++)
    {
      total_drills += AllDrills->Drill[n].PinCount;
      total_drills += AllDrills->Drill[n].ViaCount;
      total_drills += AllDrills->Drill[n].UnplatedCount;
    }

  stringlist = malloc (512L + AllDrills->DrillN * 64L);
  sprintf (stringlist,
	   "There are %d different drill sizes used in this layout, %d holes total\n\n"
	   "Drill Diam. (mils)      # of Pins     # of Vias    # of Elements    # Unplated\n",
	   AllDrills->DrillN, total_drills);
  thestring = stringlist;
  while (*thestring != '\0')
    thestring++;
  for (n = 0; n < AllDrills->DrillN; n++)
    {
      sprintf (thestring,
	       "  %5d                 %5d          %5d            %5d           %5d\n",
	       AllDrills->Drill[n].DrillSize / 100,
	       AllDrills->Drill[n].PinCount, AllDrills->Drill[n].ViaCount,
	       AllDrills->Drill[n].ElementN,
	       AllDrills->Drill[n].UnplatedCount);
      while (*thestring != '\0')
	thestring++;
    }
  FreeDrillInfo (AllDrills);
#ifdef TCL_PCB
  reportString =StrDup(NULL, stringlist);
  SaveFree (stringlist);
#else
  {
     /* create dialog box */
     Widget popup;
     popup = CreateDialogBox (stringlist, &button, 1, "Drill Report");
     StartDialog (popup);

     /* wait for dialog to complete */
     DialogEventLoop (&ReturnCode);
     EndDialog (popup);
     SaveFree (stringlist);
  }
#endif
}

/*
void
ReportDialogORIG (void)
{
  void *ptr1, *ptr2, *ptr3;
  int type;
  char report[2048];

  switch (type = SearchScreen (Crosshair.X, Crosshair.Y,
			       REPORT_TYPES, &ptr1, &ptr2, &ptr3))
    {
    case VIA_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->via_tree->root, 0);
	    return;
	  }
#endif
	PinTypePtr via = (PinTypePtr) ptr2;
	if (TEST_FLAG (HOLEFLAG, via))
	  sprintf (&report[0], "VIA ID# %ld  Flags:0x%08lx\n"
		   "(X,Y) = (%d, %d)\n"
		   "It is a pure hole of diameter %0.2f mils\n"
		   "Name = \"%s\""
		   "%s", via->ID, via->Flags, via->X,
		   via->Y, via->DrillingHole / 100.0, EMPTY (via->Name),
		   TEST_FLAG (LOCKFLAG, via) ? "It is LOCKED\n" : "");
	else
	  sprintf (&report[0], "VIA ID# %ld   Flags:0x%08lx\n"
		   "(X,Y) = (%d, %d)\n"
		   "Copper width = %0.2f mils  Drill width = %0.2f mils\n"
		   "Clearance width in polygons = %0.2f mils\n"
		   "Solder mask hole = %0.2f mils\n"
		   "Name = \"%s\""
		   "%s", via->ID, via->Flags, via->X,
		   via->Y, via->Thickness / 100., via->DrillingHole / 100.,
		   via->Clearance / 200., via->Mask / 100.,
		   EMPTY (via->Name), TEST_FLAG (LOCKFLAG, via) ?
		   "It is LOCKED\n" : "");
	break;
      }
    case PIN_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->pin_tree->root, 0);
	    return;
	  }
#endif
	PinTypePtr Pin = (PinTypePtr) ptr2;
	ElementTypePtr element = (ElementTypePtr) ptr1;

	PIN_LOOP (element);
	{
	  if (pin == Pin)
	    break;
	}
	END_LOOP;
	if (TEST_FLAG (HOLEFLAG, Pin))
	  sprintf (&report[0], "PIN ID# %ld  Flags:0x%08lx\n"
		   "(X,Y) = (%d, %d)\n"
		   "It is a mounting hole, Drill width = %0.2f mils\n"
		   "It is owned by element %s\n"
		   "%s", Pin->ID, Pin->Flags,
		   Pin->X, Pin->Y, Pin->DrillingHole / 100.,
		   EMPTY (element->Name[1].TextString),
		   TEST_FLAG (LOCKFLAG, Pin) ? "It is LOCKED\n" : "");
	else
	  sprintf (&report[0],
		   "PIN ID# %ld   Flags:0x%08lx\n" "(X,Y) = (%d, %d)\n"
		   "Copper width = %0.2f mils  Drill width = %0.2f mils\n"
		   "Clearance width to Polygon = %0.2f mils\n"
		   "Solder mask hole = %0.2f mils\n" "Name = \"%s\"\n"
		   "It is owned by element %s\n" "As pin number %s\n"
		   "%s",
		   Pin->ID, Pin->Flags, Pin->X, Pin->Y, Pin->Thickness / 100.,
		   Pin->DrillingHole / 100., Pin->Clearance / 200.,
		   Pin->Mask / 100., EMPTY (Pin->Name),
		   EMPTY (element->Name[1].TextString), EMPTY (Pin->Number),
		   TEST_FLAG (LOCKFLAG, Pin) ? "It is LOCKED\n" : "");
	break;
      }
    case LINE_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    LayerTypePtr layer = (LayerTypePtr) ptr1;
	    __r_dump_tree (layer->line_tree->root, 0);
	    return;
	  }
#endif
	LineTypePtr line = (LineTypePtr) ptr2;
	sprintf (&report[0], "LINE ID# %ld   Flags:0x%08lx\n"
		 "FirstPoint(X,Y) = (%d, %d)  ID = %ld\n"
		 "SecondPoint(X,Y) = (%d, %d)  ID = %ld\n"
		 "Width = %0.2f mils.\nClearance width in polygons = %0.2f mils.\n"
		 "It is on layer %d\n"
		 "and has name %s\n"
		 "%s",
		 line->ID, line->Flags,
		 line->Point1.X, line->Point1.Y,
		 line->Point1.ID, line->Point2.X, line->Point2.Y,
		 line->Point2.ID, line->Thickness / 100.,
		 line->Clearance / 200., GetLayerNumber (PCB->Data,
							 (LayerTypePtr) ptr1),
		 UNKNOWN (line->Number), TEST_FLAG (LOCKFLAG,
						    line) ? "It is LOCKED\n" :
		 "");
	break;
      }
    case RATLINE_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->rat_tree->root, 0);
	    return;
	  }
#endif
	RatTypePtr line = (RatTypePtr) ptr2;
	sprintf (&report[0], "RAT-LINE ID# %ld   Flags:0x%08lx\n"
		 "FirstPoint(X,Y) = (%d, %d) ID = %ld "
		 "connects to layer group %d\n"
		 "SecondPoint(X,Y) = (%d, %d) ID = %ld "
		 "connects to layer group %d\n",
		 line->ID, line->Flags,
		 line->Point1.X, line->Point1.Y,
		 line->Point1.ID, line->group1,
		 line->Point2.X, line->Point2.Y,
		 line->Point2.ID, line->group2);
	break;
      }
    case ARC_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    LayerTypePtr layer = (LayerTypePtr) ptr1;
	    __r_dump_tree (layer->arc_tree->root, 0);
	    return;
	  }
#endif
	ArcTypePtr Arc = (ArcTypePtr) ptr2;
	BoxTypePtr box = GetArcEnds (Arc);

	sprintf (&report[0], "ARC ID# %ld   Flags:0x%08lx\n"
		 "CenterPoint(X,Y) = (%d, %d)\n"
		 "Radius = %0.2f mils, Thickness = %0.2f mils\n"
		 "Clearance width in polygons = %0.2f mils\n"
		 "StartAngle = %ld degrees, DeltaAngle = %ld degrees\n"
		 "Bounding Box is (%d,%d), (%d,%d)\n"
		 "That makes the end points at (%d,%d) and (%d,%d)\n"
		 "It is on layer %d\n"
		 "%s", Arc->ID, Arc->Flags,
		 Arc->X, Arc->Y, Arc->Width / 100., Arc->Thickness / 100.,
		 Arc->Clearance / 200., Arc->StartAngle, Arc->Delta,
		 Arc->BoundingBox.X1, Arc->BoundingBox.Y1,
		 Arc->BoundingBox.X2, Arc->BoundingBox.Y2, box->X1,
		 box->Y1, box->X2, box->Y2, GetLayerNumber (PCB->Data,
							    (LayerTypePtr)
							    ptr1),
		 TEST_FLAG (LOCKFLAG, Arc) ? "It is LOCKED\n" : "");
	break;
      }
    case POLYGON_TYPE:
      {
	PolygonTypePtr Polygon = (PolygonTypePtr) ptr2;

	sprintf (&report[0], "POLYGON ID# %ld   Flags:0x%08lx\n"
		 "It's bounding box is (%d,%d) (%d,%d)\n"
		 "It has %d points and could store %d more\n"
		 "without using more memory.\n"
		 "It resides on layer %d\n"
		 "%s", Polygon->ID,
		 Polygon->Flags, Polygon->BoundingBox.X1,
		 Polygon->BoundingBox.Y1, Polygon->BoundingBox.X2,
		 Polygon->BoundingBox.Y2, Polygon->PointN,
		 Polygon->PointMax - Polygon->PointN,
		 GetLayerNumber (PCB->Data, (LayerTypePtr) ptr1),
		 TEST_FLAG (LOCKFLAG, Polygon) ? "It is LOCKED\n" : "");
	break;
      }
    case PAD_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->pad_tree->root, 0);
	    return;
	  }
#endif
	PadTypePtr Pad = (PadTypePtr) ptr2;
	ElementTypePtr element = (ElementTypePtr) ptr1;

	PAD_LOOP (element);
	{
	  {
	    if (pad == Pad)
	      break;
	  }
	}
	END_LOOP;
	sprintf (&report[0], "PAD ID# %ld   Flags:0x%08lx\n"
		 "FirstPoint(X,Y) = (%d, %d)  ID = %ld\n"
		 "SecondPoint(X,Y) = (%d, %d)  ID = %ld\n"
		 "Width = %0.2f mils.\nClearance width in polygons = %0.2f mils.\n"
		 "Solder mask width = %0.2f mils\n"
		 "Name = \"%s\"\n"
		 "It is owned by SMD element %s\n"
		 "As pin number %s and is on the %s\n"
		 "side of the board.\n"
		 "%s", Pad->ID,
		 Pad->Flags, Pad->Point1.X,
		 Pad->Point1.Y, Pad->Point1.ID, Pad->Point2.X, Pad->Point2.Y,
		 Pad->Point2.ID, Pad->Thickness / 100., Pad->Clearance / 200.,
		 Pad->Mask / 100., EMPTY (Pad->Name),
		 EMPTY (element->Name[1].TextString), EMPTY (Pad->Number),
		 TEST_FLAG (ONSOLDERFLAG,
			    Pad) ? "solder (bottom)" : "component",
		 TEST_FLAG (LOCKFLAG, Pad) ? "It is LOCKED\n" : "");
	break;
      }
    case ELEMENT_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->element_tree->root, 0);
	    return;
	  }
#endif
	ElementTypePtr element = (ElementTypePtr) ptr2;
	sprintf (&report[0], "ELEMENT ID# %ld   Flags:0x%08lx\n"
		 "BoundingBox (%d,%d) (%d,%d)\n"
		 "Descriptive Name \"%s\"\n"
		 "Name on board \"%s\"\n"
		 "Part number name \"%s\"\n"
		 "It's name is %0.2f mils tall and is located at (X,Y) = (%d,%d)\n"
		 "%s"
		 "Mark located at point (X,Y) = (%d,%d)\n"
		 "It is on the %s side of the board.\n"
		 "%s",
		 element->ID, element->Flags,
		 element->BoundingBox.X1, element->BoundingBox.Y1,
		 element->BoundingBox.X2, element->BoundingBox.Y2,
		 EMPTY (element->Name[0].TextString),
		 EMPTY (element->Name[1].TextString),
		 EMPTY (element->Name[2].TextString),
		 0.45 * element->Name[1].Scale, element->Name[1].X,
		 element->Name[1].Y, TEST_FLAG (HIDENAMEFLAG, element) ?
		 "But it's hidden\n" : "", element->MarkX,
		 element->MarkY, TEST_FLAG (ONSOLDERFLAG,
					    element) ? "solder (bottom)" :
		 "component", TEST_FLAG (LOCKFLAG, element) ?
		 "It is LOCKED\n" : "");
	break;
      }
    case TEXT_TYPE:
#ifndef NDEBUG
      if (ShiftPressed ())
	{
	  LayerTypePtr layer = (LayerTypePtr) ptr1;
	  __r_dump_tree (layer->text_tree->root, 0);
	  return;
	}
#endif
    case ELEMENTNAME_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->name_tree[NAME_INDEX (PCB)]->root, 0);
	    return;
	  }
#endif
	char laynum[32];
	TextTypePtr text = (TextTypePtr) ptr2;

	if (type == TEXT_TYPE)
	  sprintf (laynum, "is on layer %d",
		   GetLayerNumber (PCB->Data, (LayerTypePtr) ptr1));
	sprintf (&report[0], "TEXT ID# %ld   Flags:0x%08lx\n"
		 "Located at (X,Y) = (%d,%d)\n"
		 "Characters are %0.2f mils tall\n"
		 "Value is \"%s\"\n"
		 "Direction is %d\n"
		 "The bounding box is (%d,%d) (%d, %d)\n"
		 "It %s\n"
		 "%s", text->ID, text->Flags,
		 text->X, text->Y, 0.45 * text->Scale,
		 text->TextString, text->Direction,
		 text->BoundingBox.X1, text->BoundingBox.Y1,
		 text->BoundingBox.X2, text->BoundingBox.Y2,
		 (type == TEXT_TYPE) ? laynum : "is an element name.",
		 TEST_FLAG (LOCKFLAG, text) ? "It is LOCKED\n" : "");
	break;
      }
    case LINEPOINT_TYPE:
    case POLYGONPOINT_TYPE:
      {
	PointTypePtr point = (PointTypePtr) ptr2;
	sprintf (&report[0], "POINT ID# %ld. Points don't have flags.\n"
		 "Located at (X,Y) = (%d,%d)\n"
		 "It belongs to a %s on layer %d\n", point->ID,
		 point->X, point->Y,
		 (type == LINEPOINT_TYPE) ? "line" : "polygon",
		 GetLayerNumber (PCB->Data, (LayerTypePtr) ptr1));
	break;
      }
    case NO_TYPE:
      report[0] = '\0';
      break;

    default:
      sprintf (&report[0], "Unknown\n");
      break;
    }

  if (report[0] == '\0')
    {
      Message ("Nothing found to report on\n");
      return;
    }
  HideCrosshair (False);

#ifdef TCL_PCB
  CreateDialogBox (&report[0], button, 1, "Report");
#else
  {
      // create dialog box
     Widget popup;
     popup = CreateDialogBox (&report[0], &button, 1, "Report");
     StartDialog (popup);

     // wait for dialog to complete
     DialogEventLoop (&ReturnCode);
     EndDialog (popup);
  }
#endif
  RestoreCrosshair (False);
}

*/


/* Smart reporting routine. Report parameter for specified
 * object. If not, look for cursor position to get object.
 * Compatible with *original* code.
 * Require to take care of "isSummary" and "SelObjectEntry"
 * before calling this routine to make sure, it works even if
 * called from *original* PCB  code (Like action.c).
 */

void
ReportDialog (void)
{
  void *ptr1, *ptr2, *ptr3;
  int type;
  char report[2048];

   if(SelObjectEntry) {
      type = SelObjectEntry->obj_type;
      ptr1 = SelObjectEntry->dataPtr1;
      ptr2 = SelObjectEntry->dataPtr2;
   } else {
      type = SearchScreen (Crosshair.X, Crosshair.Y, REPORT_TYPES, &ptr1, &ptr2, &ptr3);
   }
   switch (type)
    {
    case VIA_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->via_tree->root, 0);
	    return;
	  }
#endif
	PinTypePtr via = (PinTypePtr) ptr2;
	if (!isSummary) {
	if (TEST_FLAG (HOLEFLAG, via))
	  sprintf (&report[0], "VIA ID# %ld  Flags:0x%08lx\n"
		   "(X,Y) = (%d, %d)\n"
		   "It is a pure hole of diameter %0.2f mils\n"
		   "Name = \"%s\""
		   "%s", via->ID, via->Flags, via->X,
		   via->Y, via->DrillingHole / 100.0, EMPTY (via->Name),
		   TEST_FLAG (LOCKFLAG, via) ? "It is LOCKED\n" : "");
	else
	  sprintf (&report[0], "VIA ID# %ld   Flags:0x%08lx\n"
		   "(X,Y) = (%d, %d)\n"
		   "Copper width = %0.2f mils  Drill width = %0.2f mils\n"
		   "Clearance width in polygons = %0.2f mils\n"
		   "Solder mask hole = %0.2f mils\n"
		   "Name = \"%s\""
		   "%s", via->ID, via->Flags, via->X,
		   via->Y, via->Thickness / 100., via->DrillingHole / 100.,
		   via->Clearance / 200., via->Mask / 100.,
		   EMPTY (via->Name), TEST_FLAG (LOCKFLAG, via) ?
		   "It is LOCKED\n" : "");
	} else {
	   sprintf (&report[0], "VIA ID# %ld  Flags:0x%08lx Name = \"%s\"", via->ID,
		   via->Flags, EMPTY (via->Name));
	}
	break;
      }
    case PIN_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->pin_tree->root, 0);
	    return;
	  }
#endif
	PinTypePtr Pin = (PinTypePtr) ptr2;
	ElementTypePtr element = (ElementTypePtr) ptr1;

	PIN_LOOP (element);
	{
	  if (pin == Pin)
	    break;
	}
	END_LOOP;
	if (!isSummary) {
	if (TEST_FLAG (HOLEFLAG, Pin))
	  sprintf (&report[0], "PIN ID# %ld  Flags:0x%08lx\n"
		   "(X,Y) = (%d, %d)\n"
		   "It is a mounting hole, Drill width = %0.2f mils\n"
		   "It is owned by element %s\n"
		   "%s", Pin->ID, Pin->Flags,
		   Pin->X, Pin->Y, Pin->DrillingHole / 100.,
		   EMPTY (element->Name[1].TextString),
		   TEST_FLAG (LOCKFLAG, Pin) ? "It is LOCKED\n" : "");
	else
	  sprintf (&report[0],
		   "PIN ID# %ld   Flags:0x%08lx\n" "(X,Y) = (%d, %d)\n"
		   "Copper width = %0.2f mils  Drill width = %0.2f mils\n"
		   "Clearance width to Polygon = %0.2f mils\n"
		   "Solder mask hole = %0.2f mils\n" "Name = \"%s\"\n"
		   "It is owned by element %s\n" "As pin number %s\n"
		   "%s",
		   Pin->ID, Pin->Flags, Pin->X, Pin->Y, Pin->Thickness / 100.,
		   Pin->DrillingHole / 100., Pin->Clearance / 200.,
		   Pin->Mask / 100., EMPTY (Pin->Name),
		   EMPTY (element->Name[1].TextString), EMPTY (Pin->Number),
		   TEST_FLAG (LOCKFLAG, Pin) ? "It is LOCKED\n" : "");
	} else {
	  sprintf (&report[0], "PIN ID# %ld  Flags:0x%08lx Name = \"%s\"", Pin->ID,
		  Pin->Flags, EMPTY (Pin->Number));
	}
	break;
      }
    case LINE_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    LayerTypePtr layer = (LayerTypePtr) ptr1;
	    __r_dump_tree (layer->line_tree->root, 0);
	    return;
	  }
#endif
	LineTypePtr line = (LineTypePtr) ptr2;
	if (!isSummary) {
	sprintf (&report[0], "LINE ID# %ld   Flags:0x%08lx\n"
		 "FirstPoint(X,Y) = (%d, %d)  ID = %ld\n"
		 "SecondPoint(X,Y) = (%d, %d)  ID = %ld\n"
		 "Width = %0.2f mils.\nClearance width in polygons = %0.2f mils.\n"
		 "It is on layer %d\n"
		 "and has name %s\n"
		 "%s",
		 line->ID, line->Flags,
		 line->Point1.X, line->Point1.Y,
		 line->Point1.ID, line->Point2.X, line->Point2.Y,
		 line->Point2.ID, line->Thickness / 100.,
		 line->Clearance / 200., GetLayerNumber (PCB->Data,
							 (LayerTypePtr) ptr1),
		 UNKNOWN (line->Number), TEST_FLAG (LOCKFLAG,
						    line) ? "It is LOCKED\n" :
		 "");
	} else {
	  sprintf (&report[0], "LINE ID# %ld   Flags:0x%08lx Name = \"%s\" Layer = %d",
		line->ID, line->Flags, UNKNOWN (line->Number),
		GetLayerNumber (PCB->Data, (LayerTypePtr) ptr1));
	}
	break;
      }
    case RATLINE_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->rat_tree->root, 0);
	    return;
	  }
#endif
	RatTypePtr line = (RatTypePtr) ptr2;
	if (!isSummary) {
	sprintf (&report[0], "RAT-LINE ID# %ld   Flags:0x%08lx\n"
		 "FirstPoint(X,Y) = (%d, %d) ID = %ld "
		 "connects to layer group %d\n"
		 "SecondPoint(X,Y) = (%d, %d) ID = %ld "
		 "connects to layer group %d\n",
		 line->ID, line->Flags,
		 line->Point1.X, line->Point1.Y,
		 line->Point1.ID, line->group1,
		 line->Point2.X, line->Point2.Y,
		 line->Point2.ID, line->group2);
	} else {
	  sprintf (&report[0], "RAT-LINE ID# %ld   Flags:0x%08lx", line->ID, line->Flags);
	}
	break;
      }
    case ARC_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    LayerTypePtr layer = (LayerTypePtr) ptr1;
	    __r_dump_tree (layer->arc_tree->root, 0);
	    return;
	  }
#endif
	ArcTypePtr Arc = (ArcTypePtr) ptr2;
	BoxTypePtr box = GetArcEnds (Arc);

	if (!isSummary) {
	 sprintf (&report[0], "ARC ID# %ld   Flags:0x%08lx\n"
		 "CenterPoint(X,Y) = (%d, %d)\n"
		 "Radius = %0.2f mils, Thickness = %0.2f mils\n"
		 "Clearance width in polygons = %0.2f mils\n"
		 "StartAngle = %ld degrees, DeltaAngle = %ld degrees\n"
		 "Bounding Box is (%d,%d), (%d,%d)\n"
		 "That makes the end points at (%d,%d) and (%d,%d)\n"
		 "It is on layer %d\n"
		 "%s", Arc->ID, Arc->Flags,
		 Arc->X, Arc->Y, Arc->Width / 100., Arc->Thickness / 100.,
		 Arc->Clearance / 200., Arc->StartAngle, Arc->Delta,
		 Arc->BoundingBox.X1, Arc->BoundingBox.Y1,
		 Arc->BoundingBox.X2, Arc->BoundingBox.Y2, box->X1,
		 box->Y1, box->X2, box->Y2, GetLayerNumber (PCB->Data,
							    (LayerTypePtr)
							    ptr1),
		 TEST_FLAG (LOCKFLAG, Arc) ? "It is LOCKED\n" : "");
	} else {
	  sprintf (&report[0], "ARC ID# %ld   Flags:0x%08lx Layer = %d", Arc->ID,
		Arc->Flags, GetLayerNumber (PCB->Data, (LayerTypePtr) ptr1));
	}
	break;
      }
    case POLYGON_TYPE:
      {
	PolygonTypePtr Polygon = (PolygonTypePtr) ptr2;

	if (!isSummary) {
	sprintf (&report[0], "POLYGON ID# %ld   Flags:0x%08lx\n"
		 "It's bounding box is (%d,%d) (%d,%d)\n"
		 "It has %d points and could store %d more\n"
		 "without using more memory.\n"
		 "It resides on layer %d\n"
		 "%s", Polygon->ID,
		 Polygon->Flags, Polygon->BoundingBox.X1,
		 Polygon->BoundingBox.Y1, Polygon->BoundingBox.X2,
		 Polygon->BoundingBox.Y2, Polygon->PointN,
		 Polygon->PointMax - Polygon->PointN,
		 GetLayerNumber (PCB->Data, (LayerTypePtr) ptr1),
		 TEST_FLAG (LOCKFLAG, Polygon) ? "It is LOCKED\n" : "");
	} else {
	  sprintf (&report[0], "POLYGON ID# %ld   Flags:0x%08lx Layer = %d", Polygon->ID,
		Polygon->Flags, GetLayerNumber (PCB->Data, (LayerTypePtr) ptr1));
	}
	break;
      }
    case PAD_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->pad_tree->root, 0);
	    return;
	  }
#endif
	PadTypePtr Pad = (PadTypePtr) ptr2;
	ElementTypePtr element = (ElementTypePtr) ptr1;

	PAD_LOOP (element);
	{
	  {
	    if (pad == Pad)
	      break;
	  }
	}
	END_LOOP;
	if (!isSummary) {
	sprintf (&report[0], "PAD ID# %ld   Flags:0x%08lx\n"
		 "FirstPoint(X,Y) = (%d, %d)  ID = %ld\n"
		 "SecondPoint(X,Y) = (%d, %d)  ID = %ld\n"
		 "Width = %0.2f mils.\nClearance width in polygons = %0.2f mils.\n"
		 "Solder mask width = %0.2f mils\n"
		 "Name = \"%s\"\n"
		 "It is owned by SMD element %s\n"
		 "As pin number %s and is on the %s\n"
		 "side of the board.\n"
		 "%s", Pad->ID,
		 Pad->Flags, Pad->Point1.X,
		 Pad->Point1.Y, Pad->Point1.ID, Pad->Point2.X, Pad->Point2.Y,
		 Pad->Point2.ID, Pad->Thickness / 100., Pad->Clearance / 200.,
		 Pad->Mask / 100., EMPTY (Pad->Name),
		 EMPTY (element->Name[1].TextString), EMPTY (Pad->Number),
		 TEST_FLAG (ONSOLDERFLAG,
			    Pad) ? "solder (bottom)" : "component",
		 TEST_FLAG (LOCKFLAG, Pad) ? "It is LOCKED\n" : "");
	} else {
	 sprintf (&report[0], "PAD ID# %ld   Flags:0x%08lx Name = \"%s\"", Pad->ID,
		 Pad->Flags, EMPTY (Pad->Name));
	}
	break;
      }
    case ELEMENT_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->element_tree->root, 0);
	    return;
	  }
#endif
	ElementTypePtr element = (ElementTypePtr) ptr2;
	if (!isSummary) {
	sprintf (&report[0], "ELEMENT ID# %ld   Flags:0x%08lx\n"
		 "BoundingBox (%d,%d) (%d,%d)\n"
		 "Descriptive Name \"%s\"\n"
		 "Name on board \"%s\"\n"
		 "Part number name \"%s\"\n"
		 "It's name is %0.2f mils tall and is located at (X,Y) = (%d,%d)\n"
		 "%s"
		 "Mark located at point (X,Y) = (%d,%d)\n"
		 "It is on the %s side of the board.\n"
		 "%s",
		 element->ID, element->Flags,
		 element->BoundingBox.X1, element->BoundingBox.Y1,
		 element->BoundingBox.X2, element->BoundingBox.Y2,
		 EMPTY (element->Name[0].TextString),
		 EMPTY (element->Name[1].TextString),
		 EMPTY (element->Name[2].TextString),
		 0.45 * element->Name[1].Scale, element->Name[1].X,
		 element->Name[1].Y, TEST_FLAG (HIDENAMEFLAG, element) ?
		 "But it's hidden\n" : "", element->MarkX,
		 element->MarkY, TEST_FLAG (ONSOLDERFLAG,
					    element) ? "solder (bottom)" :
		 "component", TEST_FLAG (LOCKFLAG, element) ?
		 "It is LOCKED\n" : "");
	} else {
	 sprintf (&report[0], "ELEMENT ID# %ld   Flags:0x%08lx Descriptive Name= \"%s\" Name on board = \"%s\"",
		element->ID, element->Flags, EMPTY (element->Name[0].TextString),
		 EMPTY (element->Name[1].TextString));
	}
	break;
      }
    case TEXT_TYPE:
#ifndef NDEBUG
      if (ShiftPressed ())
	{
	  LayerTypePtr layer = (LayerTypePtr) ptr1;
	  __r_dump_tree (layer->text_tree->root, 0);
	  return;
	}
#endif
    case ELEMENTNAME_TYPE:
      {
#ifndef NDEBUG
	if (ShiftPressed ())
	  {
	    __r_dump_tree (PCB->Data->name_tree[NAME_INDEX (PCB)]->root, 0);
	    return;
	  }
#endif
	char laynum[32];
	TextTypePtr text = (TextTypePtr) ptr2;

	if (type == TEXT_TYPE)
	  sprintf (laynum, "is on layer %d",
		   GetLayerNumber (PCB->Data, (LayerTypePtr) ptr1));
	if (!isSummary) {
	sprintf (&report[0], "TEXT ID# %ld   Flags:0x%08lx\n"
		 "Located at (X,Y) = (%d,%d)\n"
		 "Characters are %0.2f mils tall\n"
		 "Value is \"%s\"\n"
		 "Direction is %d\n"
		 "The bounding box is (%d,%d) (%d, %d)\n"
		 "It %s\n"
		 "%s", text->ID, text->Flags,
		 text->X, text->Y, 0.45 * text->Scale,
		 text->TextString, text->Direction,
		 text->BoundingBox.X1, text->BoundingBox.Y1,
		 text->BoundingBox.X2, text->BoundingBox.Y2,
		 (type == TEXT_TYPE) ? laynum : "is an element name.",
		 TEST_FLAG (LOCKFLAG, text) ? "It is LOCKED\n" : "");
	} else {
	  sprintf (&report[0], "TEXT ID# %ld   Flags:0x%08lx It %s", text->ID,
		text->Flags, (type == TEXT_TYPE) ? laynum : "is an element name.");
	}
	break;
      }
    case LINEPOINT_TYPE:
    case POLYGONPOINT_TYPE:
      {
	PointTypePtr point = (PointTypePtr) ptr2;
	if (!isSummary) {
	sprintf (&report[0], "POINT ID# %ld. Points don't have flags.\n"
		 "Located at (X,Y) = (%d,%d)\n"
		 "It belongs to a %s on layer %d\n", point->ID,
		 point->X, point->Y,
		 (type == LINEPOINT_TYPE) ? "line" : "polygon",
		 GetLayerNumber (PCB->Data, (LayerTypePtr) ptr1));
	} else {
	  sprintf (&report[0], "POINT ID# %ld. It belongs to a %s on layer %d", point->ID,
		(type == LINEPOINT_TYPE) ? "line" : "polygon",
		 GetLayerNumber (PCB->Data, (LayerTypePtr) ptr1));
	}
	break;
      }
    case NO_TYPE:
      report[0] = '\0';
      break;

    default:
      sprintf (&report[0], "Unknown\n");
      break;
    }

  if (report[0] == '\0')
    {
      Message ("Nothing found to report on\n");
      reportString = NULL;
      return;
    }
  HideCrosshair (False);

#ifdef TCL_PCB
  reportString =StrDup(NULL, &report[0]);
#else
  {
     /* create dialog box */
     Widget popup;
     popup = CreateDialogBox (&report[0], &button, 1, "Report");
     StartDialog (popup);

     /* wait for dialog to complete */
     DialogEventLoop (&ReturnCode);
     EndDialog (popup);
  }
#endif
  RestoreCrosshair (False);
}


void
ReportFoundPins (void)
{
  static DynamicStringType list;
  char temp[64];
  int col = 0;

  DSClearString (&list);
  DSAddString (&list, "The following pins/pads are FOUND:\n");
  ELEMENT_LOOP (PCB->Data);
  {
    PIN_LOOP (element);
    {
      if (TEST_FLAG (FOUNDFLAG, pin))
	{
	  sprintf (temp, "%s-%s,%c",
		   NAMEONPCB_NAME (element),
		   pin->Number,
		   ((col++ % (COLUMNS + 1)) == COLUMNS) ? '\n' : ' ');
	  DSAddString (&list, temp);
	}
    }
    END_LOOP;
    PAD_LOOP (element);
    {
      if (TEST_FLAG (FOUNDFLAG, pad))
	{
	  sprintf (temp, "%s-%s,%c",
		   NAMEONPCB_NAME (element), pad->Number,
		   ((col++ % (COLUMNS + 1)) == COLUMNS) ? '\n' : ' ');
	  DSAddString (&list, temp);
	}
    }
    END_LOOP;
  }
  END_LOOP;
  HideCrosshair (False);
#ifdef TCL_PCB
  reportString = StrDup(NULL, list.Data);
#else
  {
     /* create dialog box */
     Widget popup;
     popup = CreateDialogBox (list.Data, &button, 1, "Report");
     StartDialog (popup);

     /* wait for dialog to complete */
     DialogEventLoop (&ReturnCode);
     EndDialog (popup);
  }
#endif
  RestoreCrosshair (False);
}

