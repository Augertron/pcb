/*----------------------------------------------------------------------------*/
/* tclpcb.c:                                                                  */
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
#include "misc.h"
#include "create.h"
#include "crosshair.h"
#include "buffer.h"
#include "set.h"
#include "error.h"
#include "mymem.h"
#include "file.h"
#include "draw.h"
#include "library.h"

/*----------------------------------------------------------------------*/
/* Deal with systems which don't define va_copy().                      */
/*----------------------------------------------------------------------*/
#ifndef HAVE_VA_COPY
  #ifdef HAVE___VA_COPY
    #define va_copy(a, b) __va_copy(a, b)
  #else
    #define va_copy(a, b) a = b
  #endif
#endif


#ifdef TCL_PCB

#include "tclpcb.h"

Tcl_HashTable PCBTagTable;
Tcl_Interp *pcbinterp;
Tcl_Interp *consoleinterp;

/*----------------------------------------------------------------------*/

static cmdstruct pcb_commands[] =
{
   {"about", pcbtcl_about},
   {"arc", pcbtcl_arc},
   {"buffer", pcbtcl_buffer},
   {"crosshair", pcbtcl_crosshair},
   {"delete", pcbtcl_delete},
   {"deselect", pcbtcl_deselect},
   {"drc", pcbtcl_drc},
   {"drill", pcbtcl_drill},
   {"element", pcbtcl_element},
   {"grid", pcbtcl_grid},
   {"layer", pcbtcl_layer},
   {"layout", pcbtcl_layout},
   {"library", pcbtcl_library},
   {"line", pcbtcl_line},
   {"mode", pcbtcl_mode},
   {"netlist", pcbtcl_netlist},
   {"object", pcbtcl_object},
   {"option", pcbtcl_option},
   {"pad", pcbtcl_pad},
   {"pin", pcbtcl_pin},
   {"pinout", pcbtcl_pinout},
   {"place", pcbtcl_place},
   {"pointer", pcbtcl_pointer},
   {"polygon", pcbtcl_polygon},
   {"print", pcbtcl_print},
   {"quit", pcbtcl_quit},
   {"rectangle", pcbtcl_rectangle},
   {"redo", pcbtcl_redo},
   {"route", pcbtcl_route},
   {"scan", pcbtcl_scan},
   {"select", pcbtcl_select},
   {"show", pcbtcl_show},
   {"style", pcbtcl_style},
   {"text", pcbtcl_text},
   {"thermal", pcbtcl_thermal},
   {"undo", pcbtcl_undo},
   {"via", pcbtcl_via},
   {"zoom", pcbtcl_zoom},
   {"", NULL} /* sentinel */
};

/*----------------------------------------------------------------------*/
/* Redraw the entire layout.						*/
/*----------------------------------------------------------------------*/

void
RedrawLayout()
{
    XRectangle area;
    Region region;

    region = XCreateRegion();
    area.x = 0;
    area.y = 0;
    area.width = Output.Width;
    area.height = Output.Height;
    XUnionRectWithRegion(&area, region, region);
    DrawClipped(region);
}

/*----------------------------------------------------------------------*/
/* Reimplement GetUserInput() from src/dialog.c				*/
/* This uses the status line in PCB for input, as did the original	*/
/*----------------------------------------------------------------------*/

char *
GetUserInput(char *MessageText, char *OutputString)
{
   char *string = NULL;
   char *outString = NULL;

   if (OutputString != NULL)
      Tcl_SetVar(pcbinterp, "genvar", OutputString, TCL_GLOBAL_ONLY);
   else
      Tcl_SetVar(pcbinterp, "genvar", "", TCL_GLOBAL_ONLY);

   if (MessageText != NULL) {
      string = Tcl_Alloc(32 + strlen(MessageText));
      sprintf(string, "GetUserInput \"%s\" genvar {}", MessageText);
   }
   else
      sprintf(string, "GetUserInput \"\" genvar {}");

   Tcl_EvalEx(pcbinterp, string, -1, TCL_GLOBAL_ONLY);
   Tcl_Free(string);

   // need to do this as it gets free'd in action.c
   // or wrap free in action.c with ifdef TCL_PCB
   outString = strdup(Tcl_GetVar(pcbinterp, "genvar", TCL_GLOBAL_ONLY));
   return outString;
}

/*----------------------------------------------------------------------*/
/* Reimplement GetUserInput() from src/dialog.c				*/
/* This is a console/terminal-based standin for a popup dialog.		*/
/*----------------------------------------------------------------------*/


char *
GetConsoleInput(char *OutputString)
{
   char *string;
   Tcl_Obj *objPtr;
   int length;

   if (consoleinterp != pcbinterp)	/* tkcon console being used */
   {
      if (OutputString != NULL)
      {
	 string = Tcl_Alloc(20 + strlen(OutputString));
	 sprintf(string, "pcb::dialog \"\" \"%s\"\n", OutputString);
	 Tcl_EvalEx(pcbinterp, string, -1, 0);
	 Tcl_Free(string);
      }
      else
	 Tcl_EvalEx(pcbinterp, "pcb::dialog", 11, 0);
   }
   else		/* terminal-based */
   {
      if (OutputString != NULL)
      {
	 tcl_printf(stdout, "%s", OutputString);
	 tcl_stdflush(stdout);	 
      }
      Tcl_EvalEx(pcbinterp, "gets stdin", 10, 0);
   }

   objPtr = Tcl_GetObjResult(pcbinterp);
   string = Tcl_GetStringFromObj(objPtr, &length);

   if (length > 0)
      if (*(string + length - 1) == '\n')
	 length--;

   SendEnterNotify();
   XSync(Dpy, False);

   if (length == 0)
      return NULL;
   else
      return StrDup(NULL, string);
}

/*----------------------------------------------------------------------*/
/* Reimplement strdup() to use Tcl_Alloc().				*/
/*----------------------------------------------------------------------*/

char *Tcl_Strdup(const char *s)
{
   char *snew;
   int slen;

   slen = 1 + strlen(s);
   snew = Tcl_Alloc(slen);
   if (snew != NULL)
      memcpy(snew, s, slen);

   return snew;
}

/*----------------------------------------------------------------------*/
/* Resolve conflicts between PCB commands and Tcl/Tk commands		*/
/*----------------------------------------------------------------------*/

int resolve_conflict(ClientData clientData, Tcl_Interp *interp, int objc,
		Tcl_Obj *CONST objv[])
{
   char *objv0 = Tcl_GetString(objv[0]);
   char *optr = objv0;
   char *tcl_name;
   Tcl_Obj **newobjv;
   int result, i;

   if (!strncmp(optr, "::", 2)) optr += 2;
   if (!strncmp(optr, "pcb::", 5))
      return TCL_ERROR;		   /* Treat as an error so PCB command	*/
				   /* will be attempted.		*/

   tcl_name = Tcl_Alloc(5 + strlen(objv0));
   strcpy(tcl_name, "tcl_");
   strcat(tcl_name, optr);

   newobjv = (Tcl_Obj **)Tcl_Alloc(objc * sizeof(Tcl_Obj *));
   newobjv[0] = Tcl_NewStringObj(tcl_name, -1);
   for (i = 1; i < objc; i++)
      newobjv[i] = objv[i];

   result = Tcl_EvalObjv(interp, objc, newobjv, 0);

   Tcl_DecrRefCount(newobjv[0]);
   Tcl_Free((char *)newobjv);

   if (result == TCL_OK)
      return result;

   Tcl_ResetResult(interp);
   return TCL_ERROR;
}

/*----------------------------------------------------------------------*/
/* Reimplement vfprintf() as a call to Tcl_Eval().			*/
/*----------------------------------------------------------------------*/

void tcl_vprintf(FILE *f, const char *fmt, va_list args_in)
{
   va_list args;
   static char outstr[128] = "puts -nonewline std";
   char *outptr, *bigstr = NULL, *finalstr = NULL;
   int i, nchars, result, escapes = 0;

   /* If we are printing an error message, we want to bring attention	*/
   /* to it by mapping the console window and raising it, as necessary.	*/
   /* I'd rather do this internally than by Tcl_Eval(), but I can't	*/
   /* find the right window ID to map!					*/

   if ((f == stderr) && (consoleinterp != pcbinterp)) {
      Tk_Window tkwind;
      tkwind = Tk_MainWindow(consoleinterp);
      if ((tkwind != NULL) && (!Tk_IsMapped(tkwind)))
	 result = Tcl_Eval(consoleinterp, "wm deiconify .\n");
      result = Tcl_Eval(consoleinterp, "raise .\n");
   }

   strcpy (outstr + 19, (f == stderr) ? "err \"" : "out \"");
   outptr = outstr;

   /* This mess circumvents problems with systems which do not have	*/
   /* va_copy() defined.  Some define __va_copy();  otherwise we must	*/
   /* assume that args = args_in is valid.				*/

   va_copy(args, args_in);
   nchars = vsnprintf(outptr + 24, 102, fmt, args);
   va_end(args);

   if (nchars >= 102) {
      va_copy(args, args_in);
      bigstr = Tcl_Alloc(nchars + 26);
      strncpy(bigstr, outptr, 24);
      outptr = bigstr;
      vsnprintf(outptr + 24, nchars + 2, fmt, args);
      va_end(args);
    }
    else if (nchars == -1) nchars = 126;

    for (i = 24; *(outptr + i) != '\0'; i++) {
       if (*(outptr + i) == '\"' || *(outptr + i) == '[' ||
	  	*(outptr + i) == ']' || *(outptr + i) == '\\')
	  escapes++;
    }

    if (escapes > 0) {
      finalstr = Tcl_Alloc(nchars + escapes + 26);
      strncpy(finalstr, outptr, 24);
      escapes = 0;
      for (i = 24; *(outptr + i) != '\0'; i++) {
	  if (*(outptr + i) == '\"' || *(outptr + i) == '[' ||
	    		*(outptr + i) == ']' || *(outptr + i) == '\\') {
	     *(finalstr + i + escapes) = '\\';
	     escapes++;
	  }
	  *(finalstr + i + escapes) = *(outptr + i);
      }
      outptr = finalstr;
    }

    *(outptr + 24 + nchars + escapes) = '\"';
    *(outptr + 25 + nchars + escapes) = '\0';

    result = Tcl_Eval(consoleinterp, outptr);

    if (bigstr != NULL) Tcl_Free(bigstr);
    if (finalstr != NULL) Tcl_Free(finalstr);
}

/*------------------------------------------------------*/
/* Console output flushing which goes along with the	*/
/* routine tcl_vprintf() above.				*/
/*------------------------------------------------------*/

void tcl_stdflush(FILE *f)
{
   Tcl_SavedResult state;
   static char stdstr[] = "::flush stdxxx";
   char *stdptr = stdstr + 11;

   Tcl_SaveResult(pcbinterp, &state);
   strcpy(stdptr, (f == stderr) ? "err" : "out");
   Tcl_Eval(pcbinterp, stdstr);
   Tcl_RestoreResult(pcbinterp, &state);
}

/*----------------------------------------------------------------------*/
/* Reimplement fprintf() as a call to Tcl_Eval().			*/
/*----------------------------------------------------------------------*/

void tcl_printf(FILE *f, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  tcl_vprintf(f, format, ap);
  va_end(ap);
}

/*----------------------------------------------------------------------*/
/* Implement tag callbacks on functions					*/
/* Find any tags associated with a command and execute them.		*/
/*----------------------------------------------------------------------*/

int PCBTagCallback(Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    int objidx, result = TCL_OK;
    char *postcmd, *substcmd, *newcmd, *sptr, *sres;
    char *croot = Tcl_GetString(objv[0]);
    Tcl_HashEntry *entry;
    Tcl_SavedResult state;
    int reset = FALSE;
    int i, llen;

    /* Skip over namespace qualifier, if any */

    if (!strncmp(croot, "::", 2)) croot += 2;
    if (!strncmp(croot, "pcb::", 5)) croot += 5;

    entry = Tcl_FindHashEntry(&PCBTagTable, croot);
    postcmd = (entry) ? (char *)Tcl_GetHashValue(entry) : NULL;

    if (postcmd)
    {
	substcmd = (char *)Tcl_Alloc(strlen(postcmd) + 1);
	strcpy(substcmd, postcmd);
	sptr = substcmd;

	/*--------------------------------------------------------------*/
	/* Parse "postcmd" for Tk-substitution escapes			*/
	/* Allowed escapes are:						*/
	/* 	%W	substitute the tk path of the calling window	*/
	/*	%r	substitute the previous Tcl result string	*/
	/*	%R	substitute the previous Tcl result string and	*/
	/*		reset the Tcl result.				*/
	/*	%[0-5]  substitute the argument to the original command	*/
	/*	%N	substitute all arguments as a list		*/
	/*	%%	substitute a single percent character		*/
	/*	%*	(all others) no action: print as-is.		*/
	/*--------------------------------------------------------------*/

	while ((sptr = strchr(sptr, '%')) != NULL)
	{
	    switch (*(sptr + 1))
	    {
		case 'W': {
		    char *tkpath = NULL;
		    Tk_Window tkwind = Tk_MainWindow(interp);
		    if (tkwind != NULL) tkpath = Tk_PathName(tkwind);
		    if (tkpath == NULL)
			newcmd = (char *)Tcl_Alloc(strlen(substcmd));
		    else
			newcmd = (char *)Tcl_Alloc(strlen(substcmd) + strlen(tkpath));

		    strcpy(newcmd, substcmd);

		    if (tkpath == NULL)
			strcpy(newcmd + (int)(sptr - substcmd), sptr + 2);
		    else
		    {
			strcpy(newcmd + (int)(sptr - substcmd), tkpath);
			strcat(newcmd, sptr + 2);
		    }
		    Tcl_Free(substcmd);
		    substcmd = newcmd;
		    sptr = substcmd;
		    } break;

		case 'R':
		    reset = TRUE;
		case 'r':
		    sres = (char *)Tcl_GetStringResult(interp);
		    newcmd = (char *)Tcl_Alloc(strlen(substcmd)
				+ strlen(sres) + 1);
		    strcpy(newcmd, substcmd);
		    sprintf(newcmd + (int)(sptr - substcmd), "\"%s\"", sres);
		    strcat(newcmd, sptr + 2);
		    Tcl_Free(substcmd);
		    substcmd = newcmd;
		    sptr = substcmd;
		    break;

		case '0': case '1': case '2': case '3': case '4': case '5':
		    objidx = (int)(*(sptr + 1) - '0');
		    if ((objidx >= 0) && (objidx < objc))
		    {
		        newcmd = (char *)Tcl_Alloc(strlen(substcmd)
				+ strlen(Tcl_GetString(objv[objidx])));
		        strcpy(newcmd, substcmd);
			strcpy(newcmd + (int)(sptr - substcmd),
				Tcl_GetString(objv[objidx]));
			strcat(newcmd, sptr + 2);
			Tcl_Free(substcmd);
			substcmd = newcmd;
			sptr = substcmd;
		    }
		    else if (objidx >= objc)
		    {
		        newcmd = (char *)Tcl_Alloc(strlen(substcmd) + 1);
		        strcpy(newcmd, substcmd);
			strcpy(newcmd + (int)(sptr - substcmd), sptr + 2);
			Tcl_Free(substcmd);
			substcmd = newcmd;
			sptr = substcmd;
		    }
		    else sptr++;
		    break;

		case 'N':
		    llen = 1;
		    for (i = 1; i < objc; i++)
		       llen += (1 + strlen(Tcl_GetString(objv[i])));
		    newcmd = (char *)Tcl_Alloc(strlen(substcmd) + llen);
		    strcpy(newcmd, substcmd);
		    strcpy(newcmd + (int)(sptr - substcmd), "{");
		    for (i = 1; i < objc; i++) {
		       strcat(newcmd, Tcl_GetString(objv[i]));
		       if (i < (objc - 1))
			  strcat(newcmd, " ");
		    }
		    strcat(newcmd, "}");
		    strcat(newcmd, sptr + 2);
		    Tcl_Free(substcmd);
		    substcmd = newcmd;
		    sptr = substcmd;
		    break;

		case '%':
		    newcmd = (char *)Tcl_Alloc(strlen(substcmd) + 1);
		    strcpy(newcmd, substcmd);
		    strcpy(newcmd + (int)(sptr - substcmd), sptr + 1);
		    Tcl_Free(substcmd);
		    substcmd = newcmd;
		    sptr = substcmd;
		    break;

		default:
		    break;
	    }
	}

	/* Fprintf(stderr, "Substituted tag callback is \"%s\"\n", substcmd); */
	/* Flush(stderr); */

	Tcl_SaveResult(interp, &state);
	result = Tcl_Eval(interp, substcmd);
	if ((result == TCL_OK) && (reset == FALSE))
	    Tcl_RestoreResult(interp, &state);
	else
	    Tcl_DiscardResult(&state);

	Tcl_Free(substcmd);
    }
    return result;
}

/*--------------------------------------------------------------*/
/* Add a command tag callback					*/
/*--------------------------------------------------------------*/

int pcbtcl_tag(ClientData clientData,
        Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
    Tcl_HashEntry *entry;
    char *hstring;
    int new;

    if (objc != 2 && objc != 3)
	return TCL_ERROR;

    entry = Tcl_CreateHashEntry(&PCBTagTable, Tcl_GetString(objv[1]), &new);
    if (entry == NULL) return TCL_ERROR;

    hstring = (char *)Tcl_GetHashValue(entry);
    if (objc == 2)
    {
	Tcl_SetResult(interp, hstring, NULL);
	return TCL_OK;
    }

    if (strlen(Tcl_GetString(objv[2])) == 0)
    {
	Tcl_DeleteHashEntry(entry);
    }
    else
    {
	hstring = strdup(Tcl_GetString(objv[2]));
	Tcl_SetHashValue(entry, hstring);
    }
    return TCL_OK;
}

/*
 * Event handlers
 */


void pcb_redrawarea(ClientData clientData, XEvent *eventPtr)
{
   XExposeEvent *exposeEventPtr = (XExposeEvent *)eventPtr;
   XRectangle exposeRect;
   Region myRegion = XCreateRegion();

   Tcl_ServiceAll();

   exposeRect.x = (short)exposeEventPtr->x;
   exposeRect.y = (short)exposeEventPtr->y;
   exposeRect.width = (unsigned short)exposeEventPtr->width;
   exposeRect.height = (unsigned short)exposeEventPtr->height;

   XUnionRectWithRegion(&exposeRect, myRegion, myRegion);
   DrawClipped(myRegion);
}

/*-----------------------------------------------------*/

/*
 * Startup function, which allows the Tcl startup script
 * to pass the names of windows we need to know back to
 * the code.
 */

int _pcb_start(ClientData clientData,
        Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[])
{
   int i, x, y, locobjc = objc;
   unsigned int width, height;
   char *argv;
   FILE *inputFile = NULL;
   XRectangle Big;
   char *sname;
   Tk_Window tktop, tkwind, tkpcb;
   Tcl_Obj **locobjv = (Tcl_Obj **)objv;

   /* Pick up various windows from the PCBOpts() variable */
   sname = Tcl_GetVar2(pcbinterp, "PCBOpts", "drawing", TCL_NAMESPACE_ONLY);
   tktop = Tk_MainWindow(interp);
   if (sname) {
      tkwind = Tk_NameToWindow(interp, sname, tktop);
   }
   else {
      tcl_printf(stderr, "Error: PCBOpts(drawing) must be defined!\n");
      return TCL_ERROR;
   }

   if (tkwind == (Tk_Window)0)
      return TCL_ERROR;

   Tk_MapWindow(tkwind);
   Output.Output = tkwind;
   Output.OutputWindow = Tk_WindowId(tkwind);
   Output.Toplevel = tktop;

   /* Pick up the remaining windows from PCBOpts(), if they are defined */
   sname = Tcl_GetVar2(pcbinterp, "PCBOpts", "status", TCL_NAMESPACE_ONLY);
   if (sname)
      Output.StatusLine = Tk_NameToWindow(interp, sname, tktop);

   sname = Tcl_GetVar2(pcbinterp, "PCBOpts", "menu", TCL_NAMESPACE_ONLY);
   if (sname)
      Output.Menu = Tk_NameToWindow(interp, sname, tktop);

   sname = Tcl_GetVar2(pcbinterp, "PCBOpts", "cursorpos", TCL_NAMESPACE_ONLY);
   if (sname)
      Output.CursorPosition = Tk_NameToWindow(interp, sname, tktop);

   sname = Tcl_GetVar2(pcbinterp, "PCBOpts", "panner", TCL_NAMESPACE_ONLY);
   if (sname)
      Output.panner = Tk_NameToWindow(interp, sname, tktop);

   /* Others may need to be added. . . */

   /* Set up exposure event handling */
   /* Note that key press and release events cannot be put here, or	*/
   /* they shadow the tk "bind" command.				*/

   Tk_CreateEventHandler(Output.Output, ExposureMask,
	(Tk_EventProc *)pcb_redrawarea, NULL);
   Tk_CreateEventHandler(Output.Output, StructureNotifyMask |
		EnterWindowMask | LeaveWindowMask | PointerMotionMask,
		(Tk_EventProc *)OutputEvent, NULL);

   tkpcb = Tk_NameToWindow(interp, ".pcb", tktop);
   GetApplicationResources(tkpcb);

   /* Post-initialization handling of Settings */

   if (Settings.LineThickness > MAX_LINESIZE ||
		Settings.LineThickness < MIN_LINESIZE)
     Settings.LineThickness = 1000;
   if (Settings.ViaThickness > MAX_PINORVIASIZE ||
		Settings.ViaThickness < MIN_PINORVIASIZE)
      Settings.ViaThickness = 4000;
   if (Settings.ViaDrillingHole <= 0)
      Settings.ViaDrillingHole = DEFAULT_DRILLINGHOLE * Settings.ViaThickness / 100;

   /* parse geometry string, ignore offsets */

   i = XParseGeometry (Settings.Size, &x, &y, &width, &height);
   if (!(i & WidthValue) || !(i & HeightValue)) {
      /* failed; use default setting */
      XParseGeometry (DEFAULT_SIZE, &x, &y, &width, &height);
   }

   Settings.MaxWidth = MIN(MAX_COORD, MAX ((BDimension) width * 100, MIN_SIZE));
   Settings.MaxHeight = MIN(MAX_COORD, MAX ((BDimension) height * 100, MIN_SIZE));
   Settings.Volume = MIN(100, MAX(-100, Settings.Volume));

   ParseGroupString (Settings.Groups, &Settings.LayerGroups);
   ParseRouteString (Settings.Routes, &Settings.RouteStyle[0], 1);

   /* More initialization of structure "Output" */
   InitDeviceDriver();
   PCB = CreateNewPCB(True);
   ResetStackAndVisibility();
   InitGC();

   /* Required to have default font to write text on layout */
   CreateDefaultFont();
   UpdateSettingsOnScreen();

   /* To-do:  Reset drawing area according to width and height (?) */

   /* Functions copied from InitWidgets() */
   InitCrosshair();
   InitBuffers();
   SetMode(NO_MODE);

   GetSizeOfDrawingArea();

   UpRegion = XCreateRegion();
   FullRegion = XCreateRegion();
   Big.x = Big.y = 0;
   Big.width = Big.height = TO_SCREEN(MAX_COORD);
   XUnionRectWithRegion (&Big, FullRegion, FullRegion);
   InitErrorLog();

   //Progname = Tcl_GetString(objv[0]);
   Progname = strdup("PCB");
   
   switch (--locobjc) {
      case 0:                     /* only program name */
         SetZoom(PCB->Zoom);
         break;

      case 1:                     /* load an initial layout */
         ++locobjv;
	 argv = Tcl_GetString(locobjv[0]);
         if (argv[0] == '-') {

	    /* The arguments are passed	as a single argv---is	*/
	    /* that an error in the shell script syntax?	*/

	    while(*argv != ' ' && *argv != '\0') argv++;
	    while(*argv == ' ') argv++;
	    if (*argv == '\0') Usage();
	 }

         /* keep filename even if initial load command failed;
          * file might not exist
          */
         if ((inputFile = fopen(argv, "r"))) {
           fclose(inputFile);
           LoadPCB(argv);
         }
         PCB->Filename = MyStrdup (argv, "_pcb_start()");

         break;

      default:                    /* error */
         Usage();
         break;
   }

   LoadBackgroundImage (Settings.BackgroundImage);
   /* modified by NP 3/10/2005 */
   /* modified by NP 2/14/2005 */
   /* read the library file */
   ReadLibraryContents ();
   Tcl_Eval(interp, "LibraryDialog");

   /* display netlist dialog if netlist initialized */
   if (PCB->NetlistLib.MenuN)
      Tcl_Eval(interp, "NetlistDialog");

   /* Print parameters held by the Tcl interface */
   PrintInit();

   pcbAddTimeOut(0, 1000 * Settings.BackupInterval, CB_Backup, NULL);

   /* To-do: Execute startup scripts */

   /* Allow the GUI script to set a callback that will finish  */
   /* the GUI initialization upon completion of this function. */
   return PCBTagCallback(interp, objc, objv);
}

/*--------------------------------------------------------------*/
/* Initialization procedure for Tcl/Tk				*/
/*--------------------------------------------------------------*/

int
Tclpcb_Init(Tcl_Interp *interp)
{
   char command[256];
   Tk_Window tktop;
   char *tmp_s;
   char *cadroot;
   char version_string[20], commandName[32];
   int i;

   /* Interpreter sanity checks */
   if (interp == NULL) return TCL_ERROR;

   /* Remember the interpreter */
   pcbinterp = interp;

   if (Tcl_InitStubs(interp, "8.1", 0) == NULL) return TCL_ERROR;

   tmp_s = getenv("PCB_LIB_DIR");
   if (tmp_s == NULL) tmp_s = PCBLIBDIR;

   tktop = Tk_MainWindow(interp);

   Dpy = Tk_Display(tktop);

   /* Create all of the commands (from PCB functions and actions) */
   /* (Disabled---we are replacing this functionality) */

/* RegisterUserCommands(interp, tktop);
   RegisterUserActions(interp, tktop); */

   strcpy(commandName, "pcb::");
   for (i = 0; pcb_commands[i].func != NULL; i++) {
      strcpy(commandName + 5, pcb_commands[i].cmdstr);
      Tcl_CreateObjCommand(interp, commandName,
		(Tcl_ObjCmdProc *)pcb_commands[i].func,
		(ClientData)tktop, (Tcl_CmdDeleteProc *)NULL);
   }

   /* Command which creates a "simple" window (this is a top-	*/
   /* level command, not one in the pcb namespace, which	*/
   /* is why I treat it separately from the other commands).	*/

   Tcl_CreateObjCommand(interp, "simple",
		(Tcl_ObjCmdProc *)Tk_SimpleObjCmd,
		(ClientData)tktop, (Tcl_CmdDeleteProc *) NULL);

   /* Register the "start" function, which passes the name of	*/
   /* the ("simple") drawing window back to the code.		*/

   Tcl_CreateObjCommand(interp, "pcb::start",
		(Tcl_ObjCmdProc *)_pcb_start,
		(ClientData)tktop, (Tcl_CmdDeleteProc *) NULL);

   /* Register the "tag" command for tag callbacks */

   Tcl_CreateObjCommand(interp, "pcb::tag",
		(Tcl_ObjCmdProc *)pcbtcl_tag,
		(ClientData)tktop, (Tcl_CmdDeleteProc *) NULL);

   /* New command-line interface */

   sprintf(command, "lappend auto_path %s", tmp_s);
   Tcl_Eval(interp, command);
   if (!strstr(tmp_s, "tcl")) {
      sprintf(command, "lappend auto_path %s/tcl", tmp_s);
      Tcl_Eval(interp, command);
   }

   if (strcmp(tmp_s, PCBLIBDIR))
      Tcl_Eval(interp, "lappend auto_path " PCBLIBDIR );

   /* Set $PCB_LIB_DIR as a Tcl variable */

   Tcl_SetVar(interp, "PCB_LIB_DIR", tmp_s, TCL_GLOBAL_ONLY);

   /* Set $CAD_ROOT as a Tcl variable */

   cadroot = getenv("CAD_ROOT");
   if (cadroot == NULL) cadroot = CAD_DIR;
   Tcl_SetVar(interp, "CAD_ROOT", cadroot, TCL_GLOBAL_ONLY);

   /* Set $PCB_VERSION as a Tcl variable */

   sprintf(version_string, "%s", PROG_REVISION);
   Tcl_SetVar(interp, "PCB_REVISION", version_string, TCL_GLOBAL_ONLY);

   sprintf(version_string, "%s", PROG_VERSION);
   Tcl_SetVar(interp, "PCB_VERSION", version_string, TCL_GLOBAL_ONLY);

   /* Export the namespace commands */

   Tcl_Eval(interp, "namespace eval pcb namespace export *");
   Tcl_PkgProvide(interp, "Pcb", version_string);

   /* Initialize the console interpreter, if there is one. */

   if ((consoleinterp = Tcl_GetMaster(interp)) == NULL)
      consoleinterp = interp;

   /* Initialize the command tag table */

   Tcl_InitHashTable(&PCBTagTable, TCL_STRING_KEYS);

   return TCL_OK;
}

/*--------------------------------------------------------------*/
/* String table case-insensitive lookup (swiped from Magic)	*/
/*--------------------------------------------------------------*/

int
TableLookup(char *str, char *(table[]))
{
   int match = -2;     /* result, initialized to -2 = no match */
   int pos;
   int ststart = 0;

   /* search for match */

   for (pos = 0; table[pos] != NULL; pos++) {
      char *tabc = table[pos];
      char *strc = &(str[ststart]);
      while (*strc!='\0' && *tabc!=' ' &&
		((*tabc==*strc) || (isupper(*tabc) && islower(*strc) &&
		(tolower(*tabc)== *strc)) || (islower(*tabc) &&
		isupper(*strc) && (toupper(*tabc)== *strc)))) {
	 strc++;
	 tabc++;
      }

      if (*strc == '\0') {	/* entry matches */

         if (*tabc == ' ' || *tabc == '\0') {
	    /* exact match - record it and terminate search */
	    match = pos;
	    break;
         }
         else if (match == -2) {
	    /* inexact match and no previous match - record	*/
	    /* this one and continue search			*/
	    match = pos;
         }

         else {
	    /* previous match, so string is ambiguous unless	*/
	    /* exact match exists.  Mark ambiguous for now, and	*/
	    /* continue search.					*/
	    match = -1;
	 }
      }
   }
   return (match);
}

/*--------------------------------------------------------------*/
/* Application  resource helper subroutines			*/
/*--------------------------------------------------------------*/

Pixel
SetColorResource(Tk_Window tkwind, char *resource, char *fallback)
{
   Colormap cmap;
   Tk_Uid  pcbuid;
   XColor color, exact;

   cmap = Tk_Colormap(tkwind);
   if ((pcbuid = Tk_GetOption(tkwind, resource, "Color")) == NULL)
      pcbuid = (Tk_Uid)fallback;
   XAllocNamedColor(Dpy, cmap, (char *)pcbuid, &color, &exact);
   return color.pixel;
}

/*--------------------------------------------------------------*/

XFontStruct *
SetFontResource(Tk_Window tkwind, char *resource, char *fallback)
{
   Tk_Uid  pcbuid;

   if ((pcbuid = Tk_GetOption(tkwind, resource, "Font")) == NULL)
      pcbuid = (Tk_Uid)fallback;
   return XLoadQueryFont(Dpy, (char *)pcbuid);
}

/*--------------------------------------------------------------*/

String
SetStringResource(Tk_Window tkwind, char *resource, char *fallback)
{
   Tk_Uid  pcbuid;

   if ((pcbuid = Tk_GetOption(tkwind, resource, "String")) == NULL)
      pcbuid = (Tk_Uid)fallback;
   return (String)pcbuid;
}

/*--------------------------------------------------------------*/

int
SetIntResource(Tk_Window tkwind, char *resource, char *fallback)
{
   Tk_Uid  pcbuid;

   if ((pcbuid = Tk_GetOption(tkwind, resource, "Int")) == NULL)
      pcbuid = (Tk_Uid)fallback;
   return (int)atoi(pcbuid);
}

/*--------------------------------------------------------------*/

long
SetLongResource(Tk_Window tkwind, char *resource, char *fallback)
{
   Tk_Uid  pcbuid;

   if ((pcbuid = Tk_GetOption(tkwind, resource, "Int")) == NULL)
      pcbuid = (Tk_Uid)fallback;
   return (long)atoi(pcbuid);
}

/*--------------------------------------------------------------*/

float
SetFloatResource(Tk_Window tkwind, char *resource, char *fallback)
{
   Tk_Uid  pcbuid;

   if ((pcbuid = Tk_GetOption(tkwind, resource, "Long")) == NULL)
      pcbuid = (Tk_Uid)fallback;
   return (float)atof(pcbuid);
}

/*--------------------------------------------------------------*/

Boolean
SetBooleanResource(Tk_Window tkwind, char *resource, char *fallback)
{
   Tk_Uid  pcbuid;
   int idx;
   static char *tf[] = {
	"0", "false", "no", "off",
	"1", "true", "yes", "on"};

   if ((pcbuid = Tk_GetOption(tkwind, resource, "Boolean")) == NULL)
      pcbuid = (Tk_Uid)fallback;

   idx = TableLookup((char *)pcbuid, tf);
   return ((idx < 4) ? False : True);
}

/*--------------------------------------------------------------*/
/* Handle application resources from resource.tcl (Tk "option"	*/
/* command) and set fallback resources.				*/
/*--------------------------------------------------------------*/

void
GetApplicationResources(Tk_Window tkwind)
{
   memset (&Settings, 0, sizeof (SettingType));

   /* Options need to be handled one at a time (?) */
   /* Default settings should be the same as Fallback[] in src/main.c */

   Settings.bgColor = SetColorResource(tkwind, "background", "white");
   Settings.CrossColor = SetColorResource(tkwind, "crossColor", "red");
   Settings.CrosshairColor = SetColorResource(tkwind, "crosshairColor", "yellow");
   Settings.GridColor = SetColorResource(tkwind, "gridColor", "red");
   Settings.ConnectedColor = SetColorResource(tkwind, "connectedColor", "green");
   Settings.ElementColor = SetColorResource(tkwind, "elementColor", "black");
   Settings.WarnColor = SetColorResource(tkwind, "warnColor", "hot pink");
   Settings.InvisibleObjectsColor = SetColorResource(tkwind,
		"invisibleObjectsColor", "gray80");
   Settings.InvisibleMarkColor = SetColorResource(tkwind,
		"invisibleMarkColor", "gray70");
   Settings.LayerColor[0] = SetColorResource(tkwind, "layerColor1", "brown4");
   Settings.LayerColor[1] = SetColorResource(tkwind, "layerColor2", "RoyalBlue3");
   Settings.LayerColor[2] = SetColorResource(tkwind, "layerColor3", "DodgerBlue4");
   Settings.LayerColor[3] = SetColorResource(tkwind, "layerColor4", "OrangeRed3");
   Settings.LayerColor[4] = SetColorResource(tkwind, "layerColor5", "PaleGreen4");
   Settings.LayerColor[5] = SetColorResource(tkwind, "layerColor6", "burlywood4");
   Settings.LayerColor[6] = SetColorResource(tkwind, "layerColor7", "turquoise4");
   Settings.LayerColor[7] = SetColorResource(tkwind, "layerColor8", "ForestGreen");
   Settings.ViaColor = SetColorResource(tkwind, "viaColor", "gray50");
   Settings.ElementSelectedColor = SetColorResource(tkwind,
		"elementSelectedColor", "cyan");
   Settings.LayerSelectedColor[0] = SetColorResource(tkwind,
		"layerSelectedColor1", "cyan");
   Settings.LayerSelectedColor[1] = SetColorResource(tkwind,
		"layerSelectedColor2", "cyan");
   Settings.LayerSelectedColor[2] = SetColorResource(tkwind,
		"layerSelectedColor3", "cyan");
   Settings.LayerSelectedColor[3] = SetColorResource(tkwind,
		"layerSelectedColor4", "cyan");
   Settings.LayerSelectedColor[4] = SetColorResource(tkwind,
		"layerSelectedColor5", "cyan");
   Settings.LayerSelectedColor[5] = SetColorResource(tkwind,
		"layerSelectedColor6", "cyan");
   Settings.LayerSelectedColor[6] = SetColorResource(tkwind,
		"layerSelectedColor7", "cyan");
   Settings.LayerSelectedColor[7] = SetColorResource(tkwind,
		"layerSelectedColor8", "cyan");
   Settings.OffLimitColor = SetColorResource(tkwind, "offLimitColor", "gray80");
   Settings.PinColor = SetColorResource(tkwind, "pinColor", "gray30");
   Settings.PinSelectedColor = SetColorResource(tkwind, "pinSelectedColor", "cyan");

   Settings.Groups = SetStringResource(tkwind, "layerGroups", "1,s:2,c:3:4:5:6:7:8");
   Settings.Size = SetStringResource(tkwind, "size", "6000x5000");
   Settings.FileCommand = SetStringResource(tkwind, "fileCommand", "cat '%f'");
   Settings.FilePath = SetStringResource(tkwind, "filePath", ".");

   Settings.PinoutFont[0] = SetFontResource(tkwind, "pinoutFont0",
		"-*-courier-bold-r-*-*-24-*-*-*-*-*-*-*");
   Settings.PinoutFont[1] = SetFontResource(tkwind, "pinoutFont1",
		"-*-courier-bold-r-*-*-12-*-*-*-*-*-*-*");
   Settings.PinoutFont[2] = SetFontResource(tkwind, "pinoutFont2",
		"-*-courier-bold-r-*-*-8-*-*-*-*-*-*-*");
   Settings.PinoutFont[3] = SetFontResource(tkwind, "pinoutFont3",
		"-*-courier-bold-r-*-*-4-*-*-*-*-*-*-*");
   Settings.PinoutFont[4] = SetFontResource(tkwind, "pinoutFont4",
		"-*-courier-bold-r-*-*-2-*-*-*-*-*-*-*");

   Settings.Zoom = SetFloatResource(tkwind, "zoom", "3.0");
   Settings.Grid = SetFloatResource(tkwind, "grid", "10.0");

   Settings.BackupInterval = SetLongResource(tkwind, "backupInterval", "60");
   Settings.AlignmentDistance = SetIntResource(tkwind, "alignmentDistance", "200");
   Settings.Bloat = SetIntResource(tkwind, "bloat", "699");
   Settings.minWid = SetIntResource(tkwind, "minWid", "800");
   Settings.minSlk = SetIntResource(tkwind, "minSlk", "800");
   Settings.CharPerLine = SetLongResource(tkwind, "charactersPerLine", "80");
   Settings.HistorySize = SetIntResource(tkwind, "historySize", "100");
   Settings.Keepaway = SetIntResource(tkwind, "keepawayWidth", "1000");

   Settings.ClearLine = SetBooleanResource(tkwind, "clearLines", "True");

   /* (Still needs more to complete...) */
   /*
    *     Since resources in resource.tcl now override
    *     FallBack values and layergroup setting is
    *     "1,c:2,s:3:4:5:6:7:8",DefaultLayerName[0] now
    *     set to "component" and DefaultLayerName[1] to
    *     "solder" to match with FallBack layergroup value.
    *     Layer sequence and group setting set accordingly
    *     both in resource.tcl and FallBack values.
    */
   Settings.DefaultLayerName[0] = SetStringResource(tkwind, "layerName1", "component");
   Settings.DefaultLayerName[1] = SetStringResource(tkwind, "layerName2", "solder");
   Settings.DefaultLayerName[2] = SetStringResource(tkwind, "layerName3", "ground");
   Settings.DefaultLayerName[3] = SetStringResource(tkwind, "layerName4", "power");
   Settings.DefaultLayerName[4] = SetStringResource(tkwind, "layerName5", "signal1");
   Settings.DefaultLayerName[5] = SetStringResource(tkwind, "layerName6", "signal2");
   Settings.DefaultLayerName[6] = SetStringResource(tkwind, "layerName7", "unused");
   Settings.DefaultLayerName[7] = SetStringResource(tkwind, "layerName8", "unused");
   Settings.ElementCommand = SetStringResource(tkwind,
                "elementCommand", "M4PATH=\"%p\";export M4PATH;echo 'include(%f)' | " GNUM4);
   Settings.ElementPath = SetStringResource(tkwind,
                "elementPath", ".:packages:circuits:/usr/lib/X11/pcb:/usr/lib/X11/pcb/circuits:/usr/lib/X11/pcb/packages");
   Settings.FontCommand = SetStringResource(tkwind,
                "fontCommand", "M4PATH=\"%p\";export M4PATH;echo 'include(%f)' | " GNUM4);
   Settings.FontFile = SetStringResource(tkwind, "fontFile", "default_font");
   Settings.FontPath = SetStringResource(tkwind, "fontPath", ".:/usr/lib/X11/pcb");
   Settings.LibraryCommand = SetStringResource(tkwind,
                "libraryCommand", "/usr/lib/X11/pcb/QueryLibrary.sh '%p' '%f' %a");
   Settings.LibraryContentsCommand = SetStringResource(tkwind,
                "libraryContentsCommand", "/usr/local/share/pcb/ListLibraryContents.sh '%p' '%f'");
   Settings.LibraryFilename = SetStringResource(tkwind, "libraryFilename", "pcblib");
   Settings.LibraryPath = SetStringResource(tkwind, "libraryPath", ".:/usr/lib/X11/pcb");
   Settings.PrintFile = SetStringResource(tkwind, "printFile", "%f.output");
   Settings.Routes = SetStringResource(tkwind,
                "routeStyles", "Signal,1000,4000,2000,1000:Power,2500,6000,3500,1000:Fat,4000,6000,3500,1500:Skinny,800,3600,2000,800");
   Settings.SaveCommand = SetStringResource(tkwind, "saveCommand", "cat - > '%f");


   Settings.PinoutNameLength = SetIntResource(tkwind, "pinoutNameLength", "8");
   Settings.PinoutOffsetX = SetIntResource(tkwind, "pinoutOffsetX", "10000");
   Settings.PinoutOffsetY = SetIntResource(tkwind, "pinoutOffsetY", "10000");
   Settings.PinoutTextOffsetX = SetIntResource(tkwind, "pinoutTextOffsetX", "0");
   Settings.PinoutTextOffsetY = SetIntResource(tkwind, "pinoutTextOffsetY", "0");
   Settings.RatThickness = SetIntResource(tkwind, "ratThickness", "1000");
   Settings.TextScale = SetIntResource(tkwind, "textScale", "100");
   Settings.ViaDrillingHole = SetIntResource(tkwind, "viaDrillingHole", "2800");
   Settings.ViaThickness = SetIntResource(tkwind, "viaThickness", "6000");
   Settings.Volume = SetIntResource(tkwind, "volume", "100");
   Settings.LineThickness = SetIntResource(tkwind, "lineThickness", "1000");

   Settings.PinoutZoom = SetFloatResource(tkwind, "pinoutZoom", "2.0");


   Settings.RaiseLogWindow = SetBooleanResource(tkwind, "raiseLogWindow", "On");
   Settings.ResetAfterElement = SetBooleanResource(tkwind, "resetAfterElement", "On");
   Settings.RingBellWhenFinished = SetBooleanResource(tkwind, "ringBellWhenFinished", "Off");
   Settings.SaveInTMP = SetBooleanResource(tkwind, "saveInTMP", "Off");
   Settings.SaveLastCommand = SetBooleanResource(tkwind, "saveLastCommand", "Off");
   Settings.StipplePolygons = SetBooleanResource(tkwind, "stipplePolygons", "Off");
   Settings.UniqueNames = SetBooleanResource(tkwind, "uniqueNames", "On");
   Settings.SnapPin = SetBooleanResource(tkwind, "snapPin", "On");
   Settings.UseLogWindow = SetBooleanResource(tkwind, "useLogWindow", "Off");
   Settings.AllDirectionLines = SetBooleanResource(tkwind, "allDirectionLines", "Off");

   Settings.RatColor = SetColorResource(tkwind, "ratColor", "DarkGoldenrod");
   Settings.RatSelectedColor = SetColorResource(tkwind, "ratSelectedColor", "cyan");
   Settings.ViaSelectedColor = SetColorResource(tkwind, "viaSelectedColor", "cyan");

/* Variables not available in Fallback[] array but avail in resource.tcl */
   Settings.MaskColor = SetColorResource(tkwind, "maskColor", "red");

   Settings.LibraryTree = SetStringResource(tkwind,
                "libraryTree", "/usr/local/share/pcb/newlib");
   Settings.Media = SetStringResource(tkwind, "media", "a4");
   Settings.MenuFile = SetStringResource(tkwind,
                "menuFile", "/usr/local/share/pcb/pcb-menu.res");
   Settings.Shrink = SetIntResource(tkwind, "shrink", "500");

/* Variables not available in either Fallback[] array or resource.tcl */

  Settings.DumpMenuFile = SetBooleanResource(tkwind, "dumpMenuFile", "False");
  Settings.RubberBandMode = SetBooleanResource(tkwind, "rubberBandMode", "True");
  Settings.SwapStartDirection = SetBooleanResource(tkwind,
                "swapStartDirection", "True");

  Settings.ActionString = SetStringResource(tkwind, "actionString", NULL);
  Settings.BackgroundImage = SetStringResource(tkwind,
                "backgroundImage", "pcb-background.ppm");
  Settings.RatCommand = SetStringResource(tkwind, "ratCommand", "cat %f");
  Settings.RatPath = SetStringResource(tkwind, "ratPath", ".");
  Settings.ScriptFilename = SetStringResource(tkwind, "scriptFilename", NULL);
}

#endif /* TCL_PCB */

