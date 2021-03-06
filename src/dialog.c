/* $Id: dialog.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $ */

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


/* dialog routines
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdarg.h>

#include "global.h"

#include "data.h"
#include "dialog.h"
#include "error.h"
#include "mymem.h"
#include "misc.h"

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

RCSID("$Id: dialog.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $");

/* ---------------------------------------------------------------------------
 * some local identifiers
 */
static long int ReturnCode;	/* used by standard dialogs */

#ifdef TCL_PCB
extern Tcl_Interp *pcbinterp;
extern char* reportString;

#else
/* ---------------------------------------------------------------------------
 * include the icon data
 */
#include "icon.xbm"

static void CB_OK (Widget, XtPointer, XtPointer);

#endif

/* ---------------------------------------------------------------------------
 * some local prototypes
 */
static void CenterDialog (Widget, Boolean, Boolean);
void SendEnterNotify (void);

char *
StrDup(char **oldstr, char *str)
{
    char *newstr;

    if (str != NULL)
    {
	  newstr = (char *) malloc((unsigned) (strlen(str) + 1));
	  (void) strcpy(newstr, str);
    }
    else newstr = NULL;

    if (oldstr != (char **) NULL)
    {
	  if (*oldstr != NULL)
	    free(*oldstr);
	  *oldstr = newstr;
    }
    return (newstr);
}

char *
PrintString(char *fmt, ...)
{
    va_list args;
    static char *outstr = NULL;
    static int outlen;
    int nchars;

    if (outstr == NULL)
    {
	  outlen = 100;
	  outstr = (char *) malloc((unsigned) (outlen + 1));
    }

    va_start(args, fmt);
    nchars = vsnprintf(outstr, outlen, fmt, args);
    va_end(args);

    if (nchars >= outlen)
    {
	  outlen = nchars + 1;
	  free(outstr);
	  outstr = (char *) malloc((unsigned) (outlen + 1));
	  va_start(args, fmt);
	  vsnprintf(outstr, outlen, fmt, args);
	  va_end(args);
    }
    if (nchars == -1)
	return NULL;

    return outstr;
}


#ifdef TCL_PCB

/* No more required */
/* ---------------------------------------------------------------------------
 * creates a text box. Genereally use for reporting window.
 */

void
CreateTextBox (char *title, char *labelText)
{
    char evalstr[2100];

    sprintf(evalstr, "DisplayReport \"%s\" {%s}", title, labelText);
    Tcl_Eval(pcbinterp, evalstr);
}

/* ---------------------------------------------------------------------------
 * creates a dialog box as a child of ... (see arguments)
 */

Widget
CreateDialogBox (char *labelText,
		     char** Buttons, size_t Count, char *title)
{
    char *evalstr, *newstr;
    int pos, defaultresponce = 0;/*  Default button for the dialog.
                                  *  (0 for  the  leftmost  button, and so on).
                                  *  If less than zero or an empty string then
                                  *  there won't be any default button.
                                  */

    newstr = StrDup(NULL, labelText);
    /* evalstr = PrintString("tk_dialog .dialog \"%s\""
     *                  " \"%s\" {} %d ", title, newstr, defaultresponce);
     */
    evalstr = PrintString("tk_dialog .pcb.dialog \"%s\""
                       " {%s} {} %d ", title, newstr, defaultresponce);
    free(newstr);

    for (pos = 0; Buttons[pos] != 0; pos++)
    {
	   newstr = StrDup(NULL, evalstr);
	   evalstr = PrintString("%s \"%s\" ", newstr, Buttons[pos]);
	   free(newstr);
    }

    Tcl_EvalEx(pcbinterp, evalstr, -1, 0);
    Tcl_ResetResult (pcbinterp);
    return (Widget)NULL;
}

/* ---------------------------------------------------------------------------
 * prints out a warning
 */

void
MessageDialog (char *MessageText)
{
    static char *msgresponses[] = {"Ok", 0 };

    /* create dialog box */
    CreateDialogBox (MessageText, msgresponses, ENTRIES(msgresponses), "Information");
}

/* ----------------------------------------------------------------------
 * confirmation dialog
 */
Boolean
ConfirmDialog (char *MessageText)
{
    Tcl_Obj *objPtr;
    int code, result, pos;
    char *evalstr, *newstr;
    int defaultresponce = 0;     /*  Default button for the dialog.
                                  *  (0 for  the  leftmost  button, and so on).
                                  *  If less than zero or an empty string then
                                  *  there won't be any default button.
                                  */
    static char *dlgresponses[] = { "No/Cancel", "Ok", 0 };

    newstr = StrDup(NULL, MessageText);
    evalstr = PrintString("tk_dialog .dialog \"Confirm\""
                       	   " \"%s\" {} %d ", newstr, defaultresponce);
    free(newstr);

    for (pos = 0; dlgresponses[pos] != 0; pos++)
    {
	   newstr = StrDup(NULL, evalstr);
	   evalstr = PrintString("%s \"%s\" ", newstr, dlgresponses[pos]);
	   free(newstr);
    }

    Tcl_EvalEx(pcbinterp, evalstr, -1, 0);
    objPtr = Tcl_GetObjResult(pcbinterp);
    result = Tcl_GetIntFromObj(pcbinterp, objPtr, &code);

    Tcl_ResetResult (pcbinterp);
    if (result == TCL_OK)
       return (Boolean)code;
    else
       return (Boolean)0;
}

/* ---------------------------------------------------------------------------
 * pops up and 'About' dialog
 */

void
AboutDialog (void)
{
    char aboutStr[1024];

    sprintf(aboutStr, "This is PCB, an interactive\n"
			   "printed circuit board editor\n"
			   "version " VERSION "\n\n"
			   "Compiled on " __DATE__ " at " __TIME__ "\n\n"
			   "by harry eaton\n\n"
			   "Copyright (C) Thomas Nau 1994, 1995, 1996, 1997\n"
			   "Copyright (C) Harry Eaton 1998-2004\n"
			   "Copyright (C) C. Scott Ananian 2001\n\n"
			   "It is licensed under the terms of the GNU\n"
			   "General Public License version 2\n"
			   "See the LICENSE file for more information\n\n"
			   "If you have problems, hints or suggestions,\n"
			   "send mail to:\n"
			   "Thomas.Nau@rz.uni-ulm.de\n"
			   "          or            \n"
			   "haceaton@aplcomm.jhuapl.edu");
   reportString =StrDup(NULL, aboutStr);

/*
    CreateTextBox ("About PCB", "This is PCB, an interactive\n"
			   "printed circuit board editor\n"
			   "version " VERSION "\n\n"
			   "Compiled on " __DATE__ " at " __TIME__ "\n\n"
			   "by harry eaton\n\n"
			   "Copyright (C) Thomas Nau 1994, 1995, 1996, 1997\n"
			   "Copyright (C) Harry Eaton 1998-2004\n"
			   "Copyright (C) C. Scott Ananian 2001\n\n"
			   "It is licensed under the terms of the GNU\n"
			   "General Public License version 2\n"
			   "See the LICENSE file for more information\n\n"
			   "If you have problems, hints or suggestions,\n"
			   "send mail to:\n"
			   "Thomas.Nau@rz.uni-ulm.de\n"
			   "          or            \n"
			   "haceaton@aplcomm.jhuapl.edu");
*/
}

/* ----------------------------------------------------------------------
 * confirmation dialog for replacing existing files
 * the 'ALL' button is used for a global OK
 */

int
ConfirmReplaceFileDialog (char *MessageText, Boolean AllButton)
{
  /*
  static DialogButtonType buttons1[] = {
    {"defaultButton", " No/Cancel ", CB_OK, (XtPointer) CANCEL_BUTTON, NULL},
    {"okButton", "    OK    ", CB_OK, (XtPointer) OK_BUTTON, NULL},
    {"okButton", "Sequence OK", CB_OK, (XtPointer) ALL_BUTTON, NULL}
  };
  */

    Tcl_Obj *objPtr;
    int code, result, pos;
    char *evalstr, *newstr;
    int defaultresponce = 0;     /*  Default button for the dialog.
                                  *  (0 for  the  leftmost  button, and so on).
                                  *  If less than zero or an empty string then
                                  *  there won't be any default button.
                                  */
    static char *buttons[] = { "No/Cancel", "Ok", "Sequence OK", 0 };

    newstr = StrDup(NULL, MessageText);
    evalstr = PrintString("tk_dialog .dialog \"Confirm\""
                       	   " \"%s\" {} %d ", newstr, defaultresponce);
    free(newstr);

    for (pos = 0; buttons[pos] != 0; pos++)
    {
	   newstr = StrDup(NULL, evalstr);
	   evalstr = PrintString("%s \"%s\" ", newstr, buttons[pos]);
	   free(newstr);
    }

    Tcl_EvalEx(pcbinterp, evalstr, -1, 0);
    objPtr = Tcl_GetObjResult(pcbinterp);
    result = Tcl_GetIntFromObj(pcbinterp, objPtr, &code);

    Tcl_ResetResult (pcbinterp);
    if (result == TCL_OK)
       return code;
    else
       return -1;
}


#else    /* !TCL_PCB */

/* ----------------------------------------------------------------------
 * confirmation dialog
 */
Boolean
ConfirmDialog (char *MessageText)
{
  Widget popup;
  static DialogButtonType buttons[] = {
    {"defaultButton", "No/Cancel", CB_OK, (XtPointer) CANCEL_BUTTON, NULL},
    {"okButton", "   OK   ", CB_OK, (XtPointer) OK_BUTTON, NULL}
  };

  /* create dialog box */
  popup =
    CreateDialogBox (MessageText, buttons, ENTRIES (buttons), "Confirm");
  StartDialog (popup);

  /* wait for dialog to complete */
  DialogEventLoop (&ReturnCode);
  EndDialog (popup);
  return (ReturnCode == OK_BUTTON);
}

/* ---------------------------------------------------------------------------
 * thanks to Ellen M. Sentovich and Rick L. Spickelm for their hint in
 * xrn
 *
 * creates a dialog box as a child of ... (see arguments)
 */
Widget
CreateDialogBox (char *LabelText,
		 DialogButtonTypePtr Buttons, size_t Count, char *title)
{
  Widget dialog, popup;

  /* create the popup shell */
  popup = XtVaCreatePopupShell (title, transientShellWidgetClass,
				Output.Toplevel,
				XtNinput, True,
				XtNallowShellResize, True, NULL);
  dialog = XtVaCreateManagedWidget ("dialog", dialogWidgetClass,
				    popup, XtNlabel, LabelText, NULL);

  /* now we add the buttons and install the accelerators */
  AddButtons (dialog, NULL, Buttons, Count);
  while (Count--)
    XtInstallAccelerators (dialog, Buttons[Count].W);
  return (popup);
}

/* ---------------------------------------------------------------------------
 * prints out a warning
 */

void
MessageDialog (char *MessageText)
{
  Widget popup;
  static DialogButtonType buttons[] = {
    {"defaultButton", "  OK  ", CB_OK, (XtPointer) OK_BUTTON, NULL}
  };

  /* create dialog box */
  popup = CreateDialogBox (MessageText, buttons, ENTRIES (buttons), "Information");
  StartDialog (popup);

  /* wait for dialog to complete */
  DialogEventLoop (&ReturnCode);
  EndDialog (popup);
}

/* ---------------------------------------------------------------------------
 * pops up and 'About' dialog
 */

/* ACTION(About,AboutDialog) */
void
AboutDialog (void)
{
  Widget popup, dialog;
  Pixmap icon = BadAlloc;
  static DialogButtonType button =
    { "defaultButton", "  OK  ", CB_OK, (XtPointer) OK_BUTTON, NULL };

  /* create dialog box */
  popup = CreateDialogBox ("This is PCB, an interactive\n"
			   "printed circuit board editor\n"
			   "version " VERSION "\n\n"
			   "Compiled on " __DATE__ " at " __TIME__ "\n\n"
			   "by harry eaton\n\n"
			   "Copyright (C) Thomas Nau 1994, 1995, 1996, 1997\n"
			   "Copyright (C) harry eaton 1998-2004\n"
			   "Copyright (C) C. Scott Ananian 2001\n\n"
			   "It is licensed under the terms of the GNU\n"
			   "General Public License version 2\n"
			   "See the LICENSE file for more information\n\n"
			   "If you have problems, hints or suggestions,\n"
			   "send mail to:\n"
			   "bumpelo at comcast dot net\n\n",
			   &button, 1, "About PCB");
  if ((dialog = XtNameToWidget (popup, "dialog")) != NULL)
    {
      Screen *screen = XtScreen (Output.Toplevel);

      if ((icon = XCreatePixmapFromBitmapData (Dpy, XtWindow (Output.Toplevel),
					       icon_bits, icon_width,
					       icon_height,
					       BlackPixelOfScreen (screen),
					       WhitePixelOfScreen (screen),
					       DefaultDepthOfScreen (screen)))
	  != BadAlloc)
	{
	  XtVaSetValues (dialog, XtNicon, icon, NULL);
	}
    }

  StartDialog (popup);

  /* wait for dialog to complete */
  DialogEventLoop (&ReturnCode);
  EndDialog (popup);
  if (icon != BadAlloc)
    XFreePixmap (Dpy, icon);
}

/* ----------------------------------------------------------------------
 * confirmation dialog for replacing existing files
 * the 'ALL' button is used for a global OK
 */
int
ConfirmReplaceFileDialog (char *MessageText, Boolean AllButton)
{
  Widget popup;
  static DialogButtonType buttons[] = {
    {"defaultButton", " No/Cancel ", CB_OK, (XtPointer) CANCEL_BUTTON, NULL},
    {"okButton", "    OK    ", CB_OK, (XtPointer) OK_BUTTON, NULL},
    {"okButton", "Sequence OK", CB_OK, (XtPointer) ALL_BUTTON, NULL}
  };

  /* create dialog box */
  popup = CreateDialogBox (MessageText, buttons,
			   ENTRIES (buttons) - (AllButton ? 0 : 1),
			   "Confirm");
  StartDialog (popup);

  /* wait for dialog to complete */
  DialogEventLoop (&ReturnCode);
  EndDialog (popup);
  return (ReturnCode);
}


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

#endif  /* TCL_PCB */

/* ---------------------------------------------------------------------------
 * centers the widget around the current cursor position
 */
static void
CenterDialog (Widget Popup, Boolean CenterX, Boolean CenterY)
{
  Window root, child;
  int root_x, root_y, child_x, child_y;
  unsigned int mask;
  Dimension width, height;

  /* get current pointer position relativ to it's parent */
  XQueryPointer (Dpy, XtWindow (Popup),
		 &root, &child, &root_x, &root_y, &child_x, &child_y, &mask);

  /* get the dialogs size */
  XtVaGetValues (Popup, XtNheight, &height, XtNwidth, &width, NULL);

  /* make sure position is inside our screen */
  if (CenterX)
    {
      if ((root_x -= (width / 2)) < 0)
	{
	  root_x = 0;
	}
      else
	{
	  if (root_x > WidthOfScreen (XtScreen (Popup)) - width)
	    root_x = WidthOfScreen (XtScreen (Popup)) - width;
	}
    }

  if (CenterY)
    {
      if ((root_y -= (height / 2)) < 0)
	{
	  root_y = 0;
	}
      else
	{
	  if (root_y > HeightOfScreen (XtScreen (Popup)) - height)
	    root_y = HeightOfScreen (XtScreen (Popup)) - height;
	}
    }

  XtVaSetValues (Popup, XtNx, root_x, XtNy, root_y, NULL);
}

/* ---------------------------------------------------------------------------
 * pops up a dialog
 * saves output window in pixmap
 * first realize the dialog and
 * focus keyboard events from the application to it
 */
void
StartDialog (Widget Popup)
{
  Atom protocol;

  XtRealizeWidget (Popup);
  CenterDialog (Popup, True, True);
  XtPopup (Popup, XtGrabExclusive);
  protocol = XInternAtom (Dpy, "WM_DELETE_WINDOW", True);
  if (protocol != None)
    XSetWMProtocols (Dpy, XtWindow (Popup), &protocol, 1);
}

/* ---------------------------------------------------------------------------
 * removes a dialog from screen and from memory
 * pixmap is released by next dialog
 */
void
EndDialog (Widget Popup)
{
  XtPopdown (Popup);
  XtDestroyWidget (Popup);
  XFlush (Dpy);
  XSync (Dpy, False);
}

/* ---------------------------------------------------------------------------
 * adds the buttons to a dialog
 */
Widget
AddButtons (Widget Parent, Widget Top,
	    DialogButtonTypePtr Buttons, size_t Count)
{
  int i;

  for (i = 0; i < Count; i++)
    {
      /* skip button if there's no label,
       * used for dialogs without default button
       */
      if (!Buttons[i].Label)
	continue;
      Buttons[i].W = XtVaCreateManagedWidget (Buttons[i].Name,
					      commandWidgetClass, Parent,
					      XtNlabel, Buttons[i].Label,
					      XtNfromHoriz,
					      i ? Buttons[i - 1].W : NULL,
					      XtNfromVert, Top, XtNresizable,
					      True, NULL);

      XtAddCallback (Buttons[i].W,
		     XtNcallback, Buttons[i].Callback, Buttons[i].ClientData);
    }
  return (Buttons[Count - 1].W);
}

/* ---------------------------------------------------------------------------
 * waits until returncode is differnt from -1
 * see also CB_OK()
 */
long int
DialogEventLoop (long int *Code)
{
  XEvent event;

  for (*Code = -1; *Code == -1;)
    {
      XtAppNextEvent (Context, &event);
      /* exit if window manager says delete window */
      if (event.type == ClientMessage)
	return (*Code);
      XtDispatchEvent (&event);
    }
  return (*Code);
}

/* ---------------------------------------------------------------------------
 * displays a single line prompt (message)
 */
void
MessagePrompt (char *MessageText)
{
  /* set some resources for the input fields */
  if (Output.Message && (!MessageText || !*MessageText))
    {
      XUnmapWindow (Dpy, XtWindow (Output.Message));
      XMapWindow (Dpy, XtWindow (Output.StatusLine));
      Output.Message = NULL;
    }
  else
    {
      XUnmapWindow (Dpy, XtWindow (Output.StatusLine));

      /* the message label */
      Output.Message =
	XtVaCreateManagedWidget ("messageText", labelWidgetClass,
				 Output.MasterForm, XtNlabel,
				 EMPTY (MessageText), XtNfromHoriz,
				 Output.Control, XtNfromVert, Output.Porthole,
				 LAYOUT_BOTTOM, NULL);
    }
}

/* ---------------------------------------------------------------------------
 * called by action ActionFinishInputDialog() to indicate that a user input
 * is finished. The passed parameter signals either OK (True) or Cancel
 */
void
FinishInputDialog (Boolean OK)
{
  ReturnCode = OK ? OK_BUTTON : CANCEL_BUTTON;
}

/* ---------------------------------------------------------------------------
 * since all my dialogs are modal the X server wont create 'EnterNotify'
 * events during the dialog is popped up. This breaks the crosshair
 * visibility management. Therefore I check if the pointer is inside
 * the output window and generate a 'EnterNotify' event.
 */
void
SendEnterNotify (void)
{
  XEnterWindowEvent event;
  Window root, child;
  int root_x, root_y, child_x, child_y;
  unsigned int mask;

  if (XQueryPointer (Dpy, Output.OutputWindow, &root, &child,
		     &root_x, &root_y, &child_x, &child_y, &mask))
    {
      event.type = EnterNotify;
      event.display = Dpy;
      event.window = Output.OutputWindow;
      event.subwindow = child;
      event.root = root;
      event.detail = NotifyInferior;
      event.x = child_y;
      event.y = child_x;
      event.x_root = root_x;
      event.y_root = root_y;
      event.mode = 0;
      event.state = 0;
      event.time = 0;
      event.focus = True;
      event.same_screen = True;
      XSendEvent (Dpy, Output.OutputWindow, True,
		  EnterWindowMask, (XEvent *) & event);
    }
}

/* ---------------------------------------------------------------------------
 * Various functions to manage the history buffer.
 */

typedef struct {
  /* int caret; - future */
  char *buffer;
} HistoryElement;

HistoryElement *history_buffer = 0;
int history_size, history_last, history_cur;

void
history_init (void)
{
  if (Settings.HistorySize == 0)
    return;
  history_size = Settings.HistorySize + 2;
  if (history_size < 3)
    history_size = 3;
  history_buffer = calloc (history_size, sizeof (HistoryElement));
  history_last = history_cur = 0;
}

HistoryElement *
history_get(int index)
{
  int i;

  if (!history_buffer)
    history_init();
  if (!history_buffer)
    return NULL;

  i = (index + history_size) % history_size;
  return history_buffer+i;
}

void
history_put(char *str, int temp)
{
  if (!history_buffer)
    history_init();
  if (!history_buffer)
    return;

  /* Avoid storing duplicates.  */
  if (temp
      || !history_buffer[history_last].buffer
      || strcmp (str, history_buffer[history_last].buffer) != 0)
    {
      history_last = (history_last + 1) % history_size;
      if (history_buffer[history_last].buffer)
	free (history_buffer[history_last].buffer);
      history_buffer[history_last].buffer = MyStrdup (str, "history_put()");
    }

  /* Make sure there is at least one NULL entry in the loop.  */
  if (history_buffer[(history_last+1) % history_size].buffer)
    {
      free (history_buffer[(history_last+1) % history_size].buffer);
      history_buffer[(history_last+1) % history_size].buffer = 0;
    }
  if (temp)
    history_last = (history_last - 1 + history_size) % history_size;
}

/* ---------------------------------------------------------------------------
 * gets a string from user, memory is allocated for the string,
 * the user is responsible for releasing the allocated memory
 * string might be empty if flag is set
 */

#ifndef TCL_PCB

static  Widget inputfield;

char *
GetUserInput (char *MessageText, char *OutputString)
{
  char *string;
  Dimension messageWidth, viewportWidth;

  /* display single line message */
  MessagePrompt (MessageText);

  /* calculate size */
  XtVaGetValues (Output.Message, XtNwidth, &messageWidth, NULL);
  XtVaGetValues (Output.Porthole, XtNwidth, &viewportWidth, NULL);

  /* the input field itself */
  inputfield = XtVaCreateManagedWidget ("inputField", asciiTextWidgetClass,
					Output.MasterForm, XtNresizable, True,
					XtNeditType, XawtextEdit,
					XtNfromHoriz, Output.Message,
					XtNfromVert, Output.Porthole, XtNstring,
					EMPTY (OutputString), XtNinsertPosition,
					OutputString ? strlen(OutputString) : 0,
					XtNwidth, 
					MAX (100, viewportWidth - messageWidth),
					LAYOUT_BOTTOM_RIGHT, NULL);

  /* set focus to input field, override default translations
   * and install accelerators
   * grap all events --> make widget modal
   */
  XtSetKeyboardFocus (Output.Toplevel, inputfield);
  XtOverrideTranslations (inputfield,
			  XtParseTranslationTable (InputTranslations));
  XtInstallAccelerators (inputfield, Output.MasterForm);
  XtAddGrab (inputfield, True, False);
  XtRealizeWidget (inputfield);

  /* -1 means the history queue doesn't contain the current line yet.  */
  history_cur = -1;

  /* wait for input to complete and allocate memory if necessary */
  if (DialogEventLoop (&ReturnCode) == OK_BUTTON)
    {
      /* strip white space and return string */
      XtVaGetValues (inputfield, XtNstring, &string, NULL);
      string = StripWhiteSpaceAndDup (string);
      history_put (string, 0);
    }
  else
    string = NULL;

  /* restore normal outfit */
  XtRemoveGrab (inputfield);
  XtUnmanageChild (inputfield);
  XtDestroyWidget (inputfield);
  MessagePrompt (NULL);

  /* force an 'EnterNotify' event if the pointer has moved
   * into the output area
   */
  SendEnterNotify ();
  XSync (Dpy, False);
  return (string);
}

#endif /* !TCL_PCB --- see tcltk/tclpcb.c for Tcl version */

#ifdef TCL_PCB

void
ActionCommandHistory (Widget w, XEvent *Event, String *Params, Cardinal *num)
{
  Tk_Window tkwind;
  tkwind = (Tk_Window)w;
}


#else /*(!TCL_PCB)---Xt definitions */


/* ACTION(CommandHistory,ActionCommandHistory) */
void
ActionCommandHistory (Widget W, XEvent *Event, String *Params, Cardinal *num)
{
  char *string;
  HistoryElement *he;
  int offs = 0;

  if (!Settings.HistorySize || !Params[0])
    {
      XBell(Dpy, 100);
      return;
    }
  if (strcmp (Params[0], "prev") == 0)
    offs = -1;
  else if (strcmp (Params[0], "next") == 0)
    offs = +1;
  else
    {
      fprintf(stderr, "Usage: CommandHistory(prev|next)\n");
      return;
    }

  if (history_cur == -1
      || history_cur == (history_last + 1) % history_size)
    {
      XtVaGetValues (inputfield, XtNstring, &string, NULL);
      history_put (string, 1);
      history_cur = (history_last + 1) % history_size;
    }

  if (offs)
    {
      he = history_get (history_cur + offs);
      if (!he->buffer)
	{
	  XBell(Dpy, 100);
	  return;
	}
      history_cur += offs + history_size;
      history_cur %= history_size;
      XtVaSetValues (inputfield, XtNstring, he->buffer, NULL);
      XtVaSetValues (inputfield, XtNinsertPosition, strlen(he->buffer), NULL);
    }
}


#endif /* (TCL_PCB) */



