/*----------------------------------------------------------------------------*/
/* tclcommandPR.c:                                                            */
/*      Tcl routines for PCB command-line functions                           */
/* Copyright (c) 2004, 2005  Tim Edwards, MultiGiG, Inc.                      */
/*                           Paramesh Santanam, MultiGiG, Inc., Nishit Patel  */
/*----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>	 /* for tolower(), toupper() */

#include <tk.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "config.h"
#include "output.h"	/* also includes "global.h" */
#include "macro.h"
#include "data.h"

#include "crosshair.h"
#include "buffer.h"
#include "set.h"
#include "file.h"
#include "create.h"
#include "misc.h"
#include "draw.h"
#include "error.h"
#include "action.h"
#include "polygon.h"
#include "report.h"
#include "printpanner.h"
#include "print.h"
#include "gui.h"
#include "rubberband.h"
#include "rotate.h"
#include "const.h"
#include <math.h>

#include "control.h"
#include "find.h"
#include "undo.h"
#include "change.h"
#include "select.h"
#include "djopt.h"
#include "autoroute.h"
#include "autoplace.h"
#include "rats.h"
#include "pinout.h"
#include "search.h"

/* check for "round()" function */
#ifndef round
#define round(x) (double)((int)((double)(x) + (((x) < 0) ? -0.5 : 0.5)))
#endif

/* some local identifiers */
static PinoutTypePtr Pinout = NULL;
static PrintDeviceTypePtr Device;
static PrintInitType DeviceInfo;

static Boolean OutlineFlag = True, AlignmentFlag = False,
               DrillHelperFlag = False, ColorFlag = False,
               DOSFlag = False, SilkscreenTextFlag = False;

extern int djopt_get_auto_only();
extern void WarpPointer (Boolean);
extern Boolean SeekPad (LibraryEntryType *, ConnectionType *, Boolean);
extern char* reportString;

#ifdef TCL_PCB

#include "tclpcb.h"

/*----------------------------------------------------------------------*/
/*  Command "pad" ---                                                   */
/*----------------------------------------------------------------------*/

int pcbtcl_pad(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "report", "size", "name", NULL
   };
   enum OptionId {
      PAD_REPORT, PAD_SIZE, PAD_NAME
   };

   int result, idx;
   char *value = NULL;
   BDimension size;
   char tmpVal[128];
   Boolean isAbsolute = True;
   PadTypePtr Pad = NULL;

   if (objc == 1) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option>");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
                options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
         options, "pad option", 0, &idx)) != TCL_OK)
       return result;

   switch (idx)
   {
       case PAD_REPORT:
           ReportFoundPins();
           break;

       case PAD_SIZE:
           /* Change size of selected Pad */
           if (objc >= 3) {
               value = Tcl_GetString(objv[2]);
               if((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;

               strcpy(tmpVal, value);
               if (objc == 3)
                   strtok(tmpVal, "m");
               else if (objc == 4)
                   value = Tcl_GetString(objv[3]);

               if (strstr(value, "mm"))
                   size = (int)(round(atof(tmpVal) / COOR_TO_MM));
               else if(strstr(value, "mil"))
                   size = (int)(round(atof(tmpVal) * 100));
               else
                   size = (int)(round(atof(tmpVal)));

               /* Refer line no. 149, 150 in pcb-menu.h file */
               HideCrosshair (True);
               if (ChangeSelectedSize (PAD_TYPE, size, isAbsolute))
                   SetChangedFlag (True);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : pad size <\"[+|-]\"value>\"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case PAD_NAME:
           ELEMENT_LOOP (PCB->Data);
           {
               PAD_LOOP (element);
               {
                   if (TEST_FLAG (SELECTEDFLAG, pad)) {
                       Pad = pad;
                       break;
                   }
               }
               END_LOOP;
           }
           END_LOOP;
           if(Pad && objc == 3) {
               value = Tcl_GetString(objv[2]);
               if (TEST_FLAG (DISPLAYNAMEFLAG, Pad)) {
                   ErasePadName (Pad);
                   Pad->Name = strdup(value);
                   DrawPadName (Pad, 0);
               } else {
                   Pad->Name = strdup(value);
               }
           } else if (Pad) {
               Tcl_SetResult(interp, Pad->Name, 0);
               return TCL_OK;
           } else {
                return TCL_ERROR;
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"pad\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "pin" ---                                                   */
/*----------------------------------------------------------------------*/

int pcbtcl_pin(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "square", "hole", "report", "size", "name", "clearance", "plug", NULL
   };
   enum OptionId {
      PIN_SQUARE, PIN_HOLE, PIN_REPORT, PIN_SIZE, PIN_NAME,
      PIN_CLEARANCE, PIN_PLUG
   };

   int result, idx;
   char *value = NULL;
   BDimension size = 0;
   char tmpVal[128];
   Boolean isAbsolute = True;
   PinTypePtr Pin = NULL;

   if (objc == 1) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option>");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
               options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
            options, "pin option", 0, &idx)) != TCL_OK)
       return result;

    switch (idx) {
	case PIN_HOLE:
	case PIN_SIZE:
	case PIN_CLEARANCE:

           if (objc >= 3) {
               value = Tcl_GetString(objv[2]);
               if((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;
               strcpy(tmpVal, value);
               if (objc == 3)
                   strtok(tmpVal, "m");
               else if (objc == 4)
                   value = Tcl_GetString(objv[3]);

               if (strstr(value, "mm"))
                   size = (BDimension)(round(atof(tmpVal) / COOR_TO_MM));
               else if(strstr(value, "mil"))
                   size = (BDimension)(round(atof(tmpVal) * 100.0));
               else
                   size = (BDimension)(round(atof(tmpVal)));
	    }
	    break;
    }

    switch (idx) {
       case PIN_SQUARE:
           if(objc == 3) {
               value = Tcl_GetString(objv[2]);
               if(!strcmp(value, "on")) {
                   if (SetSelectedSquare (PIN_TYPE))
                       SetChangedFlag (True);
               } else if(!strcmp(value, "off")) {
                   if (ClrSelectedSquare (PIN_TYPE))
                       SetChangedFlag (True);
               }
           } else {
               /* Toggle Square flag of selected pins */
               /* Menu:Select -> (Change square-flag of selected objects) Pins.
                */
               if (ChangeSelectedSquare (PIN_TYPE | PAD_TYPE))
                   SetChangedFlag (True);
               /*
                * Tcl_SetResult(interp, "Valid options are : {on|off}", 0);
                * return TCL_ERROR;
                */
           }
           break;

       case PIN_HOLE:
           /* Change DrillHole of selected Pin */
	   if (objc >= 3) {
               /* Refer line no. 161, 162 in pcb-menu.h file */
               HideCrosshair (True);
               if (ChangeSelected2ndSize (PIN_TYPE, size, isAbsolute))
                   SetChangedFlag (True);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : pin hole <\"[+|-]\"value>\"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case PIN_PLUG:
           /* Change plug flag of selected Pin */
	   if (objc >= 2) {
               HideCrosshair (True);
               if (ChangeSelectedPlugs(PIN_TYPE))
                   SetChangedFlag (True);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : pin plug", 0);
               return TCL_ERROR;
           }
           break;

       case PIN_CLEARANCE:
           /* Change clearance around selected Pin */
	   if (objc >= 3) {
               /* Refer line no. 161, 162 in pcb-menu.h file */
               HideCrosshair (True);
               if (ChangeSelectedClearSize (PIN_TYPE, size, isAbsolute))
                   SetChangedFlag (True);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : pin clearance "
			"<\"[+|-]\"value>\"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case PIN_REPORT:
           {
               Tcl_Obj *pobj = NULL;
               
               ReportFoundPins();
               if(reportString) {
                   pobj = Tcl_NewStringObj(reportString, -1);
                   Tcl_SetObjResult(interp, pobj);
                   SaveFree (reportString);
               }
           }
           break;

       case PIN_SIZE:
           /* Change size of selected Pin */
           if (objc >= 3) {
               value = Tcl_GetString(objv[2]);
               if((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;

               strcpy(tmpVal, value);
               if (objc == 3)
                   strtok(tmpVal, "m");
               else if (objc == 4)
                   value = Tcl_GetString(objv[3]);
               
               if (strstr(value, "mm"))
                   size = atof(tmpVal)/COOR_TO_MM;
               else if(strstr(value, "mil"))
                   size =  atof(tmpVal)*100;
               else
                   size = atof(tmpVal);

               /* Refer line no. 151, 152 in pcb-menu.h file */
               HideCrosshair (True);
               if (ChangeSelectedSize (PIN_TYPE, size, isAbsolute))
                   SetChangedFlag (True);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : pin size <\"[+|-]\"value>\"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case PIN_NAME:
           ELEMENT_LOOP (PCB->Data);
           {
               PIN_LOOP (element);
               {
                   if (TEST_FLAG (SELECTEDFLAG, pin)) {
                       Pin = pin;
                       break;
                   }
               }
               END_LOOP;
           }
           END_LOOP;

           if(Pin && objc == 3) {
               value = Tcl_GetString(objv[2]);
               if (TEST_FLAG (DISPLAYNAMEFLAG, Pin)) {
                   ErasePinName (Pin);
                   Pin->Name = strdup(value);
                   DrawPinName (Pin, 0);
               } else {
                   Pin->Name = strdup(value);
               }
           } else if (Pin) {
               Tcl_SetResult(interp, Pin->Name, 0);
               return TCL_OK;
           } else {
               return TCL_ERROR;
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"pin\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "pinout" ---                                                */
/*----------------------------------------------------------------------*/

int pcbtcl_pinout(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "create", "delete", "height", "name", "redraw", "title", "width", "zoom", NULL
   };
   enum OptionId {
      PINOUT_CREATE, PINOUT_DELETE, PINOUT_HEIGHT, PINOUT_NAME, PINOUT_REDRAW,
      PINOUT_TITLE, PINOUT_WIDTH, PINOUT_ZOOM
   };

   int result, idx;
   double dval = 0;
   Location X = 0, Y = 0;
   ElementTypePtr element;

   /* Don't create pinout window if there no element is
    * under pointer.
    */
   if (objc == 1) {
       if ((SearchScreen(Crosshair.X, Crosshair.Y, ELEMENT_TYPE,
                        (void **)&element, (void **)&element,
                        (void **)&element)) == NO_TYPE) {
           return TCL_ERROR;
       } else {
           return TCL_OK;
       }
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
               options, "pinout option", 0, &idx)) != TCL_OK)
       return result;

   switch (idx)
   {
       case PINOUT_CREATE:
           if(objc == 4) {
               if (Tcl_GetDoubleFromObj(interp, objv[2], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   X = (Location)(dval*100);
               if (Tcl_GetDoubleFromObj(interp, objv[3], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   Y = (Location)(dval*100);
           } else {
               X = Crosshair.X;
               Y = Crosshair.Y;
           }
           if ((SearchScreen(X, Y, ELEMENT_TYPE, (void **)&element,
                            (void **)&element, (void **)&element)) != NO_TYPE) {
               if (Pinout) PinoutDismiss(Pinout);
               Pinout = PinoutWindow(Output.Toplevel, element);
           } else {
               return TCL_ERROR;
           }
           break;

       case PINOUT_DELETE:
           if (Pinout)
              PinoutDismiss(Pinout);
           Pinout = NULL;
           break;

       case PINOUT_NAME:
           /* report the name of the element */
           if (Pinout && Pinout->Element.Name[1].TextString) {
               Tcl_SetResult(interp, Pinout->Element.Name[1].TextString, NULL);
           }
           break;

       case PINOUT_REDRAW:
           if (Pinout)
               RedrawPinoutWindow(Pinout);
           break;

       case PINOUT_TITLE:
           /* report the title from the Pinout structure */
           if (Pinout)
               Tcl_SetResult(interp, Pinout->Title, NULL);
           break;

       case PINOUT_HEIGHT:
           if (Pinout)
               Tcl_SetObjResult(interp,
                   Tcl_NewIntObj((int)(Pinout->MaxY * Pinout->scale)));
           break;

       case PINOUT_WIDTH:
           if (Pinout)
               Tcl_SetObjResult(interp,
                   Tcl_NewIntObj((int)(Pinout->MaxX * Pinout->scale)));
           break;

       case PINOUT_ZOOM:
           if(Pinout) {
               if (objc ==3) {
                   char *value = Tcl_GetString(objv[2]);
                   if (!strcmp(value, "out") && Pinout->Zoom < MAX_ZOOM)
                       Pinout->Zoom += 1.0;
                   else if (!strcmp(value, "in") && Pinout->Zoom > MIN_ZOOM)
                       Pinout->Zoom -= 1.0;
                   //Settings.PinoutZoom = Pinout->Zoom;
                   Pinout->scale = 1/ (100 * exp (Pinout->Zoom * LN_2_OVER_2));
               } else {
                   Tcl_SetObjResult(interp, Tcl_NewIntObj((int)(Pinout->Zoom)));
               }
       }
       break;
   }
   return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/*  Command "place" ---                                                 */
/*----------------------------------------------------------------------*/

int pcbtcl_place(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
    /* Check against Tk "place" command first, if more than one option  */
    /* is specified.                                                    */

   if (objc > 1)
       return (resolve_conflict(clientData, interp, objc, objv));
   else if (objc == 1 && ConfirmDialog("Auto-placement can NOT be undone.\n"
                  "Do you want to continue anyway?\n")) {
       HideCrosshair (True);
       watchCursor();
       if (AutoPlaceSelected())
           SetChangedFlag(True);
       restoreCursor();
       RestoreCrosshair(True);
   }

   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "pointer" ---                                               */
/*----------------------------------------------------------------------*/

int pcbtcl_pointer(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "down", "left", "mark", "right", "up", NULL
   };
   enum PointerId {
     POINTER_DOWN, POINTER_LEFT, POINTER_MARK,
     POINTER_RIGHT, POINTER_UP
   };

   static char *markOptions[] = {
       "center", "set", "reset", NULL
   };
   enum MarkId {
      POINTER_MARK_CENTER, POINTER_MARK_SET,
      POINTER_MARK_RESET
   };

   int result, idx, midx, xval=0, yval = 0;
   Location x, y, dx, dy;
   Tcl_Obj *pobj, *lobj;

   if (objc == 1) {
       lobj = Tcl_NewListObj(0, NULL);
       pobj = Tcl_NewDoubleObj((double)(Crosshair.X/100));
       Tcl_ListObjAppendElement(interp, lobj, pobj);
       pobj = Tcl_NewDoubleObj((double)(Crosshair.Y/100));
       Tcl_ListObjAppendElement(interp, lobj, pobj);
       Tcl_SetObjResult(interp, lobj);
       return PCBTagCallback(interp, objc, objv);
   } else if (objc >= 2) {
       if ((Tcl_GetIntFromObj(interp, objv[1], &xval) == TCL_OK)) {
           /* Parse command line for new relative pointer position */
           if (Tcl_GetIntFromObj(interp, objv[1], &xval) != TCL_OK)
               return TCL_ERROR;
           if (Tcl_GetIntFromObj(interp, objv[2], &yval) != TCL_OK)
               return TCL_ERROR;
       } else {
           if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
                   options, "option", 0, &idx)) != TCL_OK)
               return result;
       }
   } else {
       Tcl_SetResult(interp, "Usage : pointer  [up|down|left|right]  \n\
                         pointer  \"[\"[+|-]\"<deltax> \"[+|-]\"<deltay>]\" \n\
                         pointer  mark \"[set|reset|center]\"", 0);
       return TCL_ERROR; 
   }

   if(objc >= 2) {
       switch (idx)
       {
           case POINTER_MARK:
               if(objc == 3) {
                   if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
		       (CONST84 char **) markOptions, "mark option", 
		       0, &midx)) != TCL_OK)
                       return result;

                   /* Set/Reset the Crosshair mark */
                   switch (midx)
                   {
                       case POINTER_MARK_CENTER:
                           DrawMark (True);
                           Marked.status = True;
                           Marked.X = Crosshair.X;
                           Marked.Y = Crosshair.Y;
                           DrawMark (False);
                           break;
 	
                       case POINTER_MARK_SET:
                           if(!Marked.status) {
                               Marked.status = True;
                               Marked.X = Crosshair.X;
                               Marked.Y = Crosshair.Y;
                               DrawMark (False);
                           } else {
                               Tcl_SetResult(interp, "Crosshair mark already set! ", 0);
                               return TCL_ERROR; 
                           }
                           break;

                   case POINTER_MARK_RESET:
                           if(Marked.status) {
                               DrawMark (True);
                               Marked.status = False;
                           } else {
                               Tcl_SetResult(interp, "Crosshair mark already reset! ", 0);
                               return TCL_ERROR;
                           }
                           break;
                   }
               } else if(objc == 2) {
                   if (Marked.status) {
                       DrawMark (True);
                       Marked.status = False;
               } else {
                   Marked.status = True;
                   Marked.X = Crosshair.X;
                   Marked.Y = Crosshair.Y;
                   DrawMark (False);
               }
           } else {
               Tcl_SetResult(interp, "Usage : pointer mark \"[set|reset|center]\"", 0);
               return TCL_ERROR; 	
           }
           SetCursorStatusLine ();
           break;

       case POINTER_DOWN:
       case POINTER_LEFT:
       case POINTER_RIGHT:
       case POINTER_UP:
       default :
           if(idx == POINTER_DOWN) yval = 1;
           else if(idx == POINTER_LEFT) xval = -1;
           else if(idx == POINTER_RIGHT) xval = 1;
           else if(idx == POINTER_UP) yval = -1;

           /* save old crosshair position */
           x = Crosshair.X;
           y = Crosshair.Y;
           dx = (Location) (xval * PCB->Grid);
           dy = (Location) (yval * PCB->Grid);
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
           break;
       }
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "polygon" ---                                               */
/*----------------------------------------------------------------------*/

int pcbtcl_polygon(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
      "close", "draw", "previous", "toggle_clearance", NULL
   };
   enum OptionId {
      POLYGON_CLOSE, POLYGON_DRAW, POLYGON_PREVIOUS ,
      POLYGON_TOGGLE_CLEARANCE
   };

   static char *tClearanceOptions[] = {
      "object", "selected_lines", "selected_arcs", "selected_objects", NULL
   };
   enum TClearanceOptionId {
      POLYGON_TC_OBJECT, POLYGON_TC_SELECTED_LINES,
      POLYGON_TC_SELECTED_ARCS, POLYGON_TC_SELECTED_OBJECTS
   };

   int result, idx, tidx;
   double xval = 0, yval = 0;
   int objIdx = 0;
   PolygonTypePtr polygon;

   if (objc == 1) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option>");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
               options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
           options, "option", 0, &idx)) != TCL_OK)
       return result;

   switch (idx)
   {
       case POLYGON_CLOSE:
           if(Settings.Mode == POLYGON_MODE) {
               HideCrosshair (True);
               ClosePolygon ();
               RestoreCrosshair (True);
            }
            break;

       case POLYGON_DRAW:
           if ((objc >= 8) && (objc%2 == 0)) {
               polygon = CreateNewPolygon (CURRENT, CLEARPOLYFLAG);
               if (!polygon)
                   return TCL_ERROR;
               
               for (objIdx = 1; objIdx < objc/2; objIdx ++) {
                   Tcl_GetDoubleFromObj(interp, objv[objIdx*2], &xval);
                   Tcl_GetDoubleFromObj(interp, objv[(objIdx*2)+1], &yval);
                   
                   CreateNewPointInPolygon (polygon, 
                                 (Location)(xval*100), (Location)(yval*100));
               }

               SetPolygonBoundingBox(polygon);

               AddObjectToCreateUndoList (POLYGON_TYPE, CURRENT, 
	                                  polygon, polygon);
               IncrementUndoSerialNumber ();
               UpdatePIPFlags (NULL, NULL, CURRENT, True);
               DrawPolygon (CURRENT, polygon, 0);
               Draw ();
           } else {
               Tcl_SetResult(interp, "x1 y1 x2 y2 x3 y3 ... values required.", 0);
               return TCL_ERROR; 
           }
           break;

       /* go back to the previous point */
       case POLYGON_PREVIOUS:
           if(Settings.Mode == POLYGON_MODE) {
               HideCrosshair (True);
               GoToPreviousPoint ();
               RestoreCrosshair (True);
           }
           break;

       /* changes the join (clearance through polygons) of objects */
       /* Refer "option new_lines_arcs_clear_polygons" command before using */
       case POLYGON_TOGGLE_CLEARANCE:
           if(objc == 3) {
               if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	           (CONST84 char **) tClearanceOptions, "option", 
		   0, &tidx)) != TCL_OK)
                   return result;
               HideCrosshair (True);
               switch (tidx)
               {
                   case POLYGON_TC_OBJECT:
                       {
                           int type;
                           void *ptr1, *ptr2, *ptr3;
                           
                           if ((type = SearchScreen (Crosshair.X, Crosshair.Y, 
			                             CHANGEJOIN_TYPES, &ptr1, 
						     &ptr2, &ptr3)) != NO_TYPE)
                               if (ChangeObjectJoin (type, ptr1, ptr2, ptr3))
                                   SetChangedFlag (True);
                               break;
                       }

                   case POLYGON_TC_SELECTED_LINES:
                       if (ChangeSelectedJoin (LINE_TYPE))
                           SetChangedFlag (True);
                       break;

                   case POLYGON_TC_SELECTED_ARCS:
                       if (ChangeSelectedJoin (ARC_TYPE))
                           SetChangedFlag (True);
                       break;

                    case POLYGON_TC_SELECTED_OBJECTS:
                        if (ChangeSelectedJoin (CHANGEJOIN_TYPES))
                           SetChangedFlag (True);
                         break;
               }
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : polygon toggle_clearance [object|selected_lines|selected_arcs|selected_objects]", 0);
               return TCL_ERROR;
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"polygon\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}



/*----------------------------------------------------------------------*/
/*  Initialize the DeviceInfo to sane values at startup		*/
/*----------------------------------------------------------------------*/

void
PrintInit()
{
   /* Set defalut value */
   if (!Device)
       Device = PrintingDevice[0].Info;   /* Set default driver to PS */

   /* initialize command if called the first time */
   if (!PCB->PrintFilename)
       PCB->PrintFilename = EvaluateFilename (Settings.PrintFile, NULL,
                 PCB->Filename ? PCB->Filename :"noname", NULL);

   /* Default media A4 size */
   DeviceInfo.SelectedMedia = &Media[2];

   /* clear offset --> recenter output */
   DeviceInfo.OffsetX = 0;
   DeviceInfo.OffsetY = 0;

   DeviceInfo.FP = NULL;
   DeviceInfo.MirrorFlag = FALSE;
   DeviceInfo.RotateFlag = FALSE;
   DeviceInfo.InvertFlag = FALSE;

   DeviceInfo.OffsetX = 0;
   DeviceInfo.OffsetY = 0;
   DeviceInfo.Scale = 1.0;

   DeviceInfo.BoundingBox.X1 = 0;
   DeviceInfo.BoundingBox.Y1 = 0;
   DeviceInfo.BoundingBox.X2 = 0;
   DeviceInfo.BoundingBox.Y2 = 0;
}

/*----------------------------------------------------------------------*/
/*  Command "print" ---                                                 */
/*----------------------------------------------------------------------*/


int pcbtcl_print(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "alignment", "color", "dos", "drill_helper", "driver", "file",
       "invert", "media", "mirror", "offset", "outline", "rotate",
       "scale", NULL
   };
   enum OptionId {
      PRINT_ALIGNMENT, PRINT_COLOR, PRINT_DOS, PRINT_DRILLHELPER,
      PRINT_DRIVER, PRINT_FILE, PRINT_INVERT, PRINT_MEDIA,
      PRINT_MIRROR, PRINT_OFFSET, PRINT_OUTLINE, PRINT_ROTATE,
      PRINT_SCALE
   };

   /* Driver sequence is as per data in src/data.c */
   static char *driveroption[] = {
       "ps", "eps", "gbr", NULL
   };
   enum DriverOptionId {
      PRINT_DRIVER_PS, PRINT_DRIVER_EPS, PRINT_DRIVER_GBR
   };

   /* Media sequence is as per data in src/printpanner.c */
   static char *mediaoption[] = {
       "user", "A3", "A4", "A5", "letter", "11x17", "ledger",
       "legal", "executive", "C", "D", NULL
   };

   int result, idx, subdidx, submidx, ival;
   char *value = NULL;
   char rStr[8];
   Tcl_Obj *pobj;


   if (objc == 1) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option>");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
               options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
          options, "print option", 0, &idx)) != TCL_OK)
        return result;

   if (objc == 3)
       value = Tcl_GetString(objv[2]);

   switch (idx)
   {
       case PRINT_ALIGNMENT:
           if(objc > 2) {
               if(!strcmp(value, "on"))
                   AlignmentFlag = True;
               else if(!strcmp(value, "off"))
                   AlignmentFlag = False;
               else {
                   Tcl_SetResult(interp, "Valid options are : {on|off}", 0);
                   return TCL_ERROR;
           }
       } else {
           if (AlignmentFlag)
               strcpy(rStr,"on");
           else
               strcpy(rStr,"off");
           pobj = Tcl_NewStringObj(rStr,-1);
           Tcl_SetObjResult(interp, pobj);
       }
       break;

       case PRINT_COLOR:
           if(objc > 2) {
               if(!strcmp(value, "on"))
                   ColorFlag = True;
               else if(!strcmp(value, "off"))
                   ColorFlag = False;
               else {
                   Tcl_SetResult(interp, "Valid options are : {on|off}", 0);
                   return TCL_ERROR; 
           }
       } else {
           if (ColorFlag)
               strcpy(rStr,"on");
           else
               strcpy(rStr,"off");
           pobj = Tcl_NewStringObj(rStr,-1);
           Tcl_SetObjResult(interp, pobj);
       }
       break;

       case PRINT_DOS:
           if(objc > 2) {
               if(!strcmp(value, "on"))
                   DOSFlag = True;
               else if(!strcmp(value, "off"))
                   DOSFlag = False;
               else {
                   Tcl_SetResult(interp, "Valid options are : {on|off}", 0);
                   return TCL_ERROR;
               }
           } else {
               if (DOSFlag)
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               pobj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, pobj);
           }
           break;

       case PRINT_DRILLHELPER:
           if(objc > 2) {
               if(!strcmp(value, "on"))
                   DrillHelperFlag = True;
               else if(!strcmp(value, "off"))
                   DrillHelperFlag = False;
               else {
                   Tcl_SetResult(interp, "Valid options are : {on|off}", 0);
                   return TCL_ERROR; 	
               }
           }else {
               if (DrillHelperFlag)
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               pobj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, pobj);
           }
           break;

       case PRINT_DRIVER:
           if(objc > 2) {
               if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	           (CONST84 char**) driveroption, "print driver option", 
		   0, &subdidx)) != TCL_OK)
                   return result;
               switch (subdidx)
               {
                   case PRINT_DRIVER_PS:
                       Device = PrintingDevice[0].Info;     /*  PS driver */
                       break;

                   case PRINT_DRIVER_EPS:
                       Device = PrintingDevice[1].Info;     /*  EPS driver */
                       break;

                   case PRINT_DRIVER_GBR:
                       Device = PrintingDevice[2].Info;     /*  GERBER driver */
                       break;
               }
           } else {
               if (Device == NULL) {
                   Tcl_SetResult(interp, "No driver currently set", 0);
               } else {
                   /* Return current print driver name */
                   pobj = Tcl_NewStringObj(Device->Suffix, -1);
                   Tcl_SetObjResult(interp, pobj);
               }
       }
       break;

       case PRINT_FILE:
           if(objc > 2) {
               SaveFree (PCB->PrintFilename);
               PCB->PrintFilename = StripWhiteSpaceAndDup(value);
               Print(PCB->PrintFilename, DeviceInfo.Scale, DeviceInfo.MirrorFlag,
                       DeviceInfo.RotateFlag, ColorFlag,
                       DeviceInfo.InvertFlag, OutlineFlag, AlignmentFlag,
                       DrillHelperFlag, DOSFlag, Device, 
		       DeviceInfo.SelectedMedia, DeviceInfo.OffsetX, 
		       DeviceInfo.OffsetY, SilkscreenTextFlag);
           } else {
               /* Return current print file name */
               pobj = Tcl_NewStringObj(PCB->PrintFilename, -1);
               Tcl_SetObjResult(interp, pobj);
           }
           break;

       case PRINT_INVERT:
           if(objc > 2) {
               if(!strcmp(value, "on"))
                   DeviceInfo.InvertFlag = True;
               else if(!strcmp(value, "off"))
                   DeviceInfo.InvertFlag = False;
               else {
                   Tcl_SetResult(interp, "Valid options are : {on|off}", 0);
                   return TCL_ERROR;
               }
           } else {
               if (DeviceInfo.InvertFlag)
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               pobj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, pobj);
           }
           break;

       case PRINT_MEDIA:
           if(objc > 2) {
               if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	           (CONST84 char**) mediaoption, "print media option", 
		   0, &submidx)) != TCL_OK)
                   return result;
               DeviceInfo.SelectedMedia = &Media[submidx];
           } else {
               /* Return current print media name */
               if(DeviceInfo.SelectedMedia && DeviceInfo.SelectedMedia->Name){
                   pobj = Tcl_NewStringObj(DeviceInfo.SelectedMedia->Name, -1);
                   Tcl_SetObjResult(interp, pobj);
               }
           }
           break;

       case PRINT_MIRROR:
           if(objc > 2) {
               if(!strcmp(value, "on"))
                   DeviceInfo.MirrorFlag = True;
               else if(!strcmp(value, "off"))
                   DeviceInfo.MirrorFlag = False;
               else {
                   Tcl_SetResult(interp, "Valid options are : {on|off}", 0);
                   return TCL_ERROR; 
               }
           } else {
               if (DeviceInfo.MirrorFlag)
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               pobj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, pobj);
           }
           break;

       case PRINT_OFFSET:
           if(objc == 4) {
	       /* Note:  value is in (negative) micro-inches! */
	       /* Need to parse for sane unit types like "in" and "mm" */

               if (Tcl_GetIntFromObj(interp, objv[2], &ival) != TCL_OK)
                   return TCL_ERROR;
               else
                   DeviceInfo.OffsetX = (BDimension)ival;
               if (Tcl_GetIntFromObj(interp, objv[3], &ival) != TCL_OK)
                   return TCL_ERROR;
               else
                   DeviceInfo.OffsetY = (BDimension)ival;
           } else if (objc == 2) {
	       Tcl_Obj *lobj;
               lobj = Tcl_NewListObj(0, NULL);
               pobj = Tcl_NewIntObj((int)DeviceInfo.OffsetX);
	       Tcl_ListObjAppendElement(interp, lobj, pobj);
               pobj = Tcl_NewIntObj((int)DeviceInfo.OffsetY);
	       Tcl_ListObjAppendElement(interp, lobj, pobj);
               Tcl_SetObjResult(interp, lobj);
	   } else {
               Tcl_WrongNumArgs(interp, objc, objv, "[<x> <y>]");
               return TCL_ERROR;
           }
           break;

       case PRINT_OUTLINE:
           if(objc > 2) {
               if(!strcmp(value, "on"))
                   OutlineFlag = True;
               else if(!strcmp(value, "off"))
                   OutlineFlag = False;
               else {
                   Tcl_SetResult(interp, "Valid options are : {on|off}", 0);
                   return TCL_ERROR;
               }
           } else {
               if (OutlineFlag)
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               pobj = Tcl_NewStringObj(rStr,-1);
                   Tcl_SetObjResult(interp, pobj);
           }
           break;

       case PRINT_ROTATE:
           if(objc > 2) {
               if(!strcmp(value, "on"))
                   DeviceInfo.RotateFlag  = True;
               else if(!strcmp(value, "off"))
                   DeviceInfo.RotateFlag  = False;
               else {
                   Tcl_SetResult(interp, "Valid options are : {on|off}", 0);
                   return TCL_ERROR;
               }
           } else {
               if (DeviceInfo.RotateFlag)
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               pobj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, pobj);
           }
           break;

       case PRINT_SCALE:
           if(objc > 2) {
               DeviceInfo.Scale = (double)atof(value);
           } else {
               pobj = Tcl_NewDoubleObj((double)DeviceInfo.Scale);
               Tcl_SetObjResult(interp, pobj);
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"print\" option", 0);
           return TCL_ERROR;
     }
     return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "quit" ---                                                  */
/*----------------------------------------------------------------------*/

int pcbtcl_quit(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   if (objc == 1) {
       if (!PCB->Changed || ConfirmDialog ("OK to lose data ?"))
           QuitApplication ();
   } else {
       Tcl_SetResult(interp, "Usage : \"quit\"", 0);
       return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/*  Command "rectangle" ---                                             */
/*----------------------------------------------------------------------*/

int pcbtcl_rectangle(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
      "draw", NULL
   };
   enum OptionId {
      RECTANGLE_DRAW
   };

   int result, idx, ulx, uly, lrx, lry;
   double dval = 0;
   PolygonTypePtr polygon;

   if (objc == 1) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option>");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
               options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
         options, "option", 0, &idx)) != TCL_OK)
       return result;

   switch (idx)
   {
       case RECTANGLE_DRAW:
           if (objc == 6) {
               if (Tcl_GetDoubleFromObj(interp, objv[2], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   ulx = (Location)(dval*100);
               if (Tcl_GetDoubleFromObj(interp, objv[3], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   uly = (Location)(dval*100);
               if (Tcl_GetDoubleFromObj(interp, objv[4], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   lrx = (Location)(dval*100);
               if (Tcl_GetDoubleFromObj(interp, objv[5], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   lry = (Location)(dval*100);

               polygon = CreateNewPolygon (CURRENT, CLEARPOLYFLAG);

               if (!polygon)
                   return TCL_ERROR;
               CreateNewPointInPolygon (polygon, ulx, uly);
               CreateNewPointInPolygon (polygon, lrx, uly);
               CreateNewPointInPolygon (polygon, lrx, lry);
               CreateNewPointInPolygon (polygon, ulx, lry);
               SetPolygonBoundingBox(polygon);

               AddObjectToCreateUndoList (POLYGON_TYPE, CURRENT, 
	                                  polygon, polygon);
               IncrementUndoSerialNumber ();
               UpdatePIPFlags (NULL, NULL, CURRENT, True);
               DrawPolygon (CURRENT, polygon, 0);
               Draw ();
           } else {
               Tcl_SetResult(interp, "ulx uly lrx lry values required.", 0);
               return TCL_ERROR;
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"rectangle\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/*  Command "redo" ---                                                  */
/*----------------------------------------------------------------------*/

int pcbtcl_redo(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   if ((Settings.Mode == POLYGON_MODE &&
         Crosshair.AttachedPolygon.PointN) ||
         Crosshair.AttachedLine.State == STATE_SECOND)
       return PCBTagCallback(interp, objc, objv);

   HideCrosshair (True);
   if ((objc == 1) && Redo(True)) {
       SetChangedFlag (True);
       if (Settings.Mode == LINE_MODE &&
	      Crosshair.AttachedLine.State != STATE_FIRST) {
           LineTypePtr line = &CURRENT->Line[CURRENT->LineN - 1];
           Crosshair.AttachedLine.Point1.X =
               Crosshair.AttachedLine.Point2.X = line->Point2.X;
           Crosshair.AttachedLine.Point1.Y =
               Crosshair.AttachedLine.Point2.Y = line->Point2.Y;
           addedLines++;
       }
   }
   RestoreCrosshair (True);
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "route" ---                                                 */
/*----------------------------------------------------------------------*/

int pcbtcl_route(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
      "all", "rats", "optimize", NULL
   };
   enum OptionId {
      ROUTE_ALL, ROUTE_RATS, ROUTE_OPTIMIZE
   };

   static char *optimizeoptions[] = {
      "auto", "auto_routed_only", "debumpify", "miter", "orthopull",
      "simple", "unjaggy", "vianudge", "viatrim", NULL
   };
   enum OptimizeOptionId {
       ROUTE_OPTIMIZE_AUTO, ROUTE_OPTIMIZE_AUTO_ROUTED_ONLY,
       ROUTE_OPTIMIZE_DEBUMPIFY, ROUTE_OPTIMIZE_MITER,
       ROUTE_OPTIMIZE_ORTHOPULL, ROUTE_OPTIMIZE_SIMPLE,
       ROUTE_OPTIMIZE_UNJAGGY, ROUTE_OPTIMIZE_VIA_NUDGE,
       ROUTE_OPTIMIZE_VIA_TRIM
   };

   int result, idx, oidx, ival = 0;
   char *name = NULL;
   char *str[2];

   if (objc > 4 || objc < 2) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option> ...");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
              options, "option", 0, &idx);
       return TCL_ERROR;
   } else if (objc == 4) {
       name = Tcl_GetString(objv[3]);
       if (!strcmp(name,"on"))
           ival = 1;
       else if (!strcmp(name, "off"))
           ival = 0;
       else {
           Tcl_SetResult(interp, "Unknown \"route\" parameter", 0);
           return TCL_ERROR;
       }
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
        options, "option", 0, &idx)) != TCL_OK)
       return result;

   switch (idx)
   {
       case ROUTE_ALL:
           HideCrosshair (True);
           watchCursor ();
           if (AutoRoute (True))
               SetChangedFlag (True);
           restoreCursor ();
           RestoreCrosshair (True);
           break;

       case ROUTE_RATS:
           // only selected rats are routed
           HideCrosshair (True);
           watchCursor ();
           if (AutoRoute (False))
               SetChangedFlag (True);
           restoreCursor ();
           RestoreCrosshair (True);
           break;

       case ROUTE_OPTIMIZE:
           if (objc == 4) {
               if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	           (CONST84 char **) optimizeoptions, "optimize option", 
		   0, &oidx)) != TCL_OK)
                   return result;

               switch (oidx)
               {
                   // This needs to be worked out---ultimately we call djopt()
                   // for most of this stuff.
                   case ROUTE_OPTIMIZE_AUTO:
                       route_auto = ival;
                       break;

                   case ROUTE_OPTIMIZE_AUTO_ROUTED_ONLY:
                       autorouted_only = ival;
                       break;

                   case ROUTE_OPTIMIZE_DEBUMPIFY:
                       route_debumpify = ival;
                       break;

                   case ROUTE_OPTIMIZE_MITER:
                       route_miter = ival;
                       break;

                   case ROUTE_OPTIMIZE_ORTHOPULL:
                       route_orthopull = ival;
                       break;

                   case ROUTE_OPTIMIZE_SIMPLE:
                       route_simple = ival;
                       break;

                   case ROUTE_OPTIMIZE_UNJAGGY:
                       route_unjaggy = ival;
                       break;

                   case ROUTE_OPTIMIZE_VIA_NUDGE:
                       route_via_nudge = ival;
                       break;

                   case ROUTE_OPTIMIZE_VIA_TRIM:
                       route_via_trim = ival;
                       break;
               }
           } else if (objc == 3) {
               name = Tcl_GetString(objv[2]);
               str[0] = StrDup(NULL, name);
               ActionDJopt (str);
           } else if (objc == 2) {
               RouteOptimize();
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"route\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

#endif /* TCL_PCB */
