/*----------------------------------------------------------------------------*/
/* tclcommandGM.c:                                                            */
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
#include <ctype.h>   /* for tolower(), toupper() */
#include <math.h>    /* for zoom factor logf() function */

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
#include "netlist.h"
#include "remove.h"
#include "draw.h"
#include "error.h"
#include "action.h"
#include "gui.h"

#include "control.h"
#include "find.h"
#include "undo.h"
#include "change.h"
#include "select.h"

#ifdef HAVE_REGEX_H
#include <regex.h>
#else
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#ifdef TCL_PCB

#include "tclpcb.h"


/*----------------------------------------------------------------------*/
/*  Command "grid" ---	                                                */
/*----------------------------------------------------------------------*/

int pcbtcl_grid(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "display", "set", "realign", "get", NULL
   };
   enum OptionId {
       GRID_DISPLAY, GRID_SET, GRID_REALIGN, GRID_GET
   };
   static char *getoptions[] = {
       "value", "units", NULL
   };
   enum GetOptionId {
       GRID_GET_VALUE, GRID_GET_UNITS
   };

   Tcl_Obj *lobj, *fobj, *sobj;
   int result, idx, gidx;
   char *value = NULL;
   char tmpVal[128];
   double gVal = 0.0;
   Boolean rflag = False;

   /* "grid" conflicts with the Tk option of the same name.  This      */
   /* should be renamed to "tcl_grid" by the setup scripts.  We try    */
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

   if (objc >= 3)
       value = Tcl_GetString(objv[2]);

   switch (idx)
   {
       case GRID_DISPLAY:
           if (value) {
               if (!strcasecmp(value, "on"))
                   Settings.DrawGrid = True;
               else if (!strcasecmp(value, "off"))
                   Settings.DrawGrid = False;
               /* Update screen data after grid display change */
               UpdateAll();
           } else {
               if (Settings.DrawGrid)
                   strcpy(tmpVal,"on");
               else
                   strcpy(tmpVal,"off");
                   lobj = Tcl_NewStringObj(tmpVal,-1);
                   Tcl_SetObjResult(interp, lobj);
           }
           break;

       case GRID_SET:
           /* Allow user to set grid value relative to current setting. */
           
           if (objc < 3) {
               Tcl_SetResult(interp, "Usage : grid set <\"[+|-]\"value>[mil|mm]", 0);
               return TCL_ERROR;
           } else {
               if ((value[0] == '-')  || (value[0] == '+'))
                   rflag = True;
           }
           strcpy(tmpVal, value);
           if (objc == 3) 
               strtok(tmpVal, "m");
           else if (objc == 4)
               value = Tcl_GetString(objv[3]);
           
           if (strstr(value, "mm"))
               gVal = atof(tmpVal) / COOR_TO_MM;
           else if (strstr(value, "mil"))
               gVal = atof(tmpVal) * 100;
           else
               gVal = atof(tmpVal);

           /* PCB->Grid = gVal;   */
           SetGrid ((rflag) ? gVal + PCB->Grid : gVal, False);
           break;

      case GRID_REALIGN:
           SetGrid(PCB->Grid, True);
           break;

      case GRID_GET:
           if (objc == 3) {
               if ((result = Tcl_GetIndexFromObj(interp, objv[2], 
	                     (CONST84 char **) getoptions, "load option", 
			     0, &gidx)) != TCL_OK)
                   return result;
           } else
               gidx = -1;

           if (PCB->Grid == (int) PCB->Grid) {
               fobj = Tcl_NewDoubleObj((double)PCB->Grid / 100.0);
               sobj = Tcl_NewStringObj("mil", 3);
           } else {
               float tmpVal = PCB->Grid * 10.0;
               fobj = Tcl_NewDoubleObj((double)(ceilf(tmpVal*COOR_TO_MM)/10.0));
               sobj = Tcl_NewStringObj("mm", 2);
           }

           switch (gidx)
           {
               case GRID_GET_VALUE:
                   Tcl_SetObjResult(interp, fobj);
                   Tcl_DecrRefCount(sobj);
                   break;

               case GRID_GET_UNITS:
                   /* Tcl_SetResult(interp, sobj, 0); */
                   Tcl_SetObjResult(interp, sobj);
                   Tcl_DecrRefCount(fobj);
                   break;

               default:
                   lobj = Tcl_NewListObj(0, NULL);
                   Tcl_ListObjAppendElement(interp, lobj, fobj);
                   Tcl_ListObjAppendElement(interp, lobj, sobj);
                   Tcl_SetObjResult(interp, lobj);
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"grid\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/*  Command "layer" ---                                                 */
/*----------------------------------------------------------------------*/

void
setLayerOnOff(int layerNum, Boolean state) {
   int group, i;

   /* check if active layer is in the group;
    * if YES, make a different one active if possible
    */
   if ((group = GetGroupOfLayer (layerNum)) ==
          GetGroupOfLayer (MIN (MAX_LAYER, INDEXOFCURRENT))) {
       /* find next visible layer and make it
        * active so this one can be turned off
        */
       for (i = (layerNum + 1) % (MAX_LAYER + 1);
                 i != layerNum; i = (i + 1) % (MAX_LAYER + 1))
           if (PCB->Data->Layer[i].On == True && GetGroupOfLayer (i) != group)
               break;

       if (i != layerNum) {
           /* Found next visible layer, activate layer. */
           ChangeGroupVisibility ((int) i, True, True);
       } else {
           /* All layer except this one are invisible. Can't
            * make this layer invisible */	
           return ;
       }
   }
   /* switch layer group on/off */
   ChangeGroupVisibility (layerNum, state, False);
   UpdateAll();
}

int pcbtcl_layer(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "active", "color", "name", "visible", "invisible", 
       "group", "members", "toggle_visible", "is_visible", NULL
   };
   enum OptionId {
       LAYER_ACTIVE, LAYER_COLOR, LAYER_NAME, LAYER_VISIBLE,
       LAYER_INVISIBLE, LAYER_GROUP, LAYER_MEMBERS, 
       LAYER_TOGGLE_VISIBLE, LAYER_IS_VISIBLE
   };

   int result, idx, lNum=0;
   XColor *color, lcolor;
   char *value = NULL;
   char layerGroups[128];
   char tmpStr[32];
   int layerNum = -1;
   int group, entry, foundGroup = 0;
   Tcl_Obj *robj;
   Tk_Window tkwind = Tk_MainWindow(interp);
   LayerTypePtr currentLayer;
   Boolean isLayerVisible = False;

   if (objc == 1) {
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
              options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
              options, "option", 0, &idx)) != TCL_OK)
       return result;

   if (PCB == NULL) return TCL_ERROR;

   /* The third argument can be a layer name or number */
   if (objc >= 3) {
       if (Tcl_GetIntFromObj(NULL, objv[2], &layerNum) != TCL_OK) {
           layerNum = -1;
           value = Tcl_GetString(objv[2]);
           if (!strcasecmp(value, "netlist")) {
               layerNum =  MAX_LAYER + 1;
           } else {
               for (lNum = 0; lNum <= MAX_LAYER+1; lNum++) {
                   if (!strcasecmp(PCB->Data->Layer[lNum].Name, value)) {
                       layerNum = lNum;
                       value = NULL;
                       break;
                   }
               }
           }
       }
   }

   currentLayer = CURRENT;

   switch (idx)
   {
       case LAYER_ACTIVE:
           if (objc == 3) { /* set active layer */
               if (layerNum >= 0  && layerNum <= MAX_LAYER+1) {
                   /* PushOnTopOfLayerStack(layerNum); */
                   ChangeGroupVisibility (layerNum, True, True);
                   currentLayer = CURRENT;
                   UpdateAll();
               } else {
                   Tcl_SetResult(interp, "Must specify a valid layer name or number", 0);
                   //return TCL_ERROR;
               }
           } else {
               robj = Tcl_NewStringObj((PCB->RatDraw ? "netlist" : 
	                               currentLayer->Name), -1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case LAYER_COLOR:
           if (objc == 2) {
               /* Report current layer color */
               lcolor.pixel = PCB->RatDraw ? PCB->RatColor : 
	                      currentLayer->Color;
               lcolor.flags = DoRed | DoGreen | DoBlue;
               XQueryColor(Dpy, Tk_Colormap(tkwind), &lcolor);
               value = (char *)Tk_NameOfColor(&lcolor);
               robj = Tcl_NewStringObj(value, -1);
               Tcl_SetObjResult(interp, robj);
           } else if (objc == 3) {
               /* Report specified layer or change current layer color */
               if (layerNum >= 0  && layerNum < MAX_LAYER + 5) {
                   switch  (layerNum) 
                   {
                       case MAX_LAYER:           /* settings for elements */
                           lcolor.pixel = PCB->ElementColor;
                           break;

                        case MAX_LAYER + 1:       /* settings for pin */
                           lcolor.pixel = PCB->PinColor;
                           break;

                        case MAX_LAYER + 2:       /* settings for vias */
                           lcolor.pixel = PCB->ViaColor;
                           break;

                        case MAX_LAYER + 3:      /* settings for objects 
			                            on other side */
                           lcolor.pixel = PCB->InvisibleObjectsColor;
                           break;

                        case MAX_LAYER + 4:      /* settings for rats */
                           lcolor.pixel = PCB->RatColor;
                           break;

                        default:
                            lcolor.pixel = PCB->Data->Layer[layerNum].Color;
                            break;
                   }
                   lcolor.flags = DoRed | DoGreen | DoBlue;
                   XQueryColor(Dpy, Tk_Colormap(tkwind), &lcolor);
                   value = (char *)Tk_NameOfColor(&lcolor);
                   robj = Tcl_NewStringObj(value, -1);
                   Tcl_SetObjResult(interp, robj);
               } else {
                   color = Tk_GetColor(interp, tkwind, Tcl_GetString(objv[2]));
                   if (color == NULL) return TCL_ERROR;
                   currentLayer->Color = (int)color->pixel;
                   UpdateAll();
               }
           } else if (objc == 4) {
               /* Changed specified layer color */
               if (layerNum >=  MAX_LAYER) {\
                   Tcl_SetResult(interp, "Can't change the color.", 0);
                   return TCL_ERROR;
               }
               color = Tk_GetColor(interp, tkwind, Tcl_GetString(objv[3]));
               if (color == NULL) return TCL_ERROR;
               PCB->Data->Layer[layerNum].Color = (int)color->pixel;                UpdateAll();
           }
           break;

       case LAYER_VISIBLE:
           if (objc != 3) {
               /* Return status of layer */
               robj = Tcl_NewBooleanObj((int)currentLayer->On);
               Tcl_SetObjResult(interp, robj);
           } else {
               if ((value != NULL) && (!strcasecmp(value, "all"))) {
                   for (lNum = 0; lNum <= MAX_LAYER; lNum++)
                       ChangeGroupVisibility(lNum, True, True);
                   UpdateAll();
               } else {
                   setLayerOnOff(layerNum, True);
               }
           }
           break;

       case LAYER_INVISIBLE:
           if (objc != 3) {
               /* Return status of layer */
               robj = Tcl_NewBooleanObj((currentLayer->On) ? 0 : 1);
               Tcl_SetObjResult(interp, robj);
           } else {
               if ((value != NULL) && (!strcasecmp(value, "all"))) {
                   for (lNum = 0; lNum < MAX_LAYER; lNum++)
                       ChangeGroupVisibility(lNum, False, True);
                   /* Activate "Silk" layer as none layer is visible */
                   PCB->SilkActive = True;
                   UpdateAll();
               } else {
                   setLayerOnOff(layerNum, False);
               }
           }
           break;

       case LAYER_TOGGLE_VISIBLE:
           if ((objc == 3)  && (layerNum >= 0) && (layerNum <= MAX_LAYER)) {
               setLayerOnOff(layerNum, !PCB->Data->Layer[layerNum].On);
           } else {
               Tcl_SetResult(interp, "Must specify a valid layer number", 0);
               return TCL_ERROR;
           }
           break;

       case LAYER_NAME:
           if (objc == 3) {
               /* 3 arguments means that we are either trying to rename  */
               /* the current layer, or trying to determine the name     */
               /* corresponding to a specific layer number.              */
               /* Report specified layer or change current layer color   */
               if (layerNum >= 0  && layerNum < MAX_LAYER + 5) {
                   switch  (layerNum)
                   {
                       case MAX_LAYER:              /* settings for elements */
                           robj = Tcl_NewStringObj("silk", -1);
                           break;

                       case MAX_LAYER + 1:          /* settings for pin */
                           robj = Tcl_NewStringObj("pins/pads", -1);
                           break;

                       case MAX_LAYER + 2:          /* settings for vias */
                           robj = Tcl_NewStringObj("vias", -1);
                           break;

                       case MAX_LAYER + 3:          /* settings for 
                                                       objects on other side */
                           robj = Tcl_NewStringObj("far side", -1);
                           break;

                       case MAX_LAYER + 4:          /* settings for rats */
                           robj = Tcl_NewStringObj("netlist", -1);
                           break;

                       default:
                           robj = Tcl_NewStringObj(PCB->Data->
			                           Layer[layerNum].Name, -1);
                           break;
                   }
                   Tcl_SetObjResult(interp, robj);
               } else if ((value != NULL) && (!strcasecmp(value, "all"))) {
                   for (lNum = 0; lNum < MAX_LAYER+1; lNum++) {
                       Tcl_AppendElement(interp, PCB->Data->Layer[lNum].Name);
                   }
               }
           } else if (objc == 4) {
               /* Require that the original name must be given in order     */
               /* to change it.  Otherwise, it's much too easy to           */
               /* accidentally change the name of the current layer.        */

               if (layerNum >= 0 && layerNum < MAX_LAYER+1) {
                   value = Tcl_GetString(objv[3]);
                   free(PCB->Data->Layer[layerNum].Name);
                   PCB->Data->Layer[layerNum].Name = strdup(value);
               }
           } else {
               robj = Tcl_NewStringObj(currentLayer->Name, -1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case LAYER_GROUP:
           if (objc == 3) {
               /* format of string is same as format in file
                * need to add checks for command line input 
                * string generated from GUI is already checked 
                */
               ParseGroupString(value, &(PCB->LayerGroups));
           } else {
               strcpy(layerGroups, "");
               for (group = 0; group < MAX_LAYER; group++) {
                   if (PCB->LayerGroups.Number[group]) {
                       for (entry = 0; entry < PCB->LayerGroups.Number[group]; 
		            entry++) {
                           switch (PCB->LayerGroups.Entries[group][entry])
                           {
                               case MAX_LAYER + COMPONENT_LAYER:
                                   strcat(layerGroups,"c");
                                   break;

                               case MAX_LAYER + SOLDER_LAYER:
                                   strcat(layerGroups, "s");
                                   break;

                               default:
                                   sprintf (tmpStr, "%d",
                                   PCB->LayerGroups.Entries[group][entry] + 1);
                                   strcat(layerGroups, tmpStr);
                                   break;
                           }
                           if (entry != PCB->LayerGroups.Number[group] - 1)
                               strcat(layerGroups, ",");
                       }
                       if (group != MAX_LAYER - 1)
                           strcat(layerGroups, ":");
                   }
               }
               robj = Tcl_NewStringObj(layerGroups, -1);
               Tcl_SetObjResult(interp, robj);
           }
           break;

       case LAYER_MEMBERS:
           if ((objc == 3)  && (layerNum > -1) && (layerNum < MAX_LAYER)) {
               for (group = 0; group < MAX_LAYER; group++) {
                   strcpy(layerGroups, "");
                   if (PCB->LayerGroups.Number[group]) {
                       for (entry = 0; entry < PCB->LayerGroups.Number[group]; 
		            entry++) {
                           switch (PCB->LayerGroups.Entries[group][entry])
                           {
                               case MAX_LAYER + COMPONENT_LAYER:
                                   strcat(layerGroups,"c");
                                   break;

                               case MAX_LAYER + SOLDER_LAYER:
                                   strcat(layerGroups, "s");
                                   break;

                               default:
                                   sprintf (tmpStr, "%d",
                                   PCB->LayerGroups.Entries[group][entry] + 1);
                                   strcat(layerGroups, tmpStr);
                                   break;
                           }
                           if (entry != PCB->LayerGroups.Number[group] - 1)
                               strcat(layerGroups, " ");
                           if ((PCB->LayerGroups.Entries[group][entry]) + 1 
			       == layerNum)
                               foundGroup = 1;
                       }
                       if (foundGroup)
                           break;
                   }
               }
               robj = Tcl_NewStringObj(layerGroups, -1);
               Tcl_SetObjResult(interp, robj);
       } else {
           Tcl_SetResult(interp, "Must specify a valid layer number", 0);
           return TCL_ERROR;
       }
       break;

       case LAYER_IS_VISIBLE:
           if ((objc == 3)  && (layerNum >=0) && (layerNum < MAX_LAYER+5)) {
               switch (layerNum)
               {
                   case MAX_LAYER:             /* settings for elements */
                       isLayerVisible = PCB->ElementOn;
                       break;

                   case MAX_LAYER + 1:         /* settings for pins */
                       isLayerVisible = PCB->PinOn;
                       break;

                   case MAX_LAYER + 2:         /* settings for vias */
                       isLayerVisible = PCB->ViaOn;
                       break;

                   case MAX_LAYER + 3:         /* settings for 
                                                  objects on other side */
                       isLayerVisible = PCB->InvisibleObjectsOn;
                       break;

                   case MAX_LAYER + 4:         /* settings for rats */
                       isLayerVisible = PCB->RatOn;
                       break;

                   default:                    /* layers */
                       isLayerVisible = PCB->Data->Layer[layerNum].On;
                       break;
               }
               robj = Tcl_NewStringObj(isLayerVisible ? "on" : "off", -1);
               Tcl_SetObjResult(interp, robj);
           } else {
               Tcl_SetResult(interp, "Must specify a valid layer number", 0);
               return TCL_ERROR;
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"layer\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/*  Command "layout" ---                                                */
/*----------------------------------------------------------------------*/

int pcbtcl_layout(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "center", "name", "new", "load", "redraw", "save", "size", 
       "size_range", "view", NULL
   };
   enum Id {
       LAYOUT_CENTER, LAYOUT_NAME, LAYOUT_NEW, LAYOUT_LOAD,
       LAYOUT_REDRAW, LAYOUT_SAVE, LAYOUT_SIZE, LAYOUT_SIZE_RANGE, LAYOUT_VIEW
   };

   Tcl_Obj *robj, *lobj;
   int result, idx, xval, yval;
   char *name = NULL;
   char data[512];
   Tcl_Obj *sobj;
   BDimension minWidth, maxWidth, minHeight, maxHeight;

   if (objc == 1) {
       //Tcl_WrongNumArgs(interp, objc, objv, "<option>");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
              options, "option", 0, &idx);
       return TCL_ERROR;
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
       options, "option", 0, &idx)) != TCL_OK)
       return result;

   if (objc >= 3)
       name = Tcl_GetString(objv[2]);

   switch (idx)
   {
       /* center cursor and move X pointer too */
       case LAYOUT_CENTER:
           CenterDisplay (Crosshair.X, Crosshair.Y, False);
           warpNoWhere ();
           break;

       case LAYOUT_NAME:
           if (name) {
               if (PCB) {
                   PCB->Name = StrDup(NULL, name);
                   SetChangedFlag(True);
               }
           } else {
               Tcl_SetResult(interp, PCB->Name, 0);
               return TCL_OK;
           }
           break;

       case LAYOUT_NEW:
           HideCrosshair (True);
           if ((!PCB->Changed) || ConfirmDialog ("OK to clear layout data?")) {
               Tcl_EvalEx(pcbinterp, "GetLayoutName", -1, 0);
               // Changed to make use of the Tcl script
               // name = GetUserInput ("enter the layout's name:", "");
               name = Tcl_GetString(Tcl_GetObjResult(pcbinterp));
               if ((!name) || (name[0] == '\0'))
                   name = "UNNAMED";
               else
                   name = StrDup(NULL, name);

               /* do emergency saving
                * clear the old struct and allocate memory for the new one
                */
               if (PCB->Changed && Settings.SaveInTMP)
                   SaveInTMP ();
               RemovePCB (PCB);
               PCB = CreateNewPCB (True);

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
           break;

        case LAYOUT_LOAD:
           HideCrosshair (True);
           if (name) {
               if ((!PCB->Changed) || ConfirmDialog ("OK to override layout data?")) {
                   LoadPCB (name);
                   /* display netlist dialog if netlist initialized */
                   if (PCB->NetlistLib.MenuN)
                       Tcl_Eval(interp, "NetlistDialog");
               }
           }
           RestoreCrosshair (True);
           break;

       case LAYOUT_REDRAW:
           RedrawLayout();
           break;

       case LAYOUT_SAVE:
           /* Save data to file */
           if (!name) {
               if (PCB->Filename) {
                   /* Save layout in file PCB->Filename */
                   SavePCB (PCB->Filename);
               } else {
                   /* raise FileDiaog window to select filename
                    * and call layout save command again.
                    */
                   if ((result = Tcl_Eval(interp, "FileDialog {save layout} {layout save}")) != TCL_OK)
                       return result;
               }
           } else {
               /* SaveAs layout as other filename */
               FILE *exist;

               if ((exist = fopen (name, "r"))) {
                   fclose (exist);
                   if (ConfirmDialog ("File exists!  Ok to overwrite ?"))
                       SavePCB (name);
               } else {
                   SavePCB (name);
               }
           }
           break;

       case LAYOUT_SIZE:
           if (objc == 2) {
               lobj = Tcl_NewListObj(0, NULL);
               robj = Tcl_NewIntObj((int)PCB->MaxWidth);
               Tcl_ListObjAppendElement(interp, lobj, robj);
               robj = Tcl_NewIntObj((int)PCB->MaxHeight);
               Tcl_ListObjAppendElement(interp, lobj, robj);
               Tcl_SetObjResult(interp, lobj);
           } else if (objc == 4) {
               /* Parse command line for new size dimensions */
               if (Tcl_GetIntFromObj(interp, objv[2], &xval) != TCL_OK)
                   return TCL_ERROR;
               if (Tcl_GetIntFromObj(interp, objv[3], &yval) != TCL_OK)
                   return TCL_ERROR;
               ChangePCBSize (xval, yval);
           } else {
               Tcl_SetResult(interp, "layout size \"[<maxwidth> <maxheight>]\"", 0);
               return TCL_ERROR;
           }
           break;

       case LAYOUT_SIZE_RANGE:
           minWidth = 0;
           maxWidth = MAX_COORD;
           minHeight = 0;
           maxHeight = MAX_COORD;
           sprintf(data, "%d %d %d %d", minWidth, maxWidth, 
	           minHeight, maxHeight);
           sobj = Tcl_NewStringObj(data, -1);
           Tcl_SetObjResult(interp, sobj);
           break;

       case LAYOUT_VIEW:
           /* Report or set viewport bounds */
           if (objc == 2) {
               lobj = Tcl_NewListObj(0, NULL);
               robj = Tcl_NewIntObj((int)Xorig);
               Tcl_ListObjAppendElement(interp, lobj, robj);
               robj = Tcl_NewIntObj((int)Yorig);
               Tcl_ListObjAppendElement(interp, lobj, robj);
               robj = Tcl_NewIntObj((int)TO_PCB_X(Output.Width));
               Tcl_ListObjAppendElement(interp, lobj, robj);
               robj = Tcl_NewIntObj((int)TO_PCB_Y(Output.Height));
               Tcl_ListObjAppendElement(interp, lobj, robj);
               Tcl_SetObjResult(interp, lobj);
           } else if ((objc == 4) || (objc == 6)) {
               int x2, y2;
               float zx, zy;

               /* Regenerate viewport bounds.  Two integers indicates */
               /* a repositioning of the origin (upper-left corner).  */
               /* Four integers specifies the entire viewport, and so */
               /* sets both the position and the zoom factor.         */

               if (Tcl_GetIntFromObj(interp, objv[2], &xval) != TCL_OK)
                   return TCL_ERROR;
               if (Tcl_GetIntFromObj(interp, objv[3], &yval) != TCL_OK)
                   return TCL_ERROR;

               if (objc == 6) {
                   if (Tcl_GetIntFromObj(interp, objv[4], &x2) != TCL_OK)
                       return TCL_ERROR;
                   if (Tcl_GetIntFromObj(interp, objv[5], &y2) != TCL_OK)
                       return TCL_ERROR;
               }
               Xorig = xval;
               Yorig = yval;
               if (objc == 6) {
		   /* Pan(X, Y, Scroll, Update) */
                   Pan(Xorig, Yorig, True, False);
                   zx = (float)Output.Width / (float)(x2 - Xorig);
                   zy = (float)Output.Height / (float)(y2 - Yorig);
                   SetZoom(logf(0.01 / ((zx > zy) ? zy : zx)) / LN_2_OVER_2);
               } else {
		   /* Pan(X, Y, Scroll, Update) */
                   Pan(Xorig, Yorig, True, True);
               }
           }
           break;

       default:
           Tcl_SetResult(interp, "Unimplemented \"layout\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "line" ---                                                  */
/*----------------------------------------------------------------------*/

int pcbtcl_line(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
	"draw", "selected_size", "size", "size_range",
	"clearance", "selected_clearance", NULL
   };
   enum OptionId {
	LINE_DRAW, LINE_SELECTED_SIZE, LINE_SIZE,
	LINE_SIZE_RANGE, LINE_CLEARANCE, LINE_SELECTED_CLEARANCE
   };

   int result, idx, llx, lly, urx, ury;
   char *value = NULL;
   int size = 0;
   double dval = 0;
   char tmpVal[128];
   LineTypePtr line;
   Boolean isAbsolute = True;
   BDimension minDim, maxDim;
   Tcl_Obj *lobj;

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
        case LINE_SELECTED_SIZE:
        case LINE_SIZE:
        case LINE_SELECTED_CLEARANCE:
        case LINE_CLEARANCE:
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
                   size = (int)(0.5 + atof(tmpVal) / COOR_TO_MM);
               else if (strstr(value, "mil"))
                   size = (int)(atof(tmpVal) * 100.0);
               else
                   size = (int)(atof(tmpVal));
	   }

        break;
    }

   switch (idx)
   {
       case LINE_DRAW:
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

               line = CreateDrawnLineOnLayer (CURRENT, llx, lly, urx, ury,
                          Settings.LineThickness, 2 * Settings.Keepaway,
                          (TEST_FLAG (AUTODRCFLAG, PCB) ? FOUNDFLAG : 0) |
                          (TEST_FLAG (CLEARNEWFLAG, PCB) ? CLEARLINEFLAG : 0));
               if(line) {
                   addedLines++;
                   AddObjectToCreateUndoList (LINE_TYPE, CURRENT, line, line);
                   IncrementUndoSerialNumber ();
                   DrawLine (CURRENT, line, 0);
                   Draw ();
               }
           } else {
               Tcl_SetResult(interp, "(llx, lly) (urx, ury) values required.", 0);
               return TCL_ERROR;
           }
           break;

       case LINE_SELECTED_SIZE:
           /* Change line thickness of selected lines  */
           if (objc >= 3) {
               HideCrosshair(True);
               if (ChangeSelectedSize (LINE_TYPE, (Location)size, isAbsolute))
                   SetChangedFlag (True);
               RestoreCrosshair(True);
           } else {
               Tcl_SetResult(interp, "Usage : line selected_size <\"[+|-]\"value> \"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case LINE_SIZE:
           /* Change line thickness of line */
           if (objc == 2) {
               /* Return Size of Line Thickness */
               lobj = Tcl_NewIntObj(Settings.LineThickness);
               Tcl_SetObjResult(interp, lobj);
           } else if (objc >= 3) {
               HideCrosshair(True);
               SetLineSize (isAbsolute ? size : size + Settings.LineThickness);
               RestoreCrosshair(True);
           } else {
               Tcl_SetResult(interp, "Usage : line size <\"[+|-]\"value> \"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case LINE_SELECTED_CLEARANCE:
           /* Change clearance around selected Line */
           if (objc >= 3) {
               HideCrosshair (True);
               if (ChangeSelectedClearSize (LINE_TYPE, size, isAbsolute))
                   SetChangedFlag (True);
                   RestoreCrosshair (True);
           } else {
               Tcl_SetResult(interp, "Usage : line selected_clearance "
                        "<\"[+|-]\"value> \"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case LINE_CLEARANCE:
           /* Report/Change clearance around Line */
           if (objc == 2) {
               /* Return Size of clearance around line */
               lobj = Tcl_NewIntObj(Settings.Keepaway);
               Tcl_SetObjResult(interp, lobj);
           } else if (objc >= 3) {
               HideCrosshair(True);
               SetKeepawayWidth (isAbsolute ? size : size + Settings.Keepaway);
               RestoreCrosshair(True);
           } else {
               Tcl_SetResult(interp, "Usage : line clearance <\"[+|-]\"value> "
                        "\"[mil|mm]\" ", 0);
               return TCL_ERROR;
           }
           break;

       case LINE_SIZE_RANGE:
           minDim = MIN_LINESIZE;\
           maxDim = MAX_LINESIZE;
           sprintf(tmpVal, "%d %d", minDim, maxDim);
           lobj = Tcl_NewStringObj(tmpVal, -1);
           Tcl_SetObjResult(interp, lobj);
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"line\" option", 0);
           return TCL_ERROR;
   }
   return PCBTagCallback(interp, objc, objv);
}

/*----------------------------------------------------------------------*/
/*  Command "library" ---                                               */
/*----------------------------------------------------------------------*/

static LibraryMenuTypePtr menuPtr = NULL;
int pcbtcl_library(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   static char *options[] = {
       "elements", "info", "pick", NULL
   };
   enum OptionId {
      LIBRARY_ELEMENTS, LIBRARY_INFO, LIBRARY_PICK
   };

   int result, idx;
   Boolean isFound = False;
   char *name = NULL;
   char libArgs[2048];
   char elemDesc[2048];
   char libDesc[2048];
   Tcl_Obj *lobj = NULL;

   if (objc == 1) {
       Tcl_WrongNumArgs(interp, objc, objv, "<option>");
       Tcl_GetIndexFromObj(interp, Tcl_NewStringObj("",0), (CONST84 char **)
                options, "option", 0, &idx);
       return TCL_ERROR;
   } else if (objc == 3) {
       name = Tcl_GetString(objv[2]);
   }

   if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
            options, "option", 0, &idx)) != TCL_OK)
       return result;

   switch (idx)
   {
       case LIBRARY_INFO:
           if (!Library.MenuN)
               ReadLibraryContents();
               lobj = Tcl_NewStringObj(0, 0);
               MENU_LOOP (&Library);
               {
                   sprintf(libDesc, "\"%s\"", menu->Name);
                   Tcl_AppendStringsToObj(lobj, libDesc, " ", 0);
               }
               END_LOOP;
               break;

       case LIBRARY_ELEMENTS:
           if (name == NULL) {
               if (menuPtr == NULL) {
                   Tcl_SetResult(interp, "Usage : library elements <library-name>", 0);
                   return TCL_ERROR;
               }
           }

           if (!Library.MenuN)
               ReadLibraryContents();
               lobj = Tcl_NewStringObj(0, 0);
               if (name) {
                   menuPtr = NULL;
                   MENU_LOOP (&Library);
                   {
                       if (!strcmp(name, menu->Name))
                           menuPtr = menu;
                   }
                   END_LOOP;
               }
               if (menuPtr) {
                   if (menuPtr->EntryN) {
                       ENTRY_LOOP (menuPtr);
                       {
                           sprintf(elemDesc, "\"%s\"", entry->ListEntry);
                           Tcl_AppendStringsToObj(lobj, elemDesc, " ", 0);
                       }
                       END_LOOP;
                   } /*else {
                       Tcl_SetResult(interp, "No elements in library!", 0);
                       return TCL_ERROR;
                   }*/
               } else {
                   Tcl_SetResult(interp, "Library not found!", 0);
                   return TCL_ERROR;
               }
               break;

       case LIBRARY_PICK:
           if (name == NULL) {
               Tcl_SetResult(interp, "Usage : library pick <element-name>", 0);
               return TCL_ERROR;
           }

           if (menuPtr) {
               ENTRY_LOOP (menuPtr);
               {
                   if (!strcmp(name, entry->ListEntry)) {
                       isFound = True;
                       if (entry->Template == (char *) -1) {
                           if (LoadElementToBuffer (PASTEBUFFER,
                                       entry->AllocatedMemory, True))
                               SetMode (PASTEBUFFER_MODE);
                       } else {
                           sprintf (libArgs, "'%s' '%s' '%s'", 
			            EMPTY (entry->Template),
                                    EMPTY (entry->Value),
                                    EMPTY (entry->Package));
                           if (LoadElementToBuffer(PASTEBUFFER, libArgs, False))
                               SetMode (PASTEBUFFER_MODE);
                       }
                       break;
                   }
               }
               END_LOOP;
           } else {
               Tcl_SetResult(interp, "No current library! Execute library elements <library-name> before lib pick <element-name>", 0);
               return TCL_ERROR;
           }
           if(!isFound) {
               Tcl_SetResult(interp, "Element not found in current library!", 0);
               return TCL_ERROR;
           }
           break;

       default :
           Tcl_SetResult(interp, "Unimplemented \"library\" option", 0);
           return TCL_ERROR;
   }
   if (lobj)
       Tcl_SetObjResult(interp, lobj);
   return PCBTagCallback(interp, objc, objv);
}


/*----------------------------------------------------------------------*/
/* Command "mode" --- set or query the drawing mode.                    */
/*----------------------------------------------------------------------*/

int pcbtcl_mode(ClientData clientData, Tcl_Interp *interp, int objc,
	Tcl_Obj *CONST objv[])
{
   enum ModeOptionId {
      MODE_NOTIFY, MODE_RELEASE, MODE_RESTORE, MODE_SAVE, MODE_STROKE
   };
   static char *modeNames[] = {
       "notify", "release", "restore", "save", "stroke",
       "pan", "none", "via", "line", "rectangle", "polygon", "paste_buffer",
       "text", "rotate", "remove", "move", "copy", "insert_point",
       "rubberband_move", "thermal", "arc", "arrow", "lock", NULL
   };
   static int modeValues[] = {
       -1, -1, -1, -1, -1,
       PAN_MODE, NO_MODE, VIA_MODE, LINE_MODE, RECTANGLE_MODE,
       POLYGON_MODE, PASTEBUFFER_MODE, TEXT_MODE, ROTATE_MODE,
       REMOVE_MODE, MOVE_MODE, COPY_MODE, INSERTPOINT_MODE,
       RUBBERBANDMOVE_MODE, THERMAL_MODE, ARC_MODE, ARROW_MODE,
       LOCK_MODE
   };
   int result, idx;

   if (objc == 1) {
       int i;
       for (i = 0; i < (sizeof(modeValues)/sizeof(int)); i++) {
           if (modeValues[i] == Settings.Mode) {
               Tcl_SetResult(interp, modeNames[i], NULL);
               return PCBTagCallback(interp, objc, objv);
           }
       }
       return TCL_ERROR;
   } else {
       if ((result = Tcl_GetIndexFromObj(interp, objv[1], (CONST84 char **)
                  modeNames, "drawing modes", 0, &idx)) != TCL_OK) {
           return result;
       }

       SetNoteXY();
       HideCrosshair(True);

       switch(idx)
       {
           case MODE_NOTIFY:
               NotifyMode();
               break;
           case MODE_RELEASE:
               ReleaseMode();	// To-do: handle strokes
               break;
           case MODE_SAVE:
               SaveMode();
               break;
           case MODE_RESTORE:
               /* Restore last mode to continue with same process */
               RestoreMode();
               break;
           case MODE_STROKE:
               DoStroke();
               break;
           default:
               SetMode(modeValues[idx]);
               break;
       }
       RestoreCrosshair(True);
   }
   return PCBTagCallback(interp, objc, objv);
}

#endif /* TCL_PCB */
