#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

#ifndef YYSTYPE
typedef union {
  int ival;
  char *sval;
  Resource *rval;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	STRING	257
# define	INCLUDE	258


extern YYSTYPE reslval;

#endif /* not BISON_Y_TAB_H */
