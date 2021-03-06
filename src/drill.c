/* $Id: drill.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $ */

/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 1994,1995,1996 Thomas Nau
 *
 *  This module, drill.c, was written and is Copyright (C) 1997 harry eaton
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "data.h"
#include "error.h"
#include "mymem.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

RCSID("$Id: drill.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $");




/*
 * some local prototypes
 */
static void FillDrill (DrillTypePtr, ElementTypePtr, PinTypePtr);
static void InitializeDrill (DrillTypePtr, PinTypePtr, ElementTypePtr);


static void
FillDrill (DrillTypePtr Drill, ElementTypePtr Element, PinTypePtr Pin)
{
  Cardinal n;
  ElementTypeHandle ptr;
  PinTypeHandle pin;

  pin = GetDrillPinMemory (Drill);
  *pin = Pin;
  if (Element)
    {
      Drill->PinCount++;
      for (n = Drill->ElementN - 1; n != -1; n--)
	if (Drill->Element[n] == Element)
	  break;
      if (n == -1)
	{
	  ptr = GetDrillElementMemory (Drill);
	  *ptr = Element;
	}
    }
  else
    Drill->ViaCount++;
  if (TEST_FLAG (HOLEFLAG, Pin))
    Drill->UnplatedCount++;
}

static void
InitializeDrill (DrillTypePtr drill, PinTypePtr pin, ElementTypePtr element)
{
  void *ptr;

  drill->DrillSize = pin->DrillingHole;
  drill->ElementN = 0;
  drill->ViaCount = 0;
  drill->PinCount = 0;
  drill->UnplatedCount = 0;
  drill->ElementMax = 0;
  drill->Element = NULL;
  drill->PinN = 0;
  drill->Pin = NULL;
  drill->PinMax = 0;
  ptr = (void *) GetDrillPinMemory (drill);
  *((PinTypeHandle) ptr) = pin;
  if (element)
    {
      ptr = (void *) GetDrillElementMemory (drill);
      *((ElementTypeHandle) ptr) = element;
      drill->PinCount = 1;
    }
  else
    drill->ViaCount = 1;
  if (TEST_FLAG (HOLEFLAG, pin))
    drill->UnplatedCount = 1;
}

static int
DrillQSort (const void *va, const void *vb)
{
  DrillType *a = (DrillType *) va;
  DrillType *b = (DrillType *) vb;
  return a->DrillSize - b->DrillSize;
}

DrillInfoTypePtr
GetDrillInfo (DataTypePtr top)
{
  DrillInfoTypePtr AllDrills;
  DrillTypePtr Drill=NULL;
  DrillType savedrill, swapdrill;
  Boolean DrillFound = False;
  Boolean NewDrill;

  AllDrills = MyCalloc (1, sizeof (DrillInfoType), "GetAllDrillInfo()");
  ALLPIN_LOOP (top);
  {
    if (!DrillFound)
      {
	DrillFound = True;
	Drill = GetDrillInfoDrillMemory (AllDrills);
	InitializeDrill (Drill, pin, element);
      }
    else
      {
	if (Drill->DrillSize == pin->DrillingHole)
	  FillDrill (Drill, element, pin);
	else
	  {
	    NewDrill = False;
	    DRILL_LOOP (AllDrills);
	    {
	      if (drill->DrillSize == pin->DrillingHole)
		{
		  Drill = drill;
		  FillDrill (Drill, element, pin);
		  break;
		}
	      else if (drill->DrillSize > pin->DrillingHole)
		{
		  if (!NewDrill)
		    {
		      NewDrill = True;
		      InitializeDrill (&swapdrill, pin, element);
		      Drill = GetDrillInfoDrillMemory (AllDrills);
		      Drill->DrillSize = pin->DrillingHole + 1;
		      Drill = drill;
		    }
		  savedrill = *drill;
		  *drill = swapdrill;
		  swapdrill = savedrill;
		}
	    }
	    END_LOOP;
	    if (AllDrills->Drill[AllDrills->DrillN - 1].DrillSize <
		pin->DrillingHole)
	      {
		Drill = GetDrillInfoDrillMemory (AllDrills);
		InitializeDrill (Drill, pin, element);
	      }
	  }
      }
  }
  ENDALL_LOOP;
  VIA_LOOP (top);
  {
    if (!DrillFound)
      {
	DrillFound = True;
	Drill = GetDrillInfoDrillMemory (AllDrills);
	Drill->DrillSize = via->DrillingHole;
	FillDrill (Drill, NULL, via);
      }
    else
      {
	if (Drill->DrillSize != via->DrillingHole)
	  {
	    DRILL_LOOP (AllDrills);
	    {
	      if (drill->DrillSize == via->DrillingHole)
		{
		  Drill = drill;
		  FillDrill (Drill, NULL, via);
		  break;
		}
	    }
	    END_LOOP;
	    if (Drill->DrillSize != via->DrillingHole)
	      {
		Drill = GetDrillInfoDrillMemory (AllDrills);
		Drill->DrillSize = via->DrillingHole;
		FillDrill (Drill, NULL, via);
	      }
	  }
	else
	  FillDrill (Drill, NULL, via);
      }
  }
  END_LOOP;
  qsort (AllDrills->Drill, AllDrills->DrillN, sizeof (DrillType), DrillQSort);
  return (AllDrills);
}

void
FreeDrillInfo (DrillInfoTypePtr Drills)
{
  DRILL_LOOP (Drills);
  {
    MyFree ((char **) &drill->Element);
    MyFree ((char **) &drill->Pin);
  }
  END_LOOP;
  MyFree ((char **) &Drills->Drill);
  SaveFree (Drills);
}
