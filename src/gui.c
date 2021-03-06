/* $Id: gui.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $ */

/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 1994,1995,1996 Thomas Nau
 *  Copyright (C) 1997, 1998, 1999, 2000, 2001 Harry Eaton
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
 *  Harry Eaton, 6697 Buttonhole Ct, Columbia, MD 21044, USA
 *  haceaton@aplcomm.jhuapl.edu
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "global.h"
#include "crosshair.h"
#include "data.h"
#include "dialog.h"
#include "gui.h"
#include "misc.h"
#include "action.h"

#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include "icon.data"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

RCSID("$Id: gui.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $");

#define DEFAULT_CURSORSHAPE	XC_crosshair
#ifdef TCL_PCB

/* Temporary hack---Redefine these cursors with existing X font cursors
 * defined in cursorfont.h.  Later we will create the pixmap cursors
 * as was done originally.
 */

#define XC_Clockwise		XC_box_spiral
#define XC_Drag			XC_hand1
#define XC_Lock			XC_circle

#else

#define XC_Clockwise		257
#define XC_Drag                 258
#define XC_Lock                 259

#endif

#define rotateIcon_x_hot 8
#define rotateIcon_y_hot 8
#define handIcon_x_hot 8
#define handIcon_y_hot 8
#define lockIcon_x_hot 8
#define lockIcon_y_hot 8

static Cursor oldCursor;
static int ShiftKeyIndex, CtrlKeyIndex;
static char ShiftKeyMask, CtrlKeyMask;

#ifdef TCL_PCB

extern Tcl_Interp *pcbinterp;
static int result;
extern char *PrintString();
extern char *StrDup();

#else

static Widget Popup;
static Boolean Abort;

#endif

void
InitGui (void)
{
  XModifierKeymap *modMap;

  modMap = XGetModifierMapping (Dpy);
  ShiftKeyIndex = modMap->modifiermap[ShiftMapIndex] / 8;
  CtrlKeyIndex = modMap->modifiermap[ControlMapIndex] / 8;
  ShiftKeyMask = 1 << (modMap->modifiermap[ShiftMapIndex] % 8);
  CtrlKeyMask = 1 << (modMap->modifiermap[ControlMapIndex] % 8);
  XFreeModifiermap (modMap);
}

/* ---------------------------------------------------------------------------
 * check if shift key is currently held down
 */
Boolean
ShiftPressed (void)
{
  char keys[32];

  XQueryKeymap (Dpy, keys);
  if (keys[ShiftKeyIndex] & ShiftKeyMask)
    return (True);
  else
    return (False);
}

/* ---------------------------------------------------------------------------
 * check if Control key is currently held down
 */
Boolean
CtrlPressed (void)
{
  char keys[32];

  XQueryKeymap (Dpy, keys);
  if (keys[CtrlKeyIndex] & CtrlKeyMask)
    return (True);
  else
    return (False);
}

/* ---------------------------------------------------------------------------
 * wait for a button or key event in output window and calls the
 * action when a button event has been detected
 * cursor key events are handled
 */
Boolean
GetLocation (char *MessageText)
{
  XEvent event;
  XAnyEvent *any = (XAnyEvent *) & event;
  KeySym keysym;
  char buffer[10];
  int oldObjState;
  int oldLineState;
  int oldBoxState;

  MessagePrompt (MessageText);
  oldObjState = Crosshair.AttachedObject.State;
  oldLineState = Crosshair.AttachedLine.State;
  oldBoxState = Crosshair.AttachedBox.State;
  HideCrosshair (True);
  Crosshair.AttachedObject.State = STATE_FIRST;
  Crosshair.AttachedLine.State = STATE_FIRST;
  Crosshair.AttachedBox.State = STATE_FIRST;
  handCursor ();
  RestoreCrosshair (True);

  /* eat up all events that would cause actions to be performed */
  for (;;)
    {
      XtAppNextEvent (Context, &event);
      switch (any->type)
	{
	case KeyRelease:
	case KeyPress:
	case ButtonPress:
	  /* first check if we are inside the output window */
	  if (any->window != Output.OutputWindow)
	    break;

	  /* evaluate cursor keys and modifier keys;
	   * dispatch the event if true
	   */
	  XLookupString ((XKeyEvent *) & event, buffer, sizeof (buffer),
			 &keysym, NULL);
	  if (IsCursorKey (keysym) || IsModifierKey (keysym))
	    XtDispatchEvent (&event);
	  else
	    {
	      /* abort on any other key;
	       * restore cursor and clear message line
	       */
	      HideCrosshair (True);
	      Crosshair.AttachedObject.State = oldObjState;
	      Crosshair.AttachedLine.State = oldLineState;
	      Crosshair.AttachedBox.State = oldBoxState;
	      RestoreCrosshair (True);
	      restoreCursor ();
	      MessagePrompt (NULL);
	      return (any->type == ButtonPress);
	    }
	  break;

	default:
	  XtDispatchEvent (&event);
	}
    }
}

/* ---------------------------------------------------------------------------*/

void
ActionGetLocation (Widget w, XEvent * e, String * argv, Cardinal * argc)
{
  GetLocation(argv[0]);
}

/* ACTION(GetXY,ActionGetLocation) */

/* ---------------------------------------------------------------------------
 * sets the X cursor for the output window (uses cursorfont)
 * XSync() is used to force a cursor change, old memory is released
 */
unsigned int
SetOutputXCursor (unsigned int Shape)
{
  unsigned int oldShape = Output.XCursorShape;
  Cursor oldCursor = Output.XCursor;

  if (Output.XCursorShape == Shape)
    return Shape;
  /* check if window exists to prevent from fatal errors */
  if (Output.OutputWindow)
    {
      Output.XCursorShape = Shape;

#ifdef TCL_PCB /* pixmap cursors for Tk */
      XC_clock_source =
        XCreateBitmapFromData (Dpy, (Output.OutputWindow), rotateIcon_bits,
                               rotateIcon_width, rotateIcon_height);
      XC_clock_mask =
        XCreateBitmapFromData (Dpy, (Output.OutputWindow), rotateMask_bits,
                               rotateMask_width, rotateMask_height);
                                                                                                    
      XC_hand_source =
        XCreateBitmapFromData (Dpy, (Output.OutputWindow), handIcon_bits,
                               handIcon_width, handIcon_height);
      XC_hand_mask =
        XCreateBitmapFromData (Dpy, (Output.OutputWindow), handMask_bits,
                               handMask_width, handMask_height);
                                                                                                    
      XC_lock_source =
        XCreateBitmapFromData (Dpy, (Output.OutputWindow), lockIcon_bits,
                               lockIcon_width, lockIcon_height);
      XC_lock_mask =
        XCreateBitmapFromData (Dpy, (Output.OutputWindow), lockMask_bits,
                               lockMask_width, lockMask_height);

#endif

      if (Shape == XC_Clockwise || Shape == XC_Drag || Shape == XC_Lock)
	{
	  Screen *screen;
	  XColor blk, wht;

	  screen = pcbScreen (Output.Toplevel);
	  blk.pixel = BlackPixelOfScreen (screen);
	  wht.pixel = WhitePixelOfScreen (screen);
	  blk.red = 0;
	  blk.green = 0;
	  blk.blue = 0;
	  wht.red = 65535;
	  wht.green = 65535;
	  wht.blue = 65535;
	  blk.flags = DoRed | DoGreen | DoBlue;
	  wht.flags = DoRed | DoGreen | DoBlue;
	  if (Shape == XC_Clockwise)
	    Output.XCursor = XCreatePixmapCursor (Dpy, XC_clock_source,
						  XC_clock_mask, &blk, &wht,
						  rotateIcon_x_hot,
						  rotateIcon_y_hot);
	  else if (Shape == XC_Drag)
	    Output.XCursor = XCreatePixmapCursor (Dpy, XC_hand_source,
						  XC_hand_mask, &blk, &wht,
						  handIcon_x_hot,
						  handIcon_y_hot);
	  else if (Shape == XC_Lock)
	    Output.XCursor = XCreatePixmapCursor (Dpy, XC_lock_source,
						  XC_lock_mask, &blk, &wht,
						  lockIcon_x_hot,
						  lockIcon_y_hot);
	}
      else
        Output.XCursor = XCreateFontCursor (Dpy, Shape);

      XDefineCursor (Dpy, Output.OutputWindow, Output.XCursor);
      XSync (Dpy, False);

      /* release old cursor and return old shape */
      if (oldCursor)
	XFreeCursor (Dpy, oldCursor);
      return (oldShape);
    }
  return (DEFAULT_CURSORSHAPE);
}

void
handCursor (void)
{
  oldCursor = SetOutputXCursor (XC_hand2);
}

void
watchCursor (void)
{
  Cursor tmp;

  tmp = SetOutputXCursor (XC_watch);
  if (tmp != XC_watch)
    oldCursor = tmp;
}

void
modeCursor (int Mode)
{
  switch (Mode)
    {
    case NO_MODE:
      SetOutputXCursor (XC_Drag);
      break;

    case VIA_MODE:
      SetOutputXCursor (XC_arrow);
      break;

    case LINE_MODE:
      SetOutputXCursor (XC_pencil);
      break;

    case ARC_MODE:
      SetOutputXCursor (XC_question_arrow);
      break;

    case ARROW_MODE:
      SetOutputXCursor (XC_left_ptr);
      break;

    case POLYGON_MODE:
      SetOutputXCursor (XC_sb_up_arrow);
      break;

    case PASTEBUFFER_MODE:
      SetOutputXCursor (XC_hand1);
      break;

    case TEXT_MODE:
      SetOutputXCursor (XC_xterm);
      break;

    case RECTANGLE_MODE:
      SetOutputXCursor (XC_ul_angle);
      break;

    case THERMAL_MODE:
      SetOutputXCursor (XC_iron_cross);
      break;

    case REMOVE_MODE:
      SetOutputXCursor (XC_pirate);
      break;

    case ROTATE_MODE:
      if (ShiftPressed ())
	SetOutputXCursor (XC_Clockwise);
      else
	SetOutputXCursor (XC_exchange);
      break;

    case COPY_MODE:
    case MOVE_MODE:
      SetOutputXCursor (XC_crosshair);
      break;

    case INSERTPOINT_MODE:
      SetOutputXCursor (XC_dotbox);
      break;

    case LOCK_MODE:
      SetOutputXCursor (XC_Lock);
    }
}

void
cornerCursor (void)
{
  Cursor shape;

  if (Crosshair.Y <= Crosshair.AttachedBox.Point1.Y)
    shape = (Crosshair.X >= Crosshair.AttachedBox.Point1.X) ?
      XC_ur_angle : XC_ul_angle;
  else
    shape = (Crosshair.X >= Crosshair.AttachedBox.Point1.X) ?
      XC_lr_angle : XC_ll_angle;
  if (Output.XCursorShape != shape)
    SetOutputXCursor (shape);
}

void
restoreCursor (void)
{
  SetOutputXCursor (oldCursor);
}

#ifdef TCL_PCB

/* Create Tk Abort Dialog */
/* TODO : Not totally compatible with old Xt routines.
 */

void
CreateAbortDialog (char *msg)
{
    int pos;
    char *evalstr, *newstr;
    int defaultresponce = 0;     /*  Default button for the dialog.
                                  *  (0 for  the  leftmost  button, and so on).
                                  *  If less than zero or an empty string then
                                  *  there won't be any default button.
                                  */
    static char *dlgresponses[] = { "Abort", 0 };

    newstr = StrDup(NULL, msg);
    evalstr = PrintString("tk_dialog .dialog \"Abort?\""
                       	   " \"%s\" {} %d ", newstr, defaultresponce);
    free(newstr);

    for (pos = 0; dlgresponses[pos] != 0; pos++)
    {
	   newstr = StrDup(NULL, evalstr);
	   evalstr = PrintString("%s \"%s\" ", newstr, dlgresponses[pos]);
	   free(newstr);
    }

    Tcl_EvalEx(pcbinterp, evalstr, -1, 0);
}

Boolean
CheckAbort (void)
{
    Tcl_Obj *objPtr;
    int code;

    objPtr = Tcl_GetObjResult(pcbinterp);
    result = Tcl_GetIntFromObj(pcbinterp, objPtr, &code);
    if (result == TCL_OK)
       return True;
    else
       return False;
}

void
EndAbort (void)
{
    Tcl_ResetResult (pcbinterp);
}


#else

/* ---------------------------------------------------------------------------
 * callback for 'AbortDialog' dialog
 */
static void
CB_Abort (Widget W, XtPointer ClientData, XtPointer CallData)
{
  Abort = True;
}

/* ---------------------------------------------------------------------------
 * creates an 'Abort' dialog
 */
void
CreateAbortDialog (char *msg)
{
  static DialogButtonType buttons[] = {
    {"defaultButton", " Abort ", CB_Abort, (XtPointer) NULL, NULL}
  };

  /* create dialog box */
  Popup = CreateDialogBox (msg, buttons, ENTRIES (buttons), "Abort?");
  Abort = False;
  StartDialog (Popup);
}

/* ---------------------------------------------------------------------------
 * dispatches all events and returns the status of
 * 'Abort'
 */
Boolean
CheckAbort (void)
{
  while (XtAppPending (Context))
    XtAppProcessEvent (Context, XtIMAll);
  return (Abort);
}

void
EndAbort (void)
{
  EndDialog (Popup);
}

#endif


void
GetXPointer (int *x, int *y)
{
  Window root, child;
  int rootX, rootY;
  unsigned int mask;

  XQueryPointer (Dpy, Output.OutputWindow, &root, &child,
		 &rootX, &rootY, x, y, &mask);
}

void
Beep (int loud)
{
  XBell (Dpy, loud);
}
