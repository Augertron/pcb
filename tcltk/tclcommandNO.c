/*----------------------------------------------------------------------------*/
/* tclcommandNO.c:                                                            */
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
#include <ctype.h>      /* for tolower(), toupper() */

#include <tk.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "config.h"
#include "output.h"     /* also includes "global.h" */
#include "macro.h"
#include "data.h"

#include "crosshair.h"
#include "buffer.h"
#include "set.h"
#include "file.h"
#include "misc.h"
#include "netlist.h"
#include "remove.h"
#include "draw.h"
#include "error.h"
#include "action.h"
#include "polygon.h"
#include "search.h"
#include "report.h"
#include "gui.h"
#include "rubberband.h"
#include "rotate.h"
#include "copy.h"

#include "control.h"
#include "find.h"
#include "undo.h"
#include "change.h"
#include "select.h"
#include "djopt.h"
#include "rats.h"
#include "move.h"
#include "tclpcb.h"

#define MAX_OBJ_REPORT 16
extern int djopt_get_auto_only();
extern void WarpPointer (Boolean);
extern Boolean SeekPad (LibraryEntryType *, ConnectionType *, Boolean);
Boolean isSummary = False;
SelObjects *SelObjectEntry;
char* reportString;

#ifdef TCL_PCB

/*----------------------------------------------------------------------*/
/*  Command "netlist" ---                                               */
/*----------------------------------------------------------------------*/

static LibraryMenuTypePtr menuPtr = NULL;
int pcbtcl_netlist(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "add_rats", "clearance", "clearance_range", "delete_rats",
       "optimize_rats", "disable", "enable", "load", "info", "connections",
       "show_pin", "show_net", "style", NULL
   };
   enum Id {
       NETLIST_ADD_RATS, NETLIST_CLEARANCE, NETLIST_CLEARANCE_RANGE,
       NETLIST_DELETE_RATS, NETLIST_OPTIMIZE_RATS, NETLIST_DISABLE,
       NETLIST_ENABLE, NETLIST_LOAD, NETLIST_INFO, NETLIST_CONNECTIONS,
       NETLIST_SHOW_PIN, NETLIST_SHOW_NET, NETLIST_STYLE
   };

   static char *ratOptions[] = {
       "all", "selected", NULL
   };
   enum AddId {
       NETLIST_ALL_RATS, NETLIST_SELECTED_RATS
   };

   int result, idx, aidx;
   char *name = NULL, *value = NULL;
   char netName[1024], tmpVal[128];
   Tcl_Obj *lobj = NULL, *nobj;
   ConnectionType Conn;
   Boolean isAbsolute = True, isFound = False;
   BDimension minDim, maxDim, size = 0;

   if (objc == 1) {
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
               options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
              options, "option", 0, &idx)) != TCL_OK)
       return result;

   if (objc >= 3) {
       name = Tcl_GetString(objv[2]);
       if((name[0] == '*') || (name[0] == ' '))
           name += 2;
   }

   switch (idx)
   {
       case NETLIST_ADD_RATS:
           if (objc == 3) {
               if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	           (CONST84 char **) ratOptions, "option", 0, &aidx)) != TCL_OK)
                   return result;
               HideCrosshair (True);
               switch (aidx)
               {
                   case NETLIST_ALL_RATS:
                       if (AddAllRats (False, NULL))
                           SetChangedFlag (True);
                       break;

                   case NETLIST_SELECTED_RATS:
                       if (AddAllRats (True, NULL))
                           SetChangedFlag (True);
                       break;
               }
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, 
	                     "Usage : netlist add_rats [all|selected]", 0);
               return TCL_ERROR;
           }
           break;

       case NETLIST_CLEARANCE:
           /* Report/Change clearance between two nets */
           if (objc == 2) {
               /* Return separation between two nets */
               nobj = Tcl_NewIntObj(Settings.Keepaway);
               Tcl_SetObjResult(interp, nobj);
           } else if (objc >= 3) {
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
                   HideCrosshair(True);
                   SetKeepawayWidth (isAbsolute ? size : size + 
		                     Settings.Keepaway);
                   RestoreCrosshair(True);
           } else {
               Tcl_SetResult(interp, "Usage : netlist clearance <\"[+|-]\"value> \"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case NETLIST_CLEARANCE_RANGE:
           minDim = MIN_LINESIZE;
           maxDim = MAX_LINESIZE;
           sprintf(tmpVal, "%d %d", minDim, maxDim);
           nobj = Tcl_NewStringObj(tmpVal, -1);
           Tcl_SetObjResult(interp, nobj);
           break;

       case NETLIST_DELETE_RATS:
           if (objc == 3) {
               if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	           (CONST84 char **) ratOptions, "option", 0, &aidx)) != TCL_OK)
                   return result;
               HideCrosshair (True);
               switch (aidx)
               {
                   case NETLIST_ALL_RATS:
                       if (DeleteRats (False))
                           SetChangedFlag (True);
                       break;

                   case NETLIST_SELECTED_RATS:
                       if (DeleteRats (True))
                           SetChangedFlag (True);
                       break;
               }
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : netlist delete_rats [all|selected]", 0);
               return TCL_ERROR;
           }
           break;

       case NETLIST_OPTIMIZE_RATS:
           SaveUndoSerialNumber ();
           DeleteRats (False);
           RestoreUndoSerialNumber ();
           AddAllRats (False, NULL);
           RestoreUndoSerialNumber ();
           IncrementUndoSerialNumber ();
           ClearAndRedrawOutput ();
           break;

       case NETLIST_DISABLE:
           if (objc == 3) {
               if (!strcmp(name, "all")) {
                   /* disable all nets */
                   MENU_LOOP (&PCB->NetlistLib);
                   {
                       menu->Name[0] = '*';
                   }
                   END_LOOP;
               } else {
                   MENU_LOOP (&PCB->NetlistLib);
                   {
                       /* disable single net */
                       char *nname = menu->Name;
                       nname +=2;
                       if (!strcmp(name, nname)) {
                           menu->Name[0] = '*';
                           isFound = True;
                       }
                   }
                   END_LOOP;
                   if(!isFound) {
                       Tcl_SetResult(interp, "No such net exists! Try \"netlist info\" first.", 0);
                       return TCL_ERROR;
                   }
               }
           } else {
               Tcl_SetResult(interp, 
	                     "Usage : netlist disable [all|<net-name>]", 0);
               return TCL_ERROR;
           }
           break;

       case NETLIST_ENABLE:
           if (objc == 3) {
               if (!strcmp(name, "all")) {
                   /* enable all nets */
                   MENU_LOOP (&PCB->NetlistLib);
                   {
                       menu->Name[0] = ' ';
                   }
                   END_LOOP;
               } else {
                   MENU_LOOP (&PCB->NetlistLib);
                   {
                       /* enable single net */
                       char *nname = menu->Name;
                       nname += 2;
                       if (!strcmp(name, nname)) {
                           menu->Name[0] = ' ';
                           isFound = True;
                       }
                   }
                   END_LOOP;
                   if(!isFound) {
                       Tcl_SetResult(interp, "No such net exists! Try \"netlist info\" first.", 0);
                       return TCL_ERROR;
                   }
               }
           } else {
               Tcl_SetResult(interp, 
	                     "Usage : netlist enable [all|<net-name>]", 0);
               return TCL_ERROR;
           }
           break;

       case NETLIST_LOAD:
           if (name) {
               HideCrosshair (True);
               if (PCB->Netlistname)
                   SaveFree (PCB->Netlistname);
               PCB->Netlistname = StripWhiteSpaceAndDup (name);
               FreeLibraryMemory (&PCB->NetlistLib);
               if (!ReadNetlist (PCB->Netlistname)) {
                   /* Create Netlist window */
                   if ((result = Tcl_Eval(interp, "NetlistDialog")) != TCL_OK) {
                       RestoreCrosshair(True);
                       return result;
                   }
               }
               RestoreCrosshair (True);
           }
           break;

      case NETLIST_INFO:
           lobj = Tcl_NewStringObj(0, 0);
           MENU_LOOP (&PCB->NetlistLib);
           {
               /* sprintf(netName, "%s",(menu->Name)+2); */
               sprintf(netName, "\"%s\"",menu->Name);
               Tcl_AppendStringsToObj(lobj, netName, " ", 0);
           }
           END_LOOP;

           break;

       case NETLIST_CONNECTIONS:
           if (name) {
               menuPtr = NULL;
               lobj = Tcl_NewStringObj(0, 0);
               MENU_LOOP (&PCB->NetlistLib);
               {
                   sprintf(netName, "%s",(menu->Name)+2);
                   if (!strcmp(name, netName)) {
                       menuPtr = menu;
                       isFound = True;
                   }
               }
               END_LOOP;
               if(!isFound) {
                   Tcl_SetResult(interp, "No such net exists! Try \"netlist info\" first.", 0);
                   return TCL_ERROR;
               }
               if (menuPtr) {
                   ENTRY_LOOP (menuPtr);
                   {
                       Tcl_AppendStringsToObj(lobj, entry->ListEntry, " ", 0);
                   }
                   END_LOOP;
               }
           } else {
               Tcl_SetResult(interp, 
	                     "Usage : netlist connections <net-name>", 0);
               return TCL_ERROR;
           }
           break;

       case NETLIST_SHOW_PIN:
           if (menuPtr && name) {
               ENTRY_LOOP (menuPtr);
               {
                   if(!strcmp(name, entry->ListEntry)) {
                       SelectPin(entry, True);
                       IncrementUndoSerialNumber();
                       Draw();
                       break;
                   }
               }
               END_LOOP;
           } else if (name) {
               MENU_LOOP (&PCB->NetlistLib);
               {
                   ENTRY_LOOP (menu);
                   {
                       if(!strcmp(name, entry->ListEntry)) {
                           SelectPin(entry, True);
                           IncrementUndoSerialNumber();
                           Draw();
                           break;
                       }
                   }
                   END_LOOP;
               }
               END_LOOP;
           } else {
               Tcl_SetResult(interp, "Usage : netlist show_pin <pin-name>", 0);
               return TCL_ERROR;
           }
           break;

       case NETLIST_SHOW_NET:
           if (name) {
               if (menuPtr && !strcmp(name, (menuPtr->Name)+2)) {
                   /* check whether net already found */
               } else {
                   MENU_LOOP (&PCB->NetlistLib);
                   {
                       sprintf(netName, "%s",(menu->Name)+2);
                       if (!strcmp(name, netName)) {
                           menuPtr = menu;
                           isFound = True;
                           break;
                       }
                   }
                   END_LOOP;
                   if(!isFound) {
                       Tcl_SetResult(interp, "No such net exists! Try \"netlist info\" first.", 0);
                       return TCL_ERROR;
                   }
               }
               if (menuPtr) {
                   InitConnectionLookup ();
                   ResetFoundPinsViasAndPads (False);
                   ResetFoundLinesAndPolygons (False);
                   SaveUndoSerialNumber ();

                   ENTRY_LOOP (menuPtr);
                   {
                       if (SeekPad (entry, &Conn, False)) {
                           RatFindHook (Conn.type, Conn.ptr1, Conn.ptr2,
                                        Conn.ptr2, True, True);
                       }
                   }
                   END_LOOP;
                   RestoreUndoSerialNumber ();
                   SelectConnection (True);
                   ResetFoundPinsViasAndPads (False);
                   ResetFoundLinesAndPolygons (False);
                   FreeConnectionLookupMemory ();
                   IncrementUndoSerialNumber ();
                   Draw ();
                   /* Highlight net in Netlist window */
                   sprintf(tmpVal, "HighlightNet {%s} ", netName);
                   Tcl_Eval(pcbinterp, tmpVal);
               } else {
                   Tcl_SetResult(interp, "Net not found!", 0);
                   return TCL_ERROR;
               }
           } else {
               Tcl_SetResult(interp, "Usage : netlist show_net <net-name>", 0);
               return TCL_ERROR;
           }
           break;

       case NETLIST_STYLE:
           lobj = Tcl_NewStringObj(0, 0);
           if (name) {
               menuPtr = NULL;
               MENU_LOOP (&PCB->NetlistLib);
               {
                   sprintf(netName, "%s",(menu->Name)+2);
                   if (!strcmp(name, netName)) {
                       isFound = True;
                       if (menu->Style)
                           Tcl_AppendStringsToObj(lobj, menu->Style, 0);
                       else
                           Tcl_AppendStringsToObj(lobj, "(unknown)", 0);
                    }
               }
               END_LOOP;
               if(!isFound) {
                   Tcl_SetResult(interp, "No such a net exist! Try \"netlist info\" first.", 0);
                   return TCL_ERROR;
               }
           } else {
               if (menuPtr) {
                   if (menuPtr->Style)
                       Tcl_AppendStringsToObj(lobj, menuPtr->Style, 0);
                   else
                       Tcl_AppendStringsToObj(lobj, "(unknown)", 0);
               } else {
                   Tcl_SetResult(interp, "Usage : netlist style <net-name>", 0);
                   return TCL_ERROR;
               }
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"netlist\" option", 0);
           return TCL_ERROR;
    }
    if (lobj)
        Tcl_SetObjResult(interp, lobj);
    return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "object" ---                                                */
/*----------------------------------------------------------------------*/

static int moreSelObjEntries, selObjEntryCount;
static SelObjects *SelObjTop, *SelObjEntry;

/* Create list of selected objects pointer, put pointer to
 * first entry in SelObjTop
 */
void
ListSelObjects(int type, void* dataPtr1, void* dataPtr2)
{
   SelObjects *newSelObjPtr;

   if (moreSelObjEntries == 0) {
       newSelObjPtr = (SelObjects *)malloc((selObjEntryCount + 100) * 
                                           sizeof(SelObjects));
       if (!newSelObjPtr) return;
       if (SelObjTop) {
           memcpy(newSelObjPtr, SelObjTop, selObjEntryCount * 
	          sizeof(SelObjects));
           free(SelObjTop);
       }
       SelObjTop = newSelObjPtr;
       SelObjEntry = &SelObjTop[selObjEntryCount];
       moreSelObjEntries = 100;
   }
   SelObjEntry->obj_type = type;
   SelObjEntry->dataPtr1 = dataPtr1;
   SelObjEntry->dataPtr2 = dataPtr2;
   SelObjEntry++;
   moreSelObjEntries--;
   selObjEntryCount++;
}

int pcbtcl_object(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "clearance", "connections", "convert", "copy_buffer",
       "cut_buffer", "delete", "display_name", "hole", "insert",
       "move", "name", "paste_buffer", "report", "rotate",
       "select", "size", "square", NULL
   };
   enum OptionId {
      OBJECT_CLEARANCE, OBJECT_CONNECTIONS, OBJECT_CONVERT,
      OBJECT_COPY_BUFFER, OBJECT_CUT_BUFFER,  OBJECT_DELETE,
      OBJECT_DISPLAY_NAME, OBJECT_HOLE, OBJECT_INSERT,
      OBJECT_MOVE, OBJECT_NAME, OBJECT_PASTE_BUFFER,
      OBJECT_REPORT, OBJECT_ROTATE, OBJECT_SELECT, OBJECT_SIZE,
      OBJECT_SQUARE	
   };

   static char *clearanceOptions[] = {
      "selected_arcs", "selected_lines", "selected_objects", "selected_pads",
       "selected_pins", "selected_vias", NULL
   };
   enum ClearanceOptionId {
      OBJECT_CLRNC_SELECTED_ARCS, OBJECT_CLRNC_SELECTED_LINES,
      OBJECT_CLRNC_SELECTED_OBJECTS, OBJECT_CLRNC_SELECTED_PADS,
      OBJECT_CLRNC_SELECTED_PINS, OBJECT_CLRNC_SELECTED_VIAS
   };

   int result, idx, cidx, type, istart = 0, lNum, layerNum = -1 ;
   float size;
   char *value = NULL;
   void *ptr1, *ptr2, *ptr3;
   double dval = 0;
   static int loop = 0;
   Location X,Y;
   Cardinal buffernum;
   BufferTypePtr buffer;
   ElementTypePtr element;
   Boolean isAbsolute = True;
   LayerTypePtr currentLayer = NULL;
   Tcl_Obj *tobj = NULL;
   extern void ListSelObjects();
   X = Crosshair.X;
   Y = Crosshair.Y;

   if (objc == 1) {
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
              options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
              options, "option", 0, &idx)) != TCL_OK)
       return result;

   if (objc >= 3) {
       value = Tcl_GetString(objv[2]);
   }

   if((idx == OBJECT_CONNECTIONS) || (idx == OBJECT_DELETE)
        || (idx == OBJECT_INSERT) || (idx == OBJECT_DISPLAY_NAME)
        || (idx == OBJECT_SELECT) || (idx == OBJECT_NAME)) {
       if (objc == 4) {
           if (Tcl_GetDoubleFromObj(interp, objv[2], &dval) != TCL_OK)
               return TCL_ERROR;
           else
               X = (Location)(dval*100);
           if (Tcl_GetDoubleFromObj(interp, objv[3], &dval) != TCL_OK)
               return TCL_ERROR;
           else
               Y = (Location)(dval*100);
       }
   }

   if((idx == OBJECT_CUT_BUFFER) || (idx == OBJECT_COPY_BUFFER)
       || (idx == OBJECT_PASTE_BUFFER)) {
       if((objc >= 4) && !strcmp(value, "buffer")) {
           if (Tcl_GetIntFromObj(interp, objv[3], &buffernum) == TCL_OK) {
               if (objc == 6) {
                   if (Tcl_GetDoubleFromObj(interp, objv[4], &dval) != TCL_OK)
                       return TCL_ERROR;
                   else
                       X = (Location)(dval*100);
                   if (Tcl_GetDoubleFromObj(interp, objv[5], &dval) != TCL_OK)
                       return TCL_ERROR;
                   else
                       Y = (Location)(dval*100);
               }
           } else {
               Tcl_SetResult(interp, "Usage: object [cut|copy|paste] buffer <bufferNum> \"<X><Y>\"", 0);
               return TCL_ERROR;
           }
       } else {
           Tcl_SetResult(interp, "Usage: object [cut|copy|paste] buffer <bufferNum> \"<X><Y>\"", 0);
           return TCL_ERROR;
       }
   }

   switch (idx)
   {
       case OBJECT_CLEARANCE:
           /* changes the clearance size of objects */
           if (objc == 3) {
               value = Tcl_GetString(objv[2]);
               if((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;
               size = atof(value);
               {
                   int type;
                   void *ptr1, *ptr2, *ptr3;

                   if ((type =
                            SearchScreen (Crosshair.X, Crosshair.Y,
                                          CHANGECLEARSIZE_TYPES, &ptr1, &ptr2,
                                          &ptr3)) != NO_TYPE)
                       if (ChangeObjectClearSize (type, ptr1, ptr2, ptr3, size, 
		           isAbsolute))
                           SetChangedFlag (True);
               }
               break;
           } else if (objc == 4) {
               if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	           (CONST84 char**) clearanceOptions, "clearance option", 
		   0, &cidx)) != TCL_OK)
                   return result;

               value = Tcl_GetString(objv[3]);
               if((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;
               size = atof(value);

               HideCrosshair (True);
               switch (cidx)
               {
                   case OBJECT_CLRNC_SELECTED_ARCS:
                       if (ChangeSelectedClearSize (ARC_TYPE, size, isAbsolute))
                           SetChangedFlag (True);
                       break;

                   case OBJECT_CLRNC_SELECTED_LINES:
                       if (ChangeSelectedClearSize (LINE_TYPE, size, 
		           isAbsolute))
                           SetChangedFlag (True);
                           break;

                   case OBJECT_CLRNC_SELECTED_OBJECTS:
                       if (ChangeSelectedClearSize (CHANGECLEARSIZE_TYPES, 
		           size, isAbsolute))
                           SetChangedFlag (True);
                           break;

                   case OBJECT_CLRNC_SELECTED_PADS:
                       if (ChangeSelectedClearSize (PAD_TYPE, size, isAbsolute))
                           SetChangedFlag (True);
                       break;

                   case OBJECT_CLRNC_SELECTED_PINS:
                       if (ChangeSelectedClearSize (PIN_TYPE, size, isAbsolute))
                           SetChangedFlag (True);
                       break;

                   case OBJECT_CLRNC_SELECTED_VIAS:
                       if (ChangeSelectedClearSize (VIA_TYPE, size, isAbsolute))
                           SetChangedFlag (True);
                       break;
               }
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage: object clearance \"[selected_arcs|selected_lines|selected_objects|selected_pads|selected_pins|selected_vias]\" <\"[+|-]\"size>", 0);
               return TCL_ERROR;
           }
           break;

       case OBJECT_CONNECTIONS:
           HideCrosshair (True);
           watchCursor ();
           LookupConnection (X, Y, True, TO_PCB (1));
           restoreCursor ();
           RestoreCrosshair (True);
           break;

       case OBJECT_CONVERT:
           if((objc == 2) || !strcmp(value, "element")) {
               /* convert selected object into an element */
               HideCrosshair (True);
               buffernum = Settings.BufferNumber;
               SetBufferNumber (MAX_BUFFER - 1);
               ClearBuffer (PASTEBUFFER);
               AddSelectedToBuffer (PASTEBUFFER, X, Y, True);
               SaveUndoSerialNumber ();
               RemoveSelected ();
               ConvertBufferToElement (PASTEBUFFER);
               RestoreUndoSerialNumber ();
               CopyPastebufferToLayout (X, Y);
               SetBufferNumber (buffernum);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage: object convert \"element\"", 0);
               return TCL_ERROR;
           }
           break;

       case OBJECT_CUT_BUFFER:
       case OBJECT_COPY_BUFFER:
       case OBJECT_PASTE_BUFFER:
           /* Cut or Copy selected objects to given buffer.
            * If (X,Y) value not given it uses CROSSHAIR data.
            * Reference : pcb-menu.h : Line-33.
            */
           if(idx != OBJECT_PASTE_BUFFER) {
               /* CUT and COPY mode  */
               buffer = &Buffers[buffernum -1];
               ClearBuffer (buffer);
               AddSelectedToBuffer (buffer, 0, 0, False);
               if(idx == OBJECT_CUT_BUFFER)
                   RemoveSelected();         /* If CUT mode then only
                                                remove selected objects */
            }
            watchCursor ();
            if (CopyPastebufferToLayout (X, Y))
                SetChangedFlag (True);
            restoreCursor ();
            break;

       case OBJECT_DELETE:
           if ((type = SearchScreen (X, Y, REMOVE_TYPES, &ptr1, &ptr2,
                                       &ptr3)) != NO_TYPE) {
               if (TEST_FLAG (LOCKFLAG, (LineTypePtr) ptr2)) {
                   Message ("Sorry, that object is locked\n");
                   break;
                   
               }
               if (type == ELEMENT_TYPE) {
                   RubberbandTypePtr ptr;
                   int i;

                   Crosshair.AttachedObject.RubberbandN = 0;
                   LookupRatLines (type, ptr1, ptr2, ptr3);
                   ptr = Crosshair.AttachedObject.Rubberband;
                   for (i = 0; i < Crosshair.AttachedObject.RubberbandN; i++) {
                       if (PCB->RatOn)
                           EraseRat ((RatTypePtr) ptr->Line);
                       MoveObjectToRemoveUndoList (RATLINE_TYPE,
                       ptr->Line, ptr->Line, ptr->Line);
                       ptr++;
                   }
               }
               RemoveObject (type, ptr1, ptr2, ptr3);
               IncrementUndoSerialNumber ();
               SetChangedFlag (True);
           }
           break;

       case OBJECT_DISPLAY_NAME:
           /* toggle displaying of pin/pad/via names */
           HideCrosshair (True);
           switch (SearchScreen (X, Y, 
	           ELEMENT_TYPE | PIN_TYPE | PAD_TYPE | VIA_TYPE, 
                   (void **) &ptr1, (void **) &ptr2, (void **) &ptr3))
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
           RestoreCrosshair (True);
           break;

       case OBJECT_HOLE:
           /* changes the drilling hole size of objects/selected objects */
           if (objc == 3) {
               value = Tcl_GetString(objv[2]);
               if((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;
               size = atof(value);

               HideCrosshair (True);
               {
                   int type;
                   void *ptr1, *ptr2, *ptr3;

                   if ((type = SearchScreen (Crosshair.X, Crosshair.Y, 
		        CHANGE2NDSIZE_TYPES, &ptr1, &ptr2, &ptr3)) != NO_TYPE)
                       if (ChangeObject2ndSize (type, ptr1, ptr2, ptr3, size, 
		           isAbsolute, True))
                           SetChangedFlag (True);
               }
               RestoreCrosshair (True);
           } else if (objc == 4 && (!strcmp(value, "selected"))) {
               value = Tcl_GetString(objv[3]);
               if((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;
               size = atof(value);

               HideCrosshair (True);
               if (ChangeSelected2ndSize (PIN_TYPES, size, isAbsolute))
                   SetChangedFlag (True);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : object hole \"seleted\" \"[+|-]\"<size>", 0);
               return TCL_ERROR;
           }
           break;

       case OBJECT_INSERT:
           // Todo.
           break;

       case OBJECT_MOVE:
           if(objc == 3) {
               if (!strcmp(value, "otherside")) {
                   /* Note: Object is flipped only if pointer is on object */
                   if ((SearchScreen (Crosshair.X, Crosshair.Y, ELEMENT_TYPE,
                                      (void **) &element, (void **) &element,
                                      (void **) &element)) != NO_TYPE) {
                       HideCrosshair(True);
                       ChangeElementSide(element, 2 * Crosshair.Y - 
		                         PCB->MaxHeight);
                       UpdatePIPFlags(NULL, element, NULL, True);
                       IncrementUndoSerialNumber();
                       Draw();
                       RestoreCrosshair(True);
                   }
               } else {
                   int type;
                   void *ptr1, *ptr2, *ptr3;

                   /* Move object to current or specified layer */
                   if (!strcmp(value, "current")) {
                       currentLayer = CURRENT;
                       layerNum = -1;
                   } else if (Tcl_GetIntFromObj(NULL, objv[2], 
		              &layerNum) != TCL_OK) {
                       layerNum = -1;
                       value = Tcl_GetString(objv[2]);
                       for (lNum = 0; lNum < MAX_LAYER; lNum++) {
                           if (!strcasecmp(PCB->Data->Layer[lNum].Name,value)) {
                               layerNum = lNum;
                               value = NULL;
                               break;
                           }
                       }
                   }
                   if (!currentLayer) {
                       if(layerNum < 0 || layerNum >= MAX_LAYER) {
                           Tcl_SetResult(interp, "Must specify a valid layer name or number", 0);
                           return TCL_ERROR;
                       } else {
                           currentLayer =  &PCB->Data->Layer[layerNum];
                       }
                   }

                   if ((type = SearchScreen (Crosshair.X, Crosshair.Y, 
		                             MOVETOLAYER_TYPES, &ptr1, &ptr2, 
					     &ptr3)) != NO_TYPE)
                       if (MoveObjectToLayer (type, ptr1, ptr2, ptr3, 
		                              currentLayer, False))
                           SetChangedFlag (True);
               }
           } else {
               Tcl_SetResult(interp, "Usage : object move \"[otherside|current|[<layer-num>|<layername>]]\"", 0);
               return TCL_ERROR;
           }
           break;

       case OBJECT_NAME:
           {
               int type;
               void *ptr1, *ptr2, *ptr3;

               if ((type = SearchScreen (Crosshair.X, Crosshair.Y, 
	                                 CHANGENAME_TYPES, &ptr1, &ptr2, 
					 &ptr3)) != NO_TYPE) {
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
           }
           break;

       case OBJECT_REPORT:
           if (objc == 3 ) {
               /* Generate summary only if "list" flag is supplied */
               if(!strcmp(++value, "list")) isSummary = True;
           }
           {
               void *ptr1, *ptr2, *ptr3;

               /* Look for object under cursor */
               type = SearchScreen (Crosshair.X, Crosshair.Y, REPORT_TYPES, 
	                            &ptr1, &ptr2, &ptr3);
           }
           if (type) {
               /* Called from Keystroke macro or layout */
               /* Create object report in small window */
               ReportDialog();
               if(reportString) {
                   tobj = Tcl_NewStringObj(reportString, -1);
                   Tcl_SetObjResult(interp, tobj);
                   SaveFree (reportString);
               }
           } else {
               if(loop >= 2) {
                   /* Stop asking if user not interested */
                   loop = 0 ;
                   break;
               }
               tobj = Tcl_NewStringObj(0, 0);
               /*  Called from console. report all selected objects */
               moreSelObjEntries = 0;
               selObjEntryCount = 0;
               SelObjTop = NULL;
               SelectedObjOperation();
               if(selObjEntryCount) {
                   if(selObjEntryCount > MAX_OBJ_REPORT) isSummary = True;
                   for (SelObjEntry = SelObjTop; selObjEntryCount-- > 0; 
		        SelObjEntry++) {
                       SelObjectEntry = SelObjEntry;
                       ReportDialog ();
                       Tcl_AppendStringsToObj(tobj, reportString, "\n", 0);
                       SaveFree (reportString);
                   }
                   Tcl_SetObjResult(interp, tobj);
                   free(SelObjTop);
                   SelObjTop = NULL;
                   SelObjectEntry = NULL; // Must make ReportDialog() compatible
                                          // with original code.
                   isSummary = False;
               } else {
                   /* Request for selection of object from layout if no
                    * object selected.
                    */
                   loop++;
                   Tcl_Eval(pcbinterp, "GetLocation {press a button at the object's location}; object report");
               }
           }
           isSummary = False;
           break;

       case OBJECT_ROTATE:
           if(objc >= 2) {
               if (objc == 5)
                   istart = 3;
               else if (objc == 4)
                   istart = 2;

               if (objc >= 4) {
                   if (Tcl_GetDoubleFromObj(interp, objv[istart], 
		                            &dval) != TCL_OK)
                       return TCL_ERROR;
                   else
                       X = (Location)(dval*100);
                   if (Tcl_GetDoubleFromObj(interp, objv[istart + 1], 
		                            &dval) != TCL_OK)
                       return TCL_ERROR;
                   else
                       Y = (Location)(dval*100);
               }
               if (!strcmp(value, "cw"))
                   RotateScreenObject (X, Y, 3);
               else if (!strcmp(value, "ccw"))
                   RotateScreenObject (X, Y, 1);
               else
                   RotateScreenObject (X, Y,  (SWAP_IDENT ? 3 : 1));
           } else {
               Tcl_SetResult(interp, "Usage : object rotate \"[cw|ccw]\" \"[<x> <y>]\"", 0);
               return TCL_ERROR;
           }
           break;

       case OBJECT_SELECT:
           if (SelectObject ())
               SetChangedFlag (True);
           break;

       case OBJECT_SIZE:
           /* changes the size of objects/selected objects */
           if (objc == 3) {
               value = Tcl_GetString(objv[2]);
               if((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;
               size = atof(value);

               HideCrosshair (True);
               {
                   int type;
                   void *ptr1, *ptr2, *ptr3;
                   if ((type = SearchScreen (Crosshair.X, Crosshair.Y, 
		                             CHANGESIZE_TYPES,
                                             &ptr1, &ptr2, &ptr3)) != NO_TYPE)
                       if (TEST_FLAG (LOCKFLAG, (PinTypePtr) ptr2))
                           Message ("Sorry, that object is locked\n");
                       if (ChangeObjectSize (type, ptr1, ptr2, ptr3, size, 
		                             isAbsolute))
                           SetChangedFlag (True);
               }
               RestoreCrosshair (True);
           } else if (objc == 4 && (!strcmp(value, "selected"))) {
               value = Tcl_GetString(objv[3]);
               if((value[0] == '-')  || (value[0] == '+'))
                   isAbsolute = False;
               size = atof(value);

               HideCrosshair (True);
               if (ChangeSelectedSize (CHANGESIZE_TYPES, size, isAbsolute))
                   SetChangedFlag (True);
               RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : object size \"seleted\" \"[+|-]\"<size>", 0);
               return TCL_ERROR;
           }
           break;

       case OBJECT_SQUARE:
           /* changes the square-flag of objects/selected objects */
           if (objc == 2) {
               /* Toggle Square flag of object */
               if ((type = SearchScreen (Crosshair.X, Crosshair.Y, 
	                                 CHANGESQUARE_TYPES,
                                         &ptr1, &ptr2, &ptr3)) != NO_TYPE)
                   if (ChangeObjectSquare (type, ptr1, ptr2, ptr3))
                       SetChangedFlag (True);
           } else if (objc == 3) {
               if (!strcmp(value, "selected")) {
                   /* Toggle Square flag of selected object */
                   if (ChangeSelectedSquare (PIN_TYPE | PAD_TYPE))
                       SetChangedFlag (True);
               } else {
                   /* Set/Reset square flag of object */
                   value = Tcl_GetString(objv[2]);

                   if ((type = SearchScreen (Crosshair.X, Crosshair.Y, 
		                             CHANGESQUARE_TYPES,
                                             &ptr1, &ptr2, &ptr3)) != NO_TYPE) {
                       if (!strcmp(value, "on") && 
		           SetObjectSquare (type, ptr1, ptr2, ptr3))
                           SetChangedFlag (True);
                       else if(!strcmp(value, "off") && 
		               ClrObjectSquare (type, ptr1, ptr2, ptr3))
                           SetChangedFlag (True);
                   }
               }
           } else if (objc == 4 && !strcmp(value, "selected")) {
               /* Set/Reset square flag of selected object */
               value = Tcl_GetString(objv[3]);
               if(!strcmp(value, "on") && 
	          SetSelectedSquare (PIN_TYPE | PAD_TYPE))
                   SetChangedFlag (True);
               else if(!strcmp(value, "off") && 
	               ClrSelectedSquare (PIN_TYPE | PAD_TYPE))
                   SetChangedFlag (True);
           } else {
               Tcl_SetResult(interp, "Usage : object square \"seleted\" \"[on|off]\"", 0);
               return TCL_ERROR;
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"object\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "option" ---                                                */
/*----------------------------------------------------------------------*/

int pcbtcl_option(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "all_direction_lines", "auto_enforce_drc_clearance",
       "auto_swap_line_start_angle", "auto_zero_delta_measurements",
       "check_polygons", "crosshair_shows_drc_clearance",
       "crosshair_snaps_to_pins_pads", "cycle_clip", "new_lines_arcs_clear_polygons",
       "only_autorouted_nets", "orthogonal_moves", "rubber_band_mode",
       "show_autorouter_trials", "thin_draw", "unique_element_names",
       "pinout_shows_number_not_name", NULL
    };
   enum OptionId {
      OPTION_ALLDIRLINES, OPTION_AUTOENFDRC, OPTION_AUTOSWAPLSA,
      OPTION_AUTOZERODM, OPTION_CHECKPOLY, OPTION_CSSHOWSDRC,
      OPTION_CSSNAP, OPTION_CYCLECLIP, OPTION_NEWLACLEARPOLY, OPTION_ONLYARNETS,
      OPTION_ORTHOMOVES, OPTION_RUBBERBAND, OPTION_SHOWARTRIALS,
      OPTION_THINDRAW, OPTION_UNIQUEELEMNAMES, OPTION_PONUMNOTNAME
   };

   int result, idx;
   char *value = NULL;
   Tcl_Obj *robj;
   char rStr[8];
   Boolean bVal = False;

   /* "option" conflicts with the Tk option of the same name.  This    */
   /* should be renamed to "tcl_option" by the setup scripts.  We try  */
   /* the Tcl command first.  If it returns an error, then we try the  */
   /* PCB command.                                                     */

   if ((result = resolve_conflict(clientData, interp, objc, objv)) == TCL_OK)
       return result;

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

   HideCrosshair (True);
   switch (idx)
   {
       case OPTION_ALLDIRLINES:
           /* Set/Toggle Alldirection Flag */
           if (objc == 3) {
               if (!strcmp(value, "toggle")) {
                   Settings.AllDirectionLines = !Settings.AllDirectionLines;
                   TOGGLE_FLAG (ALLDIRECTIONFLAG, PCB);
               } else {
                   Settings.AllDirectionLines = bVal;
                   if (bVal)
                       SET_FLAG(ALLDIRECTIONFLAG, PCB);
                   else
                       CLEAR_FLAG(ALLDIRECTIONFLAG, PCB);
               }
               AdjustAttachedObjects ();
               SetStatusLine ();
           } else {
               if (TEST_FLAG(ALLDIRECTIONFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_AUTOENFDRC:
           if (objc == 3) {
               if (bVal) {
                   SET_FLAG(AUTODRCFLAG, PCB);
                   SaveUndoSerialNumber ();
                   ResetFoundPinsViasAndPads (True);
                   RestoreUndoSerialNumber ();
                   ResetFoundLinesAndPolygons (True);
               } else
                   CLEAR_FLAG(AUTODRCFLAG, PCB);
           } else {
              if (TEST_FLAG(AUTODRCFLAG, PCB))
                  strcpy(rStr,"on");
              else
                  strcpy(rStr,"off");
              robj = Tcl_NewStringObj(rStr,-1);
              Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_AUTOSWAPLSA:
           if (objc == 3) {
               Settings.SwapStartDirection = bVal;
               if (bVal)
                   SET_FLAG(SWAPSTARTDIRFLAG, PCB);
               else
                   CLEAR_FLAG(SWAPSTARTDIRFLAG, PCB);
               SetStatusLine ();
           } else {
               if (Settings.SwapStartDirection)
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_AUTOZERODM:
           if (objc == 3) {
               if (bVal)
                   SET_FLAG(LOCALREFFLAG, PCB);
               else
                   CLEAR_FLAG(LOCALREFFLAG, PCB);
               SetStatusLine ();
           } else {
               if (TEST_FLAG(LOCALREFFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_CHECKPOLY:
           if (objc == 3) {
               if (bVal)
                   SET_FLAG(CHECKPLANESFLAG, PCB);
               else
                   CLEAR_FLAG(CHECKPLANESFLAG, PCB);
               SetStatusLine ();
               ClearAndRedrawOutput ();
           } else {
               if (TEST_FLAG(CHECKPLANESFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_CSSHOWSDRC:
           if (objc == 3) {
               Settings.ShowDRC = bVal;
               if (bVal)
                   SET_FLAG(SHOWDRCFLAG, PCB);
               else
                   CLEAR_FLAG(SHOWDRCFLAG, PCB);
           } else {
               if (Settings.ShowDRC)
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_CSSNAP:
           if (objc == 3) {
               Settings.SnapPin = bVal;
               if (bVal)
                   SET_FLAG(SNAPPINFLAG, PCB);
               else
                   CLEAR_FLAG(SNAPPINFLAG, PCB);
               SetStatusLine ();
           } else {
               if (Settings.SnapPin)
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_CYCLECLIP:
           /* Required while drawing Polygon, Line,
            * rectangle.
            */
           if TEST_FLAG(ALLDIRECTIONFLAG, PCB) {
               TOGGLE_FLAG(ALLDIRECTIONFLAG, PCB);
               PCB->Clipping = 0;
           } else
               PCB->Clipping = (PCB->Clipping + 1) % 3;
           AdjustAttachedObjects ();
           SetStatusLine ();
           break;

       case OPTION_NEWLACLEARPOLY:
           if (objc == 3) {
               if (bVal)
                   SET_FLAG(CLEARNEWFLAG, PCB);
               else
                   CLEAR_FLAG(CLEARNEWFLAG, PCB);
           } else {
               if (TEST_FLAG(CLEARNEWFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_ONLYARNETS:
           if (objc == 3) {
               if (bVal)
                   autorouted_only = 1;
               else
                   autorouted_only = 0;
           } else {
               if (djopt_get_auto_only())
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_ORTHOMOVES:
           if (objc == 3) {
               if (bVal)
                   SET_FLAG(ORTHOMOVEFLAG, PCB);
               else
                   CLEAR_FLAG(ORTHOMOVEFLAG, PCB);
           } else {
               if (TEST_FLAG(ORTHOMOVEFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_RUBBERBAND:
           if (objc == 3) {
               Settings.RubberBandMode = bVal;
               if (bVal)
                   SET_FLAG(RUBBERBANDFLAG, PCB);
               else
                   CLEAR_FLAG(RUBBERBANDFLAG, PCB);
               SetStatusLine ();
           } else {
               if (TEST_FLAG(RUBBERBANDFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_SHOWARTRIALS:
           if (objc == 3) {
               Settings.liveRouting = bVal;
               if (bVal)
                   SET_FLAG(LIVEROUTEFLAG, PCB);
               else
                   CLEAR_FLAG(LIVEROUTEFLAG, PCB);
               SetStatusLine ();
           } else {
               if (TEST_FLAG(LIVEROUTEFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_THINDRAW:
           if (objc == 3) {
               if (bVal)
                   SET_FLAG(THINDRAWFLAG, PCB);
               else
                   CLEAR_FLAG(THINDRAWFLAG, PCB);
           } else {
               if (TEST_FLAG(THINDRAWFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_UNIQUEELEMNAMES:
           if (objc == 3) {
               Settings.UniqueNames = bVal;
               if (bVal)
                   SET_FLAG(UNIQUENAMEFLAG, PCB);
               else
                   CLEAR_FLAG(UNIQUENAMEFLAG, PCB);
               SetStatusLine ();
           }else {
               if (TEST_FLAG(UNIQUENAMEFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case OPTION_PONUMNOTNAME:
           if (objc == 3) {
               if (bVal)
                   SET_FLAG(SHOWNUMBERFLAG, PCB);
               else
                   CLEAR_FLAG(SHOWNUMBERFLAG, PCB);
               SetStatusLine ();
           } else {
               if (TEST_FLAG(SHOWNUMBERFLAG, PCB))
                   strcpy(rStr,"on");
               else
                   strcpy(rStr,"off");
               robj = Tcl_NewStringObj(rStr,-1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"option\" option", 0);
           RestoreCrosshair (True);
           return TCL_ERROR;
   }
   RestoreCrosshair (True);
   return PCBTagCallback(interp, objc, objv);
}

#endif /* TCL_PCB */
