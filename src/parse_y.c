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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 3 "parse_y.y"

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

/* grammar to parse ASCII input of PCB description
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define GRIDFIT(x,g) (int)(0.5 + (int)(((x)+(g)/2.)/(g))*(g))
#include "global.h"
#include "create.h"
#include "data.h"
#include "error.h"
#include "mymem.h"
#include "misc.h"
#include "parse_l.h"
#include "remove.h"

#ifdef HAVE_LIBDMALLOC
# include <dmalloc.h> /* see http://dmalloc.com */
#endif

RCSID("$Id: parse_y.c,v 1.3 2006/11/05 17:55:34 tim Exp $");

static	LayerTypePtr	Layer;
static	PolygonTypePtr	Polygon;
static	SymbolTypePtr	Symbol;
static	int		pin_num;
static	LibraryMenuTypePtr	Menu;
static	Boolean			LayerFlag[MAX_LAYER + 2];

extern	char			*yytext;		/* defined by LEX */
extern	PCBTypePtr		yyPCB;
extern	DataTypePtr		yyData;
extern	ElementTypePtr		yyElement;
extern	FontTypePtr		yyFont;
extern	int			yylineno;		/* linenumber */
extern	char			*yyfilename;		/* in this file */

int yyerror(char *s);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 73 "parse_y.y"
typedef union YYSTYPE {
	int		number;
	float		floating;
	char		*string;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 217 "parse_y.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 229 "parse_y.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   560

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  38
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  94
/* YYNRULES -- Number of rules. */
#define YYNRULES  177
/* YYNRULES -- Number of states. */
#define YYNSTATES  576

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   288

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      34,    35,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    36,     2,    37,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    12,    25,    26,
      29,    31,    32,    33,    36,    41,    48,    55,    57,    59,
      61,    63,    70,    78,    86,    94,   101,   108,   115,   116,
     117,   122,   124,   126,   128,   129,   136,   137,   145,   146,
     156,   161,   162,   167,   168,   173,   178,   179,   180,   182,
     183,   185,   188,   190,   192,   194,   195,   198,   200,   202,
     204,   206,   208,   210,   222,   234,   245,   255,   264,   275,
     286,   287,   297,   299,   300,   302,   305,   307,   309,   311,
     313,   315,   317,   326,   328,   330,   332,   333,   342,   353,
     364,   374,   387,   400,   412,   421,   431,   441,   443,   446,
     451,   456,   457,   459,   461,   463,   465,   467,   468,   481,
     482,   498,   499,   516,   517,   536,   537,   556,   558,   561,
     563,   565,   567,   569,   571,   580,   589,   600,   611,   617,
     623,   625,   628,   630,   632,   634,   636,   645,   654,   665,
     676,   689,   702,   713,   723,   732,   746,   760,   772,   783,
     785,   788,   789,   799,   800,   810,   812,   814,   816,   819,
     821,   823,   824,   833,   842,   844,   845,   852,   854,   855,
     857,   860,   861,   871,   873,   874,   876,   879
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      39,     0,    -1,    40,    -1,    42,    -1,    45,    -1,     1,
      -1,    -1,    41,    47,    48,    53,    54,    55,    59,    60,
      61,    44,    62,   123,    -1,    -1,    43,    62,    -1,    45,
      -1,    -1,    -1,    46,   114,    -1,     7,    34,     6,    35,
      -1,     7,    34,     6,     3,     3,    35,    -1,     7,    36,
       6,     3,     3,    37,    -1,    49,    -1,    50,    -1,    51,
      -1,    52,    -1,    19,    34,     3,     3,     3,    35,    -1,
      19,    34,     3,     3,     3,     3,    35,    -1,    19,    34,
       5,     3,     3,     3,    35,    -1,    19,    36,     5,     3,
       3,     3,    37,    -1,    23,    34,     3,     3,     3,    35,
      -1,    23,    36,     3,     3,     3,    37,    -1,    23,    36,
       3,     3,     5,    37,    -1,    -1,    -1,    32,    36,     5,
      37,    -1,    56,    -1,    57,    -1,    58,    -1,    -1,    33,
      36,     3,     3,     3,    37,    -1,    -1,    33,    36,     3,
       3,     3,     3,    37,    -1,    -1,    33,    36,     3,     3,
       3,     3,     3,     3,    37,    -1,    20,    34,     3,    35,
      -1,    -1,    26,    34,     6,    35,    -1,    -1,    27,    34,
       6,    35,    -1,    27,    36,     6,    37,    -1,    -1,    -1,
      63,    -1,    -1,    64,    -1,    63,    64,    -1,    66,    -1,
      72,    -1,    73,    -1,    -1,    65,    90,    -1,     1,    -1,
      67,    -1,    68,    -1,    69,    -1,    70,    -1,    71,    -1,
       9,    36,     3,     3,     3,     3,     3,     3,     6,     3,
      37,    -1,     9,    34,     3,     3,     3,     3,     3,     3,
       6,     3,    35,    -1,     9,    34,     3,     3,     3,     3,
       3,     6,     3,    35,    -1,     9,    34,     3,     3,     3,
       3,     6,     3,    35,    -1,     9,    34,     3,     3,     3,
       6,     3,    35,    -1,    10,    36,     3,     3,     3,     3,
       3,     3,     3,    37,    -1,    10,    34,     3,     3,     3,
       3,     3,     3,     3,    35,    -1,    -1,     8,    34,     3,
       6,    35,    34,    74,    75,    35,    -1,    76,    -1,    -1,
      77,    -1,    76,    77,    -1,    79,    -1,    80,    -1,    81,
      -1,    82,    -1,    83,    -1,    84,    -1,    13,    34,     3,
       3,     3,     3,     3,    35,    -1,    87,    -1,    86,    -1,
      85,    -1,    -1,    28,    34,     3,    35,    34,    78,    88,
      35,    -1,    11,    36,     3,     3,     3,     3,     3,     3,
       3,    37,    -1,    11,    34,     3,     3,     3,     3,     3,
       3,     3,    35,    -1,    11,    34,     3,     3,     3,     3,
       3,     3,    35,    -1,    12,    36,     3,     3,     3,     3,
       3,     3,     3,     3,     3,    37,    -1,    12,    34,     3,
       3,     3,     3,     3,     3,     3,     3,     3,    35,    -1,
      12,    34,     3,     3,     3,     3,     3,     3,     3,     3,
      35,    -1,    14,    34,     3,     3,     3,     6,     3,    35,
      -1,    14,    34,     3,     3,     3,     3,     6,     3,    35,
      -1,    14,    36,     3,     3,     3,     3,     6,     3,    37,
      -1,    89,    -1,    88,    89,    -1,    34,     3,     3,    35,
      -1,    36,     3,     3,    37,    -1,    -1,    91,    -1,    93,
      -1,    95,    -1,    97,    -1,    99,    -1,    -1,    16,    34,
       6,     6,     3,     3,     3,    35,    34,    92,   101,    35,
      -1,    -1,    16,    34,     3,     6,     6,     3,     3,     3,
       3,     3,    35,    34,    94,   101,    35,    -1,    -1,    16,
      34,     3,     6,     6,     6,     3,     3,     3,     3,     3,
      35,    34,    96,   101,    35,    -1,    -1,    16,    34,     3,
       6,     6,     6,     3,     3,     3,     3,     3,     3,     3,
      35,    34,    98,   103,    35,    -1,    -1,    16,    36,     3,
       6,     6,     6,     3,     3,     3,     3,     3,     3,     3,
      37,    34,   100,   103,    35,    -1,   102,    -1,   101,   102,
      -1,   107,    -1,   108,    -1,   109,    -1,   112,    -1,   113,
      -1,    15,    36,     3,     3,     3,     3,     3,    37,    -1,
      15,    34,     3,     3,     3,     3,     3,    35,    -1,    24,
      36,     3,     3,     3,     3,     3,     3,     3,    37,    -1,
      24,    34,     3,     3,     3,     3,     3,     3,     3,    35,
      -1,    25,    36,     3,     3,    37,    -1,    25,    34,     3,
       3,    35,    -1,   104,    -1,   103,   104,    -1,   106,    -1,
     105,    -1,   111,    -1,   110,    -1,    15,    36,     3,     3,
       3,     3,     3,    37,    -1,    15,    34,     3,     3,     3,
       3,     3,    35,    -1,    24,    36,     3,     3,     3,     3,
       3,     3,     3,    37,    -1,    24,    34,     3,     3,     3,
       3,     3,     3,     3,    35,    -1,    17,    36,     3,     3,
       3,     3,     3,     3,     6,     6,     3,    37,    -1,    17,
      34,     3,     3,     3,     3,     3,     3,     6,     6,     3,
      35,    -1,    17,    34,     3,     3,     3,     3,     6,     6,
       3,    35,    -1,    17,    34,     3,     3,     3,     3,     6,
       3,    35,    -1,    17,    34,     3,     3,     3,     6,     3,
      35,    -1,    18,    36,     3,     3,     3,     3,     3,     3,
       3,     6,     6,     3,    37,    -1,    18,    34,     3,     3,
       3,     3,     3,     3,     3,     6,     6,     3,    35,    -1,
      18,    34,     3,     3,     3,     3,     3,     6,     6,     3,
      35,    -1,    18,    34,     3,     3,     3,     3,     3,     6,
       3,    35,    -1,   115,    -1,   114,   115,    -1,    -1,    21,
      36,   118,     3,    37,    34,   116,   119,    35,    -1,    -1,
      21,    34,   118,     3,    35,    34,   117,   119,    35,    -1,
       3,    -1,     4,    -1,   120,    -1,   119,   120,    -1,   121,
      -1,   122,    -1,    -1,    22,    34,     3,     3,     3,     3,
       3,    35,    -1,    22,    36,     3,     3,     3,     3,     3,
      37,    -1,   124,    -1,    -1,    29,    34,    35,    34,   125,
      35,    -1,   126,    -1,    -1,   127,    -1,   126,   127,    -1,
      -1,    30,    34,     6,     6,    35,    34,   128,   129,    35,
      -1,   130,    -1,    -1,   131,    -1,   130,   131,    -1,    31,
      34,     6,    35,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    93,    93,    94,    95,    96,   100,   100,   130,   130,
     148,   149,   154,   154,   175,   181,   187,   196,   197,   198,
     199,   202,   210,   223,   235,   248,   254,   260,   266,   269,
     271,   278,   279,   280,   283,   285,   293,   295,   304,   306,
     317,   321,   325,   333,   336,   344,   352,   353,   357,   358,
     362,   363,   367,   368,   369,   371,   371,   378,   382,   383,
     384,   385,   386,   391,   401,   412,   422,   432,   448,   453,
     463,   462,   484,   485,   489,   490,   494,   495,   496,   497,
     498,   499,   501,   506,   507,   508,   511,   510,   532,   540,
     548,   559,   567,   575,   584,   594,   610,   626,   627,   632,
     636,   640,   644,   645,   646,   647,   648,   656,   655,   674,
     673,   692,   691,   712,   710,   733,   731,   749,   750,   754,
     755,   756,   757,   758,   760,   765,   770,   774,   779,   784,
     792,   793,   797,   798,   799,   800,   802,   808,   815,   820,
     830,   842,   854,   866,   882,   902,   916,   929,   940,   953,
     954,   959,   958,   976,   975,   995,   996,  1000,  1001,  1005,
    1006,  1007,  1012,  1019,  1025,  1026,  1030,  1035,  1036,  1040,
    1041,  1047,  1046,  1056,  1057,  1061,  1062,  1066
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NUMBER", "CHAR_CONST", "FLOAT",
  "STRING", "T_PCB", "T_LAYER", "T_VIA", "T_RAT", "T_LINE", "T_ARC",
  "T_RECTANGLE", "T_TEXT", "T_ELEMENTLINE", "T_ELEMENT", "T_PIN", "T_PAD",
  "T_GRID", "T_FLAGS", "T_SYMBOL", "T_SYMBOLLINE", "T_CURSOR",
  "T_ELEMENTARC", "T_MARK", "T_GROUPS", "T_STYLES", "T_POLYGON",
  "T_NETLIST", "T_NET", "T_CONN", "T_THERMAL", "T_DRC", "'('", "')'",
  "'['", "']'", "$accept", "parse", "parsepcb", "@1", "parsedata", "@2",
  "pcbfont", "parsefont", "@3", "pcbname", "pcbgrid", "pcbgridold",
  "pcbgridnew", "pcb2grid", "pcbhigrid", "pcbcursor", "pcbthermal",
  "pcbdrc", "pcbdrc1", "pcbdrc2", "pcbdrc3", "pcbflags", "pcbgroups",
  "pcbstyles", "pcbdata", "pcbdefinitions", "pcbdefinition", "@4", "via",
  "via_hi_format", "via_2.0_format", "via_1.7_format", "via_newformat",
  "via_oldformat", "rats", "layer", "@5", "layerdata", "layerdefinitions",
  "layerdefinition", "@6", "line_hi_format", "line_1.7_format",
  "line_oldformat", "arc_hi_format", "arc_1.7_format", "arc_oldformat",
  "text_oldformat", "text_newformat", "text_hi_format", "polygonpoints",
  "polygonpoint", "element", "element_oldformat", "@7",
  "element_1.3.4_format", "@8", "element_newformat", "@9",
  "element_1.7_format", "@10", "element_hi_format", "@11",
  "elementdefinitions", "elementdefinition", "relementdefs", "relementdef",
  "pin_hi_format", "pin_1.7_format", "pin_1.6.3_format", "pin_newformat",
  "pin_oldformat", "pad_hi_format", "pad_1.7_format", "pad_newformat",
  "pad", "symbols", "symbol", "@12", "@13", "symbolid", "symboldata",
  "symboldefinitions", "symboldefinition", "hiressymbol", "pcbnetlist",
  "pcbnetdef", "nets", "netdefs", "net", "@14", "connections", "conndefs",
  "conn", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,    40,    41,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    38,    39,    39,    39,    39,    41,    40,    43,    42,
      44,    44,    46,    45,    47,    47,    47,    48,    48,    48,
      48,    49,    50,    51,    52,    53,    53,    53,    53,    54,
      54,    55,    55,    55,    56,    56,    57,    57,    58,    58,
      59,    59,    60,    60,    61,    61,    61,    61,    62,    62,
      63,    63,    64,    64,    64,    65,    64,    64,    66,    66,
      66,    66,    66,    67,    68,    69,    70,    71,    72,    72,
      74,    73,    75,    75,    76,    76,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    78,    77,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    88,    89,
      89,    89,    90,    90,    90,    90,    90,    92,    91,    94,
      93,    96,    95,    98,    97,   100,    99,   101,   101,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     103,   103,   104,   104,   104,   104,   104,   104,   104,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     114,   116,   115,   117,   115,   118,   118,   119,   119,   120,
     120,   120,   121,   122,   123,   123,   124,   125,   125,   126,
     126,   128,   127,   129,   129,   130,   130,   131
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     0,    12,     0,     2,
       1,     0,     0,     2,     4,     6,     6,     1,     1,     1,
       1,     6,     7,     7,     7,     6,     6,     6,     0,     0,
       4,     1,     1,     1,     0,     6,     0,     7,     0,     9,
       4,     0,     4,     0,     4,     4,     0,     0,     1,     0,
       1,     2,     1,     1,     1,     0,     2,     1,     1,     1,
       1,     1,     1,    11,    11,    10,     9,     8,    10,    10,
       0,     9,     1,     0,     1,     2,     1,     1,     1,     1,
       1,     1,     8,     1,     1,     1,     0,     8,    10,    10,
       9,    12,    12,    11,     8,     9,     9,     1,     2,     4,
       4,     0,     1,     1,     1,     1,     1,     0,    12,     0,
      15,     0,    16,     0,    18,     0,    18,     1,     2,     1,
       1,     1,     1,     1,     8,     8,    10,    10,     5,     5,
       1,     2,     1,     1,     1,     1,     8,     8,    10,    10,
      12,    12,    10,     9,     8,    13,    13,    11,    10,     1,
       2,     0,     9,     0,     9,     1,     1,     1,     2,     1,
       1,     0,     8,     8,     1,     0,     6,     1,     0,     1,
       2,     0,     9,     1,     0,     1,     2,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     5,     0,     2,     0,     3,     0,     4,     0,     1,
       0,     0,    57,     0,     0,     0,     9,     0,    50,     0,
      52,    58,    59,    60,    61,    62,    53,    54,     0,    13,
     149,     0,     0,     0,    28,    17,    18,    19,    20,     0,
       0,     0,     0,     0,    51,     0,    56,   102,   103,   104,
     105,   106,     0,     0,   150,     0,     0,     0,     0,     0,
      29,     0,     0,     0,     0,     0,     0,     0,   155,   156,
       0,     0,     0,    14,     0,     0,     0,     0,     0,     0,
       0,    34,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    41,    31,    32,    33,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    15,    16,     0,     0,     0,
       0,     0,     0,     0,     0,    43,    70,     0,     0,     0,
       0,     0,     0,     0,     0,   153,   151,     0,    21,     0,
       0,     0,     0,     0,    30,     0,     0,     0,    46,    73,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     161,   161,    22,    23,    24,    25,    26,    27,     0,     0,
       0,     0,    11,     0,     0,     0,     0,     0,     0,    72,
      74,    76,    77,    78,    79,    80,    81,    85,    84,    83,
       0,     0,     0,    67,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   157,   159,   160,     0,     0,    40,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,    71,    75,     0,     0,    66,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   154,   158,   152,     0,
      35,    42,     0,     0,   165,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    65,     0,    69,    68,     0,     0,
     107,     0,     0,     0,     0,    37,    44,    45,     0,     7,
     164,     0,     0,     0,     0,     0,     0,     0,     0,    64,
      63,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    86,     0,     0,     0,
       0,     0,     0,     0,     0,   117,   119,   120,   121,   122,
     123,     0,     0,     0,    39,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   101,   109,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   108,   118,     0,     0,
       0,   168,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    97,     0,     0,   111,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     167,   169,     0,     0,     0,     0,    82,     0,    94,     0,
       0,     0,    87,    98,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   162,   163,     0,   166,
     170,     0,    90,     0,     0,     0,    95,    96,     0,     0,
     110,   113,     0,     0,     0,     0,     0,     0,     0,   129,
     128,   115,     0,    89,    88,     0,     0,    99,   100,     0,
     112,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,     0,     0,     0,     0,     0,     0,   130,   133,
     132,   135,   134,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    92,    91,     0,     0,     0,     0,     0,     0,
       0,     0,   114,   131,   125,   124,     0,     0,   144,     0,
       0,     0,   116,   171,     0,     0,     0,     0,     0,     0,
       0,     0,   143,     0,     0,     0,     0,     0,   174,     0,
       0,     0,     0,     0,     0,     0,     0,   142,   148,     0,
     127,   126,     0,     0,   173,   175,     0,     0,     0,     0,
       0,     0,     0,     0,   147,     0,   172,   176,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   177,   137,   136,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   139,   138,     0,     0,     0,     0,
     141,   140,     0,     0,   146,   145
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     2,     3,     4,     5,     6,   212,     7,     8,    11,
      34,    35,    36,    37,    38,    60,    81,   101,   102,   103,
     104,   125,   148,   172,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,   149,   178,   179,   180,
     324,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     352,   353,    46,    47,   283,    48,   354,    49,   386,    50,
     429,    51,   438,   304,   305,   447,   448,   449,   450,   306,
     307,   308,   451,   452,   309,   310,    29,    30,   161,   160,
      70,   202,   203,   204,   205,   269,   270,   369,   370,   371,
     498,   513,   514,   515
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -427
static const short yypact[] =
{
      44,  -427,    20,  -427,    22,  -427,   103,  -427,   -17,  -427,
     100,    29,  -427,    24,   104,   105,  -427,    14,  -427,    62,
    -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,   108,   -17,
    -427,    56,    73,   109,    60,  -427,  -427,  -427,  -427,    85,
     102,   107,   117,   118,  -427,   113,  -427,  -427,  -427,  -427,
    -427,  -427,    69,    69,  -427,     2,   155,   143,   128,   116,
     151,   153,   174,   181,   182,   183,    25,   184,  -427,  -427,
     185,   186,   187,  -427,   188,   189,   190,   191,   192,   193,
     101,   164,   163,   196,   197,   198,   199,   200,   201,   202,
     168,   167,   170,   172,   207,   208,   209,   210,   211,   212,
     179,   203,  -427,  -427,  -427,   194,    71,   213,   215,   216,
     214,   218,   219,   195,   204,  -427,  -427,     3,   221,   223,
     224,   148,   205,   227,   206,   217,  -427,    95,   228,   229,
     230,   231,   112,   232,   233,  -427,  -427,   220,  -427,   222,
     225,   226,   234,   235,  -427,   238,   241,   236,   237,    81,
     119,   242,   239,   243,   244,   245,   246,   247,   248,   249,
     251,   251,  -427,  -427,  -427,  -427,  -427,  -427,   250,   240,
     252,   120,   255,   121,   126,   253,   127,   254,   256,    81,
    -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,
     257,   262,   258,  -427,   260,   264,   265,   266,   274,   259,
     275,   130,   -19,  -427,  -427,  -427,    -3,    -2,  -427,   261,
     273,   276,    17,  -427,   277,   278,   280,   281,   282,   283,
     286,   287,  -427,  -427,   289,   263,  -427,   292,   267,   268,
     294,   296,   269,   297,   298,   301,  -427,  -427,  -427,    -1,
    -427,  -427,   271,   270,   279,   306,   307,   308,   309,   310,
     311,   312,   284,   285,  -427,   288,  -427,  -427,   313,   314,
    -427,   315,   318,   319,   320,  -427,  -427,  -427,   290,  -427,
    -427,   323,   324,   325,   326,   327,   328,   329,   299,  -427,
    -427,   300,   331,    99,   333,   334,   335,   302,   305,   338,
     339,   340,   341,   342,   123,   343,  -427,   316,     4,   131,
     317,   321,   134,   135,    32,  -427,  -427,  -427,  -427,  -427,
    -427,   344,   345,   346,  -427,   322,   349,   350,   351,   354,
     355,   353,   357,   356,   138,  -427,   358,   330,   360,   362,
     363,   364,   365,   366,   367,   368,  -427,  -427,   369,   370,
     371,   347,   372,   373,   375,   376,   348,   377,   352,   378,
     379,   381,   -23,  -427,    99,   359,  -427,   382,   383,   385,
     386,   387,   388,   389,   390,   361,   374,   380,   384,   391,
     347,  -427,     5,   392,   393,   394,  -427,   395,  -427,   396,
     397,   398,  -427,  -427,    46,   400,    99,   399,   401,   402,
     403,   404,   405,   406,   407,   408,  -427,  -427,   409,  -427,
    -427,   410,  -427,   411,   413,   416,  -427,  -427,   412,   414,
    -427,  -427,    51,   417,   418,   124,   419,   420,   421,  -427,
    -427,  -427,   422,  -427,  -427,     6,   424,  -427,  -427,    82,
    -427,   426,   428,   429,   433,   434,   435,   436,    82,   415,
     423,  -427,   425,   139,   142,   145,   146,    67,  -427,  -427,
    -427,  -427,  -427,   430,   427,   125,   431,   437,   443,   446,
      72,   438,  -427,  -427,   449,   450,   451,   452,   453,   454,
     456,   457,  -427,  -427,  -427,  -427,   432,   458,  -427,   129,
     460,   465,  -427,  -427,   466,   467,   468,   470,   471,   472,
     473,   474,  -427,   444,   445,   475,   447,   448,   455,   478,
     480,   481,   484,   485,   486,   487,   488,  -427,  -427,   459,
    -427,  -427,   461,   462,   455,  -427,   489,   490,   493,   495,
     496,   497,   498,   499,  -427,   500,  -427,  -427,   501,   502,
     504,   505,   506,   507,   508,   509,   479,   482,   476,   512,
     513,   515,   516,   517,   518,  -427,  -427,  -427,   519,   520,
     521,   524,   525,   526,   527,   528,   529,   530,   503,   494,
     534,   536,   535,   537,  -427,  -427,   510,   511,   539,   541,
    -427,  -427,   514,   522,  -427,  -427
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -427,  -427,  -427,  -427,  -427,  -427,  -427,    50,  -427,  -427,
    -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,
    -427,  -427,  -427,  -427,    42,  -427,   523,  -427,  -427,  -427,
    -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,    57,
    -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,
    -427,  -115,  -427,  -427,  -427,  -427,  -427,  -427,  -427,  -427,
    -427,  -427,  -427,  -344,  -304,  -182,  -426,  -427,  -427,  -427,
    -427,  -427,  -427,  -427,  -427,  -427,  -427,   531,  -427,  -427,
     469,    98,  -147,  -427,  -427,  -427,  -427,  -427,  -427,  -110,
    -427,  -427,  -427,  -111
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -56
static const short yytable[] =
{
     337,   239,   264,   201,    28,    72,   137,   326,   401,   440,
     384,   350,   382,   351,   -48,    12,   236,   -49,    12,   201,
       9,   473,    13,    14,    15,    13,    14,    15,    87,    10,
     -55,    88,   238,   -55,   473,   240,   265,    73,   138,   327,
     402,   441,   412,   -48,    -8,     1,   -49,   299,    33,   300,
     301,    -6,    -8,    -8,    -8,   237,   302,   303,    39,   237,
      -8,   299,    55,   300,   301,   -12,   299,   336,   300,   301,
     302,   303,    68,    69,   127,   302,   303,   128,    45,    56,
     337,   410,   443,    59,   444,   445,   430,   443,    61,   444,
     445,   446,   173,   174,   175,   176,   446,   443,   150,   444,
     445,   151,   472,   -49,    12,    62,   446,   482,   337,   177,
      63,    13,    14,    15,   299,   156,   300,   301,   157,   -55,
      64,    65,   190,   302,   303,   191,   321,   433,   476,   322,
     434,   477,   494,    77,    31,   495,    32,    99,    40,    42,
      41,    43,    52,    57,    53,    58,    75,    66,    76,    67,
      78,   142,    79,   143,   210,   214,   211,   215,    74,    82,
     216,   219,   217,   220,   234,   328,   235,   329,   332,   334,
     333,   335,   350,   464,   351,   465,   466,    83,   467,   468,
     470,   469,   471,    80,    84,    85,    86,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,   100,   105,   106,
     107,   108,   109,   113,   114,   115,   110,   111,   112,   116,
     117,   118,   119,   120,   121,   123,   129,   122,   130,   131,
     132,   133,   213,   124,   139,   134,   140,   141,   126,   135,
     145,   152,   153,   154,   155,   158,   223,   383,   136,   159,
     146,   168,   144,   147,   169,   192,   194,   195,   196,   197,
     198,   199,   200,   207,   244,   162,   460,   163,   209,   206,
     400,   165,   164,   224,   171,   225,   227,   228,   229,   230,
     170,   166,   167,   201,   193,   208,   -12,   231,   233,   242,
     245,   246,   243,   247,   248,   249,   250,   218,   221,   251,
     252,   222,   253,   226,   232,   255,   241,   258,   254,   259,
     261,   262,   256,   260,   263,   257,   266,   267,   268,   271,
     272,   273,   274,   275,   276,   277,   281,   282,   284,   278,
     279,   285,   286,   287,   288,   280,   289,   290,   291,   292,
     293,   294,   295,   296,   298,   297,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   323,   338,   339,   340,
     325,   330,   342,   343,   344,   331,   341,   345,   346,   347,
     348,   355,   349,   357,   356,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   372,   373,   368,   374,   375,
     377,   379,   380,   376,   381,   387,   388,   378,   389,   390,
     391,   392,   393,   394,   385,   403,   404,   405,   395,     0,
     408,   409,   413,   527,   414,   415,   416,   417,   418,   396,
       0,     0,     0,     0,     0,   422,   425,   397,   398,   426,
     431,   432,   435,   436,   437,     0,   399,   442,   439,   453,
     406,   454,     0,   407,   411,   455,   456,   457,   458,   459,
       0,   419,   421,   479,   420,   423,   480,   427,   424,   481,
     461,   428,   484,   485,   486,   487,   488,   489,   462,   490,
     491,   493,   463,   496,   475,   474,   478,   492,   497,   499,
     500,   501,   483,   502,   503,   504,   505,   506,   509,   507,
     508,   516,   510,   517,   518,   511,   512,   519,   520,   521,
     522,   523,   528,   529,   524,   525,   530,   526,   531,   532,
     533,   534,   535,     0,   537,   538,   536,   539,   540,   541,
     542,   543,   544,   547,   545,   548,   549,   546,   550,   551,
     552,   553,    71,     0,   556,   554,   555,   557,   558,   559,
       0,   565,     0,   560,   561,   562,   563,   566,   564,   567,
      44,   568,   572,   569,   573,   570,     0,     0,   571,   574,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   575,
      54
};

static const short yycheck[] =
{
     304,     3,     3,    22,    21,     3,     3,     3,     3,     3,
     354,    34,    35,    36,     0,     1,    35,     0,     1,    22,
       0,   447,     8,     9,    10,     8,     9,    10,     3,     7,
      16,     6,    35,    16,   460,    37,    37,    35,    35,    35,
      35,    35,   386,    29,     0,     1,    29,    15,    19,    17,
      18,     7,     8,     9,    10,   202,    24,    25,    34,   206,
      16,    15,     6,    17,    18,    21,    15,    35,    17,    18,
      24,    25,     3,     4,     3,    24,    25,     6,    16,     6,
     384,    35,    15,    23,    17,    18,    35,    15,     3,    17,
      18,    24,    11,    12,    13,    14,    24,    15,     3,    17,
      18,     6,    35,     0,     1,     3,    24,    35,   412,    28,
       3,     8,     9,    10,    15,     3,    17,    18,     6,    16,
       3,     3,     3,    24,    25,     6,     3,     3,     3,     6,
       6,     6,     3,     5,    34,     6,    36,    36,    34,    34,
      36,    36,    34,    34,    36,    36,     3,    34,     5,    36,
      34,     3,    36,     5,    34,    34,    36,    36,     3,     6,
      34,    34,    36,    36,    34,    34,    36,    36,    34,    34,
      36,    36,    34,    34,    36,    36,    34,     3,    36,    34,
      34,    36,    36,    32,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,    33,    35,     3,
       3,     3,     3,    35,    37,    35,     6,     6,     6,    37,
       3,     3,     3,     3,     3,    36,     3,     5,     3,     3,
       6,     3,   172,    20,     3,     6,     3,     3,    34,    34,
       3,     3,     3,     3,     3,     3,   179,   352,    34,     6,
      34,     3,    37,    26,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,   212,    35,   438,    35,     6,   161,
     370,    35,    37,     6,    27,     3,     6,     3,     3,     3,
      34,    37,    37,    22,    35,    35,    21,     3,     3,     6,
       3,     3,     6,     3,     3,     3,     3,    34,    34,     3,
       3,    35,     3,    35,    35,     3,    35,     3,    35,     3,
       3,     3,    35,    34,     3,    37,    35,    37,    29,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,    35,
      35,     3,     3,     3,    34,    37,     3,     3,     3,     3,
       3,     3,     3,    34,     3,    35,     3,     3,     3,    37,
      35,     3,     3,     3,     3,     3,     3,     3,     3,     3,
      34,    34,     3,     3,     3,    34,    34,     3,     3,     6,
       3,     3,     6,     3,    34,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,    30,     3,     3,
       3,     3,     3,    35,     3,     3,     3,    35,     3,     3,
       3,     3,     3,     3,    35,     3,     3,     3,    37,    -1,
       3,     3,     3,   514,     3,     3,     3,     3,     3,    35,
      -1,    -1,    -1,    -1,    -1,     6,     3,    37,    34,     3,
       3,     3,     3,     3,     3,    -1,    35,     3,     6,     3,
      35,     3,    -1,    37,    34,     6,     3,     3,     3,     3,
      -1,    35,    34,     6,    37,    35,     3,    35,    37,     3,
      35,    37,     3,     3,     3,     3,     3,     3,    35,     3,
       3,     3,    37,     3,    37,    35,    35,    35,     3,     3,
       3,     3,    34,     3,     3,     3,     3,     3,     3,    35,
      35,     3,    35,     3,     3,    37,    31,     3,     3,     3,
       3,     3,     3,     3,    35,    34,     3,    35,     3,     3,
       3,     3,     3,    -1,     3,     3,     6,     3,     3,     3,
       3,     3,     3,    37,    35,     3,     3,    35,     3,     3,
       3,     3,    53,    -1,     3,     6,     6,     3,     3,     3,
      -1,    37,    -1,     6,     6,     6,     6,     3,    35,     3,
      17,     6,     3,     6,     3,    35,    -1,    -1,    37,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      29
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     1,    39,    40,    41,    42,    43,    45,    46,     0,
       7,    47,     1,     8,     9,    10,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    21,   114,
     115,    34,    36,    19,    48,    49,    50,    51,    52,    34,
      34,    36,    34,    36,    64,    16,    90,    91,    93,    95,
      97,    99,    34,    36,   115,     6,     6,    34,    36,    23,
      53,     3,     3,     3,     3,     3,    34,    36,     3,     4,
     118,   118,     3,    35,     3,     3,     5,     5,    34,    36,
      32,    54,     6,     3,     3,     3,     3,     3,     6,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,    36,
      33,    55,    56,    57,    58,    35,     3,     3,     3,     3,
       6,     6,     6,    35,    37,    35,    37,     3,     3,     3,
       3,     3,     5,    36,    20,    59,    34,     3,     6,     3,
       3,     3,     6,     3,     6,    34,    34,     3,    35,     3,
       3,     3,     3,     5,    37,     3,    34,    26,    60,    74,
       3,     6,     3,     3,     3,     3,     3,     6,     3,     6,
     117,   116,    35,    35,    37,    35,    37,    37,     3,     3,
      34,    27,    61,    11,    12,    13,    14,    28,    75,    76,
      77,    79,    80,    81,    82,    83,    84,    85,    86,    87,
       3,     6,     3,    35,     3,     3,     3,     3,     3,     3,
       3,    22,   119,   120,   121,   122,   119,     3,    35,     6,
      34,    36,    44,    45,    34,    36,    34,    36,    34,    34,
      36,    34,    35,    77,     6,     3,    35,     6,     3,     3,
       3,     3,    35,     3,    34,    36,    35,   120,    35,     3,
      37,    35,     6,     6,    62,     3,     3,     3,     3,     3,
       3,     3,     3,     3,    35,     3,    35,    37,     3,     3,
      34,     3,     3,     3,     3,    37,    35,    37,    29,   123,
     124,     3,     3,     3,     3,     3,     3,     3,    35,    35,
      37,     3,     3,    92,     3,     3,     3,     3,    34,     3,
       3,     3,     3,     3,     3,     3,    34,    35,     3,    15,
      17,    18,    24,    25,   101,   102,   107,   108,   109,   112,
     113,     3,     3,     3,    37,    35,     3,     3,     3,     3,
       3,     3,     6,     3,    78,    34,     3,    35,    34,    36,
      34,    34,    34,    36,    34,    36,    35,   102,     3,     3,
       3,    34,     3,     3,     3,     3,     3,     6,     3,     6,
      34,    36,    88,    89,    94,     3,    34,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,    30,   125,
     126,   127,     3,     3,     3,     3,    35,     3,    35,     3,
       3,     3,    35,    89,   101,    35,    96,     3,     3,     3,
       3,     3,     3,     3,     3,    37,    35,    37,    34,    35,
     127,     3,    35,     3,     3,     3,    35,    37,     3,     3,
      35,    34,   101,     3,     3,     3,     3,     3,     3,    35,
      37,    34,     6,    35,    37,     3,     3,    35,    37,    98,
      35,     3,     3,     3,     6,     3,     3,     3,   100,     6,
       3,    35,     3,    15,    17,    18,    24,   103,   104,   105,
     106,   110,   111,     3,     3,     6,     3,     3,     3,     3,
     103,    35,    35,    37,    34,    36,    34,    36,    34,    36,
      34,    36,    35,   104,    35,    37,     3,     6,    35,     6,
       3,     3,    35,    34,     3,     3,     3,     3,     3,     3,
       3,     3,    35,     3,     3,     6,     3,     3,   128,     3,
       3,     3,     3,     3,     3,     3,     3,    35,    35,     3,
      35,    37,    31,   129,   130,   131,     3,     3,     3,     3,
       3,     3,     3,     3,    35,    34,    35,   131,     3,     3,
       3,     3,     3,     3,     3,     3,     6,     3,     3,     3,
       3,     3,     3,     3,     3,    35,    35,    37,     3,     3,
       3,     3,     3,     3,     6,     6,     3,     3,     3,     3,
       6,     6,     6,     6,    35,    37,     3,     3,     6,     6,
      35,    37,     3,     3,    35,    37
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
#line 96 "parse_y.y"
    { YYABORT; }
    break;

  case 6:
#line 100 "parse_y.y"
    {
				/* reset flags for 'used layers';
				 * init font and data pointers
				 */
				int	i;

				if (!yyPCB)
				{
					Message("illegal fileformat\n");
					YYABORT;
				}
				for (i = 0; i < MAX_LAYER + 2; i++)
					LayerFlag[i] = False;
				yyFont = &yyPCB->Font;
				yyData = yyPCB->Data;
			}
    break;

  case 8:
#line 130 "parse_y.y"
    {
				/* reset flags for 'used layers';
				 * init font and data pointers
				 */
				int	i;

				if (!yyData || !yyFont)
				{
					Message("illegal fileformat\n");
					YYABORT;
				}
				for (i = 0; i < MAX_LAYER + 2; i++)
					LayerFlag[i] = False;
			}
    break;

  case 12:
#line 154 "parse_y.y"
    {
				/* mark all symbols invalid */
				int	i;

				if (!yyFont)
				{
					Message("illegal fileformat\n");
					YYABORT;
				}
				yyFont->Valid = False;
				for (i = 0; i <= MAX_FONTPOSITION; i++)
					yyFont->Symbol[i].Valid = False;
			}
    break;

  case 13:
#line 168 "parse_y.y"
    {
				yyFont->Valid = True;
		  		SetFontInfo(yyFont);
			}
    break;

  case 14:
#line 176 "parse_y.y"
    {
				yyPCB->Name = yyvsp[-1].string;
				yyPCB->MaxWidth = MAX_COORD;
				yyPCB->MaxHeight = MAX_COORD;
			}
    break;

  case 15:
#line 182 "parse_y.y"
    {
				yyPCB->Name = yyvsp[-3].string;
				yyPCB->MaxWidth = yyvsp[-2].number*100;
				yyPCB->MaxHeight = yyvsp[-1].number*100;
			}
    break;

  case 16:
#line 188 "parse_y.y"
    {
				yyPCB->Name = yyvsp[-3].string;
				yyPCB->MaxWidth = yyvsp[-2].number;
				yyPCB->MaxHeight = yyvsp[-1].number;
			}
    break;

  case 21:
#line 203 "parse_y.y"
    {
				yyPCB->Grid = yyvsp[-3].number*100;
				yyPCB->GridOffsetX = yyvsp[-2].number*100;
				yyPCB->GridOffsetY = yyvsp[-1].number*100;
			}
    break;

  case 22:
#line 211 "parse_y.y"
    {
				yyPCB->Grid = yyvsp[-4].number*100;
				yyPCB->GridOffsetX = yyvsp[-3].number*100;
				yyPCB->GridOffsetY = yyvsp[-2].number*100;
				if (yyvsp[-1].number)
					Settings.DrawGrid = True;
				else
					Settings.DrawGrid = False;
			}
    break;

  case 23:
#line 224 "parse_y.y"
    {
				yyPCB->Grid = yyvsp[-4].floating*100;
				yyPCB->GridOffsetX = yyvsp[-3].number*100;
				yyPCB->GridOffsetY = yyvsp[-2].number*100;
				if (yyvsp[-1].number)
					Settings.DrawGrid = True;
				else
					Settings.DrawGrid = False;
			}
    break;

  case 24:
#line 236 "parse_y.y"
    {
				yyPCB->Grid = yyvsp[-4].floating;
				yyPCB->GridOffsetX = yyvsp[-3].number;
				yyPCB->GridOffsetY = yyvsp[-2].number;
				if (yyvsp[-1].number)
					Settings.DrawGrid = True;
				else
					Settings.DrawGrid = False;
			}
    break;

  case 25:
#line 249 "parse_y.y"
    {
				yyPCB->CursorX = yyvsp[-3].number*100;
				yyPCB->CursorY = yyvsp[-2].number*100;
				yyPCB->Zoom = yyvsp[-1].number*2;
			}
    break;

  case 26:
#line 255 "parse_y.y"
    {
				yyPCB->CursorX = yyvsp[-3].number;
				yyPCB->CursorY = yyvsp[-2].number;
				yyPCB->Zoom = yyvsp[-1].number;
			}
    break;

  case 27:
#line 261 "parse_y.y"
    {
				yyPCB->CursorX = yyvsp[-3].number;
				yyPCB->CursorY = yyvsp[-2].number;
				yyPCB->Zoom = yyvsp[-1].floating;
			}
    break;

  case 30:
#line 272 "parse_y.y"
    {
				yyPCB->ThermScale = yyvsp[-1].floating;
			}
    break;

  case 35:
#line 286 "parse_y.y"
    {
				Settings.Bloat = yyvsp[-3].number;
				Settings.Shrink = yyvsp[-2].number;
				Settings.minWid = yyvsp[-1].number;
			}
    break;

  case 37:
#line 296 "parse_y.y"
    {
				Settings.Bloat = yyvsp[-4].number;
				Settings.Shrink = yyvsp[-3].number;
				Settings.minWid = yyvsp[-2].number;
				Settings.minSlk = yyvsp[-1].number;
			}
    break;

  case 39:
#line 307 "parse_y.y"
    {
				Settings.Bloat = yyvsp[-6].number;
				Settings.Shrink = yyvsp[-5].number;
				Settings.minWid = yyvsp[-4].number;
				Settings.minSlk = yyvsp[-3].number;
				Settings.minDrill = yyvsp[-2].number;
			}
    break;

  case 40:
#line 318 "parse_y.y"
    {
				yyPCB->Flags = yyvsp[-1].number & PCB_FLAGS;
			}
    break;

  case 42:
#line 326 "parse_y.y"
    {
				if (ParseGroupString(yyvsp[-1].string, &yyPCB->LayerGroups))
				{
					Message("illegal layer-group string\n");
					YYABORT;
				}
			}
    break;

  case 44:
#line 337 "parse_y.y"
    {
				if (ParseRouteString(yyvsp[-1].string, &yyPCB->RouteStyle[0], 100))
				{
					Message("illegal route-style string\n");
					YYABORT;
				}
			}
    break;

  case 45:
#line 345 "parse_y.y"
    {
				if (ParseRouteString(yyvsp[-1].string, &yyPCB->RouteStyle[0], 1))
				{
					Message("illegal route-style string\n");
					YYABORT;
				}
			}
    break;

  case 55:
#line 371 "parse_y.y"
    {
				/* clear pointer to force memory allocation by 
				 * the appropriate subroutine
				 */
				yyElement = NULL;
			}
    break;

  case 57:
#line 378 "parse_y.y"
    { YYABORT; }
    break;

  case 63:
#line 392 "parse_y.y"
    {
				CreateNewVia(yyData, yyvsp[-8].number, yyvsp[-7].number, yyvsp[-6].number, yyvsp[-5].number, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].string,
					(yyvsp[-1].number & OBJ_FLAGS) | VIAFLAG);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 64:
#line 402 "parse_y.y"
    {
				CreateNewVia(yyData, yyvsp[-8].number*100, yyvsp[-7].number*100, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].number*100, yyvsp[-2].string,
					(yyvsp[-1].number & OBJ_FLAGS) | VIAFLAG);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 65:
#line 413 "parse_y.y"
    {
				CreateNewVia(yyData, yyvsp[-7].number*100, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number*100, (yyvsp[-5].number + yyvsp[-4].number)*100, yyvsp[-3].number*100, yyvsp[-2].string,
					(yyvsp[-1].number & OBJ_FLAGS) | VIAFLAG);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 66:
#line 423 "parse_y.y"
    {
				CreateNewVia(yyData, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number*100, 200*GROUNDPLANEFRAME,
					(yyvsp[-4].number + 2*MASKFRAME)*100,  yyvsp[-3].number*100, yyvsp[-2].string, (yyvsp[-1].number & OBJ_FLAGS) | VIAFLAG);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 67:
#line 433 "parse_y.y"
    {
				BDimension	hole = (yyvsp[-3].number *DEFAULT_DRILLINGHOLE);

				/* make sure that there's enough copper left */
				if (yyvsp[-3].number -hole < MIN_PINORVIACOPPER && 
					yyvsp[-3].number > MIN_PINORVIACOPPER)
					hole = yyvsp[-3].number -MIN_PINORVIACOPPER;

				CreateNewVia(yyData, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].number*100, 200*GROUNDPLANEFRAME,
					(yyvsp[-3].number + 2*MASKFRAME)*100, hole, yyvsp[-2].string, (yyvsp[-1].number & OBJ_FLAGS) | VIAFLAG);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 68:
#line 449 "parse_y.y"
    {
				CreateNewRat(yyData, yyvsp[-7].number, yyvsp[-6].number, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-5].number, yyvsp[-2].number,
					Settings.RatThickness, (yyvsp[-1].number & OBJ_FLAGS));
			}
    break;

  case 69:
#line 454 "parse_y.y"
    {
				CreateNewRat(yyData, yyvsp[-7].number*100, yyvsp[-6].number*100, yyvsp[-4].number*100, yyvsp[-3].number*100, yyvsp[-5].number, yyvsp[-2].number,
					Settings.RatThickness, (yyvsp[-1].number & OBJ_FLAGS));
			}
    break;

  case 70:
#line 463 "parse_y.y"
    {
				if (yyvsp[-3].number <= 0 || yyvsp[-3].number > MAX_LAYER + 2)
				{
					yyerror("Layernumber out of range");
					YYABORT;
				}
				if (LayerFlag[yyvsp[-3].number-1])
				{
					yyerror("Layernumber used twice");
					YYABORT;
				}
				Layer = &yyData->Layer[yyvsp[-3].number-1];

				/* memory for name is alread allocated */
				Layer->Name = yyvsp[-2].string;
				LayerFlag[yyvsp[-3].number-1] = True;
			}
    break;

  case 82:
#line 502 "parse_y.y"
    {
				CreateNewPolygonFromRectangle(Layer,
					yyvsp[-5].number*100, yyvsp[-4].number*100, (yyvsp[-5].number+yyvsp[-3].number)*100, (yyvsp[-4].number+yyvsp[-2].number)*100, yyvsp[-1].number & OBJ_FLAGS);
			}
    break;

  case 86:
#line 511 "parse_y.y"
    {
				Polygon = CreateNewPolygon(Layer, yyvsp[-2].number & OBJ_FLAGS);
			}
    break;

  case 87:
#line 515 "parse_y.y"
    {
				/* ignore junk */
				if (Polygon->PointN >= 3)
					SetPolygonBoundingBox(Polygon);
				else
				{
					Message("WARNING parsing file '%s'\n"
						"    line:        %i\n"
						"    description: 'ignored polygon (< 3 points)'\n",
						yyfilename, yylineno);
					DestroyObject(PCB->Data, POLYGON_TYPE, Layer, Polygon, Polygon);
				}
			}
    break;

  case 88:
#line 533 "parse_y.y"
    {
				CreateNewLineOnLayer(Layer, yyvsp[-7].number, yyvsp[-6].number, yyvsp[-5].number, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].number, yyvsp[-1].number & OBJ_FLAGS);
			}
    break;

  case 89:
#line 541 "parse_y.y"
    {
				CreateNewLineOnLayer(Layer, yyvsp[-7].number*100, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].number*100, yyvsp[-2].number*100, yyvsp[-1].number & OBJ_FLAGS);
			}
    break;

  case 90:
#line 549 "parse_y.y"
    {
			/* elliminate old-style rat-lines */
			if ((yyvsp[-1].number & RATFLAG) == 0)
				CreateNewLineOnLayer(Layer, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].number*100, yyvsp[-2].number*100,
					200*GROUNDPLANEFRAME, yyvsp[-1].number & OBJ_FLAGS);
			}
    break;

  case 91:
#line 560 "parse_y.y"
    {
				CreateNewArcOnLayer(Layer, yyvsp[-9].number, yyvsp[-8].number, yyvsp[-7].number, yyvsp[-3].number, yyvsp[-2].number, yyvsp[-5].number, yyvsp[-4].number, yyvsp[-1].number & OBJ_FLAGS);
			}
    break;

  case 92:
#line 568 "parse_y.y"
    {
				CreateNewArcOnLayer(Layer, yyvsp[-9].number*100, yyvsp[-8].number*100, yyvsp[-7].number*100, yyvsp[-3].number, yyvsp[-2].number, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-1].number & OBJ_FLAGS);
			}
    break;

  case 93:
#line 576 "parse_y.y"
    {
				CreateNewArcOnLayer(Layer, yyvsp[-8].number*100, yyvsp[-7].number*100, yyvsp[-6].number*100, yyvsp[-3].number, yyvsp[-2].number,
					yyvsp[-4].number*100, 200*GROUNDPLANEFRAME, yyvsp[-1].number & OBJ_FLAGS);
			}
    break;

  case 94:
#line 585 "parse_y.y"
    {
				/* use a default scale of 100% */
				CreateNewText(Layer,yyFont,yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].number, 100, yyvsp[-2].string, yyvsp[-1].number & OBJ_FLAGS);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 95:
#line 595 "parse_y.y"
    {
				if (yyvsp[-1].number & ONSILKFLAG)
				{
					LayerTypePtr lay = &yyData->Layer[MAX_LAYER +
						((yyvsp[-1].number & ONSOLDERFLAG) ? SOLDER_LAYER : COMPONENT_LAYER)];

					CreateNewText(lay ,yyFont, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].string, yyvsp[-1].number & OBJ_FLAGS);
				}
				else
					CreateNewText(Layer, yyFont, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].string, yyvsp[-1].number & OBJ_FLAGS);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 96:
#line 611 "parse_y.y"
    {
				if (yyvsp[-1].number & ONSILKFLAG)
				{
					LayerTypePtr lay = &yyData->Layer[MAX_LAYER +
						((yyvsp[-1].number & ONSOLDERFLAG) ? SOLDER_LAYER : COMPONENT_LAYER)];

					CreateNewText(lay ,yyFont, yyvsp[-6].number, yyvsp[-5].number, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].string, yyvsp[-1].number & OBJ_FLAGS);
				}
				else
					CreateNewText(Layer, yyFont, yyvsp[-6].number, yyvsp[-5].number, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].string, yyvsp[-1].number & OBJ_FLAGS);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 99:
#line 633 "parse_y.y"
    {
				CreateNewPointInPolygon(Polygon, yyvsp[-2].number*100, yyvsp[-1].number*100);
			}
    break;

  case 100:
#line 637 "parse_y.y"
    {
				CreateNewPointInPolygon(Polygon, yyvsp[-2].number, yyvsp[-1].number);
			}
    break;

  case 107:
#line 656 "parse_y.y"
    {
				yyElement = CreateNewElement(yyData, yyElement, yyFont, 0x0000,
					yyvsp[-6].string, yyvsp[-5].string, NULL, yyvsp[-4].number*100, yyvsp[-3].number*100, yyvsp[-2].number, 100, 0x0000, False);
				SaveFree(yyvsp[-6].string);
				SaveFree(yyvsp[-5].string);
				pin_num = 1;
			}
    break;

  case 108:
#line 664 "parse_y.y"
    {
				SetElementBoundingBox(yyData, yyElement, yyFont);
			}
    break;

  case 109:
#line 674 "parse_y.y"
    {
				yyElement = CreateNewElement(yyData, yyElement, yyFont, yyvsp[-9].number,
					yyvsp[-8].string, yyvsp[-7].string, NULL, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].number, False);
				SaveFree(yyvsp[-8].string);
				SaveFree(yyvsp[-7].string);
				pin_num = 1;
			}
    break;

  case 110:
#line 682 "parse_y.y"
    {
				SetElementBoundingBox(yyData, yyElement, yyFont);
			}
    break;

  case 111:
#line 692 "parse_y.y"
    {
				yyElement = CreateNewElement(yyData, yyElement, yyFont, yyvsp[-10].number,
					yyvsp[-9].string, yyvsp[-8].string, yyvsp[-7].string, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].number, False);
				SaveFree(yyvsp[-9].string);
				SaveFree(yyvsp[-8].string);
				SaveFree(yyvsp[-7].string);
				pin_num = 1;
			}
    break;

  case 112:
#line 701 "parse_y.y"
    {
				SetElementBoundingBox(yyData, yyElement, yyFont);
			}
    break;

  case 113:
#line 712 "parse_y.y"
    {
				yyElement = CreateNewElement(yyData, yyElement, yyFont, yyvsp[-12].number,
					yyvsp[-11].string, yyvsp[-10].string, yyvsp[-9].string, (yyvsp[-8].number+yyvsp[-6].number)*100, (yyvsp[-7].number+yyvsp[-5].number)*100, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].number, False);
				yyElement->MarkX = yyvsp[-8].number*100;
				yyElement->MarkY = yyvsp[-7].number*100;
				SaveFree(yyvsp[-11].string);
				SaveFree(yyvsp[-10].string);
				SaveFree(yyvsp[-9].string);
			}
    break;

  case 114:
#line 722 "parse_y.y"
    {
				SetElementBoundingBox(yyData, yyElement, yyFont);
			}
    break;

  case 115:
#line 733 "parse_y.y"
    {
				yyElement = CreateNewElement(yyData, yyElement, yyFont, yyvsp[-12].number,
					yyvsp[-11].string, yyvsp[-10].string, yyvsp[-9].string, (yyvsp[-8].number+yyvsp[-6].number), (yyvsp[-7].number+yyvsp[-5].number), yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].number, False);
				yyElement->MarkX = yyvsp[-8].number;
				yyElement->MarkY = yyvsp[-7].number;
				SaveFree(yyvsp[-11].string);
				SaveFree(yyvsp[-10].string);
				SaveFree(yyvsp[-9].string);
			}
    break;

  case 116:
#line 743 "parse_y.y"
    {
				SetElementBoundingBox(yyData, yyElement, yyFont);
			}
    break;

  case 124:
#line 761 "parse_y.y"
    {
				CreateNewLineInElement(yyElement, yyvsp[-5].number, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].number, yyvsp[-1].number);
			}
    break;

  case 125:
#line 766 "parse_y.y"
    {
				CreateNewLineInElement(yyElement, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].number*100, yyvsp[-2].number*100, yyvsp[-1].number*100);
			}
    break;

  case 126:
#line 771 "parse_y.y"
    {
				CreateNewArcInElement(yyElement, yyvsp[-7].number, yyvsp[-6].number, yyvsp[-5].number, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].number, yyvsp[-1].number);
			}
    break;

  case 127:
#line 775 "parse_y.y"
    {
				CreateNewArcInElement(yyElement, yyvsp[-7].number*100, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].number, yyvsp[-2].number, yyvsp[-1].number*100);
			}
    break;

  case 128:
#line 780 "parse_y.y"
    {
				yyElement->MarkX = yyvsp[-2].number;
				yyElement->MarkY = yyvsp[-1].number;
			}
    break;

  case 129:
#line 785 "parse_y.y"
    {
				yyElement->MarkX = yyvsp[-2].number*100;
				yyElement->MarkY = yyvsp[-1].number*100;
			}
    break;

  case 136:
#line 803 "parse_y.y"
    {
				CreateNewLineInElement(yyElement, yyvsp[-5].number + yyElement->MarkX,
					yyvsp[-4].number + yyElement->MarkY, yyvsp[-3].number + yyElement->MarkX,
					yyvsp[-2].number + yyElement->MarkY, yyvsp[-1].number);
			}
    break;

  case 137:
#line 809 "parse_y.y"
    {
				CreateNewLineInElement(yyElement, yyvsp[-5].number*100 + yyElement->MarkX,
					yyvsp[-4].number*100 + yyElement->MarkY, yyvsp[-3].number*100 + yyElement->MarkX,
					yyvsp[-2].number*100 + yyElement->MarkY, yyvsp[-1].number*100);
			}
    break;

  case 138:
#line 816 "parse_y.y"
    {
				CreateNewArcInElement(yyElement, yyvsp[-7].number + yyElement->MarkX,
					yyvsp[-6].number + yyElement->MarkY, yyvsp[-5].number, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].number, yyvsp[-1].number);
			}
    break;

  case 139:
#line 821 "parse_y.y"
    {
				CreateNewArcInElement(yyElement, yyvsp[-7].number*100 + yyElement->MarkX,
					yyvsp[-6].number*100 + yyElement->MarkY, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].number, yyvsp[-2].number, yyvsp[-1].number*100);
			}
    break;

  case 140:
#line 831 "parse_y.y"
    {
				CreateNewPin(yyElement, yyvsp[-9].number + yyElement->MarkX,
					yyvsp[-8].number + yyElement->MarkY, yyvsp[-7].number, yyvsp[-6].number, yyvsp[-5].number, yyvsp[-4].number, yyvsp[-3].string,
					yyvsp[-2].string, (yyvsp[-1].number & OBJ_FLAGS) | PINFLAG);
				SaveFree(yyvsp[-3].string);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 141:
#line 843 "parse_y.y"
    {
				CreateNewPin(yyElement, yyvsp[-9].number*100 + yyElement->MarkX,
					yyvsp[-8].number*100 + yyElement->MarkY, yyvsp[-7].number*100, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].string,
					yyvsp[-2].string, (yyvsp[-1].number & OBJ_FLAGS) | PINFLAG);
				SaveFree(yyvsp[-3].string);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 142:
#line 855 "parse_y.y"
    {
				CreateNewPin(yyElement, yyvsp[-7].number*100, yyvsp[-6].number*100, yyvsp[-5].number*100, 200*GROUNDPLANEFRAME,
					(yyvsp[-5].number + 2*MASKFRAME)*100, yyvsp[-4].number*100, yyvsp[-3].string, yyvsp[-2].string,
					(yyvsp[-1].number & OBJ_FLAGS) | PINFLAG);
				SaveFree(yyvsp[-3].string);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 143:
#line 867 "parse_y.y"
    {
				char	p_number[8];

				sprintf(p_number, "%d", pin_num++);
				CreateNewPin(yyElement, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number*100, 200*GROUNDPLANEFRAME,
					(yyvsp[-4].number + 2*MASKFRAME)*100, yyvsp[-3].number*100, yyvsp[-2].string, p_number,
					(yyvsp[-1].number & OBJ_FLAGS) | PINFLAG);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 144:
#line 883 "parse_y.y"
    {
				BDimension	hole = (yyvsp[-3].number *DEFAULT_DRILLINGHOLE);
				char		p_number[8];

				/* make sure that there's enough copper left */
				if (yyvsp[-3].number -hole < MIN_PINORVIACOPPER && 
					yyvsp[-3].number > MIN_PINORVIACOPPER)
					hole = yyvsp[-3].number -MIN_PINORVIACOPPER;

				sprintf(p_number, "%d", pin_num++);
				CreateNewPin(yyElement, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].number*100, 200*GROUNDPLANEFRAME,
					(yyvsp[-3].number + 2*MASKFRAME)*100, hole, yyvsp[-2].string, p_number,
					(yyvsp[-1].number & OBJ_FLAGS) | PINFLAG);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 145:
#line 903 "parse_y.y"
    {
				CreateNewPad(yyElement, yyvsp[-10].number + yyElement->MarkX,
					yyvsp[-9].number + yyElement->MarkY,
					yyvsp[-8].number + yyElement->MarkX,
					yyvsp[-7].number + yyElement->MarkY, yyvsp[-6].number, yyvsp[-5].number, yyvsp[-4].number,
					yyvsp[-3].string, yyvsp[-2].string, (yyvsp[-1].number & OBJ_FLAGS));
				SaveFree(yyvsp[-3].string);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 146:
#line 917 "parse_y.y"
    {
				CreateNewPad(yyElement,yyvsp[-10].number*100 + yyElement->MarkX,
					yyvsp[-9].number*100 + yyElement->MarkY, yyvsp[-8].number*100 + yyElement->MarkX,
					yyvsp[-7].number*100 + yyElement->MarkY, yyvsp[-6].number*100, yyvsp[-5].number*100, yyvsp[-4].number*100,
					yyvsp[-3].string, yyvsp[-2].string, (yyvsp[-1].number & OBJ_FLAGS));
				SaveFree(yyvsp[-3].string);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 147:
#line 930 "parse_y.y"
    {
				CreateNewPad(yyElement,yyvsp[-8].number*100,yyvsp[-7].number*100,yyvsp[-6].number*100,yyvsp[-5].number*100,yyvsp[-4].number*100, 200*GROUNDPLANEFRAME,
					(yyvsp[-4].number + 2*MASKFRAME)*100, yyvsp[-3].string,yyvsp[-2].string, (yyvsp[-1].number & OBJ_FLAGS));
				SaveFree(yyvsp[-3].string);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 148:
#line 941 "parse_y.y"
    {
				char		p_number[8];

				sprintf(p_number, "%d", pin_num++);
				CreateNewPad(yyElement,yyvsp[-7].number*100,yyvsp[-6].number*100,yyvsp[-5].number*100,yyvsp[-4].number*100,yyvsp[-3].number*100, 200*GROUNDPLANEFRAME,
					(yyvsp[-3].number + 2*MASKFRAME)*100, yyvsp[-2].string,p_number,
					(yyvsp[-1].number & OBJ_FLAGS));
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 151:
#line 959 "parse_y.y"
    {
				if (yyvsp[-3].number <= 0 || yyvsp[-3].number > MAX_FONTPOSITION)
				{
					yyerror("fontposition out of range");
					YYABORT;
				}
				Symbol = &yyFont->Symbol[yyvsp[-3].number];
				if (Symbol->Valid)
				{
					yyerror("symbol ID used twice");
					YYABORT;
				}
				Symbol->Valid = True;
				Symbol->Delta = yyvsp[-2].number;
			}
    break;

  case 153:
#line 976 "parse_y.y"
    {
				if (yyvsp[-3].number <= 0 || yyvsp[-3].number > MAX_FONTPOSITION)
				{
					yyerror("fontposition out of range");
					YYABORT;
				}
				Symbol = &yyFont->Symbol[yyvsp[-3].number];
				if (Symbol->Valid)
				{
					yyerror("symbol ID used twice");
					YYABORT;
				}
				Symbol->Valid = True;
				Symbol->Delta = yyvsp[-2].number*100;
			}
    break;

  case 162:
#line 1013 "parse_y.y"
    {
				CreateNewLineInSymbol(Symbol, yyvsp[-5].number*100, yyvsp[-4].number*100, yyvsp[-3].number*100, yyvsp[-2].number*100, yyvsp[-1].number*100);
			}
    break;

  case 163:
#line 1020 "parse_y.y"
    {
				CreateNewLineInSymbol(Symbol, yyvsp[-5].number, yyvsp[-4].number, yyvsp[-3].number, yyvsp[-2].number, yyvsp[-1].number);
			}
    break;

  case 171:
#line 1047 "parse_y.y"
    {
				Menu = CreateNewNet(&yyPCB->NetlistLib, yyvsp[-3].string, yyvsp[-2].string);
				SaveFree(yyvsp[-3].string);
				SaveFree(yyvsp[-2].string);
			}
    break;

  case 177:
#line 1067 "parse_y.y"
    {
				CreateNewConnection(Menu, yyvsp[-1].string);
				SaveFree(yyvsp[-1].string);
			}
    break;


    }

/* Line 1000 of yacc.c.  */
#line 2423 "parse_y.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
	  yychar = YYEMPTY;

	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 1072 "parse_y.y"


/* ---------------------------------------------------------------------------
 * error routine called by parser library
 */
int yyerror(s)
char *s;
{
	Message("ERROR parsing file '%s'\n"
		"    line:        %i\n"
		"    description: '%s'\n",
		yyfilename, yylineno, s);
	return(0);
}

int yywrap()
{
  return 1;
}

