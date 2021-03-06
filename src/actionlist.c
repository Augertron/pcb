/* This file is generated by gather-actions.  DO NOT EDIT.  */

#include "config.h"
#include "global.h"
#include "X11/Intrinsic.h"

extern void AboutDialog(Widget,XEvent *,String *, Cardinal *);
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

#ifndef TCL_PCB

XtActionsRec ActionList[] = {
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

#endif /* !TCL_PCB */

extern int FlagCurrentStyle(int);
extern int FlagElementName(int);
extern int FlagGrid(int);
extern int FlagGridFactor(int);
extern int FlagHaveRegex(int);
extern int FlagIsDataEmpty(int);
extern int FlagIsDataEmpty(int);
extern int FlagNetlist(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagSETTINGS(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagTESTFLAG(int);
extern int FlagZoom(int);
extern int djopt_get_auto_only(int);
extern int vendor_get_enabled(int);

struct {
  char *name;
  int (*func)(int);
  int parm;
} FlagFuncList[] = {
  {"DataEmpty", FlagIsDataEmpty, 0 },
  {"DataNonEmpty", FlagIsDataEmpty, 1 },
  {"OptAutoOnly", djopt_get_auto_only, 0 },
  {"VendorMapOn", vendor_get_enabled, 0 },
  {"alldirection", FlagTESTFLAG, ALLDIRECTIONFLAG },
  {"alldirectionlines", FlagSETTINGS, XtOffsetOf(SettingType,AllDirectionLines) },
  {"autodrc", FlagTESTFLAG, AUTODRCFLAG },
  {"checkplanes", FlagTESTFLAG, CHECKPLANESFLAG },
  {"clearline", FlagSETTINGS, XtOffsetOf(SettingType,ClearLine) },
  {"clearnew", FlagTESTFLAG, CLEARNEWFLAG },
  {"description", FlagTESTFLAG, DESCRIPTIONFLAG },
  {"drawgrid", FlagSETTINGS, XtOffsetOf(SettingType,DrawGrid) },
  {"elementname", FlagElementName, 0 },
  {"grid", FlagGrid, 0 },
  {"gridfactor", FlagGridFactor, 0 },
  {"have_regex", FlagHaveRegex, 0 },
  {"liveroute", FlagSETTINGS, XtOffsetOf(SettingType,liveRouting) },
  {"liveroute", FlagTESTFLAG, LIVEROUTEFLAG },
  {"localref", FlagTESTFLAG, LOCALREFFLAG },
  {"nameonpcb", FlagTESTFLAG, NAMEONPCBFLAG },
  {"netlist", FlagNetlist, 0 },
  {"orthomove", FlagTESTFLAG, ORTHOMOVEFLAG },
  {"raiselogwindow", FlagSETTINGS, XtOffsetOf(SettingType,RaiseLogWindow) },
  {"ratwarn", FlagSETTINGS, XtOffsetOf(SettingType,RatWarn) },
  {"resetafterelement", FlagSETTINGS, XtOffsetOf(SettingType,ResetAfterElement) },
  {"ringbellwhenfinished", FlagSETTINGS, XtOffsetOf(SettingType,RingBellWhenFinished) },
  {"rubberband", FlagTESTFLAG, RUBBERBANDFLAG },
  {"rubberbandmode", FlagSETTINGS, XtOffsetOf(SettingType,RubberBandMode) },
  {"saveintmp", FlagSETTINGS, XtOffsetOf(SettingType,SaveInTMP) },
  {"savelastcommand", FlagSETTINGS, XtOffsetOf(SettingType,SaveLastCommand) },
  {"showdrc", FlagSETTINGS, XtOffsetOf(SettingType,ShowDRC) },
  {"showdrc", FlagTESTFLAG, SHOWDRCFLAG },
  {"showmask", FlagTESTFLAG, SHOWMASKFLAG },
  {"shownumber", FlagTESTFLAG, SHOWNUMBERFLAG },
  {"showsolderside", FlagSETTINGS, XtOffsetOf(SettingType,ShowSolderSide) },
  {"snappin", FlagTESTFLAG, SNAPPINFLAG },
  {"stipplepolygons", FlagSETTINGS, XtOffsetOf(SettingType,StipplePolygons) },
  {"style", FlagCurrentStyle, 0 },
  {"swapstartdir", FlagTESTFLAG, SWAPSTARTDIRFLAG },
  {"swapstartdirection", FlagSETTINGS, XtOffsetOf(SettingType,SwapStartDirection) },
  {"thindraw", FlagTESTFLAG, THINDRAWFLAG },
  {"uniquename", FlagTESTFLAG, UNIQUENAMEFLAG },
  {"uniquenames", FlagSETTINGS, XtOffsetOf(SettingType,UniqueNames) },
  {"uselogwindow", FlagSETTINGS, XtOffsetOf(SettingType,UseLogWindow) },
  {"zoom", FlagZoom, 0 },
  {0,0,0}
};

int FlagFuncListSize = 45;
struct Resource;
extern void SizesMenuInclude(struct Resource *);

struct {
  char *name;
  void (*func)(struct Resource *);
} MenuFuncList[] = {
  {"sizes", SizesMenuInclude },
  {0,0}
};

int MenuFuncListSize = 1;
