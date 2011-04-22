/* $Id: pinout.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $ */

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
 */


/* pinout routines */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif


#include "global.h"

#include "copy.h"
#include "data.h"
#include "draw.h"
#include "mymem.h"
#include "move.h"
#include "pinout.h"
#include "rotate.h"

#ifdef TCL_PCB

extern Tcl_Interp *pcbinterp;

#else

#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/Viewport.h>

#endif /* TCL_PCB */

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

RCSID("$Id: pinout.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $");

/* ---------------------------------------------------------------------------
 * some local prototypes
 */

#ifndef TCL_PCB
static void CB_Dismiss (Widget, XtPointer, XtPointer);
static void CB_ShrinkOrEnlarge (Widget, XtPointer, XtPointer);
static void PinoutEvent (Widget, XtPointer, XEvent *, Boolean *);
#else
extern void tcl_printf(FILE *, const char *, ...);
#endif

void RedrawPinoutWindow (PinoutTypePtr);


/* ---------------------------------------------------------------------------
 * creates a new window to display an elements pinout
 */
PinoutTypePtr
PinoutWindow (pcbWidget Parent, ElementTypePtr Element)
{

#ifndef TCL_PCB
  Widget masterform, dismiss, scrollbar, viewport;
#else
  Tk_Window tkwind;
  char *sname;
#endif

  PinoutTypePtr pinout;
  Dimension tx, ty, minx, miny;

  if (!Element)
    return NULL;

  /* allocate memory for title and pinout data, init zoom factor */
  pinout =
    (PinoutTypePtr) MyCalloc (1, sizeof (PinoutType), "PinoutWindow()");
  pinout->Title =
    (String) MyCalloc (strlen (UNKNOWN (NAMEONPCB_NAME (Element))) +
		       strlen (UNKNOWN (DESCRIPTION_NAME (Element))) +
		       strlen (UNKNOWN (VALUE_NAME (Element))) + 5,
		       sizeof (char), "PinoutWindow()");
  sprintf (pinout->Title, "%s [%s,%s]", UNKNOWN (DESCRIPTION_NAME (Element)),
	   UNKNOWN (NAMEONPCB_NAME (Element)),
	   UNKNOWN (VALUE_NAME (Element)));

  /* copy element data 
   * enable output of pin- and padnames
   * move element to a 5% offset from zero position
   * set all package lines/arcs to zero with
   */
  CopyElementLowLevel (NULL, &pinout->Element, Element, False);
  minx = miny = 32767;
  PIN_LOOP (&pinout->Element);
  {
    tx = abs (pinout->Element.Pin[0].X - pin->X);
    ty = abs (pinout->Element.Pin[0].Y - pin->Y);
    if (tx != 0 && tx < minx)
      minx = tx;
    if (ty != 0 && ty < miny)
      miny = ty;
    SET_FLAG (DISPLAYNAMEFLAG, pin);
  }
  END_LOOP;

  PAD_LOOP (&pinout->Element);
  {
    tx = abs (pinout->Element.Pad[0].Point1.X - pad->Point1.X);
    ty = abs (pinout->Element.Pad[0].Point1.Y - pad->Point1.Y);
    if (tx != 0 && tx < minx)
      minx = tx;
    if (ty != 0 && ty < miny)
      miny = ty;
    SET_FLAG (DISPLAYNAMEFLAG, pad);
  }
  END_LOOP;
  if (minx < miny)
    RotateElementLowLevel (NULL, &pinout->Element,
			   pinout->Element.BoundingBox.X1,
			   pinout->Element.BoundingBox.Y1, 1);

  MoveElementLowLevel (NULL, &pinout->Element,
		       -pinout->Element.BoundingBox.X1 +
		       Settings.PinoutOffsetX,
		       -pinout->Element.BoundingBox.Y1 +
		       Settings.PinoutOffsetY);
  pinout->Zoom = Settings.PinoutZoom;
  pinout->scale = 1. / (100. * exp (pinout->Zoom * LN_2_OVER_2));
  pinout->MaxX = pinout->Element.BoundingBox.X2 + Settings.PinoutOffsetX;
  pinout->MaxY = pinout->Element.BoundingBox.Y2 + Settings.PinoutOffsetY;
  ELEMENTLINE_LOOP (&pinout->Element);
  {
    line->Thickness = 0;
  }
  END_LOOP;
  ARC_LOOP (&pinout->Element);
  {
    arc->Thickness = 0;
  }
  END_LOOP;

#ifndef TCL_PCB

  /* create shell window with viewport,
   * shrink, enlarge and exit button
   */
  pinout->Shell = XtVaCreatePopupShell ("pinout",
					topLevelShellWidgetClass,
					Parent,
					XtNtitle, pinout->Title,
					XtNallowShellResize, False,
					XtNmappedWhenManaged, False, NULL);
  masterform = XtVaCreateManagedWidget ("pinoutMasterForm",
					formWidgetClass,
					pinout->Shell,
					XtNresizable, False,
					XtNfromHoriz, NULL,
					XtNfromVert, NULL, NULL);
  viewport = XtVaCreateManagedWidget ("viewport",
				      viewportWidgetClass,
				      masterform,
				      XtNresizable, False,
				      XtNforceBars, True,
				      LAYOUT_NORMAL,
				      XtNallowHoriz, True,
				      XtNallowVert, True,
				      XtNuseBottom, True, NULL);
  pinout->Output = XtVaCreateManagedWidget ("output",
					    simpleWidgetClass,
					    viewport,
					    XtNresizable, True,
					    XtNwidth,
					    (Dimension) (pinout->MaxX *
							 pinout->scale),
					    XtNheight,
					    (Dimension) (pinout->MaxY *
							 pinout->scale),
					    NULL);
  dismiss =
    XtVaCreateManagedWidget ("dismiss", commandWidgetClass, masterform,
			     XtNfromVert, viewport, LAYOUT_BOTTOM, NULL);
  pinout->Shrink =
    XtVaCreateManagedWidget ("shrink", commandWidgetClass, masterform,
			     XtNfromVert, viewport, XtNfromHoriz, dismiss,
			     LAYOUT_BOTTOM, NULL);
  pinout->Enlarge =
    XtVaCreateManagedWidget ("enlarge", commandWidgetClass, masterform,
			     XtNfromVert, viewport, XtNfromHoriz,
			     pinout->Shrink, LAYOUT_BOTTOM, NULL);

  /* install accelerators for WM messages and to
   * move scrollbars with keys
   */
  XtInstallAccelerators (pinout->Shell, dismiss);
  if ((scrollbar = XtNameToWidget (viewport, "horizontal")) != NULL)
    XtInstallAccelerators (masterform, scrollbar);
  if ((scrollbar = XtNameToWidget (viewport, "vertical")) != NULL)
    XtInstallAccelerators (masterform, scrollbar);

  /* add event handler for viewport and callbacks for buttons
   * the pointer to the pinout structure is passed to these functions
   * as 'ClientData'
   */
  XtAddCallback (dismiss, XtNcallback, CB_Dismiss, (XtPointer) pinout);
  XtAddCallback (pinout->Shrink, XtNcallback,
		 CB_ShrinkOrEnlarge, (XtPointer) pinout);
  XtAddCallback (pinout->Enlarge, XtNcallback,
		 CB_ShrinkOrEnlarge, (XtPointer) pinout);
  XtAddEventHandler (pinout->Output,
		     ExposureMask,
		     False, (XtEventHandler) PinoutEvent, (XtPointer) pinout);

  /* realize (without mapping) and handle 'delete' messages */
  XtRealizeWidget (pinout->Shell);
  XSetWMProtocols (Dpy, XtWindow (pinout->Shell), &WMDeleteWindowAtom, 1);

  /* bring all stuff to the screen */
  XtPopup (pinout->Shell, XtGrabNone);

#else
   sname = (char *)Tcl_GetVar2(pcbinterp, "PCBOpts", "pinout", 
                               TCL_NAMESPACE_ONLY);
   if (sname) {
      tkwind = Tk_NameToWindow(pcbinterp, sname, Parent);
   }
   else {
      tcl_printf(stderr, "Error: PCBOpts(pinout) must be defined!\n");
      return 0;
   }

   if (tkwind == (Tk_Window)0)
      return 0;

   Tk_MapWindow(tkwind);
   pinout->Output = tkwind;

#endif

   return pinout;
}

/* ---------------------------------------------------------------------------
 * redraws pinout window
 */
void
RedrawPinoutWindow (PinoutTypePtr Pinout)
{
  Window window;
  Dimension width, height;

#ifdef TCL_PCB
  Tcl_ServiceAll();

  window = Tk_WindowId(Pinout->Output);
  width = Tk_Width(Pinout->Output);
  height = Tk_Height(Pinout->Output);
#else
  window = XtWindow(Pinout->Output);
  height = Pinout->MaxY * Pinout->scale;
  width = Pinout->MaxX * Pinout->scale;
#endif

  if (window)			/* check for valid ID */
    {
      /* setup drawable and zoom factor for drawing routines */
      SwitchDrawingWindow (Pinout->Zoom, window, False, False);

      clipBox.X1 = 0;
      clipBox.X2 = (int)((float)width / Pinout->scale);
      clipBox.Y1 = 0;
      clipBox.Y2 = (int)((float)height / Pinout->scale);

      /* clear background call the drawing routine */
      XFillRectangle (Dpy, window, Output.bgGC, 0, 0, MAX_COORD, MAX_COORD);
      DrawElement (&Pinout->Element, 0);

      /* reset drawing routines to normal operation */
      SwitchDrawingWindow (PCB->Zoom, Output.OutputWindow,
			   Settings.ShowSolderSide, True);

      clipBox = theScreen;
    }
}

#ifndef TCL_PCB

/* ---------------------------------------------------------------------------
 * event handler for all pinout windows
 * a pointer to the pinout struct is passed as ClientData
 */
static void
PinoutEvent (Widget W, XtPointer ClientData, XEvent * Event, Boolean * Flag)
{
  switch (Event->type)
    {
    case Expose:		/* just redraw the complete window */
      RedrawPinoutWindow ((PinoutTypePtr) ClientData);
      break;
    }
}

/* ---------------------------------------------------------------------------
 * callback routine of the dismiss buttons of all pinout windows
 * a pointer to the pinout struct is passed as ClientData
 */
static void
CB_Dismiss (Widget W, XtPointer ClientData, XtPointer CallData)
{
  PinoutTypePtr pinout = (PinoutTypePtr) ClientData;

  /* release memory */
  XtDestroyWidget (pinout->Shell);
  SaveFree (pinout->Title);
  FreeElementMemory (&pinout->Element);
  SaveFree (pinout);
}

/* ---------------------------------------------------------------------------
 * callback routine of the shrink or enlarge buttons of all windows
 * a pointer to the pinout struct is passed as ClientData
 */
static void
CB_ShrinkOrEnlarge (Widget W, XtPointer ClientData, XtPointer CallData)
{
  PinoutTypePtr pinout = (PinoutTypePtr) ClientData;

  if (W == pinout->Shrink && pinout->Zoom < MAX_ZOOM)
    pinout->Zoom += 1.0;
  if (W == pinout->Enlarge && pinout->Zoom > MIN_ZOOM)
    pinout->Zoom -= 1.0;
  XtVaSetValues (pinout->Output,
		 XtNwidth, (Dimension) (pinout->MaxX * pinout->scale),
		 XtNheight, (Dimension) (pinout->MaxY * pinout->scale), NULL);
  RedrawPinoutWindow (pinout);
}

#else

void
PinoutDismiss(PinoutTypePtr pinout)
{
   SaveFree (pinout->Title);
   FreeElementMemory (&pinout->Element);
   SaveFree (pinout);
}

#endif
