/*----------------------------------------------------------------------------*/
/* tclcommandAF.c:                                                            */
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
#include <math.h>	 /* for zoom factor logf() function */

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
#include "remove.h"
#include "draw.h"
#include "error.h"
#include "action.h"
#include "search.h"
#include "gui.h"
#include "report.h"
#include "dialog.h"

#include "control.h"
#include "find.h"
#include "undo.h"
#include "change.h"
#include "select.h"
#include "copy.h"

#ifdef HAVE_REGEX_H
#include <regex.h>
#else
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

extern Boolean isDialog;
extern char* reportString;

#ifdef TCL_PCB

#include "tclpcb.h"

/*----------------------------------------------------------------------*/
/*  Command "about" ---                                                 */
/*----------------------------------------------------------------------*/

int pcbtcl_about(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
    Tcl_Obj *aobj = NULL;
    AboutDialog();
    if(reportString) {
        aobj = Tcl_NewStringObj(reportString, -1);
        Tcl_SetObjResult(interp, aobj);
        SaveFree (reportString);
    }
    return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/*  Command "arc" ---                                                   */
/*----------------------------------------------------------------------*/

int pcbtcl_arc(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "draw", NULL
   };
   enum OptionId {
       ARC_DRAW
   };

   int result, idx, llx, lly, urx, ury;
   double dval = 0;
   ArcTypePtr arc;
   Location wx, wy, X, Y;
   int sa, dir;

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
       case ARC_DRAW:
           if (objc == 6) {
             if (Tcl_GetDoubleFromObj(interp, objv[2], &dval) != TCL_OK)
                 return TCL_ERROR;
             else
                 llx = (Location)(dval*100);
             if (Tcl_GetDoubleFromObj(interp, objv[3], &dval) != TCL_OK)
                 return TCL_ERROR;
             else
                 lly = (Location)(dval*100);
             if (Tcl_GetDoubleFromObj(interp, objv[4], &dval) != TCL_OK)
                 return TCL_ERROR;
             else
                 urx = (Location)(dval*100);
             if (Tcl_GetDoubleFromObj(interp, objv[5], &dval) != TCL_OK)
                 return TCL_ERROR;
             else
                 ury = (Location)(dval*100);

             wx = urx - llx;
             wy = ury - lly;
             if (XOR (Crosshair.AttachedBox.otherway, abs (wy) > abs (wx))) {
                 X = llx + abs (wy) * SGNZ (wx);
                 Y = ury;                            /* Set Y */
                 sa = (wx >= 0) ? 0 : 180;
#ifdef ARC45
                 if (abs (wy) / 2 >= abs (wx))
                     dir = (SGNZ (wx) == SGNZ (wy)) ? 45 : -45;
                 else
#endif
                     dir = (SGNZ (wx) == SGNZ (wy)) ? 90 : -90;
             } else {
                 Y = lly + abs (wx) * SGNZ (wy);
                 X = llx;                            /* Set X */
                 sa = (wy >= 0) ? -90 : 90;
#ifdef ARC45
                 if (abs (wx) / 2 >= abs (wy))
                     dir = (SGNZ (wx) == SGNZ (wy)) ? -45 : 45;
                 else
#endif
                     dir = (SGNZ (wx) == SGNZ (wy)) ? -90 : 90;
                 wy = wx;
             }
             arc = CreateNewArcOnLayer (CURRENT, X, Y, abs (wy), sa, dir,
                 Settings.LineThickness, 2 * Settings.Keepaway,
                 TEST_FLAG(CLEARNEWFLAG, PCB) ? CLEARLINEFLAG :0);
             if ((abs (wy) > 0) && arc) {
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
             }
           } else {
               Tcl_SetResult(interp,"(tlx, tly) (brx, bry) values required.",0);
               return TCL_ERROR;
           }
           break;
      }
   return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/*  Command "buffer" ---                                                */
/*----------------------------------------------------------------------*/

int pcbtcl_buffer(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
      "add", "breakup", "clear", "convert", "copy", "cut",
      "mirror", "load", "paste", "rotate", "save", "select", NULL
   };
   enum OptionId {
      BUFFER_ADD, BUFFER_BREAKUP, BUFFER_CLEAR, BUFFER_CONVERT,
      BUFFER_COPY, BUFFER_CUT, BUFFER_MIRROR, BUFFER_LOAD,
      BUFFER_PASTE, BUFFER_ROTATE, BUFFER_SAVE, BUFFER_SELECT
   };
   static char *loadoptions[] = {
      "layout", "element", NULL
   };
   enum LoadOptionId {
      BUFFER_LOAD_LAYOUT, BUFFER_LOAD_ELEMENT
   };

   static char *rotateoptions[] = {
      "90", "180", "270", "-90", "-180", "-270", NULL
   };
   enum RotateOptionId {
      BUFFER_ROTATE_CW_90, BUFFER_ROTATE_CW_180, BUFFER_ROTATE_CW_270,
      BUFFER_ROTATE_CCW_90, BUFFER_ROTATE_CCW_180, BUFFER_ROTATE_CCW_270
   };

   int result, idx, lidx, ridx, buffernum, istart, locobjc;
   double dval = 0;
   char *value = NULL, *name = NULL;
   BufferTypePtr buffer;
   Location X = 0, Y = 0; /* Default value..AddSelectedToBuffer()
                             routine will use CROSSHAIR data in that case. */

   if (objc == 1) {
       Tcl_Obj *robj;
       buffernum = Settings.BufferNumber + 1;
       robj = Tcl_NewIntObj(buffernum);
       Tcl_SetObjResult(interp, robj);
       return TCL_OK;
   }

   /* Check if the second argument is an integer.  If so, this is the   */
   /* buffer to act on.  Otherwise, use the current buffer.             */

   buffernum = Settings.BufferNumber;
   istart = 1;
   locobjc = objc;
   if (objc >= 2) {
      /*
       * NULL won't redirect error on command prompt if
       * buffer number is not given.
       */
      if (Tcl_GetIntFromObj(NULL, objv[1], &buffernum) != TCL_OK)
          buffernum = Settings.BufferNumber;
      else {
          buffernum--;
          istart++;
          locobjc--;
      }
   }
   if ((buffernum < 0) || (buffernum >= 5)) {
       Tcl_SetResult(interp, "Buffer number must be 1 to 5", 0);
       return TCL_ERROR;
   } else {
      buffer = &Buffers[buffernum];
   }

   if (locobjc == 1) {
       Tcl_WrongNumArgs(interp, objc, objv, "[<number>] <option>");
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[istart], (CONST84 char **)
        options, "option", istart, &idx)) != TCL_OK)
      return result;

   if (locobjc >= 3)
       value = Tcl_GetString(objv[istart + 1]);

   if((idx == BUFFER_CUT) || (idx == BUFFER_COPY) || (idx == BUFFER_PASTE)) {
       if (locobjc == 4) {
           if (Tcl_GetDoubleFromObj(interp, objv[istart + 1], &dval) != TCL_OK)
               return TCL_ERROR;
           else
               X = (Location)(dval*100);

           if (Tcl_GetDoubleFromObj(interp, objv[istart + 2], &dval) != TCL_OK)
               return TCL_ERROR;
           else
               Y = (Location)(dval*100);
       } else {
           X = Crosshair.X;
           Y = Crosshair.Y;
       }
   }

   switch (idx)
   {
       case BUFFER_ADD:
           /* Copies selected objects to appropriate buffer */
           AddSelectedToBuffer (buffer, 0, 0, False);
           break;

       case BUFFER_BREAKUP:
           SmashBufferElement (buffer);
           break;

       case BUFFER_CLEAR:
           /* clear contents of paste buffer */
           ClearBuffer (buffer);
           break;

       case BUFFER_CONVERT:
           /* converts buffer contents into an element */
           ConvertBufferToElement (buffer);
           break;

       case BUFFER_COPY:
       case BUFFER_CUT:
           HideCrosshair (True);
           ClearBuffer (buffer);
           AddSelectedToBuffer (buffer, X, Y, False);

           if(idx == BUFFER_CUT)
               if (RemoveSelected ())
                   SetChangedFlag (True);

           SetBufferNumber (buffernum);
           SetMode (PASTEBUFFER_MODE);
           RestoreCrosshair (True);
           break;

       case BUFFER_LOAD:
           if ((result = Tcl_GetIndexFromObj(interp, objv[istart + 1], 
	       (CONST84 char **) loadoptions, "load option", 0, &lidx))!=TCL_OK)
               return result;
           if (locobjc >= 4) {
               name = Tcl_GetString(objv[istart + 2]);
           } else {
               Tcl_SetResult(interp, "File name required", 0);
               return TCL_ERROR;
           }

           switch (lidx)
           {
               case BUFFER_LOAD_LAYOUT:
                   HideCrosshair (True);
                   if (name && LoadLayoutToBuffer (buffer, name)) {
                       SetBufferNumber(buffernum);
                       SetMode (PASTEBUFFER_MODE);
                   }
                   RestoreCrosshair (True);
                   break;

              case BUFFER_LOAD_ELEMENT:
                  HideCrosshair (True);
                  if (name && LoadElementToBuffer (buffer, name, True)) {
                      SetBufferNumber(buffernum);
                      SetMode (PASTEBUFFER_MODE);
                  }
                  RestoreCrosshair (True);
                  break;
           }
           break;

       case BUFFER_MIRROR:
           if (value && ((!strcmp(value, "x")) || (!strcmp(value, "X")))) {
               /* Mirror buffer horizontally    */
               RotateBuffer (buffer, (BYTE) 1);   /* CCW 90 deg. rotate*/
               MirrorBuffer (buffer);             /* 180 deg. rotate*/
               RotateBuffer (buffer, (BYTE) 3);   /* CW 90 deg. rotate */
               SetCrosshairRangeToBuffer ();
           } else if (value && ((!strcmp(value, "y")) || (!strcmp(value, "Y")))) {
               /* Mirror buffer vertically      */
               MirrorBuffer (buffer);
           } else {
               Tcl_SetResult(interp, "Usage : buffer \"[1|2|3|4|5]\" mirror [X|Y]", 0);
               return TCL_ERROR;
           }
           break;

       case BUFFER_PASTE:
           SetBufferNumber (buffernum);
           SetMode (PASTEBUFFER_MODE);
           break;

       case BUFFER_ROTATE:
           if ((result = Tcl_GetIndexFromObj(interp, objv[istart + 1], 
	       (CONST84 char **) rotateoptions, "rotate option", 0, &ridx)) 
	       != TCL_OK)
               return result;

           HideCrosshair (True);
           switch (ridx)
           {
              case BUFFER_ROTATE_CW_90:
                  RotateBuffer (buffer, (BYTE) 3);
                  break;

              case BUFFER_ROTATE_CW_270:
                  RotateBuffer (buffer, (BYTE) 1);
                  break;

              case BUFFER_ROTATE_CCW_90:
                  RotateBuffer (buffer, (BYTE) 1);
                  break;

              case BUFFER_ROTATE_CW_180:
              case BUFFER_ROTATE_CCW_180:
                  RotateBuffer (buffer, (BYTE) 1);
                  RotateBuffer (buffer, (BYTE) 1);
                  break;

              case BUFFER_ROTATE_CCW_270:
                  RotateBuffer (buffer, (BYTE) 3);
                  break;
          }
          SetCrosshairRangeToBuffer ();
          RestoreCrosshair (True);
          break;

       case BUFFER_SAVE:
           if (buffer->Data->ElementN == 0) {
               Message("Buffer has no elements!\n");
               return TCL_OK;
           }
           if (value) {
               FILE *exist;
               if ((exist = fopen (value, "r"))) {
                   fclose (exist);
                   if (ConfirmDialog ("File exists!  Ok to overwrite ?"))
                      SaveBufferElements (value);
               } else {
                   SaveBufferElements (value);
               }
           } else {
               Tcl_SetResult(interp, "File name is required to store buffer",0);
               return TCL_ERROR;
           }
           break;

       case BUFFER_SELECT:
           SetBufferNumber(buffernum);
           break;

       default:
           Tcl_SetResult(interp, "Unimplemented \"buffer\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "crosshair" ---						*/
/*----------------------------------------------------------------------*/

int pcbtcl_crosshair(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   double dval;
   Tcl_Obj *tobj, *dobj;

   if (objc == 3) {
	HideCrosshair (True);
	if (Tcl_GetDoubleFromObj(interp, objv[1], &dval) != TCL_OK)
	    return TCL_ERROR;
	else
	    Crosshair.X = (Location)(dval*100);

	if (Tcl_GetDoubleFromObj(interp, objv[2], &dval) != TCL_OK)
	    return TCL_ERROR;
	else
	    Crosshair.Y = (Location)(dval*100);
	RestoreCrosshair (True);
   }
   else if (objc == 1) {
	tobj = Tcl_NewListObj(0, NULL);
	dobj = Tcl_NewDoubleObj((double)Crosshair.X / 100.0);
	Tcl_ListObjAppendElement(interp, tobj, dobj);
	dobj = Tcl_NewDoubleObj((double)Crosshair.Y / 100.0);
	Tcl_ListObjAppendElement(interp, tobj, dobj);
	Tcl_SetObjResult(interp, tobj);
   }
   else {
	Tcl_WrongNumArgs(interp, objc, objv, "<X Y>");
	return TCL_ERROR;
   }

   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "delete" ---                                                */
/*----------------------------------------------------------------------*/

int pcbtcl_delete(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "selected", "object", "track", NULL
   };
   enum OptionId {
       DELETE_SELECTED, DELETE_OBJECT, DELETE_TRACK
   };

   static char *objectoptions[] = {
       "all", "connected", NULL
   };
   enum ObjectOptionId {
       DELETE_OBJECT_ALL, DELETE_OBJECT_CONNECTED
   };

   static char *trackoptions[] = {
       "all", "auto_only", NULL
   };
   enum ObjectTrackId {
       DELETE_TRACK_ALL, DELETE_TRACK_AUTOONLY
   };

   int result, idx, oidx, tidx, X, Y;
   double dval = 0;
   void *ptr1, *ptr2, *ptr3;
   int type, changed=False;

   if (objc == 1)
       idx = DELETE_SELECTED;
   else
       if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
            options, "option", 0, &idx)) != TCL_OK)
          return result;


   switch (idx)
   {
     case DELETE_OBJECT:
         if (objc == 3) {
             if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	         (CONST84 char **) objectoptions, "object option", 0, &oidx)) 
		 != TCL_OK) {
                 Tcl_SetResult(interp, "Usage : delete object [all|connected]|\"[<x><y>]\"", 0);
                 return TCL_ERROR;
             }
             switch (oidx)
             {
                 case DELETE_OBJECT_ALL:
                     //RemovePCB(PCB);
                     {
                         BoxType box;

                         HideCrosshair (True);
                         box.X1 = -MAX_COORD;
                         box.Y1 = -MAX_COORD;
                         box.X2 = MAX_COORD;
                         box.Y2 = MAX_COORD;
                         SelectBlock (&box, True);
                         if(RemoveSelected())
                             SetChangedFlag (True);
                         RestoreCrosshair (True);
                     }
                     break;

                 case DELETE_OBJECT_CONNECTED:
                     if (SelectConnection(True)) {
                         if(RemoveSelected())
                             SetChangedFlag (True);
                      }
                      break;
             }
         } else {
             /* Delete object from particular location on layout */
             if (objc == 4) {
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
             type = SearchScreen (X, Y, SELECT_TYPES, &ptr1, &ptr2, &ptr3);
             RemoveObject (type, ptr1, ptr2, ptr3);
             IncrementUndoSerialNumber ();
             SetChangedFlag (True);
         }
         break;

     case DELETE_SELECTED:
       HideCrosshair (True);
       if (RemoveSelected ())
	  SetChangedFlag (True);
       RestoreCrosshair (True);
       break;

     case DELETE_TRACK:
         if (objc == 3) {
             if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	         (CONST84 char **) trackoptions, "track option", 
		 0, &tidx)) != TCL_OK)
                 return TCL_ERROR;
             switch (tidx)
             {
                 case DELETE_TRACK_ALL:
                     /* Rip up all auto-routed tracks */
                     ALLLINE_LOOP (PCB->Data);
                     {
                         if (TEST_FLAG (AUTOFLAG, line) && 
			     !TEST_FLAG (LOCKFLAG, line)) {
                             RemoveObject (LINE_TYPE, layer, line, line);
                             changed = True;
                         }
                     }
                     ENDALL_LOOP;

                     VIA_LOOP (PCB->Data);
                     {
                         if (TEST_FLAG (AUTOFLAG, via) && 
			     !TEST_FLAG (LOCKFLAG, via)) {
                             RemoveObject (VIA_TYPE, via, via, via);
                             changed = True;
                         }
                     }
                     END_LOOP;
                     break;

                 case DELETE_TRACK_AUTOONLY:
                     /* Rip-up selected auto-routed tracks */
                     VISIBLELINE_LOOP (PCB->Data);
                     {
                         if (TEST_FLAGS (AUTOFLAG | SELECTEDFLAG, line)
                                      && !TEST_FLAG (LOCKFLAG, line)) {
                             RemoveObject (LINE_TYPE, layer, line, line);
                             changed = True;
                         }
                     }
                     ENDALL_LOOP;
                     if (PCB->ViaOn) {
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
                     }
                     break;
             }
             if (changed) {
                 IncrementUndoSerialNumber ();
                 SetChangedFlag (True);
             }
         }
         break;

     default :
         Tcl_SetResult(interp, "Unimplemented \"delete\" option", 0);
         return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "deselect" ---                                              */
/*----------------------------------------------------------------------*/

int pcbtcl_deselect(ClientData clientData, Tcl_Interp *interp, int objc,
        Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "object", NULL
   };
   enum OptionId {
      DESELECT_OBJECT
   };
   static char *objectoptions[] = {
       "all", "connected", NULL
   };
   enum ObjectOptionId {
       DESELECT_OBJECT_ALL, DESELECT_OBJECT_CONNECTED
   };


   int result, idx, oidx;
   double dval = 0;
   BoxType fullLayout;

   if (objc == 1) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option> ...");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("", 0), (CONST84 char **)
              options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
           options, "option", 0, &idx)) != TCL_OK)
       return result;

   switch (idx)
   {
       case DESELECT_OBJECT:
           if (objc == 3) {
               if ((result=Tcl_GetIndexFromObj(interp, objv[2],(CONST84 char **)
                      objectoptions, "object option", 0, &oidx)) != TCL_OK) {
                   Tcl_SetResult(interp, "Usage : deselect object [all|connected]|\"<x><y>\"", 0);
                   return TCL_ERROR;
               }
               switch (oidx) 
               {
                   case DESELECT_OBJECT_ALL:
                       fullLayout.X1 = 0;
                       fullLayout.Y1 = 0;
                       fullLayout.X2 = PCB->MaxWidth;
                       fullLayout.Y2 = PCB->MaxHeight;
                       HideCrosshair (True);
                       if(SelectBlock(&fullLayout, False))
                       SetChangedFlag (True);
                       RestoreCrosshair (True);
                       break;

                   case DESELECT_OBJECT_CONNECTED:
                       HideCrosshair (True);
                       if (SelectConnection(False)) {
                           IncrementUndoSerialNumber ();
                           SetChangedFlag (True);
                       }
                       RestoreCrosshair (True);
                       break;
               }
           } else {
               if (objc == 4) {
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
               }
               HideCrosshair (True);
               if(SelectObject())
                   SetChangedFlag (True);
                   RestoreCrosshair (True);
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"deselect\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "drc" ---                                                   */
/*----------------------------------------------------------------------*/

int pcbtcl_drc(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "space", "width", "overlap", "silk", "run", NULL
   };
   enum OptionId {
       DRC_SPACE, DRC_WIDTH, DRC_OVERLAP, DRC_SILK, DRC_RUN
   };

   int result, idx;
   char *value = NULL;
   Tcl_Obj *robj;
   Cardinal count;

   if (objc == 1) {
       idx = DRC_RUN;
   } else {
       if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
                options, "option", 0, &idx)) != TCL_OK)
           return result;
   }

   if (objc == 3) {
       value = Tcl_GetString(objv[2]);
   }

   switch (idx)
   {
       case DRC_SPACE:
           if (objc == 3) {
               Settings.Bloat = atof(value);
           } else {
               robj = Tcl_NewDoubleObj((double)Settings.Bloat);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case DRC_WIDTH:
           if (objc == 3) {
               Settings.minWid = atof(value);
           } else {
               robj = Tcl_NewDoubleObj((double)Settings.minWid);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case DRC_OVERLAP:
           if (objc == 3) {
               Settings.Shrink = atof(value);
           } else {
               robj = Tcl_NewDoubleObj((double)Settings.Shrink);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case DRC_SILK:
           if (objc == 3) {
               Settings.minSlk = atof(value);
           } else {
               robj = Tcl_NewDoubleObj((double)Settings.minSlk);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case DRC_RUN:
            Message ("Rules are minspace %d.%02d, minoverlap %d.%d "
                     "minwidth %d.%02d, minsilk %d.%02d\n",
                    (Settings.Bloat + 1) / 100, (Settings.Bloat + 1) % 100,
                     Settings.Shrink / 100, Settings.Shrink % 100,
                     Settings.minWid / 100, Settings.minWid % 100,
                     Settings.minSlk / 100, Settings.minSlk % 100);
            HideCrosshair (True);
            watchCursor ();
            count = DRCAll ();
            if (count == 0)
                Message ("No DRC problems found.\n");
            else
                Message ("found %d design rule errors\n", count);
            restoreCursor ();
            RestoreCrosshair (True);
            break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"drc\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/*  Command "drill" ---                                                 */
/*----------------------------------------------------------------------*/

int pcbtcl_drill(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "summary", "vendor", NULL
   };
   enum OptionId {
      DRILL_SUMMARY, DRILL_VENDOR
   };

   int result, idx;

   if (objc == 1) {
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("", 0), (CONST84 char **)
              options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
              options, "drill option", 0, &idx)) != TCL_OK)
       return result;

   switch (idx)
   {
       case DRILL_SUMMARY:
           {
               Tcl_Obj *dobj = NULL;
               ReportDrills();
               if(reportString) {
                   dobj = Tcl_NewStringObj(reportString, -1);
                   Tcl_SetObjResult(interp, dobj);
                   SaveFree (reportString);
               }
           }
           break;

       case DRILL_VENDOR:
           /* Enable/Disable vendor map */
           ActionToggleVendor((Widget) NULL,(XEvent *) NULL,
                   (String *) NULL, (Cardinal *) NULL);
           break;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "element" ---                                               */
/*----------------------------------------------------------------------*/

int pcbtcl_element(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
      "breakup", "display_name", "flip_selected", "name", "save_connections",
      "save_unused_pins", "square", NULL
   };
   enum OptionId {
      ELEMENT_BREAKUP,  ELEMENT_DISPLAY_NAME,
      ELEMENT_FLIP_SELECTED, ELEMENT_NAME, ELEMENT_SAVE_CONNECTIONS,
      ELEMENT_SAVE_UNUSED_PINS, ELEMENT_SQUARE
   };

  static char *dispNameOptions[] = {
      "object", "selected_elements", NULL
   };
   enum DispNameId {
      ELEMENT_DISPLAY_NAME_OBJECT, ELEMENT_DISPLAY_NAME_SELELEMENTS
   };

   int result, idx, didx, X , Y, ival;
   FILE *fp;
   Boolean unused;
   char *value = NULL;
   double dval = 0;

#if defined(HAVE_REGCOMP)
#define REGEXEC(arg)    (!regexec(&compiled, (arg), 1, &match, 0))
   regex_t compiled;
   regmatch_t match;
#else

#define REGEXEC(arg)    (re_exec((arg)) == 1)
   char *compiled;
#endif

   if (objc == 1) {
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("", 0), (CONST84 char **)
               options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
              options, "element option", 0, &idx)) != TCL_OK)
       return result;

   if (objc >= 3)
       value = Tcl_GetString(objv[2]);

   switch (idx)
   {
       case ELEMENT_BREAKUP:
           if (objc == 4) {
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
           {
               void *ptr1, *ptr2, *ptr3;

               if (SearchScreen (Crosshair.X, Crosshair.Y, ELEMENT_TYPE,
                     &ptr1, &ptr2, &ptr3) != NO_TYPE) {
                   ival = Settings.BufferNumber;
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
                   SetBufferNumber (ival);
                   SetChangedFlag (True);
               }
           }
           RestoreCrosshair (True);
           break;

       case ELEMENT_NAME:
           {
               int type;
	       char *setname;
               void *ptr1, *ptr2, *ptr3;

               if ((type = SearchScreen (Crosshair.X, Crosshair.Y, 
	                                 ELEMENT_TYPE, &ptr1, &ptr2, 
					 &ptr3)) != NO_TYPE) {
	           if (objc == 2) {
		      /* Return the 3 strings that comprise the element name */
		      ELEMENTTEXT_LOOP((ElementTypePtr)ptr1);
			 if (text->TextString)
		             Tcl_AppendElement(interp, text->TextString);
			 else
		             Tcl_AppendElement(interp, "(null)");
		      END_LOOP;
		   }
		   else if ((objc == 4) && !strcmp(Tcl_GetString(objv[2]), "set")) {
		      char *old;
                      SetChangedFlag (True);
		      setname = Tcl_GetString(objv[3]);
		      old = ChangeObjectName (ELEMENT_TYPE, ptr1, ptr2, ptr3,
				setname);
		      if (old != (char *) -1) {
			 AddObjectToChangeNameUndoList(type, ptr1, ptr2, ptr2, old);
			 IncrementUndoSerialNumber();
		      }
		      Draw ();
		   }
		   else if ((objc == 3) && !strcmp(Tcl_GetString(objv[2]), "set")) {
                      SaveUndoSerialNumber ();
                      if (QueryInputAndChangeObjectName (type, ptr1, ptr2, ptr3)) {
                          SetChangedFlag (True);
                          if (type == ELEMENT_TYPE) {
                              RubberbandTypePtr ptr;
                              int i;

                              RestoreUndoSerialNumber ();
                              Crosshair.AttachedObject.RubberbandN = 0;
                              LookupRatLines (type, ptr1, ptr2, ptr3);
                              ptr = Crosshair.AttachedObject.Rubberband;
                              for (i = 0; i < Crosshair.AttachedObject.RubberbandN;
                                   i++, ptr++) {
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
		   else {
		      Tcl_WrongNumArgs(interp, 2, objv, "[set [<name>]]");
		      return TCL_ERROR;
		   }
               }
	       else {
		  Tcl_SetResult(interp, "No element found at crosshair location", 0);
		  return TCL_ERROR;
	       }
           }
           break;

       case ELEMENT_DISPLAY_NAME:
           if (objc == 3 && PCB->ElementOn) {
               if ((result=Tcl_GetIndexFromObj(interp,objv[2],(CONST84 char **)
                   dispNameOptions, "display name option", 0, &didx)) != TCL_OK)
                   return result;

               HideCrosshair (True);
               switch (didx)
               {
                   case ELEMENT_DISPLAY_NAME_OBJECT:
                       {
                           int type;
                           void *ptr1, *ptr2, *ptr3;

                           if ((type = SearchScreen (Crosshair.X, Crosshair.Y,
                                                     ELEMENT_TYPE, &ptr1, &ptr2,                                                     &ptr3)) != NO_TYPE) {
                               AddObjectToFlagUndoList (type, ptr1, ptr2, ptr3);
                               EraseElementName ((ElementTypePtr) ptr2);
                               TOGGLE_FLAG(HIDENAMEFLAG, (ElementTypePtr) ptr2);
                               DrawElementName ((ElementTypePtr) ptr2, 0);
                               Draw ();
                               IncrementUndoSerialNumber ();
                           }
                       }
                       break;

                   case ELEMENT_DISPLAY_NAME_SELELEMENTS:
                       {
                           Boolean changed = False;
                           ELEMENT_LOOP (PCB->Data);
                           {
                               if ((TEST_FLAG (SELECTEDFLAG, element) ||
                                    TEST_FLAG (SELECTEDFLAG, 
				    &NAMEONPCB_TEXT (element))) &&
                                    (FRONT (element) || 
				    PCB->InvisibleObjectsOn)) {
                                   AddObjectToFlagUndoList (ELEMENT_TYPE, 
				       element, element, element);
                                   EraseElementName (element);
                                   TOGGLE_FLAG (HIDENAMEFLAG, element);
                                   DrawElementName (element, 0);
                                   changed = True;
                               }
                           }
                           END_LOOP;
                           if (changed) {
                               Draw ();
                               IncrementUndoSerialNumber ();
                           }
                       }
                       break;
               }
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : element display_name [object|selected_elements]", 0);
               return TCL_ERROR;
           }
           break;


       case ELEMENT_FLIP_SELECTED:
           ChangeSelectedElementSide ();
           break;

       case ELEMENT_SAVE_CONNECTIONS:
           if(objc >= 3) {
               if (!strcasecmp(value, "all")) {
                   /* save all connections to a file */
                   if (objc == 4)
                       value = Tcl_GetString(objv[3]);
                   else
                       value = GetUserInput ("enter filename to save connection data:", "");
                   if ((fp=CheckAndOpenFile(value,True,False,&unused))!=NULL) {
                        watchCursor ();
                        LookupConnectionsToAllElements (fp);
                        fclose (fp);
                        restoreCursor ();
                        SetChangedFlag (True);
                   }
               } else {
                   /* Save element connection by name */
                   /* pattern matching source from select.c */
                   if (PCB->ElementOn) {
#if defined(HAVE_REGCOMP) || defined(HAVE_RE_COMP)
#if defined(HAVE_REGCOMP)
                       {
                           /* compile the regular expression */
                           result = regcomp (&compiled, value,
                                   REG_EXTENDED | REG_ICASE | REG_NOSUB);
                           if (result) {
                               char errorstring[128];
                               
                               regerror (result, &compiled, errorstring, 128);
                               Message ("regexp error: %s\n", errorstring);
                               regfree (&compiled);
                               break;                /* Pattern not matching */
                           }
                       }
#else
                       {
                           /* compile the regular expression */
                           if ((compiled = re_comp (value)) != NULL) {
                               Message ("re_comp error: %s\n", compiled);
                               break;                /* Pattern not matching */
                           }
                       }
#endif
#endif /* defined(HAVE_REGCOMP) || defined(HAVE_RE_COMP) */

                       /* Compare elements by name */
                       ELEMENT_LOOP (PCB->Data);
                       {
                           String name = ELEMENT_NAME (PCB, element);
                           if (name && REGEXEC (name)) {
                               if (objc == 4)
                                   value = Tcl_GetString(objv[3]);
                               else
                                   value = GetUserInput ("enter filename to save connection data:", "");
                               if ((fp = CheckAndOpenFile (value, True, False, 
			            &unused)) != NULL) {
                                   watchCursor ();
                                   LookupElementConnections (element, fp);
                                   fclose (fp);
                                   restoreCursor ();
                                    SetChangedFlag (True);
                               }
                           }
                       }
                       END_LOOP;
                   }
               }
           } else {
               Tcl_SetResult(interp, "Usage : element save [<name>|all] <filename>", 0);
               return TCL_ERROR;
           }
           break;
	
       case ELEMENT_SAVE_UNUSED_PINS:
           if (objc == 3)
               value = Tcl_GetString(objv[2]);
           else
               value = GetUserInput ("enter filename to save unused pins data:", "");

           if ((fp = CheckAndOpenFile (value, True, False, &unused)) != NULL) {
                watchCursor ();
                LookupUnusedPins (fp);
                fclose (fp);
                restoreCursor ();
                SetChangedFlag (True);
           }
           break;

       case ELEMENT_SQUARE:
           if (objc == 3) {
               value = Tcl_GetString(objv[2]);
               if (!strcmp(value, "on")) {
                   if (SetSelectedSquare(PIN_TYPE))
                       SetChangedFlag(True);
               } else if (!strcmp(value, "off")) {
                   if (ClrSelectedSquare(PIN_TYPE))
                       SetChangedFlag(True);
               }
            } else {
                /* Toggle square flag of selected elements */
                if (ChangeSelectedSquare(ELEMENT_TYPE))
                    SetChangedFlag(True);
            }
            break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"element\" option", 0);
           return TCL_ERROR;
           break;
   }
   return PCBTagCallback(interp, objc, objv);
}

#endif /* TCL_PCB */
