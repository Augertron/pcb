/*------------------------------------------------------*/
/* tclpcb.h:						*/
/* Copyright (c) 2004  Tim Edwards, MultiGiG, Inc.	*/
/*------------------------------------------------------*/

#ifdef TCL_PCB

extern Tcl_HashTable PCBTagTable;
extern Tcl_Interp *pcbinterp;
extern Tcl_Interp *consoleinterp;

extern char *Progname;

extern int Tk_SimpleObjCmd();
extern void RegisterUserCommands();
extern void RegisterUserActions();

extern int InitGC();			/* from src/main.c */
extern void InitDeviceDriver();
extern void CB_Backup();

extern Boolean ActiveDrag();
extern void StopAutoScroll();

extern void DrawClipped();		/* from src/output.c */
extern void GetSizeOfDrawingArea();

extern void PushOnTopOfLayerStack();	/* from src/control.c */

extern void ActionToggleVendor();	/* from src/vendor.c */

extern Boolean ConfirmDialog();		/* from src/dialog.c */
extern void MessageDialog();
extern void SendEnterNotify();
extern char *StrDup();

extern void NotifyMode();		/* from src/action.c */
extern void ReleaseMode();
extern void DoStroke();			
extern void SetNoteXY();

/* Forward declarations */

extern void GetApplicationResources();
extern int PCBTagCallback();
extern int resolve_conflict();
extern void tcl_printf(FILE *, const char *, ...);
extern void tcl_stdflush(FILE *);
extern void RedrawLayout();
extern char *GetConsoleInput(char *);
extern void PrintInit();

/*-----------------------*/
/* Tcl 8.4 compatibility */
/*-----------------------*/

#ifndef CONST84
#define CONST84
#endif

/*----------------------------------------------------------------------*/
/* Tcl/Tk command-line interface commands for PCB			*/
/*----------------------------------------------------------------------*/

extern int pcbtcl_about();
extern int pcbtcl_arc();
extern int pcbtcl_buffer();
extern int pcbtcl_crosshair();
extern int pcbtcl_delete();
extern int pcbtcl_deselect();
extern int pcbtcl_drc();
extern int pcbtcl_drill();
extern int pcbtcl_element();
extern int pcbtcl_grid();
extern int pcbtcl_layer();
extern int pcbtcl_layout();
extern int pcbtcl_library();
extern int pcbtcl_line();
extern int pcbtcl_mode();
extern int pcbtcl_netlist();
extern int pcbtcl_object();
extern int pcbtcl_option();
extern int pcbtcl_pad();
extern int pcbtcl_pin();
extern int pcbtcl_pinout();
extern int pcbtcl_place();
extern int pcbtcl_pointer();
extern int pcbtcl_polygon();
extern int pcbtcl_print();
extern int pcbtcl_quit();
extern int pcbtcl_rectangle();
extern int pcbtcl_redo();
extern int pcbtcl_route();
extern int pcbtcl_scan();
extern int pcbtcl_select();
extern int pcbtcl_scan();
extern int pcbtcl_show();
extern int pcbtcl_style();
extern int pcbtcl_text();
extern int pcbtcl_thermal();
extern int pcbtcl_undo();
extern int pcbtcl_via();
extern int pcbtcl_zoom();

typedef struct {
    const char *cmdstr;
    int		(*func)();
} cmdstruct;

typedef struct selObjs
{
    int obj_type;
    void *dataPtr1;
    void *dataPtr2;
} SelObjects;

/*----------------------------------------------------------------------*/

#endif /* TCL_PCB */

