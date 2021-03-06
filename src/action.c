/* $Id: action.c,v 1.2 2006/11/05 17:55:34 tim Exp $ */

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

/* action routines for output window
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>

#ifdef TCL_PCB
#include <tk.h>
#endif

#include "global.h"

#include "action.h"
#include "autoplace.h"
#include "autoroute.h"
#include "buffer.h"
#include "change.h"
#include "command.h"
#include "control.h"
#include "copy.h"
#include "create.h"
#include "crosshair.h"
#include "data.h"
#include "dialog.h"
#include "draw.h"
#include "error.h"
#include "file.h"
#include "fileselect.h"
#include "find.h"
#include "gui.h"
#include "insert.h"
#include "line.h"
#include "lgdialog.h"
#include "mymem.h"
#include "misc.h"
#include "move.h"
#include "netlist.h"
#include "output.h"
#include "pinout.h"
#include "polygon.h"
#include "printdialog.h"
#include "rats.h"
#include "remove.h"
#include "report.h"
#include "rotate.h"
#include "rubberband.h"
#include "search.h"
#include "select.h"
#include "set.h"
#include "sizedialog.h"
#include "undo.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

RCSID("$Id: action.c,v 1.2 2006/11/05 17:55:34 tim Exp $");

/* ---------------------------------------------------------------------------
 * some local types
 */
typedef enum
{
  F_AddSelected,
  F_All,
  F_AllConnections,
  F_AllRats,
  F_AllUnusedPins,
  F_Arc,
  F_Arrow,
  F_Block,
  F_Description,
  F_Center,
  F_Clear,
  F_ClearAndRedraw,
  F_ClearList,
  F_Close,
  F_Connection,
  F_Convert,
  F_Copy,
  F_CycleClip,
  F_DeleteRats,
  F_Drag,
  F_DrillReport,
  F_Element,
  F_ElementByName,
  F_ElementConnections,
  F_ElementToBuffer,
  F_Find,
  F_FlipElement,
  F_FoundPins,
  F_Grid,
  F_InsertPoint,
  F_Layer,
  F_Layout,
  F_LayoutAs,
  F_LayoutToBuffer,
  F_Line,
  F_LineSize,
  F_Mirror,
  F_Move,
  F_NameOnPCB,
  F_Netlist,
  F_None,
  F_Notify,
  F_Object,
  F_ObjectByName,
  F_PasteBuffer,
  F_PadByName,
  F_PinByName,
  F_PinOrPadName,
  F_Pinout,
  F_Polygon,
  F_PreviousPoint,
  F_RatsNest,
  F_Rectangle,
  F_Redraw,
  F_Release,
  F_Remove,
  F_RemoveSelected,
  F_Report,
  F_Reset,
  F_ResetLinesAndPolygons,
  F_ResetPinsViasAndPads,
  F_Restore,
  F_Rotate,
  F_Save,
  F_Scroll,
  F_Selected,
  F_SelectedArcs,
  F_SelectedElements,
  F_SelectedLines,
  F_SelectedNames,
  F_SelectedObjects,
  F_SelectedPads,
  F_SelectedPins,
  F_SelectedTexts,
  F_SelectedVias,
  F_SelectedRats,
  F_Stroke,
  F_Text,
  F_TextByName,
  F_TextScale,
  F_Thermal,
  F_ToggleAllDirections,
  F_ToggleAutoDRC,
  F_ToggleClearLine,
  F_ToggleGrid,
  F_ToggleMask,
  F_ToggleName,
  F_ToggleObject,
  F_ToggleShowDRC,
  F_ToggleLiveRoute,
  F_ToggleRubberBandMode,
  F_ToggleStartDirection,
  F_ToggleSnapPin,
  F_ToggleThindraw,
  F_ToggleOrthoMove,
  F_ToggleLocalRef,
  F_ToggleCheckPlanes,
  F_ToggleUniqueNames,
  F_Via,
  F_ViaByName,
  F_Value,
  F_ViaDrillingHole,
  F_ViaSize,
  F_Zoom
}
FunctionID;

typedef struct			/* used to identify subfunctions */
{
  char *Identifier;
  FunctionID ID;
}
FunctionType, *FunctionTypePtr;

/* ---------------------------------------------------------------------------
 * some local identifiers
 */
static PointType InsertedPoint;
static LayerTypePtr lastLayer;
static struct
{
  PolygonTypePtr poly;
  LineType line;
}
fake;

static struct
{
  int X;
  int Y;
  Cardinal Buffer;
  Boolean Click;
  Boolean Moving;		/* selected type clicked on */
  int Hit;			/* move type clicked on */
  void *ptr1;
  void *ptr2;
  void *ptr3;
}
Note;

static Cardinal polyIndex = 0;
static Boolean IgnoreMotionEvents = False;
static Boolean saved_mode = False;
#ifdef HAVE_LIBSTROKE
static Boolean mid_stroke = False;
static BoxType StrokeBox;
#endif
static FunctionType Functions[] = {
  {"AddSelected", F_AddSelected},
  {"All", F_All},
  {"AllConnections", F_AllConnections},
  {"AllRats", F_AllRats},
  {"AllUnusedPins", F_AllUnusedPins},
  {"Arc", F_Arc},
  {"Arrow", F_Arrow},
  {"Block", F_Block},
  {"Description", F_Description},
  {"Center", F_Center},
  {"Clear", F_Clear},
  {"ClearAndRedraw", F_ClearAndRedraw},
  {"ClearList", F_ClearList},
  {"Close", F_Close},
  {"Connection", F_Connection},
  {"Convert", F_Convert},
  {"Copy", F_Copy},
  {"CycleClip", F_CycleClip},
  {"DeleteRats", F_DeleteRats},
  {"Drag", F_Drag},
  {"DrillReport", F_DrillReport},
  {"Element", F_Element},
  {"ElementByName", F_ElementByName},
  {"ElementConnections", F_ElementConnections},
  {"ElementToBuffer", F_ElementToBuffer},
  {"Find", F_Find},
  {"FlipElement", F_FlipElement},
  {"FoundPins", F_FoundPins},
  {"Grid", F_Grid},
  {"InsertPoint", F_InsertPoint},
  {"Layer", F_Layer},
  {"Layout", F_Layout},
  {"LayoutAs", F_LayoutAs},
  {"LayoutToBuffer", F_LayoutToBuffer},
  {"Line", F_Line},
  {"LineSize", F_LineSize},
  {"Mirror", F_Mirror},
  {"Move", F_Move},
  {"NameOnPCB", F_NameOnPCB},
  {"Netlist", F_Netlist},
  {"None", F_None},
  {"Notify", F_Notify},
  {"Object", F_Object},
  {"ObjectByName", F_ObjectByName},
  {"PasteBuffer", F_PasteBuffer},
  {"PadByName", F_PadByName},
  {"PinByName", F_PinByName},
  {"PinOrPadName", F_PinOrPadName},
  {"Pinout", F_Pinout},
  {"Polygon", F_Polygon},
  {"PreviousPoint", F_PreviousPoint},
  {"RatsNest", F_RatsNest},
  {"Rectangle", F_Rectangle},
  {"Redraw", F_Redraw},
  {"Release", F_Release},
  {"Remove", F_Remove},
  {"RemoveSelected", F_RemoveSelected},
  {"Report", F_Report},
  {"Reset", F_Reset},
  {"ResetLinesAndPolygons", F_ResetLinesAndPolygons},
  {"ResetPinsViasAndPads", F_ResetPinsViasAndPads},
  {"Restore", F_Restore},
  {"Rotate", F_Rotate},
  {"Save", F_Save},
  {"Scroll", F_Scroll},
  {"Selected", F_Selected},
  {"SelectedArcs", F_SelectedArcs},
  {"SelectedElements", F_SelectedElements},
  {"SelectedLines", F_SelectedLines},
  {"SelectedNames", F_SelectedNames},
  {"SelectedObjects", F_SelectedObjects},
  {"SelectedPins", F_SelectedPins},
  {"SelectedPads", F_SelectedPads},
  {"SelectedRats", F_SelectedRats},
  {"SelectedTexts", F_SelectedTexts},
  {"SelectedVias", F_SelectedVias},
  {"Stroke", F_Stroke},
  {"Text", F_Text},
  {"TextByName", F_TextByName},
  {"TextScale", F_TextScale},
  {"Thermal", F_Thermal},
  {"Toggle45Degree", F_ToggleAllDirections},
  {"ToggleClearLine", F_ToggleClearLine},
  {"ToggleGrid", F_ToggleGrid},
  {"ToggleMask", F_ToggleMask},
  {"ToggleName", F_ToggleName},
  {"ToggleObject", F_ToggleObject},
  {"ToggleRubberBandMode", F_ToggleRubberBandMode},
  {"ToggleStartDirection", F_ToggleStartDirection},
  {"ToggleSnapPin", F_ToggleSnapPin},
  {"ToggleThindraw", F_ToggleThindraw},
  {"ToggleCheckPlanes", F_ToggleCheckPlanes},
  {"ToggleLocalRef", F_ToggleLocalRef},
  {"ToggleOrthoMove", F_ToggleOrthoMove},
  {"ToggleShowDRC", F_ToggleShowDRC},
  {"ToggleLiveRoute", F_ToggleLiveRoute},
  {"ToggleAutoDRC", F_ToggleAutoDRC},
  {"ToggleUniqueNames", F_ToggleUniqueNames},
  {"Value", F_Value},
  {"Via", F_Via},
  {"ViaByName", F_ViaByName},
  {"ViaSize", F_ViaSize},
  {"ViaDrillingHole", F_ViaDrillingHole},
  {"Zoom", F_Zoom}
};

/* ---------------------------------------------------------------------------
 * some local routines
 */
void WarpPointer (Boolean);
static int GetFunctionID (String);
static void AdjustAttachedBox (void);
static void NotifyLine (void);
static void NotifyBlock (void);
void NotifyMode (void);
static void ClearWarnings (void);
#ifdef HAVE_LIBSTROKE
static void FinishStroke (void);
extern void stroke_init (void);
extern void stroke_record (int x, int y);
extern int stroke_trans (char *s);
#endif
static void ChangeFlag (Widget, XEvent *, String *, int, char *);

#ifdef TCL_PCB
extern Tcl_CmdProc *TclExecutePcbCommand();
extern Tcl_Interp *pcbinterp;
extern char *reportString;
#endif

#ifdef HAVE_LIBSTROKE

/* ---------------------------------------------------------------------------
 * FinishStroke - try to recognize the stroke sent
 */
void
FinishStroke (void)
{
  char msg[255];
  int type;
  unsigned long num;
  void *ptr1, *ptr2, *ptr3;

  mid_stroke = False;
  if (stroke_trans (msg))
    {
      num = atoi (msg);
      switch (num)
	{
	case 456:
	  if (Settings.Mode == LINE_MODE)
	    {
	      SetMode (LINE_MODE);
	    }
	  break;
	case 9874123:
	case 74123:
	case 987412:
	case 8741236:
	case 874123:
	  RotateScreenObject (StrokeBox.X1, StrokeBox.Y1, SWAP_IDENT ? 1 : 3);
	  break;
	case 7896321:
	case 786321:
	case 789632:
	case 896321:
	  RotateScreenObject (StrokeBox.X1, StrokeBox.Y1, SWAP_IDENT ? 3 : 1);
	  break;
	case 258:
	  SetMode (LINE_MODE);
	  break;
	case 852:
	  SetMode (ARROW_MODE);
	  break;
	case 1478963:
	  ActionUndo (0, 0, 0, 0);
	  break;
	case 147423:
	case 147523:
	case 1474123:
	  Redo (True);
	  break;
	case 148963:
	case 147863:
	case 147853:
	case 145863:
	  SetMode (VIA_MODE);
	  break;
	case 951:
	case 9651:
	case 9521:
	case 9621:
	case 9851:
	case 9541:
	case 96521:
	case 96541:
	case 98541:
	  SetZoom (1000);	/* special zoom extents */
	  break;
	case 159:
	case 1269:
	case 1259:
	case 1459:
	case 1569:
	case 1589:
	case 12569:
	case 12589:
	case 14589:
	  {
	    Location x = (StrokeBox.X1 + StrokeBox.X2) / 2;
	    Location y = (StrokeBox.Y1 + StrokeBox.Y2) / 2;
	    int z;
	    z =
	      1 +
	      log (fabs (StrokeBox.X2 - StrokeBox.X1) / Output.Width) /
	      log (2.0);
	    z =
	      MAX (z,
		   1 +
		   log (fabs (StrokeBox.Y2 - StrokeBox.Y1) / Output.Height) /
		   log (2.0));
	    SetZoom (z);

	    CenterDisplay (x, y, False);
	    break;
	  }

	default:
	  Message ("Unknown stroke %s\n", msg);
	  break;
	}
    }
  else
    Beep (Settings.Volume);
}
#endif

/* ---------------------------------------------------------------------------
 * Clear warning color from pins/pads
 */
static void
ClearWarnings ()
{
  Settings.RatWarn = False;
  ALLPIN_LOOP (PCB->Data);
  {
    if (TEST_FLAG (WARNFLAG, pin))
      {
	CLEAR_FLAG (WARNFLAG, pin);
	DrawPin (pin, 0);
      }
  }
  ENDALL_LOOP;
  ALLPAD_LOOP (PCB->Data);
  {
    if (TEST_FLAG (WARNFLAG, pad))
      {
	CLEAR_FLAG (WARNFLAG, pad);
	DrawPad (pad, 0);
      }
  }
  ENDALL_LOOP;
  UpdatePIPFlags (NULL, NULL, NULL, False);
  Draw ();
}

#ifdef TCL_PCB
static void
CB_Click (ClientData unused)
#else
static void
CB_Click (XtPointer unused, pcbIntervalId * time)
#endif
{
  if (Note.Click)
    {
      Note.Click = False;
      if (Note.Moving && !ShiftPressed ())
	{
	  HideCrosshair (True);
	  Note.Buffer = Settings.BufferNumber;
	  SetBufferNumber (MAX_BUFFER - 1);
	  ClearBuffer (PASTEBUFFER);
	  AddSelectedToBuffer (PASTEBUFFER, Note.X, Note.Y, True);
	  SaveUndoSerialNumber ();
	  RemoveSelected ();
	  SaveMode ();
	  saved_mode = True;
	  SetMode (PASTEBUFFER_MODE);
	  RestoreCrosshair (True);
	}
      else if (Note.Hit && !ShiftPressed ())
	{
	  HideCrosshair (True);
	  SaveMode ();
	  saved_mode = True;
	  SetMode (CtrlPressed() ? COPY_MODE : MOVE_MODE);
	  Crosshair.AttachedObject.Ptr1 = Note.ptr1;
	  Crosshair.AttachedObject.Ptr2 = Note.ptr2;
	  Crosshair.AttachedObject.Ptr3 = Note.ptr3;
	  Crosshair.AttachedObject.Type = Note.Hit;
	  AttachForCopy (Note.X, Note.Y);
	  RestoreCrosshair (True);
	}
      else
	{
	  BoxType box;

	  Note.Hit = 0;
	  Note.Moving = False;
	  HideCrosshair (True);
	  SaveUndoSerialNumber ();
	  box.X1 = -MAX_COORD;
	  box.Y1 = -MAX_COORD;
	  box.X2 = MAX_COORD;
	  box.Y2 = MAX_COORD;
	  /* unselect first if shift key not down */
	  if (!ShiftPressed () && SelectBlock (&box, False))
	    SetChangedFlag (True);
	  NotifyBlock ();
	  Crosshair.AttachedBox.Point1.X = Note.X;
	  Crosshair.AttachedBox.Point1.Y = Note.Y;
	  RestoreCrosshair (True);
	}
    }
}

void
ReleaseMode (void)
{
  BoxType box;

  if (Note.Click)
    {
      BoxType box;

      box.X1 = -MAX_COORD;
      box.Y1 = -MAX_COORD;
      box.X2 = MAX_COORD;
      box.Y2 = MAX_COORD;

      Note.Click = False;	/* inhibit timer action */
      SaveUndoSerialNumber ();
      /* unselect first if shift key not down */
      if (!ShiftPressed ())
	{
/*      Original code --
	  if (SelectBlock (&box, False))
	    SetChangedFlag (True);
	  if (Note.Moving)
	    {
	      Note.Moving = 0;
	      Note.Hit = 0;
   	      return;
	    }
*/
	  if (SelectBlock (&box, False)) {
	    SetChangedFlag (True);
	    if (Note.Moving)
	    {
	        Note.Moving = 0;
	        Note.Hit = 0;
	    }
   	    return;
          }
	}
      RestoreUndoSerialNumber ();
      if (SelectObject ())
	SetChangedFlag (True);
      Note.Hit = 0;
      Note.Moving = 0;
    }
  else if (Note.Moving)
    {
      RestoreUndoSerialNumber ();
      NotifyMode ();
      ClearBuffer (PASTEBUFFER);
      SetBufferNumber (Note.Buffer);
      Note.Moving = False;
      Note.Hit = 0;
    }
  else if (Note.Hit)
    {
      NotifyMode ();
      Note.Hit = 0;
    }
  else if (Settings.Mode == ARROW_MODE)
    {
      box.X1 = MIN (Crosshair.AttachedBox.Point1.X,
		    Crosshair.AttachedBox.Point2.X);
      box.Y1 = MIN (Crosshair.AttachedBox.Point1.Y,
		    Crosshair.AttachedBox.Point2.Y);
      box.X2 = MAX (Crosshair.AttachedBox.Point1.X,
		    Crosshair.AttachedBox.Point2.X);
      box.Y2 = MAX (Crosshair.AttachedBox.Point1.Y,
		    Crosshair.AttachedBox.Point2.Y);
      RestoreUndoSerialNumber ();
      if (SelectBlock (&box, True))
	SetChangedFlag (True);
      else if (Bumped)
	IncrementUndoSerialNumber ();
      Crosshair.AttachedBox.State = STATE_FIRST;
    }
  if (saved_mode)
    RestoreMode ();
  saved_mode = False;
}

#ifdef TCL_PCB

/*
 * Actions copied from src/actionlist.c
 */

extern void AboutDialog();
extern void ActionActiveWhen(Widget,XEvent *,String *, Cardinal *);
extern void ActionApplyVendor(Widget,XEvent *,String *, Cardinal *);
extern void ActionCheckWhen(Widget,XEvent *,String *, Cardinal *);
extern void ActionCommandHistory(Widget,XEvent *,String *, Cardinal *);
extern void ActionDisableVendor(Widget,XEvent *,String *, Cardinal *);
extern void ActionDoWindows(Widget,XEvent *,String *, Cardinal *);
extern void ActionEnableVendor(Widget,XEvent *,String *, Cardinal *);
extern void ActionExecuteAction(Widget,XEvent *,String *, Cardinal *);
extern void ActionExecuteFile(Widget,XEvent *,String *, Cardinal *);
extern void ActionGetLocation(Widget,XEvent *,String *, Cardinal *);
extern void ActionLoadVendor(Widget,XEvent *,String *, Cardinal *);
extern void ActionSizesLabel(Widget,XEvent *,String *, Cardinal *);
extern void ActionToggleVendor(Widget,XEvent *,String *, Cardinal *);
extern void ActionUnloadVendor(Widget,XEvent *,String *, Cardinal *);
extern void djopt_set_auto_only(Widget,XEvent *,String *, Cardinal *);

/* Definition of actionRec moved to global.h */
static actionRec ActionList[] = {
  {"About", AboutDialog},
  {"ActiveWhen", ActionActiveWhen},
  {"ApplyVendor", ActionApplyVendor},
  {"CheckWhen", ActionCheckWhen},
  {"CommandHistory", ActionCommandHistory},
  {"DisableVendor", ActionDisableVendor},
  {"DoWindows", ActionDoWindows},
  {"EnableVendor", ActionEnableVendor},
  {"ExecuteAction", ActionExecuteAction},
  {"ExecuteFile", ActionExecuteFile},
  {"GetXY", ActionGetLocation},
  {"LoadVendor", ActionLoadVendor},
  {"OptAutoOnly", djopt_set_auto_only},
  {"SizesLabel", ActionSizesLabel},
  {"ToggleVendor", ActionToggleVendor},
  {"UnloadVendor", ActionUnloadVendor},
  {0,0}
};

struct { char *name; int type; } ActionTypeList[] = {
  {"About", 0},
  {"ActiveWhen", 'p'},
  {"ApplyVendor", 0},
  {"CheckWhen", 'p'},
  {"CommandHistory", 0},
  {"DisableVendor", 0},
  {"DoWindows", 0},
  {"EnableVendor", 0},
  {"ExecuteAction", 0},
  {"ExecuteFile", 0},
  {"GetXY", 0},
  {"LoadVendor", 0},
  {"OptAutoOnly", 0},
  {"SizesLabel", 'p'},
  {"ToggleVendor", 0},
  {"UnloadVendor", 0},
  {0,0}
};

int ActionListSize = 16;

/*
 * Actions copied from src/main.c
 */

extern void ActionDJopt();	/* from djopt.c */

static actionRec Actions[] = {
  {"AutoPlaceSelected", ActionAutoPlaceSelected},
  {"AutoRoute", ActionAutoRoute},
  {"ButtonThree", ActionButton3},
  {"SetSame", ActionSetSame},
  {"MovePointer", ActionMovePointer},
  {"ToggleHideName", ActionToggleHideName},
  {"ChangeHole", ActionChangeHole},
  {"ToggleThermal", ActionToggleThermal},
  {"Atomic", ActionAtomic},
  {"AdjustStyle", ActionAdjustStyle},
  {"RouteStyle", ActionRouteStyle},
  {"DRC", ActionDRCheck},
  {"Flip", ActionFlip},
  {"SetValue", ActionSetValue},
  {"FinishInputDialog", ActionFinishInputDialog},
  {"Quit", ActionQuit},
  {"Connection", ActionConnection},
  {"Display", ActionDisplay},
  {"Report", ActionReport},
  {"ListAct", ActionListAct},
  {"Mode", ActionMode},
  {"RemoveSelected", ActionRemoveSelected},
  {"DeleteRats", ActionDeleteRats},
  {"AddRats", ActionAddRats},
  {"MarkCrosshair", ActionMarkCrosshair},
  {"ChangeSize", ActionChangeSize},
  {"ChangeClearSize", ActionChangeClearSize},
  {"ChangeDrillSize", ActionChange2ndSize},
  {"ChangeName", ActionChangeName},
  {"ChangeSquare", ActionChangeSquare},
  {"ChangeOctagon", ActionChangeOctagon},
  {"ChangeJoin", ActionChangeJoin},
  {"Select", ActionSelect},
  {"Unselect", ActionUnselect},
  {"Save", ActionSave},
  {"Load", ActionLoad},
  {"Print", ActionPrint},
  {"New", ActionNew},
  {"SwapSides", ActionSwapSides},
  {"Bell", ActionBell},
  {"PasteBuffer", ActionPasteBuffer},
  {"Undo", ActionUndo},
  {"Redo", ActionRedo},
  {"RipUp", ActionRipUp},
  {"Polygon", ActionPolygon},
  {"EditLayerGroups", ActionEditLayerGroups},
  {"MoveToCurrentLayer", ActionMoveToCurrentLayer},
  {"SwitchDrawingLayer", ActionSwitchDrawingLayer},
  {"ToggleVisibility", ActionToggleVisibility},
  {"MoveObject", ActionMoveObject},
  {"djopt", ActionDJopt},
  {"GetLoc", ActionGetLocation},
  {"SetFlag", ActionSetFlag},
  {"ClrFlag", ActionClrFlag},
  {"ChangeFlag", ActionChangeFlag}
};

/*
 * Action callback function
 */

int
TclExecutePcbAction(clientData, interp, argc, argv)
   ClientData clientData;
   Tcl_Interp *interp;
   int argc;
   CONST char *argv[];
{
   Cardinal locargc = (Cardinal)argc;
   Tk_Window tkwind = (Tk_Window)clientData;
   int i,j;

   Tcl_Eval(interp, "puts stdout { Executing command....}");

   for (i = 0; i < ENTRIES (Actions); i++)
      if (!strcasecmp(Actions[i].actionName, argv[0]))
          break;

   if (i == ENTRIES(Actions))
   {
        for (j = 0; j < ENTRIES (ActionList); j++)
          if (!strcasecmp(ActionList[j].actionName, argv[0]))
            break;

        if (j == ENTRIES(ActionList))
              return TCL_ERROR;

        (ActionList[j].actionFunc)((Window)Tk_WindowId(tkwind),
		(XEvent *)NULL, (String *)argv, &locargc);
   }
   else
        (Actions[i].actionFunc)((Window)Tk_WindowId(tkwind),
		(XEvent *)NULL, (String *)argv, &locargc);

   return TCL_OK;
}

/*
 * Register the actions and actionList with Tcl
 */

void
RegisterUserActions(interp, tkwind)
   Tcl_Interp *interp;
   Tk_Window tkwind;
{
   int i;
   char command_name[32], *cmdptr;

   Tcl_Eval(interp, "puts stdout {RegisterUserActions() routine called}");

   sprintf(command_name, "pcb::");
   cmdptr = command_name + 5;
   for (i = 0; i < ENTRIES(Functions); i++) {
      sprintf(cmdptr, Functions[i].Identifier);
      Tcl_CreateCommand(interp, cmdptr,
		(Tcl_CmdProc *)TclExecutePcbCommand,
		(ClientData)tkwind, (Tcl_CmdDeleteProc *)NULL);
   }

   /* More actions, defined in src/main.c */

   for (i = 0; i < ENTRIES(Actions); i++) {
      sprintf(cmdptr, Actions[i].actionName);
      Tcl_CreateCommand(interp, cmdptr,
		(Tcl_CmdProc *)TclExecutePcbAction,
		(ClientData)tkwind, (Tcl_CmdDeleteProc *)NULL);
   }

   /* Actionlist, defined in src/actionlist.c */

   for (i = 0; i < ENTRIES(ActionList); i++) {
      sprintf(cmdptr, ActionList[i].actionName);
      Tcl_CreateCommand(interp, cmdptr,
		(Tcl_CmdProc *)TclExecutePcbAction,
		(ClientData)tkwind, (Tcl_CmdDeleteProc *)NULL);
   }

}

#endif /* (TCL_PCB) */

/* ---------------------------------------------------------------------------
 * get function ID of passed string
 */
static int
GetFunctionID (String Ident)
{
  int i;

  i = ENTRIES (Functions);
  while (i)
    if (!strcasecmp (Ident, Functions[--i].Identifier))
      return ((int) Functions[i].ID);
  return (-1);
}

/* ---------------------------------------------------------------------------
 * set new coordinates if in 'RECTANGLE' mode
 * the cursor shape is also adjusted
 */
static void
AdjustAttachedBox (void)
{
  if (Settings.Mode == ARC_MODE)
    {
      Crosshair.AttachedBox.otherway = ShiftPressed ();
      return;
    }
  switch (Crosshair.AttachedBox.State)
    {
    case STATE_SECOND:		/* one corner is selected */
      {
	/* update coordinates */
	Crosshair.AttachedBox.Point2.X = Crosshair.X;
	Crosshair.AttachedBox.Point2.Y = Crosshair.Y;

	/* set pointer shape depending on location relative
	 * to first corner
	 */
	cornerCursor ();
	break;
      }

    default:
      modeCursor (RECTANGLE_MODE);
      break;
    }
}

/* ---------------------------------------------------------------------------
 * adjusts the objects which are to be created like attached lines...
 */
void
AdjustAttachedObjects (void)
{
  PointTypePtr pnt;
  switch (Settings.Mode)
    {
      /* update at least an attached block (selection) */
    case NO_MODE:
    case ARROW_MODE:
      if (Crosshair.AttachedBox.State)
	{
	  Crosshair.AttachedBox.Point2.X = Crosshair.X;
	  Crosshair.AttachedBox.Point2.Y = Crosshair.Y;
	}
      break;

      /* rectangle creation mode */
    case RECTANGLE_MODE:
    case ARC_MODE:
      AdjustAttachedBox ();
      break;

      /* polygon creation mode */
    case POLYGON_MODE:
      AdjustAttachedLine ();
      break;
      /* line creation mode */
    case LINE_MODE:
      if (PCB->RatDraw || PCB->Clipping == 0)
	AdjustAttachedLine ();
      else
	AdjustTwoLine (PCB->Clipping - 1);
      break;
      /* point insertion mode */
    case INSERTPOINT_MODE:
      pnt = AdjustInsertPoint ();
      if (pnt)
	InsertedPoint = *pnt;
      break;
    case ROTATE_MODE:
      modeCursor (ROTATE_MODE);
      break;
    }
}

/* ---------------------------------------------------------------------------
 * creates points of a line
 */
static void
NotifyLine (void)
{
  int type = NO_TYPE;
  void *ptr1, *ptr2, *ptr3;

  if (!Marked.status || TEST_FLAG (LOCALREFFLAG, PCB))
    SetLocalRef (Crosshair.X, Crosshair.Y, True);
  switch (Crosshair.AttachedLine.State)
    {
    case STATE_FIRST:		/* first point */
      if (PCB->RatDraw && SearchScreen (Crosshair.X, Crosshair.Y,
					PAD_TYPE | PIN_TYPE, &ptr1, &ptr1,
					&ptr1) == NO_TYPE)
	{
	  Beep (Settings.Volume);
	  break;
	}
      if (TEST_FLAG (AUTODRCFLAG, PCB) && Settings.Mode == LINE_MODE)
	{
	  type = SearchScreen (Crosshair.X, Crosshair.Y,
			       PIN_TYPE | PAD_TYPE | VIA_TYPE, &ptr1, &ptr2,
			       &ptr3);
	  LookupConnection (Crosshair.X, Crosshair.Y, True, TO_PCB (1));
	}
      if (type == PIN_TYPE || type == VIA_TYPE)
	{
	  Crosshair.AttachedLine.Point1.X =
	    Crosshair.AttachedLine.Point2.X = ((PinTypePtr) ptr2)->X;
	  Crosshair.AttachedLine.Point1.Y =
	    Crosshair.AttachedLine.Point2.Y = ((PinTypePtr) ptr2)->Y;
	}
      else if (type == PAD_TYPE)
	{
	  PadTypePtr pad = (PadTypePtr) ptr2;
	  float d1, d2;
	  d1 = SQUARE (Crosshair.X - pad->Point1.X) +
	    SQUARE (Crosshair.Y - pad->Point1.Y);
	  d2 = SQUARE (Crosshair.X - pad->Point2.X) +
	    SQUARE (Crosshair.Y - pad->Point2.Y);
	  if (d2 < d1)
	    {
	      Crosshair.AttachedLine.Point1 =
		Crosshair.AttachedLine.Point2 = pad->Point2;
	    }
	  else
	    {
	      Crosshair.AttachedLine.Point1 =
		Crosshair.AttachedLine.Point2 = pad->Point1;
	    }
	}
      else
	{
	  Crosshair.AttachedLine.Point1.X =
	    Crosshair.AttachedLine.Point2.X = Crosshair.X;
	  Crosshair.AttachedLine.Point1.Y =
	    Crosshair.AttachedLine.Point2.Y = Crosshair.Y;
	}
      Crosshair.AttachedLine.State = STATE_SECOND;
      break;

    case STATE_SECOND:
      /* fall through to third state too */
      lastLayer = CURRENT;
    default:			/* all following points */
      Crosshair.AttachedLine.State = STATE_THIRD;
      break;
    }
}

/* ---------------------------------------------------------------------------
 * create first or second corner of a marked block
 */
static void
NotifyBlock (void)
{
  HideCrosshair (True);
  switch (Crosshair.AttachedBox.State)
    {
    case STATE_FIRST:		/* setup first point */
      Crosshair.AttachedBox.Point1.X =
	Crosshair.AttachedBox.Point2.X = Crosshair.X;
      Crosshair.AttachedBox.Point1.Y =
	Crosshair.AttachedBox.Point2.Y = Crosshair.Y;
      Crosshair.AttachedBox.State = STATE_SECOND;
      break;

    case STATE_SECOND:		/* setup second point */
      Crosshair.AttachedBox.State = STATE_THIRD;
      break;
    }
  RestoreCrosshair (True);
}


/* ---------------------------------------------------------------------------
 *
 * does what's appropriate for the current mode setting. This normaly
 * means creation of an object at the current crosshair location.
 *
 * new created objects are added to the create undo list of course
 */
void
NotifyMode (void)
{
  void *ptr1, *ptr2, *ptr3;
  int type;

  if (Settings.RatWarn)
    ClearWarnings ();
  switch (Settings.Mode)
    {
    case ARROW_MODE:
      {
	int test;

	Note.Click = True;
	/* do something after click time */
	pcbAddTimeOut (Context, CLICK_TIME, CB_Click, NULL);
	/* see if we clicked on something already selected
	 * (Note.Moving) or clicked on a MOVE_TYPE
	 * (Note.Hit)
	 */
	for (test = (SELECT_TYPES | MOVE_TYPES) & ~RATLINE_TYPE;
	     test; test &= ~type)
	  {
	    type = SearchScreen (Note.X, Note.Y, test, &ptr1, &ptr2, &ptr3);
	    if (!Note.Hit && (type & MOVE_TYPES) &&
		!TEST_FLAG (LOCKFLAG, (PinTypePtr) ptr2))
	      {
		Note.Hit = type;
		Note.ptr1 = ptr1;
		Note.ptr2 = ptr2;
		Note.ptr3 = ptr3;
	      }
	    if (!Note.Moving && (type & SELECT_TYPES) &&
		TEST_FLAG (SELECTEDFLAG, (PinTypePtr) ptr2))
	      Note.Moving = True;
	    if ((Note.Hit && Note.Moving) || type == NO_TYPE)
	      break;
	  }
	break;
      }

    case VIA_MODE:
      {
	PinTypePtr via;

	if (!PCB->ViaOn)
	  {
	    Message ("You must turn via visibility on before\n"
		     "you can place vias\n");
	    break;
	  }
	if ((via = CreateNewVia (PCB->Data, Note.X, Note.Y,
				 Settings.ViaThickness, 2 * Settings.Keepaway,
				 0, Settings.ViaDrillingHole, NULL,
				 VIAFLAG)) != NULL)
	  {
	    UpdatePIPFlags (via, (ElementTypePtr) via, NULL, False);
	    AddObjectToCreateUndoList (VIA_TYPE, via, via, via);
	    IncrementUndoSerialNumber ();
	    DrawVia (via, 0);
	    Draw ();
	  }
	break;
      }

    case ARC_MODE:
      {
	switch (Crosshair.AttachedBox.State)
	  {
	  case STATE_FIRST:
	    Crosshair.AttachedBox.Point1.X =
	      Crosshair.AttachedBox.Point2.X = Note.X;
	    Crosshair.AttachedBox.Point1.Y =
	      Crosshair.AttachedBox.Point2.Y = Note.Y;
	    Crosshair.AttachedBox.State = STATE_SECOND;
	    break;

	  case STATE_SECOND:
	  case STATE_THIRD:
	    {
	      ArcTypePtr arc;
	      Location wx, wy;
	      int sa, dir;

	      wx = Note.X - Crosshair.AttachedBox.Point1.X;
	      wy = Note.Y - Crosshair.AttachedBox.Point1.Y;
	      if (XOR (Crosshair.AttachedBox.otherway, abs (wy) > abs (wx)))
		{
		  Crosshair.AttachedBox.Point2.X =
		    Crosshair.AttachedBox.Point1.X + abs (wy) * SGNZ (wx);
		  sa = (wx >= 0) ? 0 : 180;
#ifdef ARC45
		  if (abs (wy) / 2 >= abs (wx))
		    dir = (SGNZ (wx) == SGNZ (wy)) ? 45 : -45;
		  else
#endif
		    dir = (SGNZ (wx) == SGNZ (wy)) ? 90 : -90;
		}
	      else
		{
		  Crosshair.AttachedBox.Point2.Y =
		    Crosshair.AttachedBox.Point1.Y + abs (wx) * SGNZ (wy);
		  sa = (wy >= 0) ? -90 : 90;
#ifdef ARC45
		  if (abs (wx) / 2 >= abs (wy))
		    dir = (SGNZ (wx) == SGNZ (wy)) ? -45 : 45;
		  else
#endif
		    dir = (SGNZ (wx) == SGNZ (wy)) ? -90 : 90;
		  wy = wx;
		}
	      if (abs (wy) > 0 && (arc = CreateNewArcOnLayer (CURRENT,
							      Crosshair.
							      AttachedBox.
							      Point2.X,
							      Crosshair.
							      AttachedBox.
							      Point2.Y,
							      abs (wy), sa,
							      dir,
							      Settings.
							      LineThickness,
							      2 * Settings.
							      Keepaway,
							      TEST_FLAG
							      (CLEARNEWFLAG,
							       PCB) ?
							      CLEARLINEFLAG :
							      0)))
		{
		  BoxTypePtr bx;

		  bx = GetArcEnds (arc);
		  Crosshair.AttachedBox.Point1.X =
		    Crosshair.AttachedBox.Point2.X = bx->X2;
		  Crosshair.AttachedBox.Point1.Y =
		    Crosshair.AttachedBox.Point2.Y = bx->Y2;
		  AddObjectToCreateUndoList (ARC_TYPE, CURRENT, arc, arc);
		  IncrementUndoSerialNumber ();
		  addedLines++;
		  DrawArc (CURRENT, arc, 0);
		  Draw ();
		  Crosshair.AttachedBox.State = STATE_THIRD;
		}
	      break;
	    }
	  }
	break;
      }
    case LOCK_MODE:
      {
	type = SearchScreen (Note.X, Note.Y, LOCK_TYPES, &ptr1, &ptr2, &ptr3);
	if (type == ELEMENT_TYPE)
	  {
	    ElementTypePtr element = (ElementTypePtr) ptr2;

	    TOGGLE_FLAG (LOCKFLAG, element);
	    PIN_LOOP (element);
	    {
	      TOGGLE_FLAG (LOCKFLAG, pin);
	      CLEAR_FLAG (SELECTEDFLAG, pin);
	    }
	    END_LOOP;
	    PAD_LOOP (element);
	    {
	      TOGGLE_FLAG (LOCKFLAG, pad);
	      CLEAR_FLAG (SELECTEDFLAG, pad);
	    }
	    END_LOOP;
	    CLEAR_FLAG (SELECTEDFLAG, element);
	    /* always re-draw it since I'm too lazy
	     * to tell if a selected flag changed
	     */
	    DrawElement (element, 0);
	    Draw ();
	    /* Handled by callback commnad */
	    //ReportDialog ();
	  }
	else if (type != NO_TYPE)
	  {
	    TextTypePtr thing = (TextTypePtr) ptr3;
	    TOGGLE_FLAG (LOCKFLAG, thing);
	    if (TEST_FLAG (LOCKFLAG, thing)
		&& TEST_FLAG (SELECTEDFLAG, thing))
	      {
		/* this is not un-doable since LOCK isn't */
		CLEAR_FLAG (SELECTEDFLAG, thing);
		DrawObject (type, ptr1, ptr2, 0);
		Draw ();
	      }
	    /* Handled by callback commnad */
	    //ReportDialog ();
	  }
	break;
      }
    case THERMAL_MODE:
      {
	int LayerThermFlag;
	int LayerPIPFlag = L0PIPFLAG << INDEXOFCURRENT;

	if (((type
	      =
	      SearchScreen (Note.X, Note.Y, PIN_TYPES, &ptr1, &ptr2,
			    &ptr3)) != NO_TYPE)
	    && TEST_FLAG (LayerPIPFlag, (PinTypePtr) ptr3)
	    && !TEST_FLAG (HOLEFLAG, (PinTypePtr) ptr3))
	  {
	    AddObjectToFlagUndoList (type, ptr1, ptr2, ptr3);
	    LayerThermFlag = L0THERMFLAG << INDEXOFCURRENT;
	    TOGGLE_FLAG (LayerThermFlag, (PinTypePtr) ptr3);
	    IncrementUndoSerialNumber ();
	    ClearPin ((PinTypePtr) ptr3, type, 0);
	    Draw ();
	  }
	break;
      }

    case LINE_MODE:
      /* do update of position */
      NotifyLine ();
      if (Crosshair.AttachedLine.State != STATE_THIRD)
	break;

      /* Remove anchor if clicking on start point;
       * this means we can't paint 0 length lines
       * which could be used for square SMD pads.
       * Instead use a very small delta, or change
       * the file after saving.
       */
      if (Crosshair.X == Crosshair.AttachedLine.Point1.X
	  && Crosshair.Y == Crosshair.AttachedLine.Point1.Y)
	{
	  SetMode (LINE_MODE);
	  break;
	}

      if (PCB->RatDraw)
	{
	  RatTypePtr line;
	  if ((line = AddNet ()))
	    {
	      AddObjectToCreateUndoList (RATLINE_TYPE, line, line, line);
	      IncrementUndoSerialNumber ();
	      DrawRat (line, 0);
	      Crosshair.AttachedLine.Point1.X =
		Crosshair.AttachedLine.Point2.X;
	      Crosshair.AttachedLine.Point1.Y =
		Crosshair.AttachedLine.Point2.Y;
	      Draw ();
	    }
	  break;
	}
      else
	/* create line if both ends are determined && length != 0 */
	{
	  LineTypePtr line;

	  if (PCB->Clipping
	      && Crosshair.AttachedLine.Point1.X ==
	      Crosshair.AttachedLine.Point2.X
	      && Crosshair.AttachedLine.Point1.Y ==
	      Crosshair.AttachedLine.Point2.Y
	      && (Crosshair.AttachedLine.Point2.X != Note.X
		  || Crosshair.AttachedLine.Point2.Y != Note.Y))
	    {
	      /* We will paint only the second line segment.
	         Since we only check for vias on the first segment,
	         swap them so we only paint the first segment. */
	      Crosshair.AttachedLine.Point2.X = Note.X;
	      Crosshair.AttachedLine.Point2.Y = Note.Y;
	    }

	  if ((Crosshair.AttachedLine.Point1.X !=
	       Crosshair.AttachedLine.Point2.X
	       || Crosshair.AttachedLine.Point1.Y !=
	       Crosshair.AttachedLine.Point2.Y)
	      && (line =
		  CreateDrawnLineOnLayer (CURRENT,
					  Crosshair.AttachedLine.Point1.X,
					  Crosshair.AttachedLine.Point1.Y,
					  Crosshair.AttachedLine.Point2.X,
					  Crosshair.AttachedLine.Point2.Y,
					  Settings.LineThickness,
					  2 * Settings.Keepaway,
					  (TEST_FLAG (AUTODRCFLAG, PCB) ?
					   FOUNDFLAG : 0) |
					  (TEST_FLAG (CLEARNEWFLAG, PCB) ?
					   CLEARLINEFLAG : 0))) != NULL)
	    {
	      PinTypePtr via;

	      addedLines++;
	      AddObjectToCreateUndoList (LINE_TYPE, CURRENT, line, line);
	      DrawLine (CURRENT, line, 0);
	      /* place a via if vias are visible, the layer is
	         in a new group since the last line and there
	         isn't a pin already here */
	      if (PCB->ViaOn && GetLayerGroupNumberByPointer (CURRENT) !=
		  GetLayerGroupNumberByPointer (lastLayer) &&
		  SearchObjectByLocation (PIN_TYPES, &ptr1, &ptr2, &ptr3,
					  Crosshair.AttachedLine.Point1.X,
					  Crosshair.AttachedLine.Point1.Y,
					  Settings.ViaThickness / 2) ==
		  NO_TYPE
		  && (via =
		      CreateNewVia (PCB->Data,
				    Crosshair.AttachedLine.Point1.X,
				    Crosshair.AttachedLine.Point1.Y,
				    Settings.ViaThickness,
				    2 * Settings.Keepaway, 0,
				    Settings.ViaDrillingHole, NULL,
				    VIAFLAG)) != NULL)
		{
		  UpdatePIPFlags (via, (ElementTypePtr) via, NULL, False);
		  AddObjectToCreateUndoList (VIA_TYPE, via, via, via);
		  DrawVia (via, 0);
		}
	      /* copy the coordinates */
	      Crosshair.AttachedLine.Point1.X =
		Crosshair.AttachedLine.Point2.X;
	      Crosshair.AttachedLine.Point1.Y =
		Crosshair.AttachedLine.Point2.Y;
	      IncrementUndoSerialNumber ();
	      lastLayer = CURRENT;
	    }
	  if (PCB->Clipping && (Note.X != Crosshair.AttachedLine.Point2.X
				|| Note.Y !=
				Crosshair.AttachedLine.Point2.Y)
	      && (line =
		  CreateDrawnLineOnLayer (CURRENT,
					  Crosshair.AttachedLine.Point2.X,
					  Crosshair.AttachedLine.Point2.Y,
					  Note.X, Note.Y,
					  Settings.LineThickness,
					  2 * Settings.Keepaway,
					  (TEST_FLAG (AUTODRCFLAG, PCB) ?
					   FOUNDFLAG : 0) |
					  (TEST_FLAG (CLEARNEWFLAG, PCB) ?
					   CLEARLINEFLAG : 0))) != NULL)
	    {
	      addedLines++;
	      AddObjectToCreateUndoList (LINE_TYPE, CURRENT, line, line);
	      IncrementUndoSerialNumber ();
	      DrawLine (CURRENT, line, 0);
	      /* move to new start point */
	      Crosshair.AttachedLine.Point1.X = Note.X;
	      Crosshair.AttachedLine.Point1.Y = Note.Y;
	      Crosshair.AttachedLine.Point2.X = Note.X;
	      Crosshair.AttachedLine.Point2.Y = Note.Y;
	      if (TEST_FLAG (SWAPSTARTDIRFLAG, PCB))
		{
		  PCB->Clipping ^= 3;
		  SetStatusLine ();
		}
	    }
	  Draw ();
	}
      break;

    case RECTANGLE_MODE:
      /* do update of position */
      NotifyBlock ();

      /* create rectangle if both corners are determined 
       * and width, height are != 0
       */
      if (Crosshair.AttachedBox.State == STATE_THIRD &&
	  Crosshair.AttachedBox.Point1.X != Crosshair.AttachedBox.Point2.X &&
	  Crosshair.AttachedBox.Point1.Y != Crosshair.AttachedBox.Point2.Y)
	{
	  PolygonTypePtr polygon;

	  if ((polygon = CreateNewPolygonFromRectangle (CURRENT,
							Crosshair.
							AttachedBox.Point1.X,
							Crosshair.
							AttachedBox.Point1.Y,
							Crosshair.
							AttachedBox.Point2.X,
							Crosshair.
							AttachedBox.Point2.Y,
							CLEARPOLYFLAG)) !=
	      NULL)
	    {
	      AddObjectToCreateUndoList (POLYGON_TYPE, CURRENT,
					 polygon, polygon);
	      UpdatePIPFlags (NULL, NULL, CURRENT, True);
	      IncrementUndoSerialNumber ();
	      DrawPolygon (CURRENT, polygon, 0);
	      Draw ();
	    }

	  /* reset state to 'first corner' */
	  Crosshair.AttachedBox.State = STATE_FIRST;
	}
      break;

    case TEXT_MODE:
      {
	char *string;

	if ((string = GetUserInput ("Enter text:", "")) != NULL)
	  {
	    TextTypePtr text;
	    int flag = NOFLAG;

	    if (GetLayerGroupNumberByNumber (INDEXOFCURRENT) ==
		GetLayerGroupNumberByNumber (MAX_LAYER + SOLDER_LAYER))
	      flag = ONSOLDERFLAG;
	    if ((text = CreateNewText (CURRENT, &PCB->Font, Note.X,
				       Note.Y, 0, Settings.TextScale,
				       string, flag)) != NULL)
	      {
		AddObjectToCreateUndoList (TEXT_TYPE, CURRENT, text, text);
		IncrementUndoSerialNumber ();
		DrawText (CURRENT, text, 0);
		Draw ();
	      }

	    /* free memory allocated by GetUserInput() */
	    SaveFree (string);
	  }
	break;
      }

    case POLYGON_MODE:
      {
	PointTypePtr points = Crosshair.AttachedPolygon.Points;
	Cardinal n = Crosshair.AttachedPolygon.PointN;

	/* do update of position; use the 'LINE_MODE' mechanism */
	NotifyLine ();

	/* check if this is the last point of a polygon */
	if (n >= 3 &&
	    points->X == Crosshair.AttachedLine.Point2.X &&
	    points->Y == Crosshair.AttachedLine.Point2.Y)
	  {
	    CopyAttachedPolygonToLayer ();
	    Draw ();
	    break;
	  }

	/* create new point if it's the first one or if it's
	 * different to the last one
	 */
	if (!n ||
	    points[n - 1].X != Crosshair.AttachedLine.Point2.X ||
	    points[n - 1].Y != Crosshair.AttachedLine.Point2.Y)
	  {
	    CreateNewPointInPolygon (&Crosshair.AttachedPolygon,
				     Crosshair.AttachedLine.Point2.X,
				     Crosshair.AttachedLine.Point2.Y);

	    /* copy the coordinates */
	    Crosshair.AttachedLine.Point1.X = Crosshair.AttachedLine.Point2.X;
	    Crosshair.AttachedLine.Point1.Y = Crosshair.AttachedLine.Point2.Y;
	  }
	break;
      }

    case PASTEBUFFER_MODE:
      {
	watchCursor ();
	if (CopyPastebufferToLayout (Note.X, Note.Y))
	  SetChangedFlag (True);
	restoreCursor ();
	break;
      }

    case REMOVE_MODE:
      if ((type =
	   SearchScreen (Note.X, Note.Y, REMOVE_TYPES, &ptr1, &ptr2,
			 &ptr3)) != NO_TYPE)
	{
	  if (TEST_FLAG (LOCKFLAG, (LineTypePtr) ptr2))
	    {
	      Message ("Sorry, that object is locked\n");
	      break;
	    }
	  if (type == ELEMENT_TYPE)
	    {
	      RubberbandTypePtr ptr;
	      int i;

	      Crosshair.AttachedObject.RubberbandN = 0;
	      LookupRatLines (type, ptr1, ptr2, ptr3);
	      ptr = Crosshair.AttachedObject.Rubberband;
	      for (i = 0; i < Crosshair.AttachedObject.RubberbandN; i++)
		{
		  if (PCB->RatOn)
		    EraseRat ((RatTypePtr) ptr->Line);
		  MoveObjectToRemoveUndoList (RATLINE_TYPE,
					      ptr->Line, ptr->Line,
					      ptr->Line);
		  ptr++;
		}
	    }
	  RemoveObject (type, ptr1, ptr2, ptr3);
	  IncrementUndoSerialNumber ();
	  SetChangedFlag (True);
	}
      break;

    case ROTATE_MODE:
      RotateScreenObject (Note.X, Note.Y, ShiftPressed ()? (SWAP_IDENT ?
							    1 : 3)
			  : (SWAP_IDENT ? 3 : 1));
      break;

      /* both are almost the same */
    case COPY_MODE:
    case MOVE_MODE:
      switch (Crosshair.AttachedObject.State)
	{
	  /* first notify, lookup object */
	case STATE_FIRST:
	  {
	    int types = (Settings.Mode == COPY_MODE) ?
	      COPY_TYPES : MOVE_TYPES;

	    Crosshair.AttachedObject.Type =
	      SearchScreen (Note.X, Note.Y, types,
			    &Crosshair.AttachedObject.Ptr1,
			    &Crosshair.AttachedObject.Ptr2,
			    &Crosshair.AttachedObject.Ptr3);
	    if (Crosshair.AttachedObject.Type != NO_TYPE)
	      {
		if (Settings.Mode == MOVE_MODE &&
		    TEST_FLAG (LOCKFLAG, (PinTypePtr)
			       Crosshair.AttachedObject.Ptr2))
		  {
		    Message ("Sorry, the object is locked\n");
		    Crosshair.AttachedObject.Type = NO_TYPE;
		  }
		else
		  AttachForCopy (Note.X, Note.Y);
	      }
	    break;
	  }

	  /* second notify, move or copy object */
	case STATE_SECOND:
	  if (Settings.Mode == COPY_MODE)
	    CopyObject (Crosshair.AttachedObject.Type,
			Crosshair.AttachedObject.Ptr1,
			Crosshair.AttachedObject.Ptr2,
			Crosshair.AttachedObject.Ptr3,
			Note.X - Crosshair.AttachedObject.X,
			Note.Y - Crosshair.AttachedObject.Y);
	  else
	    {
	      MoveObjectAndRubberband (Crosshair.AttachedObject.Type,
				       Crosshair.AttachedObject.Ptr1,
				       Crosshair.AttachedObject.Ptr2,
				       Crosshair.AttachedObject.Ptr3,
				       Note.X - Crosshair.AttachedObject.X,
				       Note.Y - Crosshair.AttachedObject.Y);
	      SetLocalRef (0, 0, False);
	    }
	  SetChangedFlag (True);

	  /* reset identifiers */
	  Crosshair.AttachedObject.Type = NO_TYPE;
	  Crosshair.AttachedObject.State = STATE_FIRST;
	  break;
	}
      break;

      /* insert a point into a polygon/line/... */
    case INSERTPOINT_MODE:
      switch (Crosshair.AttachedObject.State)
	{
	  /* first notify, lookup object */
	case STATE_FIRST:
	  Crosshair.AttachedObject.Type =
	    SearchScreen (Note.X, Note.Y, INSERT_TYPES,
			  &Crosshair.AttachedObject.Ptr1,
			  &Crosshair.AttachedObject.Ptr2,
			  &Crosshair.AttachedObject.Ptr3);

	  if (Crosshair.AttachedObject.Type != NO_TYPE)
	    {
	      if (TEST_FLAG (LOCKFLAG, (PolygonTypePtr)
			     Crosshair.AttachedObject.Ptr2))
		{
		  Message ("Sorry, the object is locked\n");
		  Crosshair.AttachedObject.Type = NO_TYPE;
		  break;
		}
	      else
		{
		  /* get starting point of nearest segment */
		  if (Crosshair.AttachedObject.Type == POLYGON_TYPE)
		    {
		      fake.poly =
			(PolygonTypePtr) Crosshair.AttachedObject.Ptr2;
		      polyIndex =
			GetLowestDistancePolygonPoint (fake.poly, Note.X,
						       Note.Y);
		      fake.line.Point1 = fake.poly->Points[polyIndex];
		      fake.line.Point2 = (polyIndex) ?
			fake.poly->Points[polyIndex - 1]
			: fake.poly->Points[fake.poly->PointN - 1];
		      Crosshair.AttachedObject.Ptr2 = &fake.line;

		    }
		  Crosshair.AttachedObject.State = STATE_SECOND;
		  InsertedPoint = *AdjustInsertPoint ();
		}
	    }
	  break;

	  /* second notify, insert new point into object */
	case STATE_SECOND:
	  if (Crosshair.AttachedObject.Type == POLYGON_TYPE)
	    InsertPointIntoObject (POLYGON_TYPE,
				   Crosshair.AttachedObject.Ptr1, fake.poly,
				   &polyIndex,
				   InsertedPoint.X, InsertedPoint.Y, False);
	  else
	    InsertPointIntoObject (Crosshair.AttachedObject.Type,
				   Crosshair.AttachedObject.Ptr1,
				   Crosshair.AttachedObject.Ptr2,
				   &polyIndex,
				   InsertedPoint.X, InsertedPoint.Y, False);
	  SetChangedFlag (True);

	  /* reset identifiers */
	  Crosshair.AttachedObject.Type = NO_TYPE;
	  Crosshair.AttachedObject.State = STATE_FIRST;
	  break;
	}
      break;
    }
}

/* ---------------------------------------------------------------------------
 * warp pointer to new cursor location
 */
void
WarpPointer (Boolean ignore)
{
  /* don't warp with the auto drc - that creates auto-scroll chaos */
  if (TEST_FLAG (AUTODRCFLAG, PCB) && Settings.Mode == LINE_MODE
      && Crosshair.AttachedLine.State != STATE_FIRST)
    return;

  XWarpPointer (Dpy, Output.OutputWindow, Output.OutputWindow,
		0, 0, 0, 0,
		(int) (TO_SCREEN_X (Crosshair.X)),
		(int) (TO_SCREEN_Y (Crosshair.Y)));

  /* XWarpPointer creates Motion events normally bound to
   * EventMoveCrosshair.
   * We don't do any updates when EventMoveCrosshair
   * is called the next time to prevent from rounding errors
   */
  IgnoreMotionEvents = ignore;
}

void
CallActionProc (Widget w, String a, XEvent * e, String * s, Cardinal c)
{
  XtCallActionProc (w, a, e, s, c);
}


/* ---------------------------------------------------------------------------
 * action routine to save and restore the undo serial number
 * this allows making multiple-action bindings into an atomic operation
 * that will be undone by a single Undo command
 *
 * syntax: Atomic(Save|Restore|Close|Block)
 * Save saves the undo serial number
 * Restore returns it to the last saved number
 * Close sets it to 1 greater than the last save
 * Block increments it only if was actually incremented
 * 	since the last save
 */
void
ActionAtomic (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      switch (GetFunctionID (*Params))
	{
	case F_Save:
	  SaveUndoSerialNumber ();
	  break;
	case F_Restore:
	  RestoreUndoSerialNumber ();
	  break;
	case F_Close:
	  RestoreUndoSerialNumber ();
	  IncrementUndoSerialNumber ();
	  break;
	case F_Block:
	  RestoreUndoSerialNumber ();
	  if (Bumped)
	    IncrementUndoSerialNumber ();
	  break;
	}
    }
}

/* --------------------------------------------------------------------------
 * action routine to invoke the DRC check
 * needs more work
 * syntax: DRC();
 */
void
ActionDRCheck (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  Cardinal count;

  if (*Num == 0)
    {
      Message ("Rules are minspace %d.%02d, minoverlap %d.%d "
	       "minwidth %d.%02d\n"
	       "minsilk %d.%02d, min drill %d.%02d\n",
	       (Settings.Bloat + 1) / 100, (Settings.Bloat + 1) % 100,
	       Settings.Shrink / 100, Settings.Shrink % 100,
	       Settings.minWid / 100, Settings.minWid % 100,
	       Settings.minSlk / 100, Settings.minSlk % 100,
	       Settings.minDrill / 100, Settings.minDrill % 100);
      HideCrosshair (True);
      watchCursor ();
      count = DRCAll ();
      if (count == 0)
	Message ("No DRC problems found.\n");
      else
	Message ("found %d design rule errors\n", count);
      restoreCursor ();
      RestoreCrosshair (True);
      return;
    }

  Message ("Usage:  \nDRC()\n");

}

/* --------------------------------------------------------------------------
 * action routine to flip an element to the opposite side of the board 
 * syntax: Flip(SelectedElements|Object);
 */
void
ActionFlip (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  ElementTypePtr element;
  int err = 0;

  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Object:
	  if ((SearchScreen (Crosshair.X, Crosshair.Y, ELEMENT_TYPE,
			     (void **) &element,
			     (void **) &element,
			     (void **) &element)) != NO_TYPE)
	    {
	      ChangeElementSide (element, 2 * Crosshair.Y - PCB->MaxHeight);
	      UpdatePIPFlags (NULL, element, NULL, True);
	      IncrementUndoSerialNumber ();
	      Draw ();
	    }
	  break;
	case F_Selected:
	case F_SelectedElements:
	  ChangeSelectedElementSide ();
	  break;
	default:
	  err = 1;
	  break;
	}
      RestoreCrosshair (True);
      if (!err)
	return;
    }

  Message ("Usage:  \n" "Flip(Object|Selected|SelectedElements)\n");

}


/* --------------------------------------------------------------------------
 * action routine to toggle a thermal (on the current layer) to pins or vias
 * syntax: ToggleThermal(Object|SelectePins|SelectedVias|Selected);
 */
void
ActionToggleThermal (Widget W, XEvent * Event, String * Params,
		     Cardinal * Num)
{
  void *ptr1, *ptr2, *ptr3;
  int type;
  int err = 0;

  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Object:
	  if ((type =
	       SearchScreen (Crosshair.X, Crosshair.Y, CHANGETHERMAL_TYPES,
			     &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	    {
	      ChangeObjectThermal (type, ptr1, ptr2, ptr3);
	      IncrementUndoSerialNumber ();
	      Draw ();
	    }
	  break;
	case F_SelectedPins:
	  ChangeSelectedThermals (PIN_TYPE);
	  break;
	case F_SelectedVias:
	  ChangeSelectedThermals (VIA_TYPE);
	  break;
	case F_Selected:
	case F_SelectedElements:
	  ChangeSelectedThermals (CHANGETHERMAL_TYPES);
	  break;
	default:
	  err = 1;
	  break;
	}
      RestoreCrosshair (True);
      if (!err)
	return;
    }
  Message ("Usage:  \n"
	   "ToggleThermal(Object|Selected|SelectedElements|"
	   "SelectedPins|SelectedVias)\n");
}

/* --------------------------------------------------------------------------
 * action routine to set a thermal (on the current layer) to pins or vias
 * syntax: SetThermal(Object|SelectePins|SelectedVias|Selected);
 */
void
ActionSetThermal (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  void *ptr1, *ptr2, *ptr3;
  int type;
  int err = 0;

  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Object:
	  if ((type =
	       SearchScreen (Crosshair.X, Crosshair.Y, CHANGETHERMAL_TYPES,
			     &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	    {
	      SetObjectThermal (type, ptr1, ptr2, ptr3);
	      IncrementUndoSerialNumber ();
	      Draw ();
	    }
	  break;
	case F_SelectedPins:
	  SetSelectedThermals (PIN_TYPE);
	  break;
	case F_SelectedVias:
	  SetSelectedThermals (VIA_TYPE);
	  break;
	case F_Selected:
	case F_SelectedElements:
	  SetSelectedThermals (CHANGETHERMAL_TYPES);
	  break;
	default:
	  err = 1;
	  break;
	}
      RestoreCrosshair (True);
      if (!err)
	return;
    }
  Message ("Usage:  \n"
	   "SetThermal(Object|Selected|SelectedElements|"
	   "SelectedPins|SelectedVias)\n");
}

/* --------------------------------------------------------------------------
 * action routine to clear a thermal (on the current layer) to pins or vias
 * syntax: ClearThermal(Object|SelectePins|SelectedVias|Selected);
 */
void
ActionClearThermal (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  void *ptr1, *ptr2, *ptr3;
  int type;
  int err = 0;

  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Object:
	  if ((type =
	       SearchScreen (Crosshair.X, Crosshair.Y, CHANGETHERMAL_TYPES,
			     &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	    {
	      ClrObjectThermal (type, ptr1, ptr2, ptr3);
	      IncrementUndoSerialNumber ();
	      Draw ();
	    }
	  break;
	case F_SelectedPins:
	  ClrSelectedThermals (PIN_TYPE);
	  break;
	case F_SelectedVias:
	  ClrSelectedThermals (VIA_TYPE);
	  break;
	case F_Selected:
	case F_SelectedElements:
	  ClrSelectedThermals (CHANGETHERMAL_TYPES);
	  break;
	default:
	  err = 1;
	  break;
	}
      RestoreCrosshair (True);
      if (!err)
	return;
    }
  Message ("Usage:  \n"
	   "ClearThermal(Object|Selected|SelectedElements"
	   "|SelectedPins|SelectedVias)\n");
}


/* ---------------------------------------------------------------------------
 * action routine to move the X pointer relative to the current position
 * syntax: MovePointer(deltax,deltay)
 */
void
ActionMovePointer (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  Location x, y, dx, dy;

  if (*Num == 2)
    {
      /* save old crosshair position */
      x = Crosshair.X;
      y = Crosshair.Y;
      dx = (Location) (atoi (*Params) * PCB->Grid);
      dy = (Location) (atoi (*(Params + 1)) * PCB->Grid);
      MoveCrosshairRelative (TO_SCREEN_SIGN_X (dx), TO_SCREEN_SIGN_Y (dy));
      FitCrosshairIntoGrid (Crosshair.X, Crosshair.Y);
      /* adjust pointer before erasing anything */
      /* in case there is no hardware cursor */
      WarpPointer (True);
      /* restore crosshair for erasure */
      Crosshair.X = x;
      Crosshair.Y = y;
      HideCrosshair (False);
      MoveCrosshairRelative (TO_SCREEN_SIGN_X (dx), TO_SCREEN_SIGN_Y (dy));
      /* update object position and cursor location */
      AdjustAttachedObjects ();
      SetCursorStatusLine ();
      RestoreCrosshair (False);
      return;
    }

  Message ("Usage:  \n" "MovePointer(deltax, deltay)\n");
}

/* ---------------------------------------------------------------------------
 * !!! no action routine !!!
 *
 * event handler to set the cursor according to the X pointer position
 * called from inside main.c
 */
void
EventMoveCrosshair (XMotionEvent * Event)
{
#ifdef HAVE_LIBSTROKE
  if (mid_stroke)
    {
      StrokeBox.X2 = TO_PCB_X (Event->x);
      StrokeBox.Y2 = TO_PCB_Y (Event->y);
      stroke_record (Event->x, Event->y);
      return;
    }
#endif /* HAVE_LIBSTROKE */
  /* ignore events that are caused by ActionMovePointer */
  if (!IgnoreMotionEvents)
    {
      int childX, childY;

      /* only handle the event if the pointer is still at
       * the same position to prevent slow systems from
       * slow redrawing
       */
      GetXPointer (&childX, &childY);
      if (Event->x == childX && Event->y == childY)
	{
	  if (Settings.Mode == NO_MODE && Event->state & Button1Mask)
	    {
	      Location x, y;
	      HideCrosshair (False);
	      x = TO_SCREEN_X (Crosshair.X) - Event->x;
	      y = TO_SCREEN_Y (Crosshair.Y) - Event->y;
	      CenterDisplay (x, y, True);
	      RestoreCrosshair (False);
	    }
	  else if (MoveCrosshairAbsolute (TO_PCB_X (Event->x),
					  TO_PCB_Y (Event->y)))
	    {

	      /* update object position and cursor location */
	      AdjustAttachedObjects ();
	      SetCursorStatusLine ();
	      RestoreCrosshair (False);
	    }
	}
    }
  else
    IgnoreMotionEvents = False;
}

/* ---------------------------------------------------------------------------
 * action routine to change the grid, zoom and sizes
 * the first the type of object and the second is passed to
 * the specific routine
 * the value of the second determines also if it is absolute (without a sign)
 * or relative to the current one (with sign + or -)
 * syntax: SetValue(Grid|Zoom|LineSize|TextScale|ViaDrillingHole|ViaSize, value)
 */
void
ActionSetValue (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  Boolean r;			/* flag for 'relative' value */
  float value;
  int err = 0;

  if (*Num == 2 || *Num == 3)
    {
      HideCrosshair (True);
      value = GetValue (Params + 1, &r, *Num);
      switch (GetFunctionID (*Params))
	{
	case F_ViaDrillingHole:
	  SetViaDrillingHole (r ? value : value + Settings.ViaDrillingHole,
			      False);
	  break;

	case F_Grid:
	  if (!r)
	    {
	      if ((value == (int) value && PCB->Grid == (int) PCB->Grid)
		  || (value != (int) value && PCB->Grid != (int) PCB->Grid))
		SetGrid (value + PCB->Grid, False);
	      else
		Message ("Don't combine metric/English grids like that!\n");
	    }
	  else
	    SetGrid (value, False);
	  break;

	case F_Zoom:
	  SetZoom (r ? value : value + PCB->Zoom);
	  break;

	case F_LineSize:
	case F_Line:
	  SetLineSize (r ? value : value + Settings.LineThickness);
	  break;

	case F_Via:
	case F_ViaSize:
	  SetViaSize (r ? value : value + Settings.ViaThickness, False);
	  break;

	case F_Text:
	case F_TextScale:
	  value /= 45;
	  SetTextScale (r ? value : value + Settings.TextScale);
	  break;
	default:
	  err = 1;
	  break;
	}
      RestoreCrosshair (True);
      if (!err)
	return;
    }
  Message ("Usage:  \n"
	   "SetValue(Grid|Zoom|LineSize|TextScale|"
	   "ViaDrillingHole|ViaSize, value)\n"
	   "SetValue(Grid|Zoom|LineSize|TextScale|"
	   "ViaDrillingHole|ViaSize, value, mil|mm)\n");

}

/* ---------------------------------------------------------------------------
 * quits user input, the translations of the text widget handle it 
 * syntax: FinishInput(OK|Cancel)
 */
void
ActionFinishInputDialog (Widget W, XEvent * Event,
			 String * Params, Cardinal * Num)
{
  if (*Num == 1)
    FinishInputDialog (!strcmp ("OK", *Params));
  else
    Message ("Usage:  \n" "FinishInput(OK|Cancel)\n");

}


/* ---------------------------------------------------------------------------
 * extra notify for list widgets - gives alternate data to callback
 * syntax: ListAct()
 */
void
ActionListAct (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 0)
    XtCallCallbacks (W, XtNcallback, NULL);
  else if (*Num == 1)
    XtCallCallbacks (W, XtNcallback, Params);
}

/* ---------------------------------------------------------------------------
 * reports on an object 
 * syntax: Report(Object|DrillReport|FoundPins)
 */
void
ActionReport (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    switch (GetFunctionID (*Params))
      {
      case F_Object:
	{
	  ReportDialog ();
	  break;
	}
      case F_DrillReport:
	{
	  ReportDrills ();
	  break;
	}
      case F_FoundPins:
	{
	  ReportFoundPins ();
	  break;
	}
      }
  else
    Message ("Usage:  \n" "Report(Object|DrillReport|FoundPins)\n");

}

/* ---------------------------------------------------------------------------
 * quits application
 * syntax: Quit()
 */
void
ActionQuit (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 0)
    {
      if (!PCB->Changed || ConfirmDialog ("OK to lose data ?"))
	QuitApplication ();
    }
  else
    Message ("Usage:  \n" "Quit()\n");

}

/* ---------------------------------------------------------------------------
 * searches connections of the object at the cursor position
 * syntax: Connection(Find|ResetLinesAndPolygons|ResetPinsAndVias|Reset)
 */
void
ActionConnection (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Find:
	  {
	    watchCursor ();
	    LookupConnection (Crosshair.X, Crosshair.Y, True, TO_PCB (1));
	    restoreCursor ();
	    break;
	  }

	case F_ResetLinesAndPolygons:
	  ResetFoundLinesAndPolygons (True);
	  break;

	case F_ResetPinsViasAndPads:
	  ResetFoundPinsViasAndPads (True);
	  break;

	case F_Reset:
	  SaveUndoSerialNumber ();
	  ResetFoundPinsViasAndPads (True);
	  RestoreUndoSerialNumber ();
	  ResetFoundLinesAndPolygons (True);
	  break;
	}
      RestoreCrosshair (True);
      return;
    }
  Message ("Usage:  \n"
	   "Connection(Find|ResetLinesAndPolygons|ResetPinsAndVias|Reset)\n");
}

#ifndef TCL_PCB

/* ---------------------------------------------------------------------------
 * starts input of user commands
 * syntax: Command()
 */
void
ActionCommand (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  char *command;
  static char *previous = NULL;

  HideCrosshair (True);
  if (*Num == 0)
    {
      if (Settings.SaveLastCommand)
	command = GetUserInput ("Enter command:", previous ? previous : "");
      else
	command = GetUserInput ("Enter command:", "");
      if (command != NULL)
	{
	  /* copy new comand line to save buffer */
	  SaveFree (previous);
	  previous = MyStrdup (command, "ActionCommand()");
	  ExecuteUserCommand (command);
	  SaveFree (command);
	}
    }
  else if (previous)
    {
      command = MyStrdup (previous, "ActionCommand()");
      ExecuteUserCommand (command);
      SaveFree (command);
    }
  RestoreCrosshair (True);
}

void
warpNoWhere (void)
{
  static String MovePointerCommand[] = { "MovePointer", "0", "0" };

  CallActionProc (Output.Output, MovePointerCommand[0], NULL,
		  &MovePointerCommand[1], ENTRIES (MovePointerCommand) - 1);
}

#else

void
warpNoWhere (void)
{
   /* (undefined for Tcl version---needs to be added) */
}

#endif /* (!TCL_PCB) */

/* ---------------------------------------------------------------------------
 * several display related actions
 * syntax: Display(NameOnPCB|Description|Value)
 *         Display(Grid|Center|ClearAndRedraw|Redraw)
 *         Display(CycleClip|Toggle45Degree|ToggleStartDirection)
 *         Display(ToggleGrid|ToggleRubberBandMode|ToggleUniqueNames)
 *         Display(ToggleMask|ToggleName|ToggleClearLine|ToggleSnapPin)
 *         Display(ToggleThindraw|ToggleOrthoMove|ToggleLocalRef)
 *         Display(ToggleCheckPlanes|ToggleShowDRC|ToggleAutoDRC)
 *         Display(ToggleLiveRoute)
 *         Display(Pinout|PinOrPadName)
 *	   Display(Save|Restore)
 *	   Display(Scroll, Direction)
 */
void
ActionDisplay (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  int id;
  static int saved = 0;
  int err = 0;

  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (id = GetFunctionID (*Params))
	{

	case F_Save:
	  saved = PCB->Zoom;
	  break;
	case F_Restore:
	  SetZoom (saved);
	  break;
	  /* redraw layout with clearing the background */
	case F_ClearAndRedraw:
	  ClearAndRedrawOutput ();
	  UpdateAll ();
	  break;

	  /* redraw layout without clearing the background */
	case F_Redraw:
	  {
	    BoxType area;
	    area.X1 = 0;
	    area.Y1 = 0;
	    area.X2 = Output.Width;
	    area.Y2 = Output.Height;
	    RedrawOutput (&area);
	    break;
	  }
	  /* center cursor and move X pointer too */
	case F_Center:
	  CenterDisplay (Crosshair.X, Crosshair.Y, False);
	  warpNoWhere ();
	  break;

	  /* change the displayed name of elements */
	case F_Value:
	case F_NameOnPCB:
	case F_Description:
	  ELEMENT_LOOP (PCB->Data);
	  {
	    EraseElementName (element);
	  }
	  END_LOOP;
	  CLEAR_FLAG (DESCRIPTIONFLAG | NAMEONPCBFLAG, PCB);
	  switch (id)
	    {
	    case F_Value:
	      break;
	    case F_NameOnPCB:
	      SET_FLAG (NAMEONPCBFLAG, PCB);
	      break;
	    case F_Description:
	      SET_FLAG (DESCRIPTIONFLAG, PCB);
	      break;
	    }
	  ELEMENT_LOOP (PCB->Data);
	  {
	    DrawElementName (element, 0);
	  }
	  END_LOOP;
	  Draw ();
	  break;

	  /* toggle line-adjust flag */
	case F_ToggleAllDirections:
	  TOGGLE_FLAG (ALLDIRECTIONFLAG, PCB);
	  AdjustAttachedObjects ();
	  SetStatusLine ();
	  break;

	case F_CycleClip:
	  if TEST_FLAG
	    (ALLDIRECTIONFLAG, PCB)
	    {
	      TOGGLE_FLAG (ALLDIRECTIONFLAG, PCB);
	      PCB->Clipping = 0;
	    }
	  else
	    PCB->Clipping = (PCB->Clipping + 1) % 3;
	  AdjustAttachedObjects ();
	  SetStatusLine ();
	  break;

	case F_ToggleRubberBandMode:
	  TOGGLE_FLAG (RUBBERBANDFLAG, PCB);
	  SetStatusLine ();
	  break;

	case F_ToggleStartDirection:
	  TOGGLE_FLAG (SWAPSTARTDIRFLAG, PCB);
	  SetStatusLine ();
	  break;

	case F_ToggleUniqueNames:
	  TOGGLE_FLAG (UNIQUENAMEFLAG, PCB);
	  SetStatusLine ();
	  break;

	case F_ToggleSnapPin:
	  TOGGLE_FLAG (SNAPPINFLAG, PCB);
	  SetStatusLine ();
	  break;

	case F_ToggleLocalRef:
	  TOGGLE_FLAG (LOCALREFFLAG, PCB);
	  break;

	case F_ToggleThindraw:
	  TOGGLE_FLAG (THINDRAWFLAG, PCB);
	  ClearAndRedrawOutput ();
	  break;

	case F_ToggleShowDRC:
	  TOGGLE_FLAG (SHOWDRCFLAG, PCB);
	  break;

	case F_ToggleLiveRoute:
	  TOGGLE_FLAG (LIVEROUTEFLAG, PCB);
	  break;

	case F_ToggleAutoDRC:
	  TOGGLE_FLAG (AUTODRCFLAG, PCB);
	  if (TEST_FLAG (AUTODRCFLAG, PCB) && Settings.Mode == LINE_MODE)
	    {
	      SaveUndoSerialNumber ();
	      ResetFoundPinsViasAndPads (True);
	      RestoreUndoSerialNumber ();
	      ResetFoundLinesAndPolygons (True);
	      if (Crosshair.AttachedLine.State != STATE_FIRST)
		LookupConnection (Crosshair.AttachedLine.Point1.X,
				  Crosshair.AttachedLine.Point1.Y, True, 1);
	    }
	  break;

	case F_ToggleCheckPlanes:
	  TOGGLE_FLAG (CHECKPLANESFLAG, PCB);
	  ClearAndRedrawOutput ();
	  break;

	case F_ToggleOrthoMove:
	  TOGGLE_FLAG (ORTHOMOVEFLAG, PCB);
	  break;

	case F_ToggleName:
	  TOGGLE_FLAG (SHOWNUMBERFLAG, PCB);
	  UpdateAll ();
	  break;

	case F_ToggleMask:
	  TOGGLE_FLAG (SHOWMASKFLAG, PCB);
	  UpdateAll ();
	  break;

	case F_ToggleClearLine:
	  TOGGLE_FLAG (CLEARNEWFLAG, PCB);
	  SetStatusLine ();
	  break;

	  /* shift grid alignment */
	case F_ToggleGrid:
	  {
	    int childX, childY;
	    float oldGrid;

	    GetXPointer (&childX, &childY);
	    oldGrid = PCB->Grid;
	    PCB->Grid = 1.0;
	    if (MoveCrosshairAbsolute (TO_PCB_X (childX), TO_PCB_Y (childY)))
	      RestoreCrosshair (False);	/* was hidden by MoveCrosshairAbs */
	    SetGrid (oldGrid, True);
	    break;
	  }

	  /* toggle displaying of the grid */
	case F_Grid:
	  Settings.DrawGrid = !Settings.DrawGrid;
	  UpdateAll ();
	  break;

	  /* display the pinout of an element */
	case F_Pinout:
	  {
	    ElementTypePtr element;

	    if ((SearchScreen
		 (Crosshair.X, Crosshair.Y, ELEMENT_TYPE, (void **) &element,
		  (void **) &element, (void **) &element)) != NO_TYPE)
	      PinoutWindow (Output.Toplevel, element);
	    break;
	  }

	  /* toggle displaying of pin/pad/via names */
	case F_PinOrPadName:
	  {
	    void *ptr1, *ptr2, *ptr3;

	    switch (SearchScreen (Crosshair.X, Crosshair.Y,
				  ELEMENT_TYPE | PIN_TYPE | PAD_TYPE |
				  VIA_TYPE, (void **) &ptr1, (void **) &ptr2,
				  (void **) &ptr3))
	      {
	      case ELEMENT_TYPE:
		PIN_LOOP ((ElementTypePtr) ptr1);
		{
		  if (TEST_FLAG (DISPLAYNAMEFLAG, pin))
		    ErasePinName (pin);
		  else
		    DrawPinName (pin, 0);
		  AddObjectToFlagUndoList (PIN_TYPE, ptr1, pin, pin);
		  TOGGLE_FLAG (DISPLAYNAMEFLAG, pin);
		}
		END_LOOP;
		PAD_LOOP ((ElementTypePtr) ptr1);
		{
		  if (TEST_FLAG (DISPLAYNAMEFLAG, pad))
		    ErasePadName (pad);
		  else
		    DrawPadName (pad, 0);
		  AddObjectToFlagUndoList (PAD_TYPE, ptr1, pad, pad);
		  TOGGLE_FLAG (DISPLAYNAMEFLAG, pad);
		}
		END_LOOP;
		SetChangedFlag (True);
		IncrementUndoSerialNumber ();
		Draw ();
		break;

	      case PIN_TYPE:
		if (TEST_FLAG (DISPLAYNAMEFLAG, (PinTypePtr) ptr2))
		  ErasePinName ((PinTypePtr) ptr2);
		else
		  DrawPinName ((PinTypePtr) ptr2, 0);
		AddObjectToFlagUndoList (PIN_TYPE, ptr1, ptr2, ptr3);
		TOGGLE_FLAG (DISPLAYNAMEFLAG, (PinTypePtr) ptr2);
		SetChangedFlag (True);
		IncrementUndoSerialNumber ();
		Draw ();
		break;

	      case PAD_TYPE:
		if (TEST_FLAG (DISPLAYNAMEFLAG, (PadTypePtr) ptr2))
		  ErasePadName ((PadTypePtr) ptr2);
		else
		  DrawPadName ((PadTypePtr) ptr2, 0);
		AddObjectToFlagUndoList (PAD_TYPE, ptr1, ptr2, ptr3);
		TOGGLE_FLAG (DISPLAYNAMEFLAG, (PadTypePtr) ptr2);
		SetChangedFlag (True);
		IncrementUndoSerialNumber ();
		Draw ();
		break;
	      case VIA_TYPE:
		if (TEST_FLAG (DISPLAYNAMEFLAG, (PinTypePtr) ptr2))
		  EraseViaName ((PinTypePtr) ptr2);
		else
		  DrawViaName ((PinTypePtr) ptr2, 0);
		AddObjectToFlagUndoList (VIA_TYPE, ptr1, ptr2, ptr3);
		TOGGLE_FLAG (DISPLAYNAMEFLAG, (PinTypePtr) ptr2);
		SetChangedFlag (True);
		IncrementUndoSerialNumber ();
		Draw ();
		break;
	      }
	    break;
	  }
	default:
	  err = 1;
	}
      RestoreCrosshair (True);
    }
  else if (*Num == 2)
    {
      if (GetFunctionID (*Params) == F_Scroll)
	{
	  /* direction is like keypad, e.g. 4 = left 8 = up */
	  int direction = atoi (*(Params + 1));

	  switch (direction)
	    {
	    case 0:		/* special case: reposition crosshair */
	      {
		int x, y;
		GetXPointer (&x, &y);
		if (MoveCrosshairAbsolute (TO_PCB_X (x), TO_PCB_Y (y)))
		  {
		    AdjustAttachedObjects ();
		    SetCursorStatusLine ();
		    RestoreCrosshair (False);
		  }
	      }
	      break;
	    case 1:		/* down, left */
	      CenterDisplay (-Output.Width / 2, Output.Height / 2, True);
	      break;
	    case 2:		/* down */
	      CenterDisplay (0, Output.Height / 2, True);
	      break;
	    case 3:		/* down, right */
	      CenterDisplay (Output.Width / 2, Output.Height / 2, True);
	      break;
	    case 4:		/* left */
	      CenterDisplay (-Output.Width / 2, 0, True);
	      break;
	    case 6:		/* right */
	      CenterDisplay (Output.Width / 2, 0, True);
	      break;
	    case 7:		/* up, left */
	      CenterDisplay (-Output.Width / 2, -Output.Height / 2, True);
	      break;
	    case 8:		/* up */
	      CenterDisplay (0, -Output.Height / 2, True);
	      break;
	    case 9:		/* up, right */
	      CenterDisplay (Output.Width / 2, -Output.Height / 2, True);
	      break;
	    default:
	      Message ("Bad argument (%d) to Display(Scroll)\n", direction);
	      err = 1;
	    }
	}
      else
	err = 1;
    }

  if (!err)
    return;

  Message ("Usage\n"
	   "Display(NameOnPCB|Description|Value)\n"
	   "Display(Grid|Center|ClearAndRedraw|Redraw)\n"
	   "Display(CycleClip|Toggle45Degree|ToggleStartDirection)\n"
	   "Display(ToggleGrid|ToggleRubberBandMode|ToggleUniqueNames)\n"
	   "Display(ToggleMask|ToggleName|ToggleClearLine|ToggleSnapPin)\n"
	   "Display(ToggleThindraw|ToggleOrthoMove|ToggleLocalRef)\n"
	   "Display(ToggleCheckPlanes|ToggleShowDRC|ToggleAutoDRC)\n"
	   "Display(ToggleLiveRoute)\n"
	   "Display(Pinout|PinOrPadName)\n"
	   "Display(Save|Restore)\n" "Display(Scroll, Direction)\n");

}

/*
 * ---------------------------------------------------------------------------
 * Set the X, Y values in Note to the current Crosshair position
 * ---------------------------------------------------------------------------
 */

void
SetNoteXY()
{
   Note.X = Crosshair.X;
   Note.Y = Crosshair.Y;
}

/*
 * ---------------------------------------------------------------------------
 * Perform action for F_Stroke
 * ---------------------------------------------------------------------------
 */

void
DoStroke()
{

#ifdef HAVE_LIBSTROKE

   mid_stroke = True;
   StrokeBox.X1 = Crosshair.X;
   StrokeBox.Y1 = Crosshair.Y;

#else

   if (Settings.Mode == LINE_MODE && Crosshair.AttachedLine.State != STATE_FIRST)
   {
      SetMode (LINE_MODE);
   }
   else
   {
      SaveMode();
      saved_mode = True;
      SetMode (ARROW_MODE);
      NotifyMode();
   }

#endif
}

/* ---------------------------------------------------------------------------
 * action routine to
 *   set a new mode
 *   save the current one or restore the last saved mode
 *   call an appropriate action for the current mode
 * syntax: Mode(Copy|InsertPoint|Line|Move|None|PasteBuffer|Polygon)
 *         Mode(Remove|Rectangle|Text|Via|Arrow|Thermal)
 *         Mode(Notify|Release)
 *         Mode(Save|Restore)
 */

void
ActionMode (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      SetNoteXY();
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Arc:
	  SetMode (ARC_MODE);
	  break;
	case F_Arrow:
	  SetMode (ARROW_MODE);
	  break;
	case F_Copy:
	  SetMode (COPY_MODE);
	  break;
	case F_InsertPoint:
	  SetMode (INSERTPOINT_MODE);
	  break;
	case F_Line:
	  SetMode (LINE_MODE);
	  break;
	case F_Move:
	  SetMode (MOVE_MODE);
	  break;
	case F_None:
	  SetMode (NO_MODE);
	  break;
	case F_Notify:
	  NotifyMode ();
	  break;
	case F_PasteBuffer:
	  SetMode (PASTEBUFFER_MODE);
	  break;
	case F_Polygon:
	  SetMode (POLYGON_MODE);
	  break;
#ifndef HAVE_LIBSTROKE
	case F_Release:
	  ReleaseMode ();
	  break;
#else
	case F_Release:
	  if (mid_stroke)
	    FinishStroke ();
	  else
	    ReleaseMode ();
	  break;
#endif
	case F_Remove:
	  SetMode (REMOVE_MODE);
	  break;
	case F_Rectangle:
	  SetMode (RECTANGLE_MODE);
	  break;
	case F_Rotate:
	  SetMode (ROTATE_MODE);
	  break;
	case F_Stroke:
	  DoStroke();
	  break;
	case F_Text:
	  SetMode (TEXT_MODE);
	  break;
	case F_Thermal:
	  SetMode (THERMAL_MODE);
	  break;
	case F_Via:
	  SetMode (VIA_MODE);
	  break;

	case F_Restore:	/* restore the last saved mode */
	  RestoreMode ();
	  break;

	case F_Save:		/* save currently selected mode */
	  SaveMode ();
	  break;
	}
      RestoreCrosshair (True);
      return;
    }

  Message ("Usage\n"
	   "Mode(Copy|InsertPoint|Line|Move|None|PasteBuffer|Polygon)\n"
	   "Mode(Remove|Rectangle|Text|Via|Arrow|Thermal)\n"
	   "Mode(Notify|Release)\n" "Mode(Save|Restore)\n");

}

/* ---------------------------------------------------------------------------
 * action routine to remove objects
 * syntax: RemoveSelected()
 */
void
ActionRemoveSelected (Widget W, XEvent * Event,
		      String * Params, Cardinal * Num)
{
  if (*Num == 0)
    {
      HideCrosshair (True);
      if (RemoveSelected ())
	SetChangedFlag (True);
      RestoreCrosshair (True);
    }
  else
    {
      Message ("Usage\n"
	       "RemoveSelected()\n");
    }
}

/* ---------------------------------------------------------------------------
 * action routine to ripup auto-routed tracks (All|Selected)
 * or smash an element to pieces on the layout
 * syntax: RipUp(All|Selected|Element)
 */
void
ActionRipUp (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  Boolean changed = False;

  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_All:
	  ALLLINE_LOOP (PCB->Data);
	  {
	    if (TEST_FLAG (AUTOFLAG, line) && !TEST_FLAG (LOCKFLAG, line))
	      {
		RemoveObject (LINE_TYPE, layer, line, line);
		changed = True;
	      }
	  }
	  ENDALL_LOOP;
	  VIA_LOOP (PCB->Data);
	  {
	    if (TEST_FLAG (AUTOFLAG, via) && !TEST_FLAG (LOCKFLAG, via))
	      {
		RemoveObject (VIA_TYPE, via, via, via);
		changed = True;
	      }
	  }
	  END_LOOP;

	  if (changed)
	    {
	      IncrementUndoSerialNumber ();
	      SetChangedFlag (True);
	    }
	  break;
	case F_Selected:
	  VISIBLELINE_LOOP (PCB->Data);
	  {
	    if (TEST_FLAGS (AUTOFLAG | SELECTEDFLAG, line)
		&& !TEST_FLAG (LOCKFLAG, line))
	      {
		RemoveObject (LINE_TYPE, layer, line, line);
		changed = True;
	      }
	  }
	  ENDALL_LOOP;
	  if (PCB->ViaOn)
	    VIA_LOOP (PCB->Data);
	  {
	    if (TEST_FLAGS (AUTOFLAG | SELECTEDFLAG, via)
		&& !TEST_FLAG (LOCKFLAG, via))
	      {
		RemoveObject (VIA_TYPE, via, via, via);
		changed = True;
	      }
	  }
	  END_LOOP;
	  if (changed)
	    {
	      IncrementUndoSerialNumber ();
	      SetChangedFlag (True);
	    }
	  break;
	case F_Element:
	  {
	    void *ptr1, *ptr2, *ptr3;

	    if (SearchScreen (Crosshair.X, Crosshair.Y, ELEMENT_TYPE,
			      &ptr1, &ptr2, &ptr3) != NO_TYPE)
	      {
		Note.Buffer = Settings.BufferNumber;
		SetBufferNumber (MAX_BUFFER - 1);
		ClearBuffer (PASTEBUFFER);
		CopyObjectToBuffer (PASTEBUFFER->Data, PCB->Data,
				    ELEMENT_TYPE, ptr1, ptr2, ptr3);
		SmashBufferElement (PASTEBUFFER);
		PASTEBUFFER->X = 0;
		PASTEBUFFER->Y = 0;
		SaveUndoSerialNumber ();
		EraseObject (ELEMENT_TYPE, ptr1);
		MoveObjectToRemoveUndoList (ELEMENT_TYPE, ptr1, ptr2, ptr3);
		RestoreUndoSerialNumber ();
		CopyPastebufferToLayout (0, 0);
		SetBufferNumber (Note.Buffer);
		SetChangedFlag (True);
	      }
	  }
	  break;
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * action routine to add rat lines
 * syntax: AddRats(AllRats|SelectedRats|Close)
 * The Close argument selects the shortest unselect rat on the board
 */
void
ActionAddRats (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  RatTypePtr shorty;
  float len, small;

  if (*Num == 1)
    {
      HideCrosshair (True);
      watchCursor ();
      switch (GetFunctionID (*Params))
	{
	case F_AllRats:
	  if (AddAllRats (False, NULL))
	    SetChangedFlag (True);
	  break;
	case F_SelectedRats:
	case F_Selected:
	  if (AddAllRats (True, NULL))
	    SetChangedFlag (True);
	  break;
	case F_Close:
	  small = SQUARE (MAX_COORD);
	  shorty = NULL;
	  RAT_LOOP (PCB->Data);
	  {
	    if (TEST_FLAG (SELECTEDFLAG, line))
	      continue;
	    len = SQUARE (line->Point1.X - line->Point2.X) +
	      SQUARE (line->Point1.Y - line->Point2.Y);
	    if (len < small)
	      {
		small = len;
		shorty = line;
	      }
	  }
	  END_LOOP;
	  if (shorty)
	    {
	      AddObjectToFlagUndoList (RATLINE_TYPE, shorty, shorty, shorty);
	      SET_FLAG (SELECTEDFLAG, shorty);
	      DrawRat (shorty, 0);
	      Draw ();
	      CenterDisplay ((shorty->Point2.X + shorty->Point1.X) / 2,
			     (shorty->Point2.Y + shorty->Point1.Y) / 2,
			     False);
	    }
	  break;
	}
      restoreCursor ();
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * action routine to delete rat lines
 * syntax: DeleteRats(AllRats|SelectedRats)
 */
void
ActionDeleteRats (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_AllRats:
	  if (DeleteRats (False))
	    SetChangedFlag (True);
	  break;
	case F_SelectedRats:
	case F_Selected:
	  if (DeleteRats (True))
	    SetChangedFlag (True);
	  break;
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * action routine to auto-place selected components.
 * syntax: AutoPlaceSelected
 */
void
ActionAutoPlaceSelected (Widget W, XEvent * Event,
			 String * Params, Cardinal * Num)
{
  if (*Num == 0 &&
      ConfirmDialog ("Auto-placement can NOT be undone.\n"
		     "Do you want to continue anyway?\n"))
    {
      HideCrosshair (True);
      watchCursor ();
      if (AutoPlaceSelected ())
	SetChangedFlag (True);
      restoreCursor ();
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * action routine to auto-route rat lines.
 * syntax: AutoRoute(AllRats|SelectedRats)
 */
void
ActionAutoRoute (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)		/* one parameter */
    {
      HideCrosshair (True);
      watchCursor ();
      switch (GetFunctionID (*Params))
	{
	case F_AllRats:
	  if (AutoRoute (False))
	    SetChangedFlag (True);
	  break;
	case F_SelectedRats:
	case F_Selected:
	  if (AutoRoute (True))
	    SetChangedFlag (True);
	  break;
	}
      restoreCursor ();
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * Set/Reset the Crosshair mark
 * syntax: MarkCrosshair(|Center)
 */
void
ActionMarkCrosshair (Widget W, XEvent * Event, String * Params,
		     Cardinal * Num)
{
  if (*Num == 0)
    {
      if (Marked.status)
	{
	  DrawMark (True);
	  Marked.status = False;
	}
      else
	{
	  Marked.status = True;
	  Marked.X = Crosshair.X;
	  Marked.Y = Crosshair.Y;
	  DrawMark (False);
	}
      SetCursorStatusLine ();
    }
  else if (*Num == 1 && GetFunctionID (*Params) == F_Center)
    {
      DrawMark (True);
      Marked.status = True;
      Marked.X = Crosshair.X;
      Marked.Y = Crosshair.Y;
      DrawMark (False);
      SetCursorStatusLine ();
    }
}

/* ---------------------------------------------------------------------------
 * changes the size of objects
 * syntax: ChangeSize(Object, delta)
 *         ChangeSize(SelectedLines|SelectedPins|SelectedVias, delta)
 *         ChangeSize(SelectedPads|SelectedTexts|SelectedNames, delta)
 *	   ChangeSize(SelectedElements, delta)
 */
void
ActionChangeSize (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  Boolean r;			/* indicates if absolute size is given */
  float value;

  if (*Num == 2 || *Num == 3)
    {
      value = GetValue (Params + 1, &r, *Num);
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, CHANGESIZE_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      if (TEST_FLAG (LOCKFLAG, (PinTypePtr) ptr2))
		Message ("Sorry, that object is locked\n");
	    if (ChangeObjectSize (type, ptr1, ptr2, ptr3, value, r))
	      SetChangedFlag (True);
	    break;
	  }

	case F_SelectedVias:
	  if (ChangeSelectedSize (VIA_TYPE, value, r))
	    SetChangedFlag (True);
	  break;

	case F_SelectedPins:
	  if (ChangeSelectedSize (PIN_TYPE, value, r))
	    SetChangedFlag (True);
	  break;

	case F_SelectedPads:
	  if (ChangeSelectedSize (PAD_TYPE, value, r))
	    SetChangedFlag (True);
	  break;

	case F_SelectedLines:
	  if (ChangeSelectedSize (LINE_TYPE, value, r))
	    SetChangedFlag (True);
	  break;

	case F_SelectedTexts:
	  if (ChangeSelectedSize (TEXT_TYPE, value, r))
	    SetChangedFlag (True);
	  break;

	case F_SelectedNames:
	  if (ChangeSelectedSize (ELEMENTNAME_TYPE, value, r))
	    SetChangedFlag (True);
	  break;

	case F_SelectedElements:
	  if (ChangeSelectedSize (ELEMENT_TYPE, value, r))
	    SetChangedFlag (True);
	  break;

	case F_Selected:
	case F_SelectedObjects:
	  if (ChangeSelectedSize (CHANGESIZE_TYPES, value, r))
	    SetChangedFlag (True);
	  break;
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * changes the drilling hole size of objects
 * syntax: ChangeDrillSize(Object, delta)
 *         ChangeDrillSize(SelectedPins|SelectedVias|Selected|SelectedObjects, delta)
 */
void
ActionChange2ndSize (Widget W, XEvent * Event,
		     String * Params, Cardinal * Num)
{
  Boolean r;
  float value;

  if (*Num == 2 || *Num == 3)
    {
      value = GetValue (Params + 1, &r, *Num);
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, CHANGE2NDSIZE_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      if (ChangeObject2ndSize (type, ptr1, ptr2, ptr3, value, r, True))
		SetChangedFlag (True);
	    break;
	  }

	case F_SelectedVias:
	  if (ChangeSelected2ndSize (VIA_TYPE, value, r))
	    SetChangedFlag (True);
	  break;

	case F_SelectedPins:
	  if (ChangeSelected2ndSize (PIN_TYPE, value, r))
	    SetChangedFlag (True);
	  break;
	case F_Selected:
	case F_SelectedObjects:
	  if (ChangeSelected2ndSize (PIN_TYPES, value, r))
	    SetChangedFlag (True);
	  break;
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * changes the clearance size of objects
 * syntax: ChangeClearSize(Object, delta)
 *         ChangeClearSize(SelectedPins|SelectedPads|SelectedVias, delta)
 *	   ChangeClearSize(SelectedLines|SelectedArcs, delta
 *	   ChangeClearSize(Selected|SelectedObjects, delta)
 */
void
ActionChangeClearSize (Widget W, XEvent * Event,
		       String * Params, Cardinal * Num)
{
  Boolean r;
  float value;

  if (*Num == 2 || *Num == 3)
    {
      value = 2 * GetValue (Params + 1, &r, *Num);
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y,
			       CHANGECLEARSIZE_TYPES, &ptr1, &ptr2,
			       &ptr3)) != NO_TYPE)
	      if (ChangeObjectClearSize (type, ptr1, ptr2, ptr3, value, r))
		SetChangedFlag (True);
	    break;
	  }
	case F_SelectedVias:
	  if (ChangeSelectedClearSize (VIA_TYPE, value, r))
	    SetChangedFlag (True);
	  break;
	case F_SelectedPads:
	  if (ChangeSelectedClearSize (PAD_TYPE, value, r))
	    SetChangedFlag (True);
	  break;
	case F_SelectedPins:
	  if (ChangeSelectedClearSize (PIN_TYPE, value, r))
	    SetChangedFlag (True);
	  break;
	case F_SelectedLines:
	  if (ChangeSelectedClearSize (LINE_TYPE, value, r))
	    SetChangedFlag (True);
	  break;
	case F_SelectedArcs:
	  if (ChangeSelectedClearSize (ARC_TYPE, value, r))
	    SetChangedFlag (True);
	  break;
	case F_Selected:
	case F_SelectedObjects:
	  if (ChangeSelectedClearSize (CHANGECLEARSIZE_TYPES, value, r))
	    SetChangedFlag (True);
	  break;
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * sets the name of objects
 * syntax: ChangeName(Object)
 *         ChangeName(Layout|Layer)
 */
void
ActionChangeName (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  char *name;

  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	  /* change the name of an object */
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, CHANGENAME_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      {
		SaveUndoSerialNumber ();
		if (QueryInputAndChangeObjectName (type, ptr1, ptr2, ptr3))
		  {
		    SetChangedFlag (True);
		    if (type == ELEMENT_TYPE)
		      {
			RubberbandTypePtr ptr;
			int i;

			RestoreUndoSerialNumber ();
			Crosshair.AttachedObject.RubberbandN = 0;
			LookupRatLines (type, ptr1, ptr2, ptr3);
			ptr = Crosshair.AttachedObject.Rubberband;
			for (i = 0; i < Crosshair.AttachedObject.RubberbandN;
			     i++, ptr++)
			  {
			    if (PCB->RatOn)
			      EraseRat ((RatTypePtr) ptr->Line);
			    MoveObjectToRemoveUndoList (RATLINE_TYPE,
							ptr->Line, ptr->Line,
							ptr->Line);
			  }
			IncrementUndoSerialNumber ();
			Draw ();
		      }
		  }
	      }
	    break;
	  }

	  /* change the layouts name */
	case F_Layout:
	  name = GetUserInput ("enter the layouts name:", EMPTY (PCB->Name));
	  if (name && ChangeLayoutName (name))
	    SetChangedFlag (True);
	  break;

	  /* change the name of the activ layer */
	case F_Layer:
	  name = GetUserInput ("enter the layers name:",
			       EMPTY (CURRENT->Name));
	  if (name && ChangeLayerName (CURRENT, name))
	    SetChangedFlag (True);
	  break;
	}
      RestoreCrosshair (True);
    }
}


/* ---------------------------------------------------------------------------
 * toggles the visibility of element names 
 * syntax: ToggleHideName(Object|SelectedElements)
 */
void
ActionToggleHideName (Widget W, XEvent * Event, String * Params,
		      Cardinal * Num)
{
  if (*Num == 1 && PCB->ElementOn)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type = SearchScreen (Crosshair.X, Crosshair.Y, ELEMENT_TYPE,
				      &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      {
		AddObjectToFlagUndoList (type, ptr1, ptr2, ptr3);
		EraseElementName ((ElementTypePtr) ptr2);
		TOGGLE_FLAG (HIDENAMEFLAG, (ElementTypePtr) ptr2);
		DrawElementName ((ElementTypePtr) ptr2, 0);
		Draw ();
		IncrementUndoSerialNumber ();
	      }
	    break;
	  }
	case F_SelectedElements:
	case F_Selected:
	  {
	    Boolean changed = False;
	    ELEMENT_LOOP (PCB->Data);
	    {
	      if ((TEST_FLAG (SELECTEDFLAG, element) ||
		   TEST_FLAG (SELECTEDFLAG,
			      &NAMEONPCB_TEXT (element)))
		  && (FRONT (element) || PCB->InvisibleObjectsOn))
		{
		  AddObjectToFlagUndoList (ELEMENT_TYPE, element,
					   element, element);
		  EraseElementName (element);
		  TOGGLE_FLAG (HIDENAMEFLAG, element);
		  DrawElementName (element, 0);
		  changed = True;
		}
	    }
	    END_LOOP;
	    if (changed)
	      {
		Draw ();
		IncrementUndoSerialNumber ();
	      }
	  }
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * changes the join (clearance through polygons) of objects
 * syntax: ChangeJoin(ToggleObject|SelectedLines|SelectedArcs|Selected)
 */
void
ActionChangeJoin (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_ToggleObject:
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, CHANGEJOIN_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      if (ChangeObjectJoin (type, ptr1, ptr2, ptr3))
		SetChangedFlag (True);
	    break;
	  }

	case F_SelectedLines:
	  if (ChangeSelectedJoin (LINE_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_SelectedArcs:
	  if (ChangeSelectedJoin (ARC_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_Selected:
	case F_SelectedObjects:
	  if (ChangeSelectedJoin (CHANGEJOIN_TYPES))
	    SetChangedFlag (True);
	  break;
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * changes the square-flag of objects
 * syntax: ChangeSquare(ToggleObject|SelectedElements|SelectedPins)
 */
void
ActionChangeSquare (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      /* HideCrosshair (True); */
      switch (GetFunctionID (*Params))
	{
	case F_ToggleObject:
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, CHANGESQUARE_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      if (ChangeObjectSquare (type, ptr1, ptr2, ptr3))
		SetChangedFlag (True);
	    break;
	  }

	case F_SelectedElements:
	  if (ChangeSelectedSquare (ELEMENT_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_SelectedPins:
	  if (ChangeSelectedSquare (PIN_TYPE | PAD_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_Selected:
	case F_SelectedObjects:
	  if (ChangeSelectedSquare (PIN_TYPE | PAD_TYPE))
	    SetChangedFlag (True);
	  break;
	}
      /* RestoreCrosshair (True); */
    }
}

/* ---------------------------------------------------------------------------
 * sets the square-flag of objects
 * syntax: SetSquare(ToggleObject|SelectedElements|SelectedPins)
 */
void
ActionSetSquare (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      /* HideCrosshair (True); */
      switch (GetFunctionID (*Params))
	{
	case F_ToggleObject:
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, CHANGESQUARE_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      if (SetObjectSquare (type, ptr1, ptr2, ptr3))
		SetChangedFlag (True);
	    break;
	  }

	case F_SelectedElements:
	  if (SetSelectedSquare (ELEMENT_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_SelectedPins:
	  if (SetSelectedSquare (PIN_TYPE | PAD_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_Selected:
	case F_SelectedObjects:
	  if (SetSelectedSquare (PIN_TYPE | PAD_TYPE))
	    SetChangedFlag (True);
	  break;
	}
      /* RestoreCrosshair (True); */
    }
}

/* ---------------------------------------------------------------------------
 * clears the square-flag of objects
 * syntax: ClearSquare(ToggleObject|SelectedElements|SelectedPins)
 */
void
ActionClearSquare (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      /* HideCrosshair (True); */
      switch (GetFunctionID (*Params))
	{
	case F_ToggleObject:
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, CHANGESQUARE_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      if (ClrObjectSquare (type, ptr1, ptr2, ptr3))
		SetChangedFlag (True);
	    break;
	  }

	case F_SelectedElements:
	  if (ClrSelectedSquare (ELEMENT_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_SelectedPins:
	  if (ClrSelectedSquare (PIN_TYPE | PAD_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_Selected:
	case F_SelectedObjects:
	  if (ClrSelectedSquare (PIN_TYPE | PAD_TYPE))
	    SetChangedFlag (True);
	  break;
	}
      /* RestoreCrosshair (True); */
    }
}

/* ---------------------------------------------------------------------------
 * changes the octagon-flag of objects
 * syntax: ChangeOctagon(ToggleObject|SelectedElements|Selected)
 */
void
ActionChangeOctagon (Widget W, XEvent * Event, String * Params,
		     Cardinal * Num)
{
  if (*Num == 1)
    {
      /* HideCrosshair (True); */
      switch (GetFunctionID (*Params))
	{
	case F_ToggleObject:
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, CHANGEOCTAGON_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      if (ChangeObjectOctagon (type, ptr1, ptr2, ptr3))
		SetChangedFlag (True);
	    break;
	  }

	case F_SelectedElements:
	  if (ChangeSelectedOctagon (ELEMENT_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_SelectedPins:
	  if (ChangeSelectedOctagon (PIN_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_SelectedVias:
	  if (ChangeSelectedOctagon (VIA_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_Selected:
	case F_SelectedObjects:
	  if (ChangeSelectedOctagon (PIN_TYPES))
	    SetChangedFlag (True);
	  break;
	}
      /* RestoreCrosshair (True); */
    }
}

/* ---------------------------------------------------------------------------
 * sets the octagon-flag of objects
 * syntax: ChangeOctagon(ToggleObject|SelectedElements|Selected)
 */
void
ActionSetOctagon (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      /* HideCrosshair (True); */
      switch (GetFunctionID (*Params))
	{
	case F_ToggleObject:
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, CHANGEOCTAGON_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      if (SetObjectOctagon (type, ptr1, ptr2, ptr3))
		SetChangedFlag (True);
	    break;
	  }

	case F_SelectedElements:
	  if (SetSelectedOctagon (ELEMENT_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_SelectedPins:
	  if (SetSelectedOctagon (PIN_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_SelectedVias:
	  if (SetSelectedOctagon (VIA_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_Selected:
	case F_SelectedObjects:
	  if (SetSelectedOctagon (PIN_TYPES))
	    SetChangedFlag (True);
	  break;
	}
      /* RestoreCrosshair (True); */
    }
}

/* ---------------------------------------------------------------------------
 * clears the octagon-flag of objects
 * syntax: ClearOctagon(ToggleObject|SelectedElements|Selected)
 */
void
ActionClearOctagon (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      /* HideCrosshair (True); */
      switch (GetFunctionID (*Params))
	{
	case F_ToggleObject:
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, CHANGEOCTAGON_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      if (ClrObjectOctagon (type, ptr1, ptr2, ptr3))
		SetChangedFlag (True);
	    break;
	  }

	case F_SelectedElements:
	  if (ClrSelectedOctagon (ELEMENT_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_SelectedPins:
	  if (ClrSelectedOctagon (PIN_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_SelectedVias:
	  if (ClrSelectedOctagon (VIA_TYPE))
	    SetChangedFlag (True);
	  break;

	case F_Selected:
	case F_SelectedObjects:
	  if (ClrSelectedOctagon (PIN_TYPES))
	    SetChangedFlag (True);
	  break;
	}
      /* RestoreCrosshair (True); */
    }
}

/* ---------------------------------------------------------------------------
 * changes the hole-flag of objects
 * syntax: ChangeHole(ToggleObject|SelectedVias|Selected)
 */
void
ActionChangeHole (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      /* HideCrosshair (True); */
      switch (GetFunctionID (*Params))
	{
	case F_ToggleObject:
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type = SearchScreen (Crosshair.X, Crosshair.Y, VIA_TYPE,
				      &ptr1, &ptr2, &ptr3)) != NO_TYPE
		&& ChangeHole ((PinTypePtr) ptr3))
	      IncrementUndoSerialNumber ();
	    break;
	  }

	case F_SelectedVias:
	case F_Selected:
	  if (ChangeSelectedHole ())
	    SetChangedFlag (True);
	  break;
	}
      /* RestoreCrosshair (True); */
    }
}

/* ---------------------------------------------------------------------------
 * toggles the selection of the object at the pointer location
 * or sets it if 'All', 'Block' or 'Connection' is passed
 * syntax: Select(ToggleObject)
 *         Select(All|Block|Connection)
 *         Select(ElementByName|ObjectByName|PadByName|PinByName)
 *         Select(TextByName|ViaByName)
 *         Select(Convert)
 */
void
ActionSelect (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {

      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
#if defined(HAVE_REGCOMP) || defined(HAVE_RE_COMP)
	  int type;
	  /* select objects by their names */
	case F_ElementByName:
	  type = ELEMENT_TYPE;
	  goto commonByName;
	case F_ObjectByName:
	  type = ALL_TYPES;
	  goto commonByName;
	case F_PadByName:
	  type = PAD_TYPE;
	  goto commonByName;
	case F_PinByName:
	  type = PIN_TYPE;
	  goto commonByName;
	case F_TextByName:
	  type = TEXT_TYPE;
	  goto commonByName;
	case F_ViaByName:
	  type = VIA_TYPE;
	  goto commonByName;

	commonByName:
	  {
	    char *pattern;

	    if ((pattern = GetUserInput ("pattern:", "")) != NULL)
	      if (SelectObjectByName (type, pattern))
		SetChangedFlag (True);
	    break;
	  }
#endif /* defined(HAVE_REGCOMP) || defined(HAVE_RE_COMP) */

	  /* select a single object */
	case F_ToggleObject:
	case F_Object:
	  if (SelectObject ())
	    SetChangedFlag (True);
	  break;

	  /* all objects in block */
	case F_Block:
	  {
	    BoxType box;

	    box.X1 = MIN (Crosshair.AttachedBox.Point1.X,
			  Crosshair.AttachedBox.Point2.X);
	    box.Y1 = MIN (Crosshair.AttachedBox.Point1.Y,
			  Crosshair.AttachedBox.Point2.Y);
	    box.X2 = MAX (Crosshair.AttachedBox.Point1.X,
			  Crosshair.AttachedBox.Point2.X);
	    box.Y2 = MAX (Crosshair.AttachedBox.Point1.Y,
			  Crosshair.AttachedBox.Point2.Y);
	    NotifyBlock ();
	    if (Crosshair.AttachedBox.State == STATE_THIRD &&
		SelectBlock (&box, True))
	      {
		SetChangedFlag (True);
		Crosshair.AttachedBox.State = STATE_FIRST;
	      }
	    break;
	  }

	  /* select all visible objects */
	case F_All:
	  {
	    BoxType box;

	    box.X1 = -MAX_COORD;
	    box.Y1 = -MAX_COORD;
	    box.X2 = MAX_COORD;
	    box.Y2 = MAX_COORD;
	    if (SelectBlock (&box, True))
	      SetChangedFlag (True);
	    break;
	  }

	  /* all found connections */
	case F_Connection:
	  if (SelectConnection (True))
	    {
	      IncrementUndoSerialNumber ();
	      SetChangedFlag (True);
	    }
	  break;

	case F_Convert:
	  Note.Buffer = Settings.BufferNumber;
	  SetBufferNumber (MAX_BUFFER - 1);
	  ClearBuffer (PASTEBUFFER);
	  AddSelectedToBuffer (PASTEBUFFER, Crosshair.X, Crosshair.Y, True);
	  SaveUndoSerialNumber ();
	  RemoveSelected ();
	  ConvertBufferToElement (PASTEBUFFER);
	  RestoreUndoSerialNumber ();
	  CopyPastebufferToLayout (Crosshair.X, Crosshair.Y);
	  SetBufferNumber (Note.Buffer);
	  break;
	}
      RestoreCrosshair (True);
    }
}

/* FLAG(have_regex,FlagHaveRegex,0) */
int 
FlagHaveRegex (int parm)
{
#if defined(HAVE_REGCOMP) || defined(HAVE_RE_COMP)
  return 1;
#else
  return 0;
#endif
}

/* ---------------------------------------------------------------------------
 * unselects the object at the pointer location
 * syntax: Unselect(All|Block|Connection)
 */
void
ActionUnselect (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	  /* all objects in block */
	case F_Block:
	  {
	    BoxType box;

	    box.X1 = MIN (Crosshair.AttachedBox.Point1.X,
			  Crosshair.AttachedBox.Point2.X);
	    box.Y1 = MIN (Crosshair.AttachedBox.Point1.Y,
			  Crosshair.AttachedBox.Point2.Y);
	    box.X2 = MAX (Crosshair.AttachedBox.Point1.X,
			  Crosshair.AttachedBox.Point2.X);
	    box.Y2 = MAX (Crosshair.AttachedBox.Point1.Y,
			  Crosshair.AttachedBox.Point2.Y);
	    NotifyBlock ();
	    if (Crosshair.AttachedBox.State == STATE_THIRD &&
		SelectBlock (&box, False))
	      {
		SetChangedFlag (True);
		Crosshair.AttachedBox.State = STATE_FIRST;
	      }
	    break;
	  }

	  /* unselect all visible objects */
	case F_All:
	  {
	    BoxType box;

	    box.X1 = -MAX_COORD;
	    box.Y1 = -MAX_COORD;
	    box.X2 = MAX_COORD;
	    box.Y2 = MAX_COORD;
	    if (SelectBlock (&box, False))
	      SetChangedFlag (True);
	    break;
	  }

	  /* all found connections */
	case F_Connection:
	  if (SelectConnection (False))
	    {
	      IncrementUndoSerialNumber ();
	      SetChangedFlag (True);
	    }
	  break;
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * saves data to file
 * syntax: Save(Layout|LayoutAs)
 *         Save(AllConnections|AllUnusedPins|ElementConnections)
 */
void
ActionSave (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  char *name;

  if (*Num == 1)
    switch (GetFunctionID (*Params))
      {
	/* save layout; use its original file name
	 * or 'fall thru' to next routine
	 */
      case F_Layout:
	if (PCB->Filename)
	  {
	    SavePCB (PCB->Filename);
	    break;
	  }

	/* save data to any file */
      case F_LayoutAs:
	name = FileSelectBox ("save layout as:", PCB->Filename,
			      Settings.FilePath);
	if (name)
	  {
	    FILE *exist;

	    if ((exist = fopen (name, "r")))
	      {
		fclose (exist);
		if (ConfirmDialog ("File exists!  Ok to overwrite ?"))
		  SavePCB (name);
	      }
	    else
	      SavePCB (name);
	  }
	break;

	/* save all connections to file */
      case F_AllConnections:
	{
	  FILE *fp;

	  if ((fp = OpenConnectionDataFile ()) != NULL)
	    {
	      watchCursor ();
	      LookupConnectionsToAllElements (fp);
	      fclose (fp);
	      restoreCursor ();
	      SetChangedFlag (True);
	    }
	  break;
	}

	/* save all unused pins to file */
      case F_AllUnusedPins:
	{
	  FILE *fp;

	  if ((fp = OpenConnectionDataFile ()) != NULL)
	    {
	      watchCursor ();
	      LookupUnusedPins (fp);
	      fclose (fp);
	      restoreCursor ();
	      SetChangedFlag (True);
	    }
	  break;
	}

	/* save all connections to a file */
      case F_ElementConnections:
	{
	  ElementTypePtr element;
	  FILE *fp;

	  if ((SearchScreen (Crosshair.X, Crosshair.Y, ELEMENT_TYPE,
			     (void **) &element,
			     (void **) &element,
			     (void **) &element)) != NO_TYPE)
	    {
	      if ((fp = OpenConnectionDataFile ()) != NULL)
		{
		  watchCursor ();
		  LookupElementConnections (element, fp);
		  fclose (fp);
		  restoreCursor ();
		  SetChangedFlag (True);
		}
	    }
	  break;
	}
      }
}

/* ---------------------------------------------------------------------------
 * load data
 * syntax: Load(ElementToBuffer|Layout|LayoutToBuffer|Netlist)
 */
void
ActionLoad (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  char *name;

  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	  /* load element data into buffer */
	case F_ElementToBuffer:
	  name = FileSelectBox ("load element to buffer:",
				NULL, Settings.ElementPath);
	  if (name && LoadElementToBuffer (PASTEBUFFER, name, True))
	    SetMode (PASTEBUFFER_MODE);
	  break;

	  /* load PCB data into buffer */
	case F_LayoutToBuffer:
	  name = FileSelectBox ("load file to buffer:",
				NULL, Settings.FilePath);
	  if (name && LoadLayoutToBuffer (PASTEBUFFER, name))
	    SetMode (PASTEBUFFER_MODE);
	  break;

	  /* load new data */
	case F_Layout:
	  name = FileSelectBox ("load file:", NULL, Settings.FilePath);
	  if (name)
	    if (!PCB->Changed ||
		ConfirmDialog ("OK to override layout data?"))
	      LoadPCB (name);
	  break;
	case F_Netlist:
	  if ((name =
	       FileSelectBox ("load netlist file:", NULL, Settings.RatPath)));
	  {
	    if (PCB->Netlistname)
	      SaveFree (PCB->Netlistname);
	    PCB->Netlistname = StripWhiteSpaceAndDup (name);
	    FreeLibraryMemory (&PCB->NetlistLib);
	    if (!ReadNetlist (PCB->Netlistname))
	      InitNetlistWindow (Output.Toplevel);
	  }
	  break;
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * print data
 * syntax: Print()
 */
void
ActionPrint (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 0)
    {
      /* check if layout is empty */
      if (!IsDataEmpty (PCB->Data))
	PrintDialog ();
      else
	Message ("can't print empty layout");
    }
}

/* ---------------------------------------------------------------------------
 * starts a new layout
 * syntax: New()
 */
void
ActionNew (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  char *name;

  if (*Num == 0)
    {
      HideCrosshair (True);
      if (!PCB->Changed || ConfirmDialog ("OK to clear layout data?"))
	{
	  name = GetUserInput ("enter the layout's name:", "");
	  if (!name)
	    return;

	  /* do emergency saving
	   * clear the old struct and allocate memory for the new one
	   */
	  if (PCB->Changed && Settings.SaveInTMP)
	    SaveInTMP ();
	  RemovePCB (PCB);
	  PCB = CreateNewPCB (True);
	  InitNetlistWindow (Output.Toplevel);

	  /* setup the new name and reset some values to default */
	  PCB->Name = name;
	  ResetStackAndVisibility ();
	  CreateDefaultFont ();
	  SetCrosshairRange (0, 0, PCB->MaxWidth, PCB->MaxHeight);
	  UpdateSettingsOnScreen ();
	  ScaleOutput (Output.Width, Output.Height);
	  SetZoom (2);
	  CenterDisplay (PCB->MaxWidth / 2, PCB->MaxHeight / 2, False);
	  ClearAndRedrawOutput ();
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * swap visible sides
 */
void
ActionSwapSides (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  Location x, y;

  x = TO_SCREEN_X (Crosshair.X);
  y = TO_SCREEN_Y (Crosshair.Y);
  SwapBuffers ();
  Settings.ShowSolderSide = !Settings.ShowSolderSide;
  /* set silk colors as needed */
  PCB->Data->SILKLAYER.Color = PCB->ElementColor;
  PCB->Data->BACKSILKLAYER.Color = PCB->InvisibleObjectsColor;
  /* change the display */
  if (CoalignScreen (x, y, Crosshair.X, Crosshair.Y))
    warpNoWhere ();

  /* update object position and cursor location */
  AdjustAttachedObjects ();
  ClearAndRedrawOutput ();
  SetCursorStatusLine ();
  SetStatusLine ();
}

/* ---------------------------------------------------------------------------
 * no operation, just for testing purposes
 * syntax: Bell(volume)
 */
void
ActionBell (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  Beep (*Num == 1 ? atoi (*Params) : Settings.Volume);
}

/* ---------------------------------------------------------------------------
 * paste buffer operations
 * syntax: PasteBuffer(AddSelected|Clear|1..MAX_BUFFER)
 *         PasteBuffer(Rotate, 1..3)
 *         PasteBuffer(Convert|Save|Restore|Mirror)
 */
void
ActionPasteBuffer (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  char *name;

  HideCrosshair (True);
  if (*Num == 1)
    {
      switch (GetFunctionID (*Params))
	{
	  /* clear contents of paste buffer */
	case F_Clear:
	  ClearBuffer (PASTEBUFFER);
	  break;

	  /* copies objects to paste buffer */
	case F_AddSelected:
	  AddSelectedToBuffer (PASTEBUFFER, 0, 0, False);
	  break;

	  /* converts buffer contents into an element */
	case F_Convert:
	  ConvertBufferToElement (PASTEBUFFER);
	  break;

	  /* break up element for editing */
	case F_Restore:
	  SmashBufferElement (PASTEBUFFER);
	  break;

	  /* Mirror buffer */
	case F_Mirror:
	  MirrorBuffer (PASTEBUFFER);
	  break;

	case F_Save:
	  if (PASTEBUFFER->Data->ElementN == 0)
	    {
	      Message ("Buffer has no elements!\n");
	      break;
	    }
	  name = FileSelectBox ("save buffer elements as:",
				DESCRIPTION_NAME (PASTEBUFFER->Data->Element),
				Settings.LibraryTree);
	  if (name)
	    {
	      FILE *exist;

	      if ((exist = fopen (name, "r")))
		{
		  fclose (exist);
		  if (ConfirmDialog ("File exists!  Ok to overwrite ?"))
		    SaveBufferElements (name);
		}
	      else
		SaveBufferElements (name);
	    }
	  break;

	  /* set number */
	default:
	  {
	    int number = atoi (*Params);

	    /* correct number */
	    if (number)
	      SetBufferNumber (number - 1);
	  }
	}
    }
  if (*Num == 2)
    {
      switch (GetFunctionID (*Params))
	{
	case F_Rotate:
	  {
	    BYTE rotate = (BYTE) atoi (*(Params + 1));

	    RotateBuffer (PASTEBUFFER, rotate);
	    SetCrosshairRangeToBuffer ();
	    break;
	  }
	}
    }
  RestoreCrosshair (True);
}

/* ---------------------------------------------------------------------------
 * action routine to 'undo' operations
 * The serial number indicates the operation. This makes it possible
 * to group undo requests.
 * syntax: Undo(ClearList)
 *         Undo()
 */
void
ActionUndo (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (!Num || !*Num)
    {
      /* don't allow undo in the middle of an operation */
      if (Crosshair.AttachedObject.State != STATE_FIRST)
	return;
      if (Crosshair.AttachedBox.State != STATE_FIRST
	  && Settings.Mode != ARC_MODE)
	return;
      /* undo the last operation */

      HideCrosshair (True);
      if (Settings.Mode == POLYGON_MODE && Crosshair.AttachedPolygon.PointN)
	{
	  GoToPreviousPoint ();
	  RestoreCrosshair (True);
	  return;
	}
      /* move anchor point if undoing during line creation */
      if (Settings.Mode == LINE_MODE)
	{
	  if (Crosshair.AttachedLine.State == STATE_SECOND)
	    {
	      if (TEST_FLAG (AUTODRCFLAG, PCB))
		Undo (True);	/* undo the connection find */
	      Crosshair.AttachedLine.State = STATE_FIRST;
	      SetLocalRef (0, 0, False);
	      RestoreCrosshair (True);
	      return;
	    }
	  if (Crosshair.AttachedLine.State == STATE_THIRD)
	    {
	      int type;
	      void *ptr1, *ptr3;
	      LineTypePtr ptr2;
	      /* this search is guranteed to succeed */
	      SearchObjectByLocation (LINE_TYPE, &ptr1, (void **) &ptr2,
				      &ptr3, Crosshair.AttachedLine.Point1.X,
				      Crosshair.AttachedLine.Point1.Y, 0);
	      /* save both ends of line */
	      Crosshair.AttachedLine.Point2.X = ptr2->Point1.X;
	      Crosshair.AttachedLine.Point2.Y = ptr2->Point1.Y;
	      if ((type = Undo (True)))
		SetChangedFlag (True);
	      /* check that the undo was of the right type */
	      if ((type & UNDO_CREATE) == 0)
		{
		  /* wrong undo type, restore anchor points */
		  Crosshair.AttachedLine.Point2.X =
		    Crosshair.AttachedLine.Point1.X;
		  Crosshair.AttachedLine.Point2.Y =
		    Crosshair.AttachedLine.Point1.Y;
		  RestoreCrosshair (True);
		  return;
		}
	      /* move to new anchor */
	      Crosshair.AttachedLine.Point1.X =
		Crosshair.AttachedLine.Point2.X;
	      Crosshair.AttachedLine.Point1.Y =
		Crosshair.AttachedLine.Point2.Y;
	      /* check if an intermediate point was removed */
	      if (type & UNDO_REMOVE)
		{
		  /* this search should find the restored line */
		  SearchObjectByLocation (LINE_TYPE, &ptr1, (void **) &ptr2,
					  &ptr3,
					  Crosshair.AttachedLine.Point2.X,
					  Crosshair.AttachedLine.Point2.Y, 0);
		  Crosshair.AttachedLine.Point1.X =
		    Crosshair.AttachedLine.Point2.X = ptr2->Point2.X;
		  Crosshair.AttachedLine.Point1.Y =
		    Crosshair.AttachedLine.Point2.Y = ptr2->Point2.Y;
		}
	      FitCrosshairIntoGrid (Crosshair.X, Crosshair.Y);
	      AdjustAttachedObjects ();
	      if (--addedLines == 0)
		{
		  Crosshair.AttachedLine.State = STATE_SECOND;
		  lastLayer = CURRENT;
		}
	      else
		{
		  /* this search is guranteed to succeed too */
		  SearchObjectByLocation (LINE_TYPE, &ptr1, (void **) &ptr2,
					  &ptr3,
					  Crosshair.AttachedLine.Point1.X,
					  Crosshair.AttachedLine.Point1.Y, 0);
		  lastLayer = (LayerTypePtr) ptr1;
		}
	      RestoreCrosshair (True);
	      return;
	    }
	}
      if (Settings.Mode == ARC_MODE)
	{
	  if (Crosshair.AttachedBox.State == STATE_SECOND)
	    {
	      Crosshair.AttachedBox.State = STATE_FIRST;
	      RestoreCrosshair (True);
	      return;
	    }
	  if (Crosshair.AttachedBox.State == STATE_THIRD)
	    {
	      void *ptr1, *ptr2, *ptr3;
	      BoxTypePtr bx;
	      /* guranteed to succeed */
	      SearchObjectByLocation (ARC_TYPE, &ptr1, &ptr2, &ptr3,
				      Crosshair.AttachedBox.Point1.X,
				      Crosshair.AttachedBox.Point1.Y, 0);
	      bx = GetArcEnds ((ArcTypePtr) ptr2);
	      Crosshair.AttachedBox.Point1.X =
		Crosshair.AttachedBox.Point2.X = bx->X1;
	      Crosshair.AttachedBox.Point1.Y =
		Crosshair.AttachedBox.Point2.Y = bx->Y1;
	      AdjustAttachedObjects ();
	      if (--addedLines == 0)
		Crosshair.AttachedBox.State = STATE_SECOND;
	    }
	}
      /* undo the last destructive operation */
      if (Undo (True))
	SetChangedFlag (True);
    }
  else if (*Num == 1)
    {
      switch (GetFunctionID (*Params))
	{
	  /* clear 'undo objects' list */
	case F_ClearList:
	  ClearUndoList (False);
	  break;
	}
    }
  RestoreCrosshair (True);
}

/* ---------------------------------------------------------------------------
 * action routine to 'redo' operations
 * The serial number indicates the operation. This makes it possible
 * to group redo requests.
 * syntax: Redo()
 */
void
ActionRedo (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if ((Settings.Mode == POLYGON_MODE &&
       Crosshair.AttachedPolygon.PointN) ||
      Crosshair.AttachedLine.State == STATE_SECOND)
    return;
  HideCrosshair (True);
  if (!*Num && Redo (True))
    {
      SetChangedFlag (True);
      if (Settings.Mode == LINE_MODE &&
	  Crosshair.AttachedLine.State != STATE_FIRST)
	{
	  LineTypePtr line = &CURRENT->Line[CURRENT->LineN - 1];
	  Crosshair.AttachedLine.Point1.X =
	    Crosshair.AttachedLine.Point2.X = line->Point2.X;
	  Crosshair.AttachedLine.Point1.Y =
	    Crosshair.AttachedLine.Point2.Y = line->Point2.Y;
	  addedLines++;
	}
    }
  RestoreCrosshair (True);
}

/* ---------------------------------------------------------------------------
 * some polygon related stuff
 * syntax: Polygon(Close|PreviousPoint)
 */
void
ActionPolygon (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num == 1 && Settings.Mode == POLYGON_MODE)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	  /* close open polygon if possible */
	case F_Close:
	  ClosePolygon ();
	  break;

	  /* go back to the previous point */
	case F_PreviousPoint:
	  GoToPreviousPoint ();
	  break;
	}
      RestoreCrosshair (True);
    }
}

/* ---------------------------------------------------------------------------
 * copies a routing style into the current sizes
 * syntax: RouteStyle()
 */
void
ActionRouteStyle (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  int number;

  if (*Num == 1)
    {
      number = atoi (*Params);
      if (number > 0 && number <= NUM_STYLES)
	{
	  SetLineSize (PCB->RouteStyle[number - 1].Thick);
	  SetViaSize (PCB->RouteStyle[number - 1].Diameter, True);
	  SetViaDrillingHole (PCB->RouteStyle[number - 1].Hole, True);
	  SetKeepawayWidth (PCB->RouteStyle[number - 1].Keepaway);
	}
    }
}

/* ---------------------------------------------------------------------------
 * pops up a dialog to change certain sizes
 * syntax: AdjustStyle()
 */
void
ActionAdjustStyle (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  int number;

  if (*Num == 1)
    {
      number = atoi (*Params);
      if (number == 0)
	SizeDialog ();
      else if (number > 0 && number <= NUM_STYLES)
	StyleSizeDialog (number - 1);
    }
}

/* ---------------------------------------------------------------------------
 * Turn on or off the visibility of a layer
 */
void
ActionToggleVisibility (Widget W, XEvent * Event,
			String * Params, Cardinal * Num)
{
  int number;

  if (*Num == 1)
    {
      number = atoi (*Params) - 1;
      if (number >= 0 && number < MAX_LAYER + 2)
	{
	  if (PCB->Data->Layer[number].On == False)
	    ChangeGroupVisibility (number, True, False);
	  else if ((LayerStack[0] != number) &&
		   (GetLayerGroupNumberByNumber (number) !=
		    GetLayerGroupNumberByNumber (LayerStack[0])))
	    ChangeGroupVisibility (number, False, False);
	  UpdateControlPanel ();
	  ClearAndRedrawOutput ();
	}
    }
}

/* ---------------------------------------------------------------------------
 * changes the current drawing-layer
 * syntax: SwitchDrawingLayer()
 */
void
ActionSwitchDrawingLayer (Widget W, XEvent * Event,
			  String * Params, Cardinal * Num)
{
  int number;

  if (*Num == 1)
    {
      number = atoi (*Params) - 1;
      if (number >= 0 && number < MAX_LAYER + 2)
	{
	  ChangeGroupVisibility (number, True, True);
	  UpdateControlPanel ();
	  ClearAndRedrawOutput ();
	}
    }
}

/* ---------------------------------------------------------------------------
 * edit layer-groups
 * syntax: EditLayerGroups()
 */
void
ActionEditLayerGroups (Widget W, XEvent * Event,
		       String * Params, Cardinal * Num)
{
  if (*Num == 0 && LayerGroupDialog ())
    ClearAndRedrawOutput ();
}

/* ---------------------------------------------------------------------------
 * MoveObject
 * syntax: MoveObject(X,Y,[dim])
 */
void
ActionMoveObject (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  Location x, y;
  Boolean r1, r2;
  void *ptr1, *ptr2, *ptr3;
  int type;

  switch (*Num)
    {
    case 2:
      y = GetValue (Params + 1, &r1, *Num);
      x = GetValue (Params, &r2, *Num);
      break;
    case 3:
      y = GetValue (Params + 1, &r1, *Num);
      *(Params + 1) = *Params;
      x = GetValue (Params + 1, &r2, *Num);
      break;
    default:
      Message ("Illegal argument to MoveObject()\n");
      return;
    }
  type = SearchScreen (Crosshair.X, Crosshair.Y, MOVE_TYPES,
		       &ptr1, &ptr2, &ptr3);
  if (type == NO_TYPE)
    {
      Message ("Nothing found under crosshair\n");
      return;
    }
  if (r1)
    x -= Crosshair.X;
  if (r2)
    y -= Crosshair.Y;
  Crosshair.AttachedObject.RubberbandN = 0;
  if (TEST_FLAG (RUBBERBANDFLAG, PCB))
    LookupRubberbandLines (type, ptr1, ptr2, ptr3);
  if (type == ELEMENT_TYPE)
    LookupRatLines (type, ptr1, ptr2, ptr3);
  MoveObjectAndRubberband (type, ptr1, ptr2, ptr3, x, y);
  SetChangedFlag (True);
}

/* ---------------------------------------------------------------------------
 * moves objects to the current layer
 * syntax: MoveToCurrentLayer(Object|SelectedObjects)
 */
void
ActionMoveToCurrentLayer (Widget W, XEvent * Event,
			  String * Params, Cardinal * Num)
{
  if (*Num == 1)
    {
      HideCrosshair (True);
      switch (GetFunctionID (*Params))
	{
	case F_Object:
	  {
	    int type;
	    void *ptr1, *ptr2, *ptr3;

	    if ((type =
		 SearchScreen (Crosshair.X, Crosshair.Y, MOVETOLAYER_TYPES,
			       &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	      if (MoveObjectToLayer (type, ptr1, ptr2, ptr3, CURRENT, False))
		SetChangedFlag (True);
	    break;
	  }

	case F_SelectedObjects:
	case F_Selected:
	  if (MoveSelectedObjectsToLayer (CURRENT))
	    SetChangedFlag (True);
	  break;
	}
      RestoreCrosshair (True);
    }
}

void
ActionButton3 (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  /* We can take context sensitive actions here */

  static String PopCommand[] =
    { "XawPositionSimpleMenu", "XtMenuPopup", "pmenu", "p2menu" };
#ifdef TRANS_BTN3MENU
  XtTranslations force_translations;
  static String sticky =
    " <Motion>: highlight()\n <BtnDown>,<BtnUp>: notify()"
    " unhighlight()\n <BtnUp>:\n";
#endif
  XKeyEvent event;

  if (Settings.Mode == LINE_MODE &&
      Crosshair.AttachedLine.State != STATE_FIRST)
    {
      SetMode (LINE_MODE);
      NotifyMode ();
    }
  else
    {
      event = *(XKeyEvent *) Event;
      event.type = KeyPress;
      CallActionProc (Output.Output, PopCommand[0], Event, &PopCommand[2], 1);
#ifdef TRANS_BTN3MENU
      force_translations = XtParseTranslationTable (sticky);
      XtOverrideTranslations (ThePopMenu, force_translations);
#endif
      CallActionProc (Output.Output, PopCommand[1], (XEvent *) & event,
		      &PopCommand[2], 1);
    }
}
void
ActionSetSame (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  void *ptr1, *ptr2, *ptr3;
  int type;
  LayerTypePtr layer = CURRENT;

  type =
    SearchScreen (Crosshair.X, Crosshair.Y, CLONE_TYPES, &ptr1, &ptr2, &ptr3);
/* set layer current and size from line or arc */
  switch (type)
    {
    case LINE_TYPE:
      HideCrosshair (True);
      Settings.LineThickness = ((LineTypePtr) ptr2)->Thickness;
      Settings.Keepaway = ((LineTypePtr) ptr2)->Clearance / 2;
      layer = (LayerTypePtr) ptr1;
      if (Settings.Mode != LINE_MODE)
	SetMode (LINE_MODE);
      RestoreCrosshair (True);
      break;
    case ARC_TYPE:
      HideCrosshair (True);
      Settings.LineThickness = ((ArcTypePtr) ptr2)->Thickness;
      Settings.Keepaway = ((ArcTypePtr) ptr2)->Clearance / 2;
      layer = (LayerTypePtr) ptr1;
      if (Settings.Mode != ARC_MODE)
	SetMode (ARC_MODE);
      RestoreCrosshair (True);
      break;
    case POLYGON_TYPE:
      layer = (LayerTypePtr) ptr1;
      break;
    case VIA_TYPE:
      HideCrosshair (True);
      Settings.ViaThickness = ((PinTypePtr) ptr2)->Thickness;
      Settings.ViaDrillingHole = ((PinTypePtr) ptr2)->DrillingHole;
      Settings.Keepaway = ((PinTypePtr) ptr2)->Clearance / 2;
      if (Settings.Mode != VIA_MODE)
	SetMode (VIA_MODE);
      RestoreCrosshair (True);
      break;
    default:
      return;
    }
  if (layer != CURRENT)
    {
      ChangeGroupVisibility (GetLayerNumber (PCB->Data, layer), True, True);
      UpdateControlPanel ();
      ClearAndRedrawOutput ();
    }
  SetStatusLine ();
}


/* ---------------------------------------------------------------------------
 * sets flags on object
 * syntax: SetFlag(Object, flag)
 *         SetFlag(SelectedLines|SelectedPins|SelectedVias, flag)
 *         SetFlag(SelectedPads|SelectedTexts|SelectedNames, flag)
 *	   SetFlag(SelectedElements, flag)
 * examples:
 * :SetFlag(SelectedVias,thermal) 
 * :SetFlag(SelectedElements,square)
 */

void
ActionSetFlag (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  if (*Num != 2)
    {
      Message ("Usage:  \n"
	       "SetFlag(Object, flag)\n"
	       "SetFlag(SelectedLines|SelectedPins|SelectedVias, flag)\n"
	       "SetFlag(SelectedPads|SelectedTexts|SelectedNames, flag)\n"
	       "SetFlag(SelectedElements, flag)\n"
	       "flag = square | octagon | thermal\n");
      return;
    }
  ChangeFlag (W, Event, Params, 1, "SetFlag");
}

/* ---------------------------------------------------------------------------
 * clears flags on object
 * syntax: ClrFlag(Object, flag)
 *         ClrFlag(SelectedLines|SelectedPins|SelectedVias, flag)
 *         ClrFlag(SelectedPads|SelectedTexts|SelectedNames, flag)
 *	   ClrFlag(SelectedElements, flag)
 * examples:
 * :ClrFlag(SelectedVias,thermal) 
 * :ClrFlag(SelectedElements,square)
 */

void
ActionClrFlag (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{

  if (*Num != 2)
    {
      Message ("Usage:  \n"
	       "ClrFlag(Object, flag)\n"
	       "ClrFlag(SelectedLines|SelectedPins|SelectedVias, flag)\n"
	       "ClrFlag(SelectedPads|SelectedTexts|SelectedNames, flag)\n"
	       "ClrFlag(SelectedElements, flag)\n"
	       "flag = square | octagon | thermal\n");
      return;
    }

  ChangeFlag (W, Event, Params, 0, "ClrFlag");
}

/* ---------------------------------------------------------------------------
 * sets or clears flags on object
 * syntax: ChangeFlag(Object, flag, value)
 *         ChangeFlag(SelectedLines|SelectedPins|SelectedVias, flag, value)
 *         ChangeFlag(SelectedPads|SelectedTexts|SelectedNames, flag, value)
 *	   ChangeFlag(SelectedElements, flag, value)
 *
 * examples:
 *
 * :ChangeFlag(SelectedVias,thermal,0) 
 * :ChangeFlag(SelectedElements,square,1)
 */
void
ActionChangeFlag (Widget W, XEvent * Event, String * Params, Cardinal * Num)
{
  int val;

  if (*Num != 3)
    {
      Message ("Usage:  \n"
	       "ChangeFlag(Object, flag, value)\n"
	       "ChangeFlag(SelectedLines|SelectedPins|SelectedVias, flag, value)\n"
	       "ChangeFlag(SelectedPads|SelectedTexts|SelectedNames, flag, value)\n"
	       "ChangeFlag(SelectedElements, flag, value)\n"
	       "flag = square | octagon | thermal\n");

      return;
    }

  if (strcmp (Params[2], "0") == 0)
    val = 0;
  else if (strcmp (Params[2], "1") == 0)
    val = 1;
  else
    {
      Message ("ChangeFlag():  Value \"%s\" is not valid\n", Params[2]);
      return;
    }

  ChangeFlag (W, Event, Params, val, "ChangeFlag");
}


static void
ChangeFlag (Widget W, XEvent * Event,
	    String * Params, int value, char *cmd_name)
{
  int flag;
  char *what;
  char *flag_name;
  Boolean (*set_object) (int, void *, void *, void *);
  Boolean (*set_selected) (int);

  what = Params[0];
  flag_name = Params[1];

  if (strcmp (flag_name, "square") == 0)
    {
      flag = SQUAREFLAG;
      set_object = value ? SetObjectSquare : ClrObjectSquare;
      set_selected = value ? SetSelectedSquare : ClrSelectedSquare;
    }
  else if (strcmp (flag_name, "octagon") == 0)
    {
      flag = OCTAGONFLAG;
      set_object = value ? SetObjectOctagon : ClrObjectOctagon;
      set_selected = value ? SetSelectedOctagon : ClrSelectedOctagon;
    }
  else if (strcmp (flag_name, "thermal") == 0)
    {
      flag = L0THERMFLAG;
      set_object = value ? SetObjectThermal : ClrObjectThermal;
      set_selected = value ? SetSelectedThermals : ClrSelectedThermals;
    }
  else
    {
      Message ("%s():  Flag \"%s\" is not valid\n", cmd_name, flag_name);
      return;
    }

  HideCrosshair (True);
  switch (GetFunctionID (*Params))
    {
    case F_Object:
      {
	int type;
	void *ptr1, *ptr2, *ptr3;

	if ((type =
	     SearchScreen (Crosshair.X, Crosshair.Y, CHANGESIZE_TYPES,
			   &ptr1, &ptr2, &ptr3)) != NO_TYPE)
	  if (TEST_FLAG (LOCKFLAG, (PinTypePtr) ptr2))
	    Message ("Sorry, that object is locked\n");
	if (set_object (type, ptr1, ptr2, ptr3))
	  SetChangedFlag (True);
	break;
      }

    case F_SelectedVias:
      if (set_selected (VIA_TYPE))
	SetChangedFlag (True);
      break;

    case F_SelectedPins:
      if (set_selected (PIN_TYPE))
	SetChangedFlag (True);
      break;

    case F_SelectedPads:
      if (set_selected (PAD_TYPE))
	SetChangedFlag (True);
      break;

    case F_SelectedLines:
      if (set_selected (LINE_TYPE))
	SetChangedFlag (True);
      break;

    case F_SelectedTexts:
      if (set_selected (TEXT_TYPE))
	SetChangedFlag (True);
      break;

    case F_SelectedNames:
      if (set_selected (ELEMENTNAME_TYPE))
	SetChangedFlag (True);
      break;

    case F_SelectedElements:
      if (set_selected (ELEMENT_TYPE))
	SetChangedFlag (True);
      break;

    case F_Selected:
    case F_SelectedObjects:
      if (set_selected (CHANGESIZE_TYPES))
	SetChangedFlag (True);
      break;
    }
  RestoreCrosshair (True);

}
