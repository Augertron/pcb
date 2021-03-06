/* $Id: printpanner.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $ */

/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 1994,1995,1996 Thomas Nau
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
 *  Thomas Nau, Schlehenweg 15, 88471 Baustetten, Germany
 *  Thomas.Nau@rz.uni-ulm.de
 *
 */


/* print dialog panner routines
 * only available for X11R5 and later
 * all routines are defined 'empty' for X11R4
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

#include "global.h"

#include "data.h"
#include "menu.h"
#include "misc.h"
#include "printpanner.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

RCSID("$Id: printpanner.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $");


/* ---------------------------------------------------------------------------
 * some local identifiers
 */
static Position SliderX,	/* save original values */
  SliderY;			/* to prevent from rounding */
static Dimension SliderWidth,	/* errors caused by slider */
  SliderHeight,			/* scaling */
  PCBWidth,			/* data bounding box size */
  PCBHeight;
static MediaTypePtr MediaSelection = NULL;	/* selected media */

MediaType Media[] = {	/* first one is user defined */
  {USERMEDIANAME, 0, 0, 0, 0},
  {"  A3  ", 1169000, 1653000, 50000, 50000},
  {"  A4  ", 826000, 1169000, 50000, 50000},
  {"  A5  ", 583000, 826000, 50000, 50000},
  {"Letter", 850000, 1100000, 50000, 50000},
  {"11x17", 1100000, 1700000, 50000, 50000},
  {"Ledger", 1700000, 1100000, 50000, 50000},
  {"Legal", 850000, 1400000, 50000, 50000},
  {"Executive", 750000, 1000000, 50000, 50000},
  {"C-size", 1700000, 2200000, 50000, 50000},
  {"D-size", 2200000, 3400000, 50000, 50000}
};
static PopupEntryType MediaMenuEntries[ENTRIES (Media) + 1];
static PopupMenuType MediaMenu =
  { "media", NULL, MediaMenuEntries, NULL, NULL, NULL };
static MenuButtonType MediaMenuButton =
  { "media", "Media", &MediaMenu, NULL };

/* ---------------------------------------------------------------------------
 * a local prototype for both cases
 */
static void InitDefaultMedia (void);

/***************************************************************************** 
 * routines for X11R5 an newer
 *****************************************************************************/

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Panner.h>

/* ---------------------------------------------------------------------------
 * some local prototypes
 */
static void CB_Media (Widget, XtPointer, XtPointer);
static Widget PrintPannerCreateMediaMenu (Widget, Widget, Widget);
static void CB_Report (Widget, XtPointer, XtPointer);

/* ---------------------------------------------------------------------------
 * some local identifiers
 */
static Widget Panner,		/* the panner widget */
  SizeField;			/* label to display the media size */

/* ---------------------------------------------------------------------------
 * returns pointer to current media selection
 */
MediaTypePtr
PrintPannerGetMedia (void)
{
  return (MediaSelection);
}

/* ---------------------------------------------------------------------------
 * callback for media size selection (menu)
 */
static void
CB_Media (Widget W, XtPointer ClientData, XtPointer CallData)
{
  char size[80];
  BDimension mediawidth, mediaheight;

  /* print visible size of media */
  MediaSelection = (MediaTypePtr) ClientData;
  mediawidth = MediaSelection->Width - 2 * MediaSelection->MarginX;
  mediaheight = MediaSelection->Height - 2 * MediaSelection->MarginY;
  sprintf (size, "media size:     %4.2f\" x %4.2f\"\n"
	   "with margin: %4.2f\" x %4.2f\"\n",
	   (float) MediaSelection->Width / 100000.0,
	   (float) MediaSelection->Height / 100000.0,
	   (float) mediawidth / 100000.0, (float) mediaheight / 100000.0);

  /* update widgets */
  XtVaSetValues (MediaMenuButton.W, XtNlabel, MediaSelection->Name, NULL);
  XtVaSetValues (SizeField, XtNlabel, size, NULL);
  XtVaSetValues (Panner,
		 XtNcanvasHeight, mediaheight/100,
		 XtNcanvasWidth, mediawidth/100, NULL);
}

/* ---------------------------------------------------------------------------
 * creates 'media size' menu
 */
static Widget
PrintPannerCreateMediaMenu (Widget Parent, Widget Top, Widget Left)
{
  int i;

  /* copy media description to menuentry structure */
  for (i = 0; i < ENTRIES (Media); i++)
    {
      MediaMenuEntries[i].Name = Media[i].Name;
      MediaMenuEntries[i].Label = Media[i].Name;
      MediaMenuEntries[i].Callback = CB_Media;
      MediaMenuEntries[i].ClientData = (char *) &Media[i];
      MediaMenuEntries[i].W = NULL;
    }
  return (InitMenuButton (Parent, &MediaMenuButton, Top, Left));
}

/* ---------------------------------------------------------------------------
 * report callback to get updated slider position
 */
static void
CB_Report (Widget W, XtPointer ClientData, XtPointer CallData)
{
  XawPannerReport *report = (XawPannerReport *) CallData;
  BDimension mediawidth, mediaheight;

  mediawidth = (MediaSelection->Width - 2 * MediaSelection->MarginX)/100;
  mediaheight =(MediaSelection->Height - 2 * MediaSelection->MarginY)/100;
  SliderX = report->slider_x - (mediawidth - report->slider_width) / 2;
  SliderY = report->slider_y - (mediaheight - report->slider_height) / 2;
  PrintPannerSetSliderPosition (SliderX, SliderY);
}

/* ---------------------------------------------------------------------------
 * creates a print command dialog
 */
Widget
PrintPannerCreate (Widget Parent, Widget Top, Widget Left)
{
  Widget masterform, label, menu;
  BoxTypePtr box;

  /* init media size if called the first time */
  if (!MediaSelection)
    InitDefaultMedia ();

  masterform =
    XtVaCreateManagedWidget ("printpannerMasterForm", formWidgetClass, Parent,
			     LAYOUT_TOP, XtNfromVert, Top, XtNfromHoriz, Left,
			     XtNborderWidth, 0, NULL);

  label = XtVaCreateManagedWidget ("comment", labelWidgetClass,
				   masterform,
				   LAYOUT_TOP,
				   XtNfromVert, NULL,
				   XtNfromHoriz, NULL,
				   XtNlabel, "select media and offsets:",
				   NULL);
  menu = PrintPannerCreateMediaMenu (masterform, label, NULL);
  SizeField = XtVaCreateManagedWidget ("sizeField", labelWidgetClass,
				       masterform,
				       LAYOUT_TOP,
				       XtNfromVert, menu,
				       XtNfromHoriz, NULL,
				       XtNlabel, "", NULL);
  Panner = XtVaCreateManagedWidget ("panner", pannerWidgetClass,
				    masterform,
				    LAYOUT_TOP,
				    XtNdefaultScale, 1,
				    XtNfromVert, NULL,
				    XtNfromHoriz, SizeField,
				    XtNrubberBand, True,
				    XtNresizable, True, NULL);

  /* register callback to get updated slider position */
  XtAddCallback (Panner, XtNreportCallback, CB_Report, NULL);

  /* get layout size and initialize size label;
   * layout has already be checked to be non-empty
   */
  box = GetDataBoundingBox (PCB->Data);
  PCBWidth = (box->X2 - box->X1)/100;
  PCBHeight = (box->Y2 - box->Y1)/100;
  CB_Media (NULL, (XtPointer) MediaSelection, NULL);
  return (masterform);
}

/* ----------------------------------------------------------------------
 * set the panner offset
 * offset is specified from center of media
 */
void
PrintPannerSetSliderPosition (Position OffsetX, Position OffsetY)
{
  Dimension width, height;

  SliderX = OffsetX;
  SliderY = OffsetY;
  XtVaGetValues (Panner,
		 XtNsliderWidth, &width, XtNsliderHeight, &height, NULL);
  OffsetX += 
    ((MediaSelection->Width - 2 * MediaSelection->MarginX - 100 * (int)width) / 200);
  OffsetY += 
    ((MediaSelection->Height - 2 * MediaSelection->MarginY - 100 * (int)height) / 200);
  XtVaSetValues (Panner, XtNsliderX, OffsetX, XtNsliderY, OffsetY, NULL);
}

/* ---------------------------------------------------------------------------
 * initializes the user defined mediasize and sets it as default
 * only called once
 */
static void
InitDefaultMedia (void)
{
  int i, x, y;
  unsigned int width, height;

  /* check if the passed string is equal to a known media type
   * first entry is user defined, last one holds a zero pointer only
   */
  MediaSelection = Media + 1;
  for (i = 1; i < ENTRIES (Media) - 1; i++)
    {
      if (strcasecmp (Media[i].Name, Settings.Media) == 0)
        break;
      MediaSelection++;
    }

  if (MediaSelection != &Media[ENTRIES (Media) - 1])
    {
      /* used the found entry */
      MediaSelection = &Media[i];
      width = Media[i].Width;
      height = Media[i].Height;
      x = Media[i].MarginX;
      y = Media[i].MarginY;
    }
  else
    {
      /* no match, try to evaluate it as geometry string;
       * use first entry (user defined)
       */
      MediaSelection = Media;
      i = XParseGeometry (Settings.Media, &x, &y, &width, &height);

      /* check the syntax */
      if ((i & WidthValue) || (i & HeightValue) ||
	  (i & XValue) || (i & YValue))
	{
	  /* failed; use default setting (a4), no margin */
	  width = 826000;
	  height = 1169000;
	  x = y = 0;
	}
    }

  /* set size as user default; MediaSelection has already been set != NULL
   * to make sure that routine wont be called again
   */
  Media[0].Width = width;
  Media[0].Height = height;
  Media[0].MarginX = x;
  Media[0].MarginY = y;
}

/* ---------------------------------------------------------------------------
 * updates the panner widget
 */
void
PrintPannerUpdate (float Scale, Boolean RotateFlag, Boolean OutlineFlag,
		   Boolean AlignmentFlag)
{
  /* scale layout size */
  SliderWidth = OutlineFlag ? PCB->MaxWidth/100 : PCBWidth;
  SliderHeight = OutlineFlag ? PCB->MaxHeight/100 : PCBHeight;
  if (AlignmentFlag)
    {
      SliderWidth += Settings.AlignmentDistance / 50;
      SliderHeight += Settings.AlignmentDistance / 50;
    }
  if (RotateFlag)
    {
      Dimension save = SliderWidth;

      SliderWidth = (Dimension) ((float) SliderHeight * Scale);
      SliderHeight = (Dimension) ((float) save * Scale);
    }
  else
    {
      SliderWidth = (Dimension) ((float) SliderWidth * Scale);
      SliderHeight = (Dimension) ((float) SliderHeight * Scale);
    }

  /* update slider */
#if XlibSpecificationRelease >= 5
  XtVaSetValues (Panner,
		 XtNsliderWidth, SliderWidth,
		 XtNsliderHeight, SliderHeight, NULL);
  PrintPannerSetSliderPosition (SliderX, SliderY);
#endif
}

/* ----------------------------------------------------------------------
 * returns the size of the slider
 */
void
PrintPannerGetSize (Dimension * Width, Dimension * Height)
{
  *Width = SliderWidth;
  *Height = SliderHeight;
}

/* ----------------------------------------------------------------------
 * returns the offset of the slider
 * offset is returned from center of media
 */
void
PrintPannerGetSliderPosition (Position * OffsetX, Position * OffsetY)
{
  *OffsetX = SliderX;
  *OffsetY = SliderY;
}
