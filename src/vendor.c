/* $Id: vendor.c,v 1.2 2006/11/05 17:55:34 tim Exp $ */

/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 2004 Dan McMahill
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <math.h>
#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_REGEX_H
#include <regex.h>
#else
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include "change.h"
#include "data.h"
#include "draw.h"
#include "error.h"
#include "fileselect.h"
#include "global.h"
#include "resource.h"
#include "set.h"
#include "undo.h"
#include "vendor.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

static Boolean rematch( const char *, const char *);

/* list of vendor drills and a count of them */
static int *vendor_drills = NULL;
static int n_vendor_drills = 0;

/* lists of elements to ignore */
static char **ignore_refdes = NULL;
static int n_refdes = 0;
static char **ignore_value = NULL;
static int n_value = 0;
static char **ignore_descr = NULL;
static int n_descr = 0;


/* enable/disable mapping */
static Boolean vendorMapEnable = False;

/* type of drill mapping */
#define CLOSEST 1
#define ROUND_UP 0
static int rounding_method = ROUND_UP;

#define FREE(x) if((x) != NULL) { free (x) ; (x) = NULL; }

/* FLAG(VendorMapOn,vendor_get_enabled) */
int
vendor_get_enabled(void)
{
  return vendorMapEnable;
}

/* ************************************************************ */

/* ACTION(ToggleVendor,ActionToggleVendor) */

void
ActionToggleVendor(Widget W, XEvent *Event, String *Params, Cardinal *num)
{

  if (vendorMapEnable)
    vendorMapEnable = False;
  else
    vendorMapEnable = True;
}

/* ************************************************************ */

/* ACTION(EnableVendor,ActionEnableVendor) */

void
ActionEnableVendor(Widget W, XEvent *Event, String *Params, Cardinal *num)
{
  vendorMapEnable = True;
}

/* ************************************************************ */

/* ACTION(DisableVendor,ActionDisableVendor) */

void
ActionDisableVendor(Widget W, XEvent *Event, String *Params, Cardinal *num)
{
  vendorMapEnable = False;
}

/* ************************************************************ */

/* ACTION(UnloadVendor,ActionUnloadVendor) */

void
ActionUnloadVendor(Widget W, XEvent *Event, String *Params, Cardinal *num)
{

  /* Unload any vendor table we may have had */
  n_vendor_drills = 0;
  n_refdes = 0;
  n_value = 0;
  n_descr = 0;
  FREE (vendor_drills);
  FREE (ignore_refdes);
  FREE (ignore_value);
  FREE (ignore_descr);

}

/* ************************************************************ */

#ifdef TCL_PCB

void
ActionApplyVendor (Widget w, XEvent * e, String * argv, Cardinal * argc)
{
  Tk_Window tkwind;
  tkwind = (Tk_Window)w;
}

void
ActionLoadVendor (Widget w, XEvent * e, String * argv, Cardinal * argc)
{
  Tk_Window tkwind;
  tkwind = (Tk_Window)w;
}

#else /*(!TCL_PCB)---Xt definitions */
static void add_to_drills(char *);
static void apply_vendor_map(void);
static void process_skips(Resource *);

/* vendor name */
static char *vendor_name = NULL;

/* resource file to PCB units scale factor */
static double sf;

/* ACTION(ApplyVendor,ActionApplyVendor) */

void
ActionApplyVendor(Widget W, XEvent *Event, String *Params, Cardinal *num)
{

  apply_vendor_map();
}


/* ************************************************************ */

/* ACTION(LoadVendor,ActionLoadVendor) */

void
ActionLoadVendor(Widget W, XEvent *Event, String *Params, Cardinal *num)
{
  int i;
  char *fname = NULL;
  char *sval;
  Resource *res, *drcres, *drlres;
  int type;

  if (*num == 0)
  {
    fname = FileSelectBox ("load vendor resource:", NULL, Settings.FilePath);
  } else if ( *num == 1)
  {
    /* resource file name */
    fname = Params[0];
  } else
  {
    Message ("Usage:  LoadVendor([filename])\n");
    return ;
  }

  if (fname == NULL) 
  {
    Message ("LoadVendor():  No vendor resource file specified\n");
    return ;
  }

  /* Unload any vendor table we may have had */
  n_vendor_drills = 0;
  n_refdes = 0;
  n_value = 0;
  n_descr = 0;
  FREE (vendor_drills);
  FREE (ignore_refdes);
  FREE (ignore_value);
  FREE (ignore_descr);


  /* load the resource file */
  res = resource_parse(fname, NULL);
  if (res == NULL)
    {
      Message ("Could not load vendor resource file \"%s\"\n", fname);
      return;
    }

  /* figure out the vendor name, if specified */
  vendor_name = UNKNOWN (resource_value(res, "vendor"));

  /* figure out the units, if specified */
  sval = resource_value(res, "units");
  if ( sval == NULL )
    {
      sf = 100;
    } 
  else if ( (strcmp (sval, "mil") == 0) || (strcmp (sval, "mils") == 0) ) 
    {
      sf = 100;
    }
  else if ( (strcmp (sval, "inch") == 0)  || (strcmp (sval, "inches") == 0) ) 
    {
      sf = 100000;
    }
  else if (strcmp (sval, "mm") == 0)
    {
      /* 
       * divide by .0254 to convert mm to mils.  Then multiply by 100
       * for PCB units
       */
      sf = (100.0 / 0.0254);
    }
  else 
    {
      Message ("\"%s\" is not a supported units.  Defaulting to inch\n", sval);
      sf = 100000;
    }
  

  /* default to ROUND_UP */
  rounding_method = ROUND_UP;

  /* extract the drillmap resource */
  drlres = resource_subres(res, "drillmap");
  if ( drlres == NULL ) 
    {
      Message ("No drillmap resource found\n");
    }
  else 
    {
      sval = resource_value(drlres, "round");
      if (sval != NULL)
	{
	  if ( strcmp (sval, "up") == 0 )
	    {
	      rounding_method = ROUND_UP;
	    }
	  else if (strcmp (sval, "nearest") == 0) 
	    {
	      rounding_method = CLOSEST;
	    }
	  else 
	    {
	      Message ("\"%s\" is not a valid rounding type.  "
		       "Defaulting to up\n", sval);
	      rounding_method = ROUND_UP;
	    }
	}

      process_skips (resource_subres(drlres, "skips"));

      for (i=0; i<drlres->c; i++)
	{
	  type =  resource_type (drlres->v[i]);
	  switch ( type )
	    {
	    case 10:
	      /* just a number */
	      add_to_drills (drlres->v[i].value);
	      break;

	    default:
	      break;
	    }
	}
    }

  /* Extract the DRC resource */
  drcres = resource_subres (res, "drc");
  
  sval = resource_value(drcres, "copper_space");
  if (sval != NULL)
    {
      Settings.Bloat = floor (sf * atof(sval) + 0.5);
      Message ("Set DRC minimum copper spacing to %.2f mils\n", 
	       0.01*Settings.Bloat);
    }

  sval = resource_value(drcres, "copper_overlap");
  if (sval != NULL)
    {
      Settings.Shrink = floor (sf * atof(sval) + 0.5);
      Message ("Set DRC minimum copper overlap to %.2f mils\n", 
	       0.01*Settings.Shrink);
    }

  sval = resource_value(drcres, "copper_width");
  if (sval != NULL)
    {
      Settings.minWid = floor (sf * atof(sval) + 0.5);
      Message ("Set DRC minimum copper spacing to %.2f mils\n", 
	       0.01*Settings.minWid);
    }

  sval = resource_value(drcres, "silk_width");
  if (sval != NULL)
    {
      Settings.minSlk = floor (sf * atof(sval) + 0.5);
      Message ("Set DRC minimum silk width to %.2f mils\n", 
	       0.01*Settings.minSlk);
    }

  sval = resource_value(drcres, "min_drill");
  if (sval != NULL)
    {
      Settings.minDrill = floor (sf * atof(sval) + 0.5);
      Message ("Set DRC minimum drill diameter to %.2f mils\n", 
	       0.01*Settings.minDrill);
    }

  Message ("Loaded %d vendor drills from %s\n", n_vendor_drills, fname);
  Message ("Loaded %d RefDes skips, %d Value skips, %d Descr skips\n",
	   n_refdes, n_value, n_descr);

  vendorMapEnable = True;
  apply_vendor_map();
}

static void apply_vendor_map(void)
{
  int i;
  int changed, tot;
  Boolean state;

  state = vendorMapEnable;
  
  /* enable mapping */
  vendorMapEnable = True;

  /* reset our counts */
  changed = 0;
  tot = 0;

  /* If we have loaded vendor drills, then apply them to the design */
  if (n_vendor_drills > 0)
    {

      /* first all the vias */
      VIA_LOOP (PCB->Data);
      {
	tot++;
	if( via->DrillingHole != vendorDrillMap(via->DrillingHole) ) 
	  {
	    /* only change unlocked vias */
	    if (! TEST_FLAG (LOCKFLAG, via)) 
	      {
		if( ChangeObject2ndSize (VIA_TYPE, via, NULL, NULL, 
					 vendorDrillMap(via->DrillingHole), 
					 True, False) )
		  changed++;
		else 
		  {
		    Message ("Via at %.2f, %.2f not changed.  Possible reasons:\n"
			     "\t- pad size too small\n"
			     "\t- new size would be too large or too small\n",
			     0.01*via->X, 0.01*via->Y);
		  }
	      } 
	    else 
	      {
		Message ("Locked via at %.2f, %.2f not changed.\n",
			 0.01*via->X, 0.01*via->Y);
	      }
	  }			
      }
      END_LOOP;

      /* and now the pins */
      ELEMENT_LOOP (PCB->Data);
      {
	/*
	 * first figure out if this element should be skipped for some
	 * reason
	 */
	if ( vendorIsElementMappable(element) ) 
	  {
	    /* the element is ok to modify, so iterate over its pins */
	    PIN_LOOP (element);
	    {
	      tot++;
	      if ( pin->DrillingHole != vendorDrillMap (pin->DrillingHole) ) 
		{
		  if (! TEST_FLAG (LOCKFLAG, pin)) 
		    {
		      if (ChangeObject2ndSize (PIN_TYPE, element, pin, NULL, 
					       vendorDrillMap (pin->DrillingHole), 
					       True, False) )
			changed++;
		      else 
			{
			  Message ("Pin %s (%s) at %.2f, %.2f (element %s, %s, %s) not changed.  Possible reasons:\n"
				   "\t- pad size too small\n"
				   "\t- new size would be too large or too small\n",
				   UNKNOWN (pin->Number), UNKNOWN (pin->Name),
				   0.01*pin->X, 0.01*pin->Y,
				   UNKNOWN (NAMEONPCB_NAME (element)),
				   UNKNOWN (VALUE_NAME (element)),
				   UNKNOWN (DESCRIPTION_NAME (element)));
			}
		    } 
		  else 
		    {
		      Message ("Locked pin at %-6.2f, %-6.2f not changed.\n",
			       0.01*pin->X, 0.01*pin->Y);
		    }
		}
	    }
	    END_LOOP;
	  }
      }
      END_LOOP;
      
      Message ("Updated %d drill sizes out of %d total\n", changed, tot);

      /* Update the current Via */
      if (Settings.ViaDrillingHole != vendorDrillMap (Settings.ViaDrillingHole))
	{
	  changed++;
	  Settings.ViaDrillingHole = vendorDrillMap (Settings.ViaDrillingHole);
	  Message ("Adjusted active via hole size to be %6.2f mils\n",
		   0.01*Settings.ViaDrillingHole);
	}

      /* and update the vias for the various routing styles */
      for (i = 0; i <  NUM_STYLES; i++)
	{
	  if (PCB->RouteStyle[i].Hole != vendorDrillMap (PCB->RouteStyle[i].Hole))
	    {
	      changed++;
	      PCB->RouteStyle[i].Hole = vendorDrillMap (PCB->RouteStyle[i].Hole);
	      Message ("Adjusted %s routing style via hole size to be %6.2f mils\n",
		       PCB->RouteStyle[i].Name, 0.01*PCB->RouteStyle[i].Hole);
	      if (PCB->RouteStyle[i].Diameter < PCB->RouteStyle[i].Hole + MIN_PINORVIACOPPER)
		{
		  PCB->RouteStyle[i].Diameter = PCB->RouteStyle[i].Hole + MIN_PINORVIACOPPER;
		  Message ("Increased %s routing style via diameter to %6.2f mils\n",
			   PCB->RouteStyle[i].Name, 0.01*PCB->RouteStyle[i].Diameter);
		}
	    }
	}

      /* 
       * if we've changed anything, indicate that we need to save the
       * file, redraw things, and make sure we can undo.
       */
      if (changed) 
	{
	  SetChangedFlag (True);
	  ClearAndRedrawOutput ();
	  IncrementUndoSerialNumber ();
	}
    }

  /* restore mapping on/off*/
  vendorMapEnable = state;
}

/* add a drill size to the vendor drill list */
static void add_to_drills(char *sval)
{
  double tmpd;
  int val;
  int k, j;

  /* increment the count and make sure we have memory */
  n_vendor_drills++;
  if ( (vendor_drills = realloc (vendor_drills, 
				 n_vendor_drills*sizeof(int))) == NULL )
    {
      fprintf (stderr, "realloc() failed to allocate %d bytes\n", 
	       n_vendor_drills*sizeof(int));
      return ; 
    }
  
  /* string to a value with the units scale factor in place */
  tmpd = atof (sval);
  val = floor (sf * tmpd + 0.5);
	
  /* 
   * We keep the array of vendor drills sorted to make it easier to
   * do the rounding later.  The algorithm used here is not so efficient,
   * but we're not dealing with much in the way of data.
   */
  
  /* figure out where to insert the value to keep the array sorted.  */
  k = 0;
  while ( (k < n_vendor_drills-1) && (vendor_drills[k] < val) )
    k++;
	  
  if ( k == n_vendor_drills-1 ) 
    {
      vendor_drills[n_vendor_drills-1] = val;
    } 
  else 
    {
      /* move up the existing drills to make room */
      for(j = n_vendor_drills-1; j>k ; j--) 
	{
	  vendor_drills[j] = vendor_drills[j-1];
	}
      
      vendor_drills[k] = val;
    }
}

/* deal with the "skip" subresource */
static void process_skips(Resource *res)
{
  int type;
  int i, k;
  char *sval;
  int *cnt;
  char ***lst=NULL;

  if (res == NULL)
    return;

  for (i=0; i<res->c; i++)
    {
      type =  resource_type(res->v[i]);
      switch ( type )
	{
	case 1:
	  /* 
	   * an unnamed sub resource.  This is something like
	   * {refdes "J3"}
	   */
	  sval = res->v[i].subres->v[0].value;
	  if (sval == NULL)
	    {
	      Message("Error:  null skip value\n");
	    }
	  else
	    {
	      if (strcmp(sval, "refdes") == 0)
		{
		  cnt = &n_refdes;
		  lst = &ignore_refdes;
		}
	      else if (strcmp(sval, "value") == 0)
		{
		  cnt = &n_value;
		  lst = &ignore_value;
		}
	      else if (strcmp(sval, "descr") == 0)
		{
		  cnt = &n_descr;
		  lst = &ignore_descr;
		}
	      else
		{
		  cnt = NULL;
		}
	   
	      /* add the entry to the appropriate list */
	      if (cnt != NULL) 
		{
		  for (k=1; k<res->v[i].subres->c; k++) 
		    {
		      sval = res->v[i].subres->v[k].value;
		      (*cnt)++;
		      if( (*lst = (char **) realloc(*lst, (*cnt)*sizeof(char *))) == NULL )
			{
			  fprintf(stderr, "realloc() failed\n");
			  exit(-1);
			}
		      (*lst)[*cnt - 1] = strdup(sval);
		    }
		}
	    }
	  break;
	  
	default:
	  Message ("Ignored resource type = %d in skips= section\n", type);
	}
    }
  
}

#endif /* (TCL_PCB) */

/* for a given drill size, find the closest vendor drill size */
int vendorDrillMap(int in)
{
  int i;

  /* skip the mapping if we don't have a vendor drill table */
  if ( (n_vendor_drills == 0) || (vendor_drills == NULL ) || (vendorMapEnable == False) )
    return in;

  /* are we smaller than the smallest drill? */
  if (in <= vendor_drills[0])
    return vendor_drills[0];

  /* are we larger than the largest drill? */
  if (in > vendor_drills[n_vendor_drills-1])
    {
      Message ("Vendor drill list does not contain a drill >= %6.2f mil\n"
	       "Using %6.2f mil instead.\n", 
	       0.01*in, 0.01*vendor_drills[n_vendor_drills-1]);
      return vendor_drills[n_vendor_drills-1];
    }
  
  /* figure out which 2 drills are closest in size */
  i = 0;
  while (in > vendor_drills[i])
    i++;

  /* now round per the rounding mode */
  if (rounding_method == CLOSEST)
    {
      /* find the closest drill size */
      if( (in - vendor_drills[i-1]) > (vendor_drills[i] - in) )
	return vendor_drills[i];
      else
	return vendor_drills[i-1];
    }
  else
    {
      /* always round up */
      return vendor_drills[i];
    }
  
}

Boolean vendorIsElementMappable(ElementTypePtr element)
{
  int i;
  int noskip;

  if (vendorMapEnable == False)
    return False;

  noskip = 1;
  for (i=0 ; i<n_refdes ; i++) 
    {
      if ( (strcmp (UNKNOWN (NAMEONPCB_NAME(element) ), ignore_refdes[i]) == 0) ||
	   rematch ( ignore_refdes[i], UNKNOWN (NAMEONPCB_NAME(element) )) )
	{
	  Message ("Vendor mapping skipped because refdes = %s matches %s\n", 
		   UNKNOWN (NAMEONPCB_NAME(element) ),
		   ignore_refdes[i]);
	  noskip = 0;
	}
    }
  if ( noskip )
    for (i=0 ; i<n_value ; i++) 
      {
	if ( (strcmp (UNKNOWN (VALUE_NAME(element) ), ignore_value[i]) == 0) ||
	     rematch ( ignore_value[i], UNKNOWN (VALUE_NAME(element) )) )
	  {
	    Message ("Vendor mapping skipped because value = %s matches %s\n", 
		     UNKNOWN (VALUE_NAME(element)),
		     ignore_value[i]);
	    noskip = 0;
	  }
      }
  
  if ( noskip )
    for (i=0 ; i<n_descr ; i++) {
      if ( (strcmp (UNKNOWN (DESCRIPTION_NAME(element) ), ignore_descr[i]) == 0) ||
	   rematch ( ignore_descr[i], UNKNOWN (DESCRIPTION_NAME(element) )) )
	{
	  Message ("Vendor mapping skipped because descr = %s matches %s\n", 
		   UNKNOWN (DESCRIPTION_NAME(element) ),
		   ignore_descr[i]);
	  noskip = 0;
	}
    }
  
  if ( noskip && TEST_FLAG (LOCKFLAG, element) ) 
    {
      Message ("Vendor mapping skipped because element %s is locked\n", UNKNOWN (NAMEONPCB_NAME(element) ));
      noskip = 0;
    }

  if ( noskip )
    return True;
  else
    return False;
}

static Boolean
rematch( const char *re, const char *s)
{
  /*
   * If this system has regular expression capability, then
   * add support for regular expressions in the skip lists.
   */
  
#if defined(HAVE_REGCOMP)
  
  int result;
  regmatch_t match;
  regex_t compiled;
  
  /* compile the regular expression */
  result = regcomp (&compiled, re, REG_EXTENDED | REG_ICASE | REG_NOSUB);
  if (result)
    {
      char errorstring[128];
      
      regerror (result, &compiled, errorstring, sizeof(errorstring));
      Message ("regexp error: %s\n", errorstring);
      regfree (&compiled);
      return (False);
    }
  
  result = regexec (&compiled, s, 1, &match, 0);

  if (result == 0)
    return (True);
  else
    return (False);

  regfree (&compiled);

#elif defined(HAVE_RE_COMP)
  int m;
  char *rslt;

  /* compile the regular expression */
  if ((rslt = re_comp (re)) != NULL)
    {
      Message ("re_comp error: %s\n", rslt);
      return (False);
    }

  m = re_exec (s);

  switch m
    {
    case 1:
      return (True);
      break;

    case 0:
      return (False);
      break;

    default:
      Message ("re_exec error\n");
      break;
    }

#else
    return (False);
#endif

}
