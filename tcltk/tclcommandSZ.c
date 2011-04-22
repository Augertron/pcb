/*----------------------------------------------------------------------------*/
/* tclcommandSZ.c:                                                            */
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
#include "output.h"  /* also includes "global.h" */
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
#include "search.h"
#include "gui.h"

#include "control.h"
#include "find.h"
#include "undo.h"
#include "change.h"
#include "select.h"

static LayerTypePtr lastLayer;
extern int FlagCurrentStyle();
extern int djopt_get_auto_only();
extern void WarpPointer (Boolean);
extern Boolean SeekPad (LibraryEntryType *, ConnectionType *, Boolean);

#ifdef TCL_PCB

#include "tclpcb.h"

/*----------------------------------------------------------------------*/
/*  Command "scan" ---                                                  */
/*----------------------------------------------------------------------*/

int pcbtcl_scan(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "connection", "reset", NULL
   };
   enum OptionId {
      SCAN_CONNECTION, SCAN_RESET
   };
   static char *resetoptions[] = {
       "pads_pins_vias", "lines_polygons", "all", NULL
   };
   enum ResetOptionId {
       SCAN_RESET_PADS_PINS_VIAS, SCAN_RESET_LINES_POLYGONS, SCAN_RESET_ALL
   };

   int result, idx, ridx;

   /* Handle conflict with Tcl string "scan" command.  Because the PCB  */
   /* "scan" command syntax is so restricted, and the Tcl command is    */ 
   /* so unrestricted (any statement "scan x y" is valid), it works     */
   /* better to try the PCB command first.                              */

   if (objc == 1) {
       idx = SCAN_CONNECTION;
   } else if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
        options, "option", 0, &idx)) != TCL_OK) {
       if (objc >= 3) {
           Tcl_SavedResult state;
           Tcl_SaveResult(interp, &state);
           result = resolve_conflict(clientData, interp, objc, objv);
           if (result != TCL_OK)
               Tcl_RestoreResult(interp, &state);
           else
               Tcl_DiscardResult(&state);
           return result;
       } else return result;
   }

   switch (idx)
   {
       case SCAN_CONNECTION:
           HideCrosshair (True);
           watchCursor ();
           LookupConnection (Crosshair.X, Crosshair.Y, True, TO_PCB (1));
           restoreCursor ();
           RestoreCrosshair (True);	
           break;

      case SCAN_RESET:
           if (objc < 3) {
              Tcl_WrongNumArgs(interp, objc, objv, "<option>");
              return TCL_ERROR;
           }
           if ((result = Tcl_GetIndexFromObj(interp, objv[2], (CONST84 char **)
                    resetoptions, "reset option", 0, &ridx)) != TCL_OK)
               return result;

           switch (ridx)
           {
               case SCAN_RESET_PADS_PINS_VIAS:
                   ResetFoundPinsViasAndPads (True);
                   break;

               case SCAN_RESET_LINES_POLYGONS:
                   ResetFoundLinesAndPolygons (True);
                   break;

               case SCAN_RESET_ALL:
                   SaveUndoSerialNumber ();
                   ResetFoundPinsViasAndPads (True);
                   RestoreUndoSerialNumber ();
                   ResetFoundLinesAndPolygons (True);
                   RedrawLayout();
                   break;
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"scan\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "select" ---                                                */
/*----------------------------------------------------------------------*/

int pcbtcl_select(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "element", "pad", "pin", "text", "via", "object", NULL
   };
   enum OptionId {
       SELECT_ELEMENT, SELECT_PAD, SELECT_PIN,
       SELECT_TEXT, SELECT_VIA, SELECT_OBJECT
   };
   static char *objectoptions[] = {
       "all", "connected", NULL
   };
   enum ObjectOptionId {
       SELECT_OBJECT_ALL, SELECT_OBJECT_CONNECTED
   };


   int result, idx, oidx, type=0;
   char *name = NULL;
   double dval = 0;
   BoxType fullLayout;

   if (objc < 2) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option> ...");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
               options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
          options, "option", 0, &idx)) != TCL_OK)
       return result;

   switch (idx)
   {
       case SELECT_ELEMENT:
       case SELECT_PAD:
       case SELECT_PIN:
       case SELECT_TEXT:
       case SELECT_VIA:
           if(idx == SELECT_ELEMENT) type = ELEMENT_TYPE;
           else if(idx == SELECT_PAD) type = PAD_TYPE;
           else if(idx == SELECT_PIN) type = PIN_TYPE;
           else if(idx == SELECT_TEXT) type = TEXT_TYPE;

           if(objc == 3) {
               name = Tcl_GetString(objv[2]);
               if(SelectObjectByName (type, name))
                   SetChangedFlag (True);
           } else {
               Tcl_SetResult(interp, "Usage : select [element|pad|pin|text|via] <name>", 0);
               return TCL_ERROR;
           }
           break;

      case SELECT_OBJECT:
           if (objc == 2) {
               HideCrosshair (True);
               if (SelectObject())
                   SetChangedFlag (True);
                   RestoreCrosshair (True);
           } else if (objc == 3) {
               name = Tcl_GetString(objv[2]);
               if (strcasecmp(name, "all") && strcasecmp(name, "connected")) {
                   /* Select object by Name */
                   type = ALL_TYPES;
                   if(SelectObjectByName (type, name))
                       SetChangedFlag (True);
                   break;
               }
               if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	                     (CONST84 char **) objectoptions, "object option", 
			     0, &oidx)) != TCL_OK)
                   return result;
               switch (oidx)
               {
                   case SELECT_OBJECT_ALL:
                       fullLayout.X1 = 0;
                       fullLayout.Y1 = 0;
                       fullLayout.X2 = PCB->MaxWidth;
                       fullLayout.Y2 = PCB->MaxHeight;
                       SelectBlock(&fullLayout, True);
                       SetChangedFlag (True);
                       break;

                   case SELECT_OBJECT_CONNECTED:
                       if (SelectConnection(True)) {
                           IncrementUndoSerialNumber ();
                           SetChangedFlag (True);
                       }
                       break;
               }
           } else if (objc == 4) {
               // kludgy - set the crosshair location from the given coods
               // if not, SelectObject needs to be modified to accept coods
               if (Tcl_GetDoubleFromObj(interp, objv[2], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   Crosshair.X = (Location)(dval*100);
               if (Tcl_GetDoubleFromObj(interp, objv[3], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   Crosshair.Y = (Location)(dval*100);
               HideCrosshair (True);
               if (SelectObject())
                   SetChangedFlag (True);
               RestoreCrosshair (True);

           } else {
               Tcl_SetResult(interp, "Usage : select object [[all|connected]|<name>|<X><Y>]", 0);
               return TCL_ERROR;
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"select\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/*  Command "show" ---                                                  */
/*----------------------------------------------------------------------*/

int pcbtcl_show(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "element_name", "solder_mask", "solder_side", "vias", "pins_pads", 
       "far_side", "rats", NULL
   };
   enum OptionId {
      SHOW_ELEMENTNAME, SHOW_SOLDERMASK, SHOW_SOLDERSIDE,
      SHOW_VIAS, SHOW_PINS_PADS, SHOW_FAR_SIDE, SHOW_RATS
   };

   static char *enameoptions[] = {
       "description", "reference", "value", NULL
   };
   enum ENameOptionId {
      SHOW_ELEMENTNAME_DESCRIPTION, SHOW_ELEMENTNAME_REFERENCE,
      SHOW_ELEMENTNAME_VALUE
   };

   int result, idx, subidx;
   char *value = NULL;
   Boolean bVal = False;
   Location x, y;
   char rStr[20];
   Tcl_Obj *sobj;

   if (objc == 1) {
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
              options, "option", 0, &idx);
       return TCL_ERROR;
   }
   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
       options, "option", 0, &idx)) != TCL_OK)
       return result;

   if (objc == 3) {
       value = Tcl_GetString(objv[2]);
       if (!strcasecmp(value, "on"))
            bVal = True;
   }

   switch (idx)
   {
       case SHOW_ELEMENTNAME:
           /* Process element name to be display
            * according to given option.
            */
           if(objc == 3) {
               if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	                     (CONST84 char **) enameoptions, 
			     "element_name option", 0, &subidx)) != TCL_OK)
                   return result;

               HideCrosshair (True);
               ELEMENT_LOOP (PCB->Data);
               {
                   EraseElementName (element);
               }
               END_LOOP;
               CLEAR_FLAG (DESCRIPTIONFLAG | NAMEONPCBFLAG, PCB);
               switch (subidx)
               {
                   case SHOW_ELEMENTNAME_VALUE:
                       break;
                   case SHOW_ELEMENTNAME_REFERENCE:
                       SET_FLAG (NAMEONPCBFLAG, PCB);
                       break;
                   case SHOW_ELEMENTNAME_DESCRIPTION:
                       SET_FLAG (DESCRIPTIONFLAG, PCB);
                       break;
                   default :
                       Tcl_SetResult(interp, "Valid options : [description|\
                             reference|value]", 0);
                       RestoreCrosshair (True);
                       return TCL_ERROR;
                }

                ELEMENT_LOOP (PCB->Data);
                {
                    DrawElementName (element, 0);
                }
                END_LOOP;
                Draw ();
                RestoreCrosshair (True);
           } else {
               if (TEST_FLAG (NAMEONPCBFLAG, PCB))
                   strcpy(rStr,"reference");
               else if (TEST_FLAG (DESCRIPTIONFLAG, PCB))
                   strcpy(rStr,"description");
               else
                   strcpy(rStr,"value");

               sobj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, sobj);
           }
           break;

       case SHOW_SOLDERMASK:
           if (objc == 3) {
               if (bVal)
                   SET_FLAG(SHOWMASKFLAG, PCB);
               else
                   CLEAR_FLAG(SHOWMASKFLAG, PCB);
               UpdateAll();
           } else {
               /* If no option given then toggle soldermask flag.
                * -- Required for Screen->View soldermask menu.
                */
               //TOGGLE_FLAG(SHOWMASKFLAG, PCB);
               //UpdateAll();
               if (TEST_FLAG(SHOWMASKFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               sobj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, sobj);
           }
           break;

       case SHOW_SOLDERSIDE:
           /* Reference :ActionSwapSides() routine. */
           if (objc == 3) {
               if (!strcmp(value, "toggle" ))
                   Settings.ShowSolderSide = !Settings.ShowSolderSide;
               else
                   Settings.ShowSolderSide = bVal;

               x = TO_SCREEN_X (Crosshair.X);
               y = TO_SCREEN_Y (Crosshair.Y);
               SwapBuffers ();

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
           } else {
               if (Settings.ShowSolderSide)
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               sobj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, sobj);
           }
           break;

       case SHOW_VIAS:
           if (objc == 3)
               PCB->ViaOn = bVal;
           else {
               if (PCB->ViaOn == True)
                   PCB->ViaOn = False;
               else
                   PCB->ViaOn = True;
           }
           if (PCB->Data->ViaN != 0)
               UpdateAll();
           break;

      case SHOW_PINS_PADS:
           if (objc == 3)
              PCB->PinOn = bVal;
           else {
               if (PCB->PinOn == True)
                   PCB->PinOn = False;
               else
                   PCB->PinOn = True;
           }
           if (PCB->Data->ElementN != 0)
               UpdateAll();
           break;

       case SHOW_FAR_SIDE:
           if (objc == 3)
               PCB->InvisibleObjectsOn = bVal;
           else {
               if (PCB->InvisibleObjectsOn == True)
                   PCB->InvisibleObjectsOn = False;
               else
                   PCB->InvisibleObjectsOn = True;
           }
           PCB->Data->BACKSILKLAYER.On = (PCB->InvisibleObjectsOn &&
                                                        PCB->ElementOn);
           UpdateAll();
           break;

       case SHOW_RATS:
           if (objc == 3)
               PCB->RatOn = bVal;
           else {
               if (PCB->RatOn == True)
                   PCB->RatOn = False;
               else
                   PCB->RatOn = True;
           }
           if (PCB->Data->RatN != 0)
               UpdateAll();
           break;
       default :
           Tcl_SetResult(interp, "Unimplemented \"show\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "style" ---                                                 */
/*----------------------------------------------------------------------*/

int
getStyleNum(char *styleName)
{
   STYLE_LOOP (PCB);
   {
       if (strcasecmp (styleName, PCB->RouteStyle[n].Name) == 0)
       return n+1;
   }
   END_LOOP;
   return 0;
}

int pcbtcl_style(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "adjust", "clearance", "line_width", "name", "use", "via_hole",
       "via_size", "clearance_range", "line_width_range", "via_hole_range",
        "via_size_range", "info", "number", NULL
   };
   enum OptionId {
      STYLE_ADJUST, STYLE_CLEARANCE, STYLE_LINE_WIDTH,
      STYLE_NAME, STYLE_USE, STYLE_VIA_HOLE, STYLE_VIA_SIZE, 
      STYLE_CLEARANCE_RANGE, STYLE_LINE_WIDTH_RANGE,
      STYLE_VIA_HOLE_RANGE, STYLE_VIA_SIZE_RANGE, STYLE_INFO, STYLE_NUMBER
   };

   static int currentStyleNum;
   int result, idx;
   char *name = NULL, *newValue = NULL, data[512];
   int styleNum = 0;
   Boolean isModified = False;		
   BDimension minDim, maxDim, ival;
   Tcl_Obj *sobj;

   if (objc < 2) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option> ...");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
               options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if (objc >= 3)
       name = Tcl_GetString(objv[1]);

   if (objc >= 3  && ((styleNum = getStyleNum(name)) > 0 ||
          (Tcl_GetIntFromObj(interp, objv[1], &styleNum) == TCL_OK))) {
       if(styleNum >0 && styleNum <=  NUM_STYLES) {
           if ((result = Tcl_GetIndexFromObj(interp, objv[2], (CONST84 char **)
                  options, "option", 0, &idx)) != TCL_OK)
               return result;
       } else {
           Tcl_SetResult(interp, "No such style! ", 0);
           return TCL_ERROR;
       }
   } else {
       if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
             options, "option", 0, &idx)) != TCL_OK)
           return result;
   }

   switch (idx)
   {
       case STYLE_ADJUST:
           if(objc == 3) {
               name = Tcl_GetString(objv[2]);
               styleNum = getStyleNum(name);
               if(styleNum) {
                   sprintf(data,"Style : %s, Line Width : %d. New Line Width :",
                           PCB->RouteStyle[styleNum - 1].Name,
                           PCB->RouteStyle[styleNum - 1].Thick);
                           PCB->RouteStyle[styleNum - 1].Thick = (BDimension)
                           atoi(GetConsoleInput (data));

                   sprintf(data, "Style : %s, Via Hole : %d. New Via Hole :",
                           PCB->RouteStyle[styleNum - 1].Name,
                           PCB->RouteStyle[styleNum - 1].Hole);
                           PCB->RouteStyle[styleNum - 1].Hole = (BDimension)
                           atoi(GetConsoleInput (data));

                   sprintf(data, "Style : %s, Via Size : %d. New Via Size :",
                           PCB->RouteStyle[styleNum - 1].Name,
                           PCB->RouteStyle[styleNum - 1].Diameter);
                           PCB->RouteStyle[styleNum - 1].Diameter = (BDimension)
                           atoi(GetConsoleInput (data));

                   sprintf(data, "Style : %s, Clearance Size : %d. New Clearance Size :",
                           PCB->RouteStyle[styleNum - 1].Name,
                           PCB->RouteStyle[styleNum - 1].Keepaway);
                           PCB->RouteStyle[styleNum - 1].Keepaway = (BDimension)
                           atoi(GetConsoleInput (data));


                   name = GetConsoleInput ("New name of Style : ");
                   if(name) {
                       SaveFree (PCB->RouteStyle[styleNum - 1].Name);
                       PCB->RouteStyle[styleNum-1].Name = StripWhiteSpaceAndDup(name);
                   }
                   isModified = True;
               } else {
                   Tcl_SetResult(interp, "No such style! ", 0);
                   return TCL_ERROR;
               }
           } else {
               Tcl_WrongNumArgs(interp, objc, objv, "<style-name>");
               return TCL_ERROR;
           }
           break;

       case STYLE_CLEARANCE:
           if(objc == 4) {
               if (Tcl_GetIntFromObj(interp, objv[3], &ival) == TCL_OK) {
                  if(ival >= MIN_LINESIZE && ival <= MAX_LINESIZE) {
                       PCB->RouteStyle[styleNum - 1].Keepaway=(BDimension)ival;
                       isModified = True;
                  } else {
                       Tcl_SetResult(interp, "Value out of range.", 0);
                       return TCL_ERROR;
                  }
               }
               /* Reset style in use if parameter of selected style modified */
               if (currentStyleNum == styleNum) currentStyleNum = 0;
               break;
           } else if(objc == 2)
               styleNum = currentStyleNum;

           if (styleNum) {
               /* Return Size of Clearance */
               sobj = Tcl_NewIntObj(PCB->RouteStyle[styleNum - 1].Keepaway);
               Tcl_SetObjResult(interp, sobj);
           } else {
               Tcl_SetResult(interp, "No style in use. Try \" style use <style-name>\"", 0);
           }
           break;

       case STYLE_LINE_WIDTH:
           if(objc == 4) {
               if (Tcl_GetIntFromObj(interp, objv[3], &ival) == TCL_OK) {
                   if(ival >= MIN_LINESIZE && ival <= MAX_LINESIZE) {
                       PCB->RouteStyle[styleNum - 1].Thick = (BDimension)ival;
                       isModified = True;
                   } else {
                       Tcl_SetResult(interp, "Value out of range.", 0);
                       return TCL_ERROR;
                   }
               }
               /* Reset style in use if parameter of selected style modified */
               if(currentStyleNum == styleNum) currentStyleNum = 0;
               break;
           } else if(objc == 2)
               styleNum = currentStyleNum;

           if(styleNum) {
               /* Return Size of Line Thickness */
               sobj = Tcl_NewIntObj(PCB->RouteStyle[styleNum - 1].Thick);
               Tcl_SetObjResult(interp, sobj);
           } else {
               Tcl_SetResult(interp, "No style in use. Try \" style use <style-name>\"", 0);
           }
           break;

       case STYLE_NAME:
           if(objc == 4) {
               newValue = Tcl_GetString(objv[3]);
               SaveFree (PCB->RouteStyle[styleNum - 1].Name);
               PCB->RouteStyle[styleNum - 1].Name = StripWhiteSpaceAndDup(newValue);
               isModified = True;
               break;
           } else if(objc == 2)
               styleNum = currentStyleNum;

           if(styleNum) {
               sprintf(data, "%s",
                    styleNum ? PCB->RouteStyle[styleNum - 1].Name : "NONE");
               sobj = Tcl_NewStringObj(data,-1);
               Tcl_SetObjResult(interp, sobj);
           } else {
               Tcl_SetResult(interp, "No style in use. Try \" style use <style-name>\"", 0);
           }
           break;

       case STYLE_USE:
           if(objc == 2) {
               styleNum = FlagCurrentStyle();
               if(styleNum) {
                   sobj = Tcl_NewStringObj(PCB->RouteStyle[styleNum -1].Name,
		                           -1);
                   Tcl_SetObjResult(interp, sobj);
               } else {
                   Tcl_SetResult(interp, "No style in use. Try \" style use <style-name>\"", 0);
               }
           } else if(objc == 3) {
               if (Tcl_GetIntFromObj(interp, objv[2], &styleNum) != TCL_OK) {
                   name = Tcl_GetString(objv[2]);
                   styleNum = getStyleNum(name);
               }
               if (styleNum > 0 && styleNum <= NUM_STYLES) {
                   currentStyleNum = styleNum;
                   SetLineSize (PCB->RouteStyle[styleNum -1].Thick);
                   SetViaSize (PCB->RouteStyle[styleNum -1].Diameter, True);
                   SetViaDrillingHole (PCB->RouteStyle[styleNum -1].Hole, True);
                   SetKeepawayWidth (PCB->RouteStyle[styleNum -1].Keepaway);
                   isModified = True;
               } else {
                  Tcl_SetResult(interp, "No such style! ", 0);
                   return TCL_ERROR;
               }
           } else {
              Tcl_SetResult(interp, "Usage : style use \"<style-name>\"", 0);
              return TCL_ERROR;
           }
           break;

       case STYLE_VIA_HOLE:
           if(objc == 4) {
               if (Tcl_GetIntFromObj(interp, objv[3], &ival) == TCL_OK) {
                   if(ival >= MIN_PINORVIAHOLE &&
                      ival <= (PCB->RouteStyle[styleNum - 1].Diameter -
                      MIN_PINORVIACOPPER)) {
                       PCB->RouteStyle[styleNum - 1].Hole = (BDimension)ival;
                       isModified = True;
                   } else {
                       Tcl_SetResult(interp, "Value out of range.", 0);
                       return TCL_ERROR;
                   }
               }
               /* Reset style in use if parameter of selected style modified */
               if (currentStyleNum == styleNum) currentStyleNum = 0;
               break;
           } else if(objc == 2)
               styleNum = currentStyleNum;

           if(styleNum) {
               /* Return Size of Via Hole */
               sobj = Tcl_NewIntObj(PCB->RouteStyle[styleNum - 1].Hole);
               Tcl_SetObjResult(interp, sobj);
           } else {
               Tcl_SetResult(interp, "No style in use. Try \" style use <style-name>\"", 0);
           }
           break;

      case STYLE_VIA_SIZE:
           if(objc == 4) {
               if (Tcl_GetIntFromObj(interp, objv[3], &ival) == TCL_OK) {
                   if(ival <= MAX_PINORVIASIZE &&
                           ival >= (PCB->RouteStyle[styleNum - 1].Hole +
                           MIN_PINORVIACOPPER)) {
                       PCB->RouteStyle[styleNum - 1].Diameter=(BDimension)ival;
                       isModified = True;
                   } else {
                       Tcl_SetResult(interp, "Value out of range.", 0);
                       return TCL_ERROR;
                   }
               }
               /* Reset style in use if parameter of selected style modified */
               if(currentStyleNum == styleNum) currentStyleNum = 0;
                   break;
           } else if(objc == 2)
               styleNum = currentStyleNum;

           if(styleNum) {
               /* Return Size of Via */
               sobj = Tcl_NewIntObj(PCB->RouteStyle[styleNum - 1].Diameter);
               Tcl_SetObjResult(interp, sobj);
           } else {
               Tcl_SetResult(interp, "No style in use. Try \" style use <style-name>\"", 0);
           }
           break;

       case STYLE_CLEARANCE_RANGE:
       case STYLE_LINE_WIDTH_RANGE:
           minDim = MIN_LINESIZE;
           maxDim = MAX_LINESIZE;
           sprintf(data, "%d %d", minDim, maxDim);
           sobj = Tcl_NewStringObj(data, -1);
           Tcl_SetObjResult(interp, sobj);
           break;

       case STYLE_VIA_HOLE_RANGE:
           if(objc == 2)
               styleNum = currentStyleNum;

           if(styleNum) {
               minDim = MIN_PINORVIAHOLE;
               maxDim = PCB->RouteStyle[styleNum - 1].Diameter - 
	                MIN_PINORVIACOPPER;
               sprintf(data, "%d %d", minDim, maxDim);
               sobj = Tcl_NewStringObj(data, -1);
               Tcl_SetObjResult(interp, sobj);
           } else {
               Tcl_SetResult(interp, "No style in use. Try \" style use <style-name>\"", 0);
           }
           break;

      case STYLE_VIA_SIZE_RANGE:
           if(objc == 2)
               styleNum = currentStyleNum;

           if(styleNum) {
               minDim = PCB->RouteStyle[styleNum - 1].Hole + MIN_PINORVIACOPPER;
               maxDim = MAX_PINORVIASIZE;
               sprintf(data, "%d %d", minDim, maxDim);
               sobj = Tcl_NewStringObj(data, -1);
               Tcl_SetObjResult(interp, sobj);
           } else {
               Tcl_SetResult(interp, "No style in use. Try \" style use <style-name>\"", 0);
           }
           break;

       case STYLE_INFO:
           sobj = Tcl_NewStringObj(0, 0);
           STYLE_LOOP (PCB);
           {
               sprintf(data, "\"%s %d %d %d %d\"", style->Name, style->Thick,
                          style->Diameter, style->Hole, style->Keepaway);
                          Tcl_AppendStringsToObj(sobj, data, " ", 0);
           }
           END_LOOP;
           Tcl_SetObjResult(interp, sobj);
           break;

       case STYLE_NUMBER:
           if (objc == 2) {
               styleNum = FlagCurrentStyle();
               sobj = Tcl_NewIntObj(styleNum);
               Tcl_SetObjResult(interp, sobj);
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"style\" option", 0);
           return TCL_ERROR;
   }

   if(isModified)
       SetChangedFlag (True);
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "text" ---                                                  */
/*----------------------------------------------------------------------*/

int pcbtcl_text(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "draw", "scale", "scale_range", "selected_size", NULL
   };
   enum OptionId {
      TEXT_DRAW, TEXT_SCALE, TEXT_SCALE_RANGE, TEXT_SELECTED_SIZE
   };

   int result, idx, x, y;
   char *value = NULL;
   char tmpVal[128], data[512];
   double dval = 0;
   TextTypePtr text;
   BDimension size = 0, minDim, maxDim;
   Boolean isAbsolute = True;
   Tcl_Obj *tobj;

   /* conflict with Tk widget "text" command */
   if ((result = resolve_conflict(clientData, interp, objc, objv)) == TCL_OK)
       return result;

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
       case TEXT_DRAW:
           if (objc == 5) {
               value = Tcl_GetString(objv[2]);
               if (Tcl_GetDoubleFromObj(interp, objv[3], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   x = (Location)(dval*100);
               if (Tcl_GetDoubleFromObj(interp, objv[4], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   y = (Location)(dval*100);
           } else if(objc == 3) {
               value = Tcl_GetString(objv[2]);
               x = Crosshair.X;
               y = Crosshair.Y;
           } else {
               Tcl_SetResult(interp, "Usage : text draw <string> \"<x><y>\"", 0);
               return TCL_ERROR; 	
       }
       {
           int flag = NOFLAG;

           if (GetLayerGroupNumberByNumber (INDEXOFCURRENT) ==
               GetLayerGroupNumberByNumber (MAX_LAYER + SOLDER_LAYER))
               flag = ONSOLDERFLAG;

           if ((text = CreateNewText (CURRENT, &PCB->Font, x, y, 0,
                             Settings.TextScale, value, flag))) {
               AddObjectToCreateUndoList (TEXT_TYPE, CURRENT, text, text);
               IncrementUndoSerialNumber();
               //ClearAndRedrawOutput();
               DrawText (CURRENT, text, 0);
               Draw ();
           }
       }
       break;

       case TEXT_SELECTED_SIZE:
           /* Change size of selected Text */
           if (objc >= 3) {
               value = Tcl_GetString(objv[2]);
               if ((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;
               strcpy(tmpVal, value);

               if (objc == 3)
                   strtok(tmpVal, "m");
               else if (objc == 4)
                   value = Tcl_GetString(objv[3]);

               if (strstr(value, "mm"))
                   size = atof(tmpVal)/COOR_TO_MM;
               else if (strstr(value, "mil"))
                   size =  atof(tmpVal)*100;
               else
                   size =  atof(tmpVal);

               /* Refer line no. 153, 154 in pcb-menu.h file */
               HideCrosshair (True);
               if (ChangeSelectedSize (TEXT_TYPE, size, isAbsolute))
                   SetChangedFlag (True);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : text selected_size <\"[+|-]\"value> \"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case TEXT_SCALE:
           /* Change size of text */
           if (objc == 2) {
               /* Return Size of Text. */
               tobj = Tcl_NewIntObj(Settings.TextScale);
               Tcl_SetObjResult(interp, tobj);
           } else if (objc == 3) {
               value = Tcl_GetString(objv[2]);
               if ((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;
               size = (BDimension)atoi(value);

               HideCrosshair(True);
               SetTextScale (isAbsolute ? size : size + Settings.TextScale);
               RestoreCrosshair(True);
           } else {
               Tcl_SetResult(interp, "Usage : text scale <\"[+|-]\"value>", 0);
               return TCL_ERROR;
           }
           break;

       case TEXT_SCALE_RANGE:
           minDim = MIN_TEXTSCALE;
           maxDim = MAX_TEXTSCALE;
           sprintf(data, "%d %d", minDim, maxDim);
           tobj = Tcl_NewStringObj(data, -1);
           Tcl_SetObjResult(interp, tobj);
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"text\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "thermal" ---                                               */
/*----------------------------------------------------------------------*/

int pcbtcl_thermal(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "toggle", NULL
   };
   enum OptionId {
      THERMAL_TOGGLE
   };

   int result, idx, X, Y;
   int type;
   int LayerThermFlag;
   int LayerPIPFlag = L0PIPFLAG << INDEXOFCURRENT;
   void *ptr1, *ptr2, *ptr3;

   if (objc != 4) {
       Tcl_WrongNumArgs(interp, objc, objv, "toggle <x> <y>");
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
       options, "option", 0, &idx)) != TCL_OK)
       return result;

   switch (idx)
   {
       case THERMAL_TOGGLE:
           if (Tcl_GetIntFromObj(interp, objv[2], &X) != TCL_OK)
               return TCL_ERROR;
           if (Tcl_GetIntFromObj(interp, objv[3], &Y) != TCL_OK)
               return TCL_ERROR;
           
           type = SearchScreen (X, Y, PIN_TYPES, &ptr1, &ptr2, &ptr3);
           if ((type != NO_TYPE)
                 && TEST_FLAG (LayerPIPFlag, (PinTypePtr) ptr3)
                 && !TEST_FLAG (HOLEFLAG, (PinTypePtr) ptr3)) {
               AddObjectToFlagUndoList (type, ptr1, ptr2, ptr3);
               LayerThermFlag = L0THERMFLAG << INDEXOFCURRENT;
               TOGGLE_FLAG (LayerThermFlag, (PinTypePtr) ptr3);
               IncrementUndoSerialNumber ();
               ClearPin ((PinTypePtr) ptr3, type, 0);
               Draw ();
       }
       break;

      default :
           Tcl_SetResult(interp, "Unimplemented \"thermal\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "undo" ---                                                  */
/*----------------------------------------------------------------------*/

int pcbtcl_undo(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   char *value = NULL;

   if (objc == 1) {
      /* don't allow undo in the middle of an operation */
       if (Crosshair.AttachedObject.State != STATE_FIRST)
           return PCBTagCallback(interp, objc, objv);
       if (Crosshair.AttachedBox.State != STATE_FIRST
              && Settings.Mode != ARC_MODE)
           return PCBTagCallback(interp, objc, objv);
       /* undo the last operation */

       HideCrosshair (True);
       if (Settings.Mode == POLYGON_MODE && Crosshair.AttachedPolygon.PointN) {
           GoToPreviousPoint ();
           RestoreCrosshair (True);
           return PCBTagCallback(interp, objc, objv);
       }
       /* move anchor point if undoing during line creation */
       if (Settings.Mode == LINE_MODE) {
           if (Crosshair.AttachedLine.State == STATE_SECOND) {
               if (TEST_FLAG (AUTODRCFLAG, PCB))
                   Undo (True);                 /* undo the connection find */
               Crosshair.AttachedLine.State = STATE_FIRST;
               SetLocalRef (0, 0, False);
               RestoreCrosshair (True);
               return PCBTagCallback(interp, objc, objv);
           }
           if (Crosshair.AttachedLine.State == STATE_THIRD) {
               int type;
               void *ptr1, *ptr2, *ptr3;
               LineTypePtr lptr2;

               /* this search is guranteed to succeed */
               SearchObjectByLocation (LINE_TYPE, &ptr1, (void **) &ptr2,
                        &ptr3, Crosshair.AttachedLine.Point1.X,
                        Crosshair.AttachedLine.Point1.Y, 0);
               /* save both ends of line */
               lptr2 = (LineTypePtr)ptr2;
               Crosshair.AttachedLine.Point2.X = lptr2->Point1.X;
               Crosshair.AttachedLine.Point2.Y = lptr2->Point1.Y;
               if ((type = Undo (True)))
                   SetChangedFlag (True);
                   /* check that the undo was of the right type */
                       if ((type & UNDO_CREATE) == 0) {
                           /* wrong undo type, restore anchor points */
                           Crosshair.AttachedLine.Point2.X =
                               Crosshair.AttachedLine.Point1.X;
                           Crosshair.AttachedLine.Point2.Y =
                               Crosshair.AttachedLine.Point1.Y;
                           RestoreCrosshair (True);
                           return PCBTagCallback(interp, objc, objv);
                       }
                       /* move to new anchor */
                       Crosshair.AttachedLine.Point1.X =
                           Crosshair.AttachedLine.Point2.X;
                       Crosshair.AttachedLine.Point1.Y =
                           Crosshair.AttachedLine.Point2.Y;
                       /* check if an intermediate point was removed */
                       if (type & UNDO_REMOVE) {
                           /* this search should find the restored line */
                           SearchObjectByLocation (LINE_TYPE, &ptr1, &ptr2, 
			       &ptr3, Crosshair.AttachedLine.Point2.X,
                               Crosshair.AttachedLine.Point2.Y, 0);
                           Crosshair.AttachedLine.Point1.X =
                               Crosshair.AttachedLine.Point2.X=lptr2->Point2.X;
                           Crosshair.AttachedLine.Point1.Y =
                               Crosshair.AttachedLine.Point2.Y=lptr2->Point2.Y;
                       }
                       FitCrosshairIntoGrid (Crosshair.X, Crosshair.Y);
                       AdjustAttachedObjects ();

                   if (--addedLines == 0) {
                       Crosshair.AttachedLine.State = STATE_SECOND;
                       lastLayer = CURRENT;
                   } else {
                       /* this search is guranteed to succeed too */
                       SearchObjectByLocation (LINE_TYPE, &ptr1, 
		           (void **) &ptr2, &ptr3,
                           Crosshair.AttachedLine.Point1.X,
                           Crosshair.AttachedLine.Point1.Y, 0);
                       lastLayer = (LayerTypePtr) ptr1;
                   }
                   RestoreCrosshair (True);
                   return PCBTagCallback(interp, objc, objv);
           }
       }
       if (Settings.Mode == ARC_MODE) {
           if (Crosshair.AttachedBox.State == STATE_SECOND) {
               Crosshair.AttachedBox.State = STATE_FIRST;
               RestoreCrosshair (True);
               return PCBTagCallback(interp, objc, objv);
           }
           if (Crosshair.AttachedBox.State == STATE_THIRD) {
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
       } else if (objc == 2) {
           value = Tcl_GetString(objv[1]);
           if (!strcasecmp(value, "clear"))
               ClearUndoList (False);
       }
       RestoreCrosshair (True);

   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "via" ---                                                   */
/*----------------------------------------------------------------------*/

int pcbtcl_via(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "draw", "hole", "hole_range", "selected_hole", "selected_size",
       "size", "size_range", "toggle_visible", "clearance",
       "selected_clearance", "selected_plug", NULL
   };
   enum OptionId {
      VIA_DRAW, VIA_HOLE, VIA_HOLE_RANGE, VIA_SELECTED_HOLE,
      VIA_SELECTED_SIZE, VIA_SIZE, VIA_SIZE_RANGE, VIA_TOGGLE_VISIBLE,
      VIA_CLEARANCE, VIA_SELECTED_CLEARANCE, VIA_SELECTED_PLUG
   };

   int result, idx, x, y;
   char *value = NULL;
   char tmpVal[128];
   double dval = 0;
   PinTypePtr via;
   BDimension minDim, maxDim, size = 0;
   Boolean isAbsolute = True, bval;
   Tcl_Obj *vobj;

   if (objc == 1) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option>");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
               options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
           options, "via option", 0, &idx)) != TCL_OK)
        return result;

   if((idx == VIA_HOLE || idx == VIA_SELECTED_HOLE ||
         idx == VIA_SIZE || idx == VIA_SELECTED_SIZE ||
	 idx == VIA_CLEARANCE || idx == VIA_SELECTED_CLEARANCE)
	 && (objc >=3)) {
       value = Tcl_GetString(objv[2]);
       if((value[0] == '-')  || (value[0] == '+'))
           isAbsolute = False;

       strcpy(tmpVal, value);
       if (objc == 3)
           strtok(tmpVal, "m");
       else if (objc == 4)
           value = Tcl_GetString(objv[3]);

       if (strstr(value, "mm"))
           size = (BDimension)(0.5 + atof(tmpVal) / COOR_TO_MM);
       else if(strstr(value, "mil"))
           size = (BDimension)(0.5 + atof(tmpVal) * 100);
       else
           size = (BDimension)(0.5 + atof(tmpVal));
   }

   switch (idx)
   {
       case VIA_DRAW:
           /* If (x,y) values not given use CrossHair data */
           if (objc == 4) {
               if (Tcl_GetDoubleFromObj(interp, objv[2], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   x = (Location)(dval*100);
               if (Tcl_GetDoubleFromObj(interp, objv[3], &dval) != TCL_OK)
                   return TCL_ERROR;
               else
                   y = (Location)(dval*100);
           } else {
               x = Crosshair.X;
               y = Crosshair.Y;
           }

           if (!PCB->ViaOn) {
               Tcl_SetResult(interp, "You must turn via visibility on before \
                               you can place vias", 0);
               return TCL_ERROR; 	
           }
           if ((via = CreateNewVia (PCB->Data, x, y,
                         Settings.ViaThickness, 2 * Settings.Keepaway, 0,
                         Settings.ViaDrillingHole, NULL, VIAFLAG)) != NULL) {
               UpdatePIPFlags (via, (ElementTypePtr) via, NULL, False);
               AddObjectToCreateUndoList (VIA_TYPE, via, via, via);
               IncrementUndoSerialNumber ();
               DrawVia (via, 0);
               Draw ();
           }
           break;

       case VIA_HOLE:
           /* Report/Changes the drilling hole size of via */
           if (objc == 2) {
               /* Return Size of drilling hole size of via */
               vobj = Tcl_NewIntObj(Settings.ViaDrillingHole);
               Tcl_SetObjResult(interp, vobj);
           } else if (objc >= 3) {
               HideCrosshair (True);
               SetViaDrillingHole (isAbsolute ? size : size + 
	                           Settings.ViaDrillingHole, False);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : via hole  <\"[+|-]\"value> \"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case VIA_SELECTED_HOLE:
           /* Changes the Drilling hole size of selected objects */
           if (objc >= 3) {
               HideCrosshair (True);
               if (ChangeSelected2ndSize (VIA_TYPE, size, isAbsolute))
                   SetChangedFlag (True);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : via hole \"seleted\" <\"[+|-]\"value> \"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case VIA_SELECTED_SIZE:
           /* Change size of selected Via */
           if (objc >= 3) {
               /* Refer line no. 154, 156 in pcb-menu.h file */	
               HideCrosshair (True);
               if (ChangeSelectedSize (VIA_TYPE, size, isAbsolute))
                   SetChangedFlag (True);
                   RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : via selected_size "
			"<\"[+|-]\"value> \"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case VIA_SIZE:
           /* Report/Change size of Via */
           if (objc == 2) {
               /* Return Size of drilling hole size of via */
               vobj = Tcl_NewIntObj(Settings.ViaThickness);
               Tcl_SetObjResult(interp, vobj);
           } else if (objc >= 3) {
               HideCrosshair(True);
               SetViaSize (isAbsolute ? size : size + Settings.ViaThickness, 
	                   True);	/* Force lower than minimum if necessary */
               RestoreCrosshair(True);
           } else {
               Tcl_SetResult(interp, "Usage : via size <\"[+|-]\"value> "
			"\"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case VIA_SELECTED_CLEARANCE:
           /* Change clearance around selected Via */
           if (objc >= 3) {
               /* Refer line no. 154, 156 in pcb-menu.h file */	
               HideCrosshair (True);
               if (ChangeSelectedClearSize (VIA_TYPE, size, isAbsolute))
                   SetChangedFlag (True);
                   RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : via selected_clearance "
			"<\"[+|-]\"value> \"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case VIA_CLEARANCE:
           /* Report/Change clearance around Via */
           if (objc == 2) {
               /* Return Size of clearance around via */
               vobj = Tcl_NewIntObj(Settings.Keepaway);
               Tcl_SetObjResult(interp, vobj);
           } else if (objc >= 3) {
               HideCrosshair(True);
               SetKeepawayWidth (isAbsolute ? size : size + Settings.Keepaway);
               RestoreCrosshair(True);
           } else {
               Tcl_SetResult(interp, "Usage : via clearance <\"[+|-]\"value> "
			"\"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case VIA_TOGGLE_VISIBLE:
           if (PCB->ViaOn == True)
               PCB->ViaOn = False;
           else
               PCB->ViaOn = True;
           UpdateAll();
           break;

       case VIA_SELECTED_PLUG:
           /* Change plug flag for selected Via */
           if (objc >= 2) {
               HideCrosshair (True);
               if (ChangeSelectedPlugs(VIA_TYPE))
                   SetChangedFlag (True);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : via selected_plug", 0);
               return TCL_ERROR;
           }
           break;

       case VIA_HOLE_RANGE:
           minDim = MIN_PINORVIAHOLE;
           maxDim = Settings.ViaThickness - MIN_PINORVIACOPPER;
           sprintf(tmpVal, "%d %d", minDim, maxDim);
           vobj = Tcl_NewStringObj(tmpVal, -1);
           Tcl_SetObjResult(interp, vobj);
           break;

      case VIA_SIZE_RANGE:
           minDim = Settings.ViaDrillingHole + MIN_PINORVIACOPPER;
           maxDim = MAX_PINORVIASIZE;
           sprintf(tmpVal, "%d %d", minDim, maxDim);
           vobj = Tcl_NewStringObj(tmpVal, -1);
           Tcl_SetObjResult(interp, vobj);
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"via\" option", 0);
           return TCL_ERROR;
    }
   return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/*  Command "zoom" ---                                                  */
/*----------------------------------------------------------------------*/

int pcbtcl_zoom(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "in", "out", "set", NULL
   };
   enum OptionId {
      ZOOM_IN, ZOOM_OUT, ZOOM_SET
   };

   int result, idx;
   char *value = NULL, error[100];
   float zoom = 0;
   Boolean rflag = False;
   Tcl_Obj *zobj;


   if (objc == 1) {
       /* Return the zoom factor */
       zobj = Tcl_NewDoubleObj((double)PCB->Zoom);
       Tcl_SetObjResult(interp, zobj);
       return TCL_OK;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
           options, "zoom option", 0, &idx)) != TCL_OK)
       return result;

   HideCrosshair (True);
   RestoreCrosshair (True);
   switch (idx)
   {
       case ZOOM_IN:
           SetZoom (PCB->Zoom - 1);
           break;

     case ZOOM_OUT:
           SetZoom (PCB->Zoom + 1);
           break;

     case ZOOM_SET:
           if (objc == 3) {
               value = Tcl_GetString(objv[2]);
               if((value[0] == '-')  || (value[0] == '+')) {
                   rflag = True;
                   zoom = atof(value);
               } else if(strcasecmp ("4:1",  value) == 0) {
                   zoom = -4;
               } else if(strcasecmp ("2:1",  value) == 0) {
                   zoom = -2;
               } else if(strcasecmp ("1:1",  value) == 0) {
                   zoom = 0;
               } else if(strcasecmp ("1:2",  value) == 0) {
                   zoom = 2;
               } else if(strcasecmp ("1:4",  value) == 0) {
                   zoom = 4;
               } else if(strcasecmp ("1:8",  value) == 0) {
                   zoom = 6;
               } else if(strcasecmp ("1:16", value) == 0) {
                   zoom = 8;
               } else {
                   zoom = atof(value);
               }

               if(zoom>MAX_ZOOM || zoom<MIN_ZOOM) {
                   sprintf(error, " Zoom value must between %d and %d",
                        MIN_ZOOM,MAX_ZOOM);
                   zobj = Tcl_NewStringObj(error,-1);
                   Tcl_SetObjResult(interp, zobj);
                   return TCL_ERROR;
               }
               SetZoom (rflag ? zoom + PCB->Zoom : zoom);
           } else {
               Tcl_SetResult(interp, "Usage :  zoom set [<n>:<d> | \"[+|-]\"<value>] ", 0);
               return TCL_ERROR;
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"zoom\" option", 0);
           return TCL_ERROR;
           break;
    }
   return PCBTagCallback(interp, objc, objv);
}

#endif /* TCL_PCB */
