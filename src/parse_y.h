/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUMBER = 258,
     CHAR_CONST = 259,
     FLOAT = 260,
     STRING = 261,
     T_PCB = 262,
     T_LAYER = 263,
     T_VIA = 264,
     T_RAT = 265,
     T_LINE = 266,
     T_ARC = 267,
     T_RECTANGLE = 268,
     T_TEXT = 269,
     T_ELEMENTLINE = 270,
     T_ELEMENT = 271,
     T_PIN = 272,
     T_PAD = 273,
     T_GRID = 274,
     T_FLAGS = 275,
     T_SYMBOL = 276,
     T_SYMBOLLINE = 277,
     T_CURSOR = 278,
     T_ELEMENTARC = 279,
     T_MARK = 280,
     T_GROUPS = 281,
     T_STYLES = 282,
     T_POLYGON = 283,
     T_NETLIST = 284,
     T_NET = 285,
     T_CONN = 286,
     T_THERMAL = 287,
     T_DRC = 288
   };
#endif
#define NUMBER 258
#define CHAR_CONST 259
#define FLOAT 260
#define STRING 261
#define T_PCB 262
#define T_LAYER 263
#define T_VIA 264
#define T_RAT 265
#define T_LINE 266
#define T_ARC 267
#define T_RECTANGLE 268
#define T_TEXT 269
#define T_ELEMENTLINE 270
#define T_ELEMENT 271
#define T_PIN 272
#define T_PAD 273
#define T_GRID 274
#define T_FLAGS 275
#define T_SYMBOL 276
#define T_SYMBOLLINE 277
#define T_CURSOR 278
#define T_ELEMENTARC 279
#define T_MARK 280
#define T_GROUPS 281
#define T_STYLES 282
#define T_POLYGON 283
#define T_NETLIST 284
#define T_NET 285
#define T_CONN 286
#define T_THERMAL 287
#define T_DRC 288




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 73 "parse_y.y"
typedef union YYSTYPE {
	int		number;
	float		floating;
	char		*string;
} YYSTYPE;
/* Line 1275 of yacc.c.  */
#line 109 "parse_y.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



