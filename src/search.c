/* $Id: search.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $ */

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


/* search routines
 * some of the functions use dummy parameters
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <setjmp.h>

#include "global.h"

#include "data.h"
#include "draw.h"
#include "error.h"
#include "find.h"
#include "misc.h"
#include "rtree.h"
#include "search.h"
/* Added BY NP 7/23/2005 */
#include "create.h"
#include "remove.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

RCSID("$Id: search.c,v 1.1.1.1 2006/04/10 22:05:23 tim Exp $");


/* ---------------------------------------------------------------------------
 * some local identifiers
 */
static float PosX,		/* search position for subroutines */
  PosY;
static BDimension SearchRadius;
static BoxType SearchBox;
static LayerTypePtr SearchLayer;
static Boolean ScreenOnly = False;

/* ---------------------------------------------------------------------------
 * some local prototypes
 */
static Boolean SearchLineByLocation (LayerTypePtr *, LineTypePtr *,
				     LineTypePtr *);
static Boolean SearchArcByLocation (LayerTypePtr *, ArcTypePtr *,
				    ArcTypePtr *);
static Boolean SearchRatLineByLocation (RatTypePtr *, RatTypePtr *,
					RatTypePtr *);
static Boolean SearchTextByLocation (LayerTypePtr *, TextTypePtr *,
				     TextTypePtr *);
static Boolean SearchPolygonByLocation (LayerTypePtr *, PolygonTypePtr *,
					PolygonTypePtr *);
static Boolean SearchPinByLocation (ElementTypePtr *, PinTypePtr *,
				    PinTypePtr *);
static Boolean SearchPadByLocation (ElementTypePtr *, PadTypePtr *,
				    PadTypePtr *, Boolean);
static Boolean SearchViaByLocation (PinTypePtr *, PinTypePtr *, PinTypePtr *);
static Boolean SearchElementNameByLocation (ElementTypePtr *, TextTypePtr *,
					    TextTypePtr *, Boolean);
static Boolean SearchLinePointByLocation (LayerTypePtr *, LineTypePtr *,
					  PointTypePtr *);
static Boolean SearchPointByLocation (LayerTypePtr *, PolygonTypePtr *,
				      PointTypePtr *);
static Boolean SearchElementByLocation (ElementTypePtr *, ElementTypePtr *,
					ElementTypePtr *, Boolean);

/* ---------------------------------------------------------------------------
 * searches a via
 */
struct ans_info
{
  void **ptr1, **ptr2, **ptr3;
  Boolean BackToo;
  float area;
  jmp_buf env;
};

static int
pinorvia_callback (const BoxType * box, void *cl)
{
  struct ans_info *i = (struct ans_info *) cl;
  PinTypePtr pin = (PinTypePtr) box;

  if (!IsPointOnPin (PosX, PosY, SearchRadius, pin))
    return 0;
  *i->ptr1 = pin->Element ? pin->Element : pin;
  *i->ptr2 = *i->ptr3 = pin;
  longjmp (i->env, 1);
  return 1;			/* never reached */
}

static Boolean
SearchViaByLocation (PinTypePtr * Via, PinTypePtr * Dummy1,
		     PinTypePtr * Dummy2)
{
  struct ans_info info;

  info.ptr1 = (void **) Via;
  info.ptr2 = (void **) Dummy1;
  info.ptr3 = (void **) Dummy2;
  /* search only if via-layer is visible */
  if (!PCB->ViaOn)
    return False;
  if (setjmp (info.env) == 0)
    {
      r_search (PCB->Data->via_tree, &SearchBox, NULL, pinorvia_callback,
		&info);
      return False;
    }
  return True;
}

/* ---------------------------------------------------------------------------
 * searches a pin
 * starts with the newest element
 */
static Boolean
SearchPinByLocation (ElementTypePtr * Element, PinTypePtr * Pin,
		     PinTypePtr * Dummy)
{
  struct ans_info info;

  /* search only if pin-layer is visible */
  if (!PCB->PinOn)
    return False;
  info.ptr1 = (void **) Element;
  info.ptr2 = (void **) Pin;
  info.ptr3 = (void **) Dummy;

  if (setjmp (info.env) == 0)
    r_search (PCB->Data->pin_tree, &SearchBox, NULL, pinorvia_callback,
	      &info);
  else
    return True;
  return False;
}

static int
pad_callback (const BoxType * b, void *cl)
{
  PadTypePtr pad = (PadTypePtr) b;
  struct ans_info *i = (struct ans_info *) cl;

  if (FRONT (pad) || i->BackToo)
    {
      if (TEST_FLAG (SQUAREFLAG, pad))
	{
	  if (IsPointInSquarePad (PosX, PosY, SearchRadius, pad))
	    {
	      *i->ptr1 = pad->Element;
	      *i->ptr2 = *i->ptr3 = pad;
	      longjmp (i->env, 1);
	    }
	}
      else
	{
	  if (IsPointOnLine (PosX, PosY, SearchRadius, (LineTypePtr) pad))
	    {
	      *i->ptr1 = pad->Element;
	      *i->ptr2 = *i->ptr3 = pad;
	      longjmp (i->env, 1);
	    }
	}
    }
  return 0;
}

/* ---------------------------------------------------------------------------
 * searches a pad
 * starts with the newest element
 */
static Boolean
SearchPadByLocation (ElementTypePtr * Element, PadTypePtr * Pad,
		     PadTypePtr * Dummy, Boolean BackToo)
{
  struct ans_info info;

  /* search only if pin-layer is visible */
  if (!PCB->PinOn)
    return (False);
  info.ptr1 = (void **) Element;
  info.ptr2 = (void **) Pad;
  info.ptr3 = (void **) Dummy;
  info.BackToo = (BackToo && PCB->InvisibleObjectsOn);
  if (setjmp (info.env) == 0)
    r_search (PCB->Data->pad_tree, &SearchBox, NULL, pad_callback, &info);
  else
    return True;
  return False;
}

/* ---------------------------------------------------------------------------
 * searches ordinary line on the SearchLayer 
 */

struct line_info
{
  LineTypePtr *Line;
  PointTypePtr *Point;
  float least;
  jmp_buf env;
};

static int
line_callback (const BoxType * box, void *cl)
{
  struct line_info *i = (struct line_info *) cl;
  LineTypePtr l = (LineTypePtr) box;

  if (!IsPointOnLine (PosX, PosY, SearchRadius, l))
    return 0;
  *i->Line = l;
  *i->Point = (PointTypePtr) l;
  longjmp (i->env, 1);
  return 1;			/* never reached */
}


static Boolean
SearchLineByLocation (LayerTypePtr * Layer, LineTypePtr * Line,
		      LineTypePtr * Dummy)
{
  struct line_info info;

  info.Line = Line;
  info.Point = (PointTypePtr *) Dummy;

  *Layer = SearchLayer;
  if (setjmp (info.env) == 0)
    {
      r_search (SearchLayer->line_tree, &SearchBox, NULL, line_callback,
		&info);
      return False;
    }
  return (True);
}

static int
rat_callback (const BoxType * box, void *cl)
{
  LineTypePtr line = (LineTypePtr) box;
  struct ans_info *i = (struct ans_info *) cl;

  if (IsPointOnLine (PosX, PosY, SearchRadius, line))
    {
      *i->ptr1 = *i->ptr2 = *i->ptr3 = line;
      longjmp (i->env, 1);
    }
  return 0;
}

/* ---------------------------------------------------------------------------
 * searches rat lines if they are visible
 */
static Boolean
SearchRatLineByLocation (RatTypePtr * Line, RatTypePtr * Dummy1,
			 RatTypePtr * Dummy2)
{
  struct ans_info info;

  info.ptr1 = (void **) Line;
  info.ptr2 = (void **) Dummy1;
  info.ptr3 = (void **) Dummy2;

  if (setjmp (info.env) == 0)
    {
      r_search (PCB->Data->rat_tree, &SearchBox, NULL, rat_callback, &info);
      return False;
    }
  return (True);
}

/* ---------------------------------------------------------------------------
 * searches arc on the SearchLayer 
 */
struct arc_info
{
  ArcTypePtr *Arc, *Dummy;
  jmp_buf env;
};

static int
arc_callback (const BoxType * box, void *cl)
{
  struct arc_info *i = (struct arc_info *) cl;
  ArcTypePtr a = (ArcTypePtr) box;

  if (!IsPointOnArc (PosX, PosY, SearchRadius, a))
    return 0;
  *i->Arc = a;
  *i->Dummy = a;
  longjmp (i->env, 1);
  return 1;			/* never reached */
}


static Boolean
SearchArcByLocation (LayerTypePtr * Layer, ArcTypePtr * Arc,
		     ArcTypePtr * Dummy)
{
  struct arc_info info;

  info.Arc = Arc;
  info.Dummy = Dummy;

  *Layer = SearchLayer;
  if (setjmp (info.env) == 0)
    {
      r_search (SearchLayer->arc_tree, &SearchBox, NULL, arc_callback, &info);
      return False;
    }
  return (True);
}

static int
text_callback (const BoxType * box, void *cl)
{
  TextTypePtr text = (TextTypePtr) box;
  struct ans_info *i = (struct ans_info *) cl;

  if (POINT_IN_BOX (PosX, PosY, &text->BoundingBox))
    {
      *i->ptr2 = *i->ptr3 = text;
      longjmp (i->env, 1);
    }
  return 0;
}

/* ---------------------------------------------------------------------------
 * searches text on the SearchLayer
 */
static Boolean
SearchTextByLocation (LayerTypePtr * Layer, TextTypePtr * Text,
		      TextTypePtr * Dummy)
{
  struct ans_info info;

  *Layer = SearchLayer;
  info.ptr2 = (void **) Text;
  info.ptr3 = (void **) Dummy;

  if (setjmp (info.env) == 0)
    {
      r_search (SearchLayer->text_tree, &SearchBox, NULL, text_callback,
		&info);
      return False;
    }
  return (True);
}

/* ---------------------------------------------------------------------------
 * searches a polygon on the SearchLayer 
 */
static Boolean
SearchPolygonByLocation (LayerTypePtr * Layer,
			 PolygonTypePtr * Polygon, PolygonTypePtr * Dummy)
{
  *Layer = SearchLayer;
  POLYGON_LOOP (*Layer);
  {
    if (ScreenOnly && !VPOLY (polygon))
      continue;
    if (IsPointInPolygon (PosX, PosY, SearchRadius, polygon))
      {
	*Polygon = *Dummy = polygon;
	return (True);
      }
  }
  END_LOOP;
  return (False);
}

static int
linepoint_callback (const BoxType * b, void *cl)
{
  LineTypePtr line = (LineTypePtr) b;
  struct line_info *i = (struct line_info *) cl;
  int ret_val = 0;
  float d;

  /* some stupid code to check both points */
  d = SQUARE (PosX - line->Point1.X) + SQUARE (PosY - line->Point1.Y);
  if (d < i->least)
    {
      i->least = d;
      *i->Line = line;
      *i->Point = &line->Point1;
      ret_val = 1;
    }

  d = SQUARE (PosX - line->Point2.X) + SQUARE (PosY - line->Point2.Y);
  if (d < i->least)
    {
      i->least = d;
      *i->Line = line;
      *i->Point = &line->Point2;
      ret_val = 1;
    }
  return ret_val;
}

/* ---------------------------------------------------------------------------
 * searches a line-point on all the search layer
 */
static Boolean
SearchLinePointByLocation (LayerTypePtr * Layer, LineTypePtr * Line,
			   PointTypePtr * Point)
{
  struct line_info info;
  *Layer = SearchLayer;
  info.Line = Line;
  info.Point = Point;
  *Point = NULL;
  info.least =
    (MAX_LINE_POINT_DISTANCE + SearchRadius) * (MAX_LINE_POINT_DISTANCE +
						SearchRadius);
  if (r_search
      (SearchLayer->line_tree, &SearchBox, NULL, linepoint_callback, &info))
    return True;
  return False;
}

/* ---------------------------------------------------------------------------
 * searches a polygon-point on all layers that are switched on
 * in layerstack order
 */
static Boolean
SearchPointByLocation (LayerTypePtr * Layer,
		       PolygonTypePtr * Polygon, PointTypePtr * Point)
{
  float d, least;
  Boolean found = False;

  least = SQUARE (SearchRadius + MAX_POLYGON_POINT_DISTANCE);
  *Layer = SearchLayer;
  POLYGON_LOOP (*Layer);
  {
    if (ScreenOnly && !VPOLY (polygon))
      continue;
    POLYGONPOINT_LOOP (polygon);
    {
      d = SQUARE (point->X - PosX) + SQUARE (point->Y - PosY);
      if (d < least)
	{
	  least = d;
	  *Polygon = polygon;
	  *Point = point;
	  found = True;
	}
    }
    END_LOOP;
  }
  END_LOOP;
  if (found)
    return (True);
  return (False);
}

static int
name_callback (const BoxType * box, void *cl)
{
  TextTypePtr text = (TextTypePtr) box;
  struct ans_info *i = (struct ans_info *) cl;
  ElementTypePtr element = (ElementTypePtr) text->Element;
  float newarea;

  if ((FRONT (element) || i->BackToo) && !TEST_FLAG (HIDENAMEFLAG, element) &&
      POINT_IN_BOX (PosX, PosY, &text->BoundingBox))
    {
      /* use the text with the smallest bounding box */
      newarea = (text->BoundingBox.X2 - text->BoundingBox.X1) *
	(float) (text->BoundingBox.Y2 - text->BoundingBox.Y1);
      if (newarea < i->area)
	{
	  i->area = newarea;
	  *i->ptr1 = element;
	  *i->ptr2 = *i->ptr3 = text;
	}
      return 1;
    }
  return 0;
}

/* ---------------------------------------------------------------------------
 * searches the name of an element
 * the search starts with the last element and goes back to the beginning
 */
static Boolean
SearchElementNameByLocation (ElementTypePtr * Element,
			     TextTypePtr * Text, TextTypePtr * Dummy,
			     Boolean BackToo)
{
  struct ans_info info;

  /* package layer have to be switched on */
  if (PCB->ElementOn)
    {
      info.ptr1 = (void **) Element;
      info.ptr2 = (void **) Text;
      info.ptr3 = (void **) Dummy;
      info.area = SQUARE (MAX_COORD);
      info.BackToo = (BackToo && PCB->InvisibleObjectsOn);
      if (r_search (PCB->Data->name_tree[NAME_INDEX (PCB)], &SearchBox, NULL,
		    name_callback, &info))
	return True;
    }
  return (False);
}

static int
element_callback (const BoxType * box, void *cl)
{
  ElementTypePtr element = (ElementTypePtr) box;
  struct ans_info *i = (struct ans_info *) cl;
  float newarea;

  if ((FRONT (element) || i->BackToo) &&
      POINT_IN_BOX (PosX, PosY, &element->VBox))
    {
      /* use the element with the smallest bounding box */
      newarea = (element->VBox.X2 - element->VBox.X1) *
	(float) (element->VBox.Y2 - element->VBox.Y1);
      if (newarea < i->area)
	{
	  i->area = newarea;
	  *i->ptr1 = *i->ptr2 = *i->ptr3 = element;
	}
      return 1;
    }
  return 0;
}

/* ---------------------------------------------------------------------------
 * searches an element
 * the search starts with the last element and goes back to the beginning
 * if more than one element matches, the smallest one is taken
 */
static Boolean
SearchElementByLocation (ElementTypePtr * Element,
			 ElementTypePtr * Dummy1, ElementTypePtr * Dummy2,
			 Boolean BackToo)
{
  struct ans_info info;

  /* Both package layers have to be switched on */
  if (PCB->ElementOn && PCB->PinOn)
    {
      info.ptr1 = (void **) Element;
      info.ptr2 = (void **) Dummy1;
      info.ptr3 = (void **) Dummy2;
      info.area = SQUARE (MAX_COORD);
      info.BackToo = (BackToo && PCB->InvisibleObjectsOn);
      if (r_search
	  (PCB->Data->element_tree, &SearchBox, NULL, element_callback,
	   &info))
	return True;
    }
  return False;
}

/* ---------------------------------------------------------------------------
 * checks if a point is on a pin
 */
Boolean
IsPointOnPin (float X, float Y, float Radius, PinTypePtr pin)
{
  if (TEST_FLAG (SQUAREFLAG, pin))
    {
      BoxType b;
      BDimension t = pin->Thickness / 2;

      b.X1 = pin->X - t;
      b.X2 = pin->X + t;
      b.Y1 = pin->Y - t;
      b.Y2 = pin->Y + t;
      if (IsPointInBox (X, Y, &b, Radius))
	return (True);
    }
  else if (SQUARE (pin->X - X) + SQUARE (pin->Y - Y) <=
	   SQUARE (pin->Thickness / 2 + Radius))
    return (True);
  return (False);
}

/* ---------------------------------------------------------------------------
 * checks if a rat-line end is on a PV
 */
Boolean
IsPointOnLineEnd (Location X, Location Y, RatTypePtr Line)
{
  if (((X == Line->Point1.X) && (Y == Line->Point1.Y)) ||
      ((X == Line->Point2.X) && (Y == Line->Point2.Y)))
    return (True);
  return (False);
}

/* ---------------------------------------------------------------------------
 * checks if a line intersects with a PV
 * constant recognition by the optimizer is assumed
 *
 * let the point be (X,Y) and the line (X1,Y1)(X2,Y2)
 * the length of the line is
 *
 *   l = ((X2-X1)^2 + (Y2-Y1)^2)^0.5
 * 
 * let Q be the point of perpendicular projection of (X,Y) onto the line
 *
 *   QX = X1 +r*(X2-X1)
 *   QY = Y1 +r*(Y2-Y1)
 * 
 * with (from vector geometry)
 *
 *       (Y1-Y)(Y1-Y2)+(X1-X)(X1-X2)
 *   r = ---------------------------
 *                   l*l
 *
 *   r < 0     Q is on backward extension of the line
 *   r > 1     Q is on forward extension of the line
 *   else      Q is on the line
 *
 * the signed distance from (X,Y) to Q is
 *
 *       (Y2-Y1)(X-X1)-(X2-X1)(Y-Y1)
 *   d = ----------------------------
 *                    l
 */
Boolean
IsPointOnLine (float X, float Y, float Radius, LineTypePtr Line)
{
  register float dx, dy, dx1, dy1, l, d, r;
  Radius += ((float) Line->Thickness + 1.) / 2.0;
  if (Y + Radius < MIN (Line->Point1.Y, Line->Point2.Y) ||
      Y - Radius > MAX (Line->Point1.Y, Line->Point2.Y))
    return False;
  dx = (float) (Line->Point2.X - Line->Point1.X);
  dy = (float) (Line->Point2.Y - Line->Point1.Y);
  dx1 = (float) (Line->Point1.X - X);
  dy1 = (float) (Line->Point1.Y - Y);
  d = dx * dy1 - dy * dx1;

  /* check distance from PV to line */
  Radius *= Radius;
  if ((l = dx * dx + dy * dy) == 0.0)
    {
      l = SQUARE (Line->Point1.X - X) + SQUARE (Line->Point1.Y - Y);
      return ((l <= Radius) ? True : False);
    }
  if (d * d > Radius * l)
    return (False);

  /* they intersect if Q is on line */
  r = -(dx * dx1 + dy * dy1);
  if (r >= 0 && r <= l)
    return (True);

  /* we have to check P1 or P2 depending on the sign of r */
  if (r < 0.0)
    return ((dx1 * dx1 + dy1 * dy1) <= Radius);
  dx1 = Line->Point2.X - X;
  dy1 = Line->Point2.Y - Y;
  return ((dx1 * dx1 + dy1 * dy1) <= Radius);
}

/* ---------------------------------------------------------------------------
 * checks if a line crosses a rectangle
 */
Boolean
IsLineInRectangle (Location X1, Location Y1,
		   Location X2, Location Y2, LineTypePtr Line)
{
  LineType line;

  /* first, see if point 1 is inside the rectangle */
  /* in case the whole line is inside the rectangle */
  if (X1 < Line->Point1.X && X2 > Line->Point1.X &&
      Y1 < Line->Point1.Y && Y2 > Line->Point1.Y)
    return (True);
  /* construct a set of dummy lines and check each of them */
  line.Thickness = 0;
  line.Flags = NOFLAG;

  /* upper-left to upper-right corner */
  line.Point1.Y = line.Point2.Y = Y1;
  line.Point1.X = X1;
  line.Point2.X = X2;
  if (LineLineIntersect (&line, Line))
    return (True);

  /* upper-right to lower-right corner */
  line.Point1.X = X2;
  line.Point1.Y = Y1;
  line.Point2.Y = Y2;
  if (LineLineIntersect (&line, Line))
    return (True);

  /* lower-right to lower-left corner */
  line.Point1.Y = Y2;
  line.Point1.X = X1;
  line.Point2.X = X2;
  if (LineLineIntersect (&line, Line))
    return (True);

  /* lower-left to upper-left corner */
  line.Point2.X = X1;
  line.Point1.Y = Y1;
  line.Point2.Y = Y2;
  if (LineLineIntersect (&line, Line))
    return (True);

  return (False);
}

/* ---------------------------------------------------------------------------
 * checks if an arc crosses a square
 */
Boolean
IsArcInRectangle (Location X1, Location Y1,
		  Location X2, Location Y2, ArcTypePtr Arc)
{
  LineType line;

  /* construct a set of dummy lines and check each of them */
  line.Thickness = 0;
  line.Flags = NOFLAG;

  /* upper-left to upper-right corner */
  line.Point1.Y = line.Point2.Y = Y1;
  line.Point1.X = X1;
  line.Point2.X = X2;
  if (LineArcIntersect (&line, Arc))
    return (True);

  /* upper-right to lower-right corner */
  line.Point1.X = line.Point2.X = X2;
  line.Point1.Y = Y1;
  line.Point2.Y = Y2;
  if (LineArcIntersect (&line, Arc))
    return (True);

  /* lower-right to lower-left corner */
  line.Point1.Y = line.Point2.Y = Y2;
  line.Point1.X = X1;
  line.Point2.X = X2;
  if (LineArcIntersect (&line, Arc))
    return (True);

  /* lower-left to upper-left corner */
  line.Point1.X = line.Point2.X = X1;
  line.Point1.Y = Y1;
  line.Point2.Y = Y2;
  if (LineArcIntersect (&line, Arc))
    return (True);

  return (False);
}

/* Added BY NP 7/23/2005 */
/* Calculates boudning points for diagonal pads and returns in "point".
 * point is array of four points.
 */
void
CalculateDiagonalPadBoundingBox(PadTypePtr Pad, PointTypePtr point)
{
  double angle1, angle2, offsetX, offsetY, XMIN, XMAX, YMIN, YMAX;
  int width = (Pad->Thickness + 1)/2;

  angle1  = atan2(Pad->Point2.Y -Pad->Point1.Y, Pad->Point2.X -Pad->Point1.X);
  angle2 = angle1 - 1.5708;

  XMIN = Pad->Point1.X - width * cos(angle1);
  YMIN = Pad->Point1.Y - width * sin(angle1);
  XMAX = Pad->Point2.X + width * cos(angle1);
  YMAX = Pad->Point2.Y + width * sin(angle1);
  offsetX = width * cos(angle2);
  offsetY = width * sin(angle2);

  point[0].X = (XMAX + offsetX);
  point[0].Y = (YMAX + offsetY);
  point[1].X = (XMAX - offsetX);
  point[1].Y = (YMAX - offsetY);
  point[2].X = (XMIN - offsetX);
  point[2].Y = (YMIN - offsetY);
  point[3].X = (XMIN + offsetX);
  point[3].Y = (YMIN + offsetY);
}

/* ---------------------------------------------------------------------------
 * check if a point lies inside a square Pad
 * fixed 10/30/98 - radius can't expand both edges of a square box
 */
/* Modified BY NP 7/23/2005 */
/* Added support for  diaonal pads */
Boolean
IsPointInSquarePad (Location X, Location Y, Cardinal Radius, PadTypePtr Pad)
{
  BoxType padbox;
  register BDimension t2 = Pad->Thickness / 2;

  /* Handle dialognal pads separately */
  if ((Pad->Point1.X == Pad->Point2.X) || (Pad->Point1.Y == Pad->Point2.Y))
  {
     /* Square-end pads */
     padbox.X1 = MIN (Pad->Point1.X, Pad->Point2.X) - t2;
     padbox.X2 = MAX (Pad->Point1.X, Pad->Point2.X) + t2;
     padbox.Y1 = MIN (Pad->Point1.Y, Pad->Point2.Y) - t2;
     padbox.Y2 = MAX (Pad->Point1.Y, Pad->Point2.Y) + t2;
     return (IsPointInBox (X, Y, &padbox, Radius));
  } else {
    /*Added BY NP 7/23/2005 */
    /* Diagonal pads case  Handle pad as polygon */
    Boolean isInside = False;
    PolygonTypePtr polygon;
    polygon = CreateNewPolygonFromPad (CURRENT, (Pad)->Point1.X,
			       (Pad)->Point1.Y, (Pad)->Point2.X, (Pad)->Point2.Y,
			       (int)t2, CLEARPOLYFLAG);
    isInside = IsPointInPolygon (X, Y, 0, (polygon));
    DestroyObject (PCB->Data, POLYGON_TYPE, CURRENT,
                                                           polygon, polygon);
    return isInside;
  }
}

/* Added BY NP 7/22/2005 */
/* ---------------------------------------------------------------------------
 * check if a line lies inside a diagonal Pad
 * This method handles diagonal pad as four point polygon
 * and checks if any line(which makes polygon boundry) intersect
 * with Line or not.
 */
Boolean
IsLineInDiagonalPad (PadTypePtr Pad, LineTypePtr Line)
{
  /* Diagonal pads. */
  LineType line;
  PointType point[4];

  CalculateDiagonalPadBoundingBox((Pad), point);
  /* construct a set of dummy lines and check each of them */
  line.Thickness = 0;
  line.Flags = NOFLAG;

  /* Check intersection with "Line" using 4 lines created by four points of
  *  diagonal pads
  */
  line.Point1 = point[0];
  line.Point2 = point[1];
  if (LineLineIntersect (&line, Line))
    return (True);

  line.Point1 = point[2];
  if (LineLineIntersect (&line, Line))
    return (True);

  line.Point2 = point[3];
  if (LineLineIntersect (&line, Line))
    return (True);

  line.Point1 = point[0];
  if (LineLineIntersect (&line, Line))
    return (True);

  return (False);
}

/* Added BY NP 7/23/2005 */
/* ---------------------------------------------------------------------------
 * check if a arc lies inside a diagonal Pad
 * This method handles diagonal pad as four point polygon
 * and checks if any line(which makes polygon boundry) intersect
 * with Arc or not.
 */
Boolean
IsArcInDiagonalPad (PadTypePtr Pad, ArcTypePtr Arc)
{
  /* Diagonal pads. */
  LineType line;
  PointType point[4];

  CalculateDiagonalPadBoundingBox((Pad), point);
  /* construct a set of dummy lines and check each of them */
  line.Thickness = 0;
  line.Flags = NOFLAG;

  /* Check intersection with "Line" using 4 lines created by four points of
  *  diagonal pads
  */
  line.Point1 = point[0];
  line.Point2 = point[1];
  if (LineArcIntersect (&line, Arc))
    return (True);

  line.Point1 = point[2];
  if (LineArcIntersect (&line, Arc))
    return (True);

  line.Point1 = point[3];
  if (LineArcIntersect (&line, Arc))
    return (True);

  line.Point1 = point[0];
  if (LineArcIntersect (&line, Arc))
    return (True);

  return (False);
}

Boolean
IsPointInBox (Location X, Location Y, BoxTypePtr box, Cardinal Radius)
{
  LineType line;

  if (POINT_IN_BOX (X, Y, box))
    return (True);
  line.Thickness = 0;
  line.Flags = NOFLAG;

  line.Point1.X = box->X1;
  line.Point1.Y = box->Y1;
  line.Point2.X = box->X2;
  line.Point2.Y = box->Y1;
  if (IsPointOnLine (X, Y, Radius, &line))
    return (True);
  line.Point2.X = box->X1;
  line.Point2.Y = box->Y2;
  if (IsPointOnLine (X, Y, Radius, &line))
    return (True);
  line.Point1.X = box->X2;
  line.Point1.Y = box->Y2;
  if (IsPointOnLine (X, Y, Radius, &line))
    return (True);
  line.Point2.X = box->X2;
  line.Point2.Y = box->Y1;
  return (IsPointOnLine (X, Y, Radius, &line));
}

/* ---------------------------------------------------------------------------
 * checks if a point lies inside a polygon
 * the code assumes that the last point isn't equal to the first one
 * The algorithm fails if the point is equal to a corner
 * from news FAQ:
 *   This code is from Wm. Randolph Franklin, wrf@ecse.rpi.edu, a
 *   professor at RPI.
 *
 * extentions:
 * check if the distance between the polygon lines and the point is less
 * or equal to the radius.
 */
Boolean
IsPointInPolygon (float X, float Y, float Radius, PolygonTypePtr Polygon)
{
  Boolean inside = False;
  BoxType boundingbox = Polygon->BoundingBox;

  /* increment the size of the bounding box by the radius */
  boundingbox.X1 -= (Location) Radius;
  boundingbox.Y1 -= (Location) Radius;
  boundingbox.X2 += (Location) Radius;
  boundingbox.Y2 += (Location) Radius;

  /* quick check if the point may lay inside */
  if (POINT_IN_BOX (X, Y, &boundingbox))
    {
      PointTypePtr ptr = &Polygon->Points[0];

      /* POLYGONPOINT_LOOP decrements pointers !!! */
      POLYGONPOINT_LOOP (Polygon);
      {
	if ((((ptr->Y <= Y) && (Y < point->Y)) ||
	     ((point->Y <= Y) && (Y < ptr->Y))) &&
	    (X <
	     ((float) (point->X - ptr->X) *
	      (float) (Y - ptr->Y) / (float) (point->Y - ptr->Y) + ptr->X)))
	  inside = !inside;
	ptr = point;
      }
      END_LOOP;
      /* check the distance between the lines of the
       * polygon and the point itself
       *
       * check is done by contructing a dummy line
       */
      if (!inside)
	{
	  LineType line;

	  line.Point1 = Polygon->Points[0];
	  line.Thickness = 0;
	  line.Flags = NOFLAG;

	  /* POLYGONPOINT_LOOP decrements pointers !!! */
	  POLYGONPOINT_LOOP (Polygon);
	  {
	    line.Point2 = *point;
	    if (IsPointOnLine (X, Y, Radius, &line))
	      return (True);
	    line.Point1 = *point;
	  }
	  END_LOOP;

	}
    }
  return (inside);
}

/* ---------------------------------------------------------------------------
 * checks if a polygon intersects with a square
 */
Boolean
IsRectangleInPolygon (Location X1, Location Y1,
		      Location X2, Location Y2, PolygonTypePtr Polygon)
{
  PolygonType polygon;
  PointType points[4];

  /* construct a dummy polygon and check it */
  polygon.BoundingBox.X1 = points[0].X = points[3].X = X1;
  polygon.BoundingBox.X2 = points[1].X = points[2].X = X2;
  polygon.BoundingBox.Y1 = points[0].Y = points[1].Y = Y1;
  polygon.BoundingBox.Y2 = points[2].Y = points[3].Y = Y2;
  polygon.Points = points;
  polygon.PointN = 4;
  return (IsPolygonInPolygon (&polygon, Polygon));
}

Boolean
IsPointOnArc (float X, float Y, float Radius, ArcTypePtr Arc)
{

  register float x, y, dx, dy, r1, r2, a, d, l;
  register float pdx, pdy;
  int ang1, ang2, delta;
  int startAngle, arcDelta;

  pdx = X - Arc->X;
  pdy = Y - Arc->Y;
  l = pdx * pdx + pdy * pdy;
  /* concentric arcs, simpler intersection conditions */
  if (l == 0.0)
    {
      if (Arc->Width <= Radius + 0.5 * Arc->Thickness)
	return (True);
      else
	return (False);
    }
  r1 = Arc->Width;
  r1 *= r1;
  r2 = Radius + 0.5 * Arc->Thickness;
  r2 *= r2;
  a = 0.5 * (r1 - r2 + l) / l;
  r1 = r1 / l;
  /* add a tiny positive number for round-off problems */
  d = r1 - a * a + 1e-5;
  /* the circles are too far apart to touch */
  if (d < 0)
    return (False);
  /* project the points of intersection */
  d = sqrt (d);
  x = a * pdx;
  y = a * pdy;
  dy = -d * pdx;
  dx = d * pdy;
  /* arrgh! calculate the angles, and put them in a standard range */
  startAngle = Arc->StartAngle;
  arcDelta = Arc->Delta;
  while (startAngle < 0)
    startAngle += 360;
  if (arcDelta < 0)
    {
      startAngle += arcDelta;
      arcDelta = -arcDelta;
      while (startAngle < 0)
	startAngle += 360;
    }
  ang1 = RAD_TO_DEG * atan2 ((y + dy), -(x + dx));
  if (ang1 < 0)
    ang1 += 360;
  ang2 = RAD_TO_DEG * atan2 ((y - dy), -(x - dx));
  if (ang2 < 0)
    ang2 += 360;
  delta = ang2 - ang1;
  if (delta > 180)
    delta -= 360;
  else if (delta < -180)
    delta += 360;
  if (delta < 0)
    {
      ang1 += delta;
      delta = -delta;
      while (ang1 < 0)
	ang1 += 360;
    }
  if (ang1 >= startAngle && ang1 <= startAngle + arcDelta)
    return (True);
  if (startAngle >= ang1 && startAngle <= ang1 + delta)
    return (True);
  if (startAngle + arcDelta >= 360 && ang1 <= startAngle + arcDelta - 360)
    return (True);
  if (ang1 + delta >= 360 && startAngle <= ang1 + delta - 360)
    return (True);
  return (False);
}

/* ---------------------------------------------------------------------------
 * searches for any kind of object or for a set of object types
 * the calling routine passes two pointers to allocated memory for storing
 * the results. 
 * A type value is returned too which is NO_TYPE if no objects has been found.
 * A set of object types is passed in.
 * The object is located by it's position.
 *
 * The layout is checked in the following order:
 *   polygon-point, pin, via, line, text, elementname, polygon, element
 */
int
SearchObjectByLocation (int Type,
			void **Result1, void **Result2, void **Result3,
			Location X, Location Y, BDimension Radius)
{
  void *r1, *r2, *r3;
  int i;
  float HigherBound = 0;
  int HigherAvail = NO_TYPE;
  /* setup variables used by local functions */
  PosX = X;
  PosY = Y;
  SearchRadius = Radius;
  SearchBox.X1 = X - Radius;
  SearchBox.Y1 = Y - Radius;
  SearchBox.X2 = X + Radius;
  SearchBox.Y2 = Y + Radius;
  if (ScreenOnly)
    {
      MAKEMAX (SearchBox.X1, vxl);
      MAKEMAX (SearchBox.Y1, vyl);
      MAKEMIN (SearchBox.X2, vxh);
      MAKEMIN (SearchBox.Y2, vyh);
      if (SearchBox.X1 > SearchBox.X2 || SearchBox.Y1 > SearchBox.Y2)
	return NO_TYPE;
    }

  if (Type & RATLINE_TYPE && PCB->RatOn &&
      SearchRatLineByLocation ((RatTypePtr *) Result1,
			       (RatTypePtr *) Result2,
			       (RatTypePtr *) Result3))
    return (RATLINE_TYPE);

  if (Type & VIA_TYPE &&
      SearchViaByLocation ((PinTypePtr *) Result1,
			   (PinTypePtr *) Result2, (PinTypePtr *) Result3))
    return (VIA_TYPE);

  if (Type & PIN_TYPE &&
      SearchPinByLocation ((ElementTypePtr *) & r1,
			   (PinTypePtr *) & r2, (PinTypePtr *) & r3))
    HigherAvail = PIN_TYPE;

  if (!HigherAvail && Type & PAD_TYPE &&
      SearchPadByLocation ((ElementTypePtr *) & r1,
			   (PadTypePtr *) & r2, (PadTypePtr *) & r3, False))
    HigherAvail = PAD_TYPE;

  if (!HigherAvail && Type & ELEMENTNAME_TYPE &&
      SearchElementNameByLocation ((ElementTypePtr *) & r1,
				   (TextTypePtr *) & r2, (TextTypePtr *) & r3,
				   False))
    {
      BoxTypePtr box = &((TextTypePtr) r2)->BoundingBox;
      HigherBound = (float) (box->X2 - box->X1) * (float) (box->Y2 - box->Y1);
      HigherAvail = ELEMENTNAME_TYPE;
    }

  if (!HigherAvail && Type & ELEMENT_TYPE &&
      SearchElementByLocation ((ElementTypePtr *) & r1,
			       (ElementTypePtr *) & r2,
			       (ElementTypePtr *) & r3, False))
    {
      BoxTypePtr box = &((ElementTypePtr) r1)->BoundingBox;
      HigherBound = (float) (box->X2 - box->X1) * (float) (box->Y2 - box->Y1);
      HigherAvail = ELEMENT_TYPE;
    }

  for (i = -1; i < MAX_LAYER + 1; i++)
    {
      if (i < 0)
	SearchLayer = &PCB->Data->SILKLAYER;
      else if (i < MAX_LAYER)
	SearchLayer = LAYER_ON_STACK (i);
      else
	SearchLayer = &PCB->Data->BACKSILKLAYER;
      if (SearchLayer->On)
	{
	  if ((HigherAvail & (PIN_TYPE | PAD_TYPE)) == 0 &&
	      Type & POLYGONPOINT_TYPE &&
	      SearchPointByLocation ((LayerTypePtr *) Result1,
				     (PolygonTypePtr *) Result2,
				     (PointTypePtr *) Result3))
	    return (POLYGONPOINT_TYPE);

	  if ((HigherAvail & (PIN_TYPE | PAD_TYPE)) == 0 &&
	      Type & LINEPOINT_TYPE &&
	      SearchLinePointByLocation ((LayerTypePtr *) Result1,
					 (LineTypePtr *) Result2,
					 (PointTypePtr *) Result3))
	    return (LINEPOINT_TYPE);

	  if ((HigherAvail & (PIN_TYPE | PAD_TYPE)) == 0 && Type & LINE_TYPE
	      && SearchLineByLocation ((LayerTypePtr *) Result1,
				       (LineTypePtr *) Result2,
				       (LineTypePtr *) Result3))
	    return (LINE_TYPE);

	  if ((HigherAvail & (PIN_TYPE | PAD_TYPE)) == 0 && Type & ARC_TYPE &&
	      SearchArcByLocation ((LayerTypePtr *) Result1,
				   (ArcTypePtr *) Result2,
				   (ArcTypePtr *) Result3))
	    return (ARC_TYPE);

	  if ((HigherAvail & (PIN_TYPE | PAD_TYPE)) == 0 && Type & TEXT_TYPE
	      && SearchTextByLocation ((LayerTypePtr *) Result1,
				       (TextTypePtr *) Result2,
				       (TextTypePtr *) Result3))
	    return (TEXT_TYPE);

	  if (Type & POLYGON_TYPE &&
	      SearchPolygonByLocation ((LayerTypePtr *) Result1,
				       (PolygonTypePtr *) Result2,
				       (PolygonTypePtr *) Result3))
	    {
	      if (HigherAvail)
		{
		  BoxTypePtr box =
		    &(*(PolygonTypePtr *) Result2)->BoundingBox;
		  float area =
		    (float) (box->X2 - box->X1) * (float) (box->X2 - box->X1);
		  if (HigherBound < area)
		    break;
		  else
		    return (POLYGON_TYPE);
		}
	      else
		return (POLYGON_TYPE);
	    }
	}
    }
  /* return any previously found objects */
  if (HigherAvail & PIN_TYPE)
    {
      *Result1 = r1;
      *Result2 = r2;
      *Result3 = r3;
      return (PIN_TYPE);
    }

  if (HigherAvail & PAD_TYPE)
    {
      *Result1 = r1;
      *Result2 = r2;
      *Result3 = r3;
      return (PAD_TYPE);
    }

  if (HigherAvail & ELEMENTNAME_TYPE)
    {
      *Result1 = r1;
      *Result2 = r2;
      *Result3 = r3;
      return (ELEMENTNAME_TYPE);
    }

  if (HigherAvail & ELEMENT_TYPE)
    {
      *Result1 = r1;
      *Result2 = r2;
      *Result3 = r3;
      return (ELEMENT_TYPE);
    }

  /* search the 'invisible objects' last */
  if (!PCB->InvisibleObjectsOn)
    return (NO_TYPE);

  if (Type & PAD_TYPE &&
      SearchPadByLocation ((ElementTypePtr *) Result1,
			   (PadTypePtr *) Result2, (PadTypePtr *) Result3,
			   True))
    return (PAD_TYPE);

  if (Type & ELEMENTNAME_TYPE &&
      SearchElementNameByLocation ((ElementTypePtr *) Result1,
				   (TextTypePtr *) Result2,
				   (TextTypePtr *) Result3, True))
    return (ELEMENTNAME_TYPE);

  if (Type & ELEMENT_TYPE &&
      SearchElementByLocation ((ElementTypePtr *) Result1,
			       (ElementTypePtr *) Result2,
			       (ElementTypePtr *) Result3, True))
    return (ELEMENT_TYPE);

  return (NO_TYPE);
}

/* ---------------------------------------------------------------------------
 * searches for a object by it's unique ID. It doesn't matter if
 * the object is visible or not. The search is performed on a PCB, a
 * buffer or on the remove list.
 * The calling routine passes two pointers to allocated memory for storing
 * the results. 
 * A type value is returned too which is NO_TYPE if no objects has been found.
 */
int
SearchObjectByID (DataTypePtr Base,
		  void **Result1, void **Result2, void **Result3, int ID,
		  int type)
{
  if (type == LINE_TYPE || type == LINEPOINT_TYPE)
    {
      ALLLINE_LOOP (Base);
      {
	if (line->ID == ID)
	  {
	    *Result1 = (void *) layer;
	    *Result2 = *Result3 = (void *) line;
	    return (LINE_TYPE);
	  }
	if (line->Point1.ID == ID)
	  {
	    *Result1 = (void *) layer;
	    *Result2 = (void *) line;
	    *Result3 = (void *) &line->Point1;
	    return (LINEPOINT_TYPE);
	  }
	if (line->Point2.ID == ID)
	  {
	    *Result1 = (void *) layer;
	    *Result2 = (void *) line;
	    *Result3 = (void *) &line->Point2;
	    return (LINEPOINT_TYPE);
	  }
      }
      ENDALL_LOOP;
    }
  if (type == ARC_TYPE)
    {
      ALLARC_LOOP (Base);
      {
	if (arc->ID == ID)
	  {
	    *Result1 = (void *) layer;
	    *Result2 = *Result3 = (void *) arc;
	    return (ARC_TYPE);
	  }
      }
      ENDALL_LOOP;
    }

  if (type == TEXT_TYPE)
    {
      ALLTEXT_LOOP (Base);
      {
	if (text->ID == ID)
	  {
	    *Result1 = (void *) layer;
	    *Result2 = *Result3 = (void *) text;
	    return (TEXT_TYPE);
	  }
      }
      ENDALL_LOOP;
    }

  if (type == POLYGON_TYPE || type == POLYGONPOINT_TYPE)
    {
      ALLPOLYGON_LOOP (Base);
      {
	if (polygon->ID == ID)
	  {
	    *Result1 = (void *) layer;
	    *Result2 = *Result3 = (void *) polygon;
	    return (POLYGON_TYPE);
	  }
	if (type == POLYGONPOINT_TYPE)
	  POLYGONPOINT_LOOP (polygon);
	{
	  if (point->ID == ID)
	    {
	      *Result1 = (void *) layer;
	      *Result2 = (void *) polygon;
	      *Result3 = (void *) point;
	      return (POLYGONPOINT_TYPE);
	    }
	}
	END_LOOP;
      }
      ENDALL_LOOP;
    }
  if (type == VIA_TYPE)
    {
      VIA_LOOP (Base);
      {
	if (via->ID == ID)
	  {
	    *Result1 = *Result2 = *Result3 = (void *) via;
	    return (VIA_TYPE);
	  }
      }
      END_LOOP;
    }

  if (type == RATLINE_TYPE || type == LINEPOINT_TYPE)
    {
      RAT_LOOP (Base);
      {
	if (line->ID == ID)
	  {
	    *Result1 = *Result2 = *Result3 = (void *) line;
	    return (RATLINE_TYPE);
	  }
	if (line->Point1.ID == ID)
	  {
	    *Result1 = (void *) NULL;
	    *Result2 = (void *) line;
	    *Result3 = (void *) &line->Point1;
	    return (LINEPOINT_TYPE);
	  }
	if (line->Point2.ID == ID)
	  {
	    *Result1 = (void *) NULL;
	    *Result2 = (void *) line;
	    *Result3 = (void *) &line->Point2;
	    return (LINEPOINT_TYPE);
	  }
      }
      END_LOOP;
    }

  if (type == ELEMENT_TYPE || type == PAD_TYPE || type == PIN_TYPE
      || type == ELEMENTLINE_TYPE || type == ELEMENTNAME_TYPE
      || type == ELEMENTARC_TYPE)
    /* check pins and elementnames too */
    ELEMENT_LOOP (Base);
  {
    if (element->ID == ID)
      {
	*Result1 = *Result2 = *Result3 = (void *) element;
	return (ELEMENT_TYPE);
      }
    if (type == ELEMENTLINE_TYPE)
      ELEMENTLINE_LOOP (element);
    {
      if (line->ID == ID)
	{
	  *Result1 = (void *) element;
	  *Result2 = *Result3 = (void *) line;
	  return (ELEMENTLINE_TYPE);
	}
    }
    END_LOOP;
    if (type == ELEMENTARC_TYPE)
      ARC_LOOP (element);
    {
      if (arc->ID == ID)
	{
	  *Result1 = (void *) element;
	  *Result2 = *Result3 = (void *) arc;
	  return (ELEMENTARC_TYPE);
	}
    }
    END_LOOP;
    if (type == ELEMENTNAME_TYPE)
      ELEMENTTEXT_LOOP (element);
    {
      if (text->ID == ID)
	{
	  *Result1 = (void *) element;
	  *Result2 = *Result3 = (void *) text;
	  return (ELEMENTNAME_TYPE);
	}
    }
    END_LOOP;
    if (type == PIN_TYPE)
      PIN_LOOP (element);
    {
      if (pin->ID == ID)
	{
	  *Result1 = (void *) element;
	  *Result2 = *Result3 = (void *) pin;
	  return (PIN_TYPE);
	}
    }
    END_LOOP;
    if (type == PAD_TYPE)
      PAD_LOOP (element);
    {
      if (pad->ID == ID)
	{
	  *Result1 = (void *) element;
	  *Result2 = *Result3 = (void *) pad;
	  return (PAD_TYPE);
	}
    }
    END_LOOP;
  }
  END_LOOP;

  Message ("hace: Internal error, search for ID %d failed\n", ID);
  return (NO_TYPE);
}

/* ---------------------------------------------------------------------------
 * searches for an element by its board name.
 * The function returns a pointer to the element, NULL if not found
 */
ElementTypePtr
SearchElementByName (DataTypePtr Base, char *Name)
{
  ElementTypePtr result = NULL;

  ELEMENT_LOOP (Base);
  {
    if (element->Name[1].TextString &&
	strcmp (element->Name[1].TextString, Name) == 0)
      {
	result = element;
	return (result);
      }
  }
  END_LOOP;
  return result;
}

/* ---------------------------------------------------------------------------
 * searches the cursor position for the type 
 */
int
SearchScreen (Location X, Location Y, int Type, void **Result1,
	      void **Result2, void **Result3)
{
  int ans;

  ScreenOnly = True;
  ans = SearchObjectByLocation (Type, Result1, Result2, Result3,
				X, Y, TO_PCB (SLOP));
  ScreenOnly = False;
  return (ans);
}
