dnl $Id: configure.ac,v 1.4 2006/10/23 18:11:27 tim Exp $
dnl Process this file with autoconf to produce a configure script.

AC_INIT(src/pinout.c)
AC_PREREQ(2.52)

dnl this call will define PACKAGE and VERSION
dnl please use this as the primary reference for the version number
VERSION=`cat ./VERSION | cut -d. -f1-2`
REVISION=`cat ./VERSION | cut -d. -f3`

AM_INIT_AUTOMAKE(pcb, ${VERSION})
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

dnl Set the name of the compile target (disabled if Tcl is selected)
PCBBIN="pcb-bin"

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_CHECK_PROGS(M4, gm4 m4)

AC_CHECK_PROGS(CP, cp)
AC_SUBST(CP)

AC_PATH_PROGS(WISH, wish wish83 wish8.3 wish80 wish8.0 cygwish83 cygwish80,[none])
if test "X$WISH" = "Xnone" ; then
	AC_ERROR([Did not find the wish executible.  You need to make sure
	that tcl is installed on your system and that wish is in your path])
fi

AC_DEFINE_UNQUOTED(M4,$M4,[m4 executible])
GNUM4=$M4
AC_SUBST(GNUM4)
AC_DEFINE_UNQUOTED(GNUM4,"$M4",[m4 program used by pcb])
AC_CHECK_PROGS(LATEX, latex)
AC_CHECK_PROGS(DVIPS, dvips)
AC_CHECK_PROGS(PS2PDF, ps2pdf)

dnl Check for va_copy
AC_CACHE_CHECK([for va_copy], ac_cv_c_va_copy,
 AC_TRY_LINK(
   [#include <stdarg.h>],
   [va_list ap1, ap2;
    va_copy(ap1,ap2);
   ],
   [ac_cv_c_va_copy="yes"],
   [ac_cv_c_va_copy="no"])
 )
if test "$ac_cv_c_va_copy" = "yes"
then
    AC_DEFINE(HAVE_VA_COPY, 1, [Define if we have va_copy])
fi
AC_CACHE_CHECK([for __va_copy], ac_cv_c__va_copy,
 AC_TRY_LINK(
   [#include <stdarg.h>],
   [va_list ap1, ap2;
    __va_copy(ap1,ap2);
   ],
   [ac_cv_c__va_copy="yes"],
   [ac_cv_c__va_copy="no"])
 )
if test "$ac_cv_c__va_copy" = "yes"
then
    AC_DEFINE(HAVE__VA_COPY, 1, [Define if we have __va_copy])
fi

dnl Checks for libraries.
AC_CHECK_LIB(m, sqrt)
AC_CHECK_LIB(xnet, gethostbyname)
AC_CHECK_LIB(fl, yywrap)
AC_CHECK_FUNCS(strerror)
AC_CHECK_FUNCS(regcomp re_comp)
AC_CHECK_FUNCS(logf expf rint)
AC_PATH_XTRA
AC_ARG_WITH([libstroke],
[  --without-libstroke     disable libstroke support (default: enabled)],
AC_CHECK_LIB(stroke, main, , AC_ERROR([libstroke not found.  Please specify its location or configure --without-libstroke]))
)
AC_CHECK_LIB(X11, XOpenDisplay, , , $X_LIBS)
AC_CHECK_LIB(ICE, main, , , $X_LIBS)
AC_CHECK_LIB(SM, main, , , $X_LIBS)
AC_CHECK_LIB(Xext, main, , , $X_LIBS)
AC_CHECK_LIB(Xt, XtOpenDisplay, , , $X_LIBS)
AC_CHECK_LIB(Xmu, main, , , $X_LIBS)
AC_CHECK_LIB(Xpm, main, , , $X_LIBS)

AC_ARG_WITH([xaw],
[  --with-xaw=Xaw         Use the specified Athena Widgets type.
                          For example, Xaw, or Xaw3d.
                          [default = Xaw]],
[
if test "X$with_xaw" = "Xno" ; then
	AC_ERROR([--without-xaw is not allowed])
fi
],
[
with_xaw=Xaw
])

AC_CHECK_LIB($with_xaw, XawInitializeWidgetSet, , 
	AC_ERROR([You must have some version of Xaw installed on your system]), $X_LIBS)
LIBXAW=$with_xaw
XAWINC=$with_xaw

dnl Determine the Xfuncproto control definitions:
FC_CHECK_X_PROTO_DEFINE(FUNCPROTO)
if test -n "$fc_x_proto_value"; then
	AC_DEFINE_UNQUOTED(FUNCPROTO, $fc_x_proto_value, [FUNCPROTO used by Xaw])
fi
FC_CHECK_X_PROTO_DEFINE(NARROWPROTO)
if test -n "$fc_x_proto_value"; then
	AC_DEFINE_UNQUOTED(NARROWPROTO, $fc_x_proto_value, [NARROWPROTO used by Xaw])
fi

dnl Try to figure out if we need -Rpath for finding X11 libs
dnl at runtime.  Why autoconf doesn't already do this, I don't
dnl know...
xlib_path=""
for p in $X_LIBS ; do
    case $p in
    -L*)
	xlib_path="$xlib_path $p"
	;;

    esac
done
xlib_rpath=`echo $xlib_path | sed 's/-L/-R/g'`

pcb_save_LIBS=$LIBS
LIBS="$LIBS $X_LIBS"
rpath=""
for fl in "-Wl,-rpath " "-Wl,--rpath " "-rpath " "--rpath " "-Wl,-R" "-R" "-R " ; do
    xlib_rpath=`echo $xlib_path | sed "s/-L/$fl/g"`
    LIBS="$pcb_save_LIBS $X_LIBS $xlib_rpath"
    AC_MSG_CHECKING([if the compiler accepts ${fl}path for runtime libraries])
    AC_LINK_IFELSE([AC_LANG_PROGRAM()],
	AC_MSG_RESULT([yes])
	rpath=$fl, 
	AC_MSG_RESULT([no]))
    test -n "$rpath" && break
done
if test -n "$rpath"; then
   X_LIBS="$X_LIBS $xlib_rpath"
fi
LIBS=$pcb_save_LIBS

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(string.h)
AC_CHECK_HEADERS(regex.h)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for data types

dnl Checks for library functions.

dnl Check for GNU ld.  This needs to be qualified for Solaris, which
dnl may use gcc for the C compiler but not for the linker.

dnl check if the linker is a GNU linker

#------------------------------------------------------------
# AC_PROG_LD - find the path to the GNU or non-GNU linker
# (This stuff ripped from libtool)
#------------------------------------------------------------
AC_DEFUN([AC_PROG_LD],
[AC_ARG_WITH(gnu-ld,
[  --with-gnu-ld           assume the C compiler uses GNU ld [[default=no]]],
test "$withval" = no || pcb_with_gnu_ld=yes, pcb_with_gnu_ld=no)
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
dnl ###not for PostgreSQL### AC_REQUIRE([AC_CANONICAL_BUILD])dnl
ac_prog=ld
if test "$GCC" = yes; then
  # Check if gcc -print-prog-name=ld gives a path.
  AC_MSG_CHECKING([for ld used by GCC])
  case $host in
  *-*-mingw*)
    # gcc leaves a trailing carriage return which upsets mingw
    ac_prog=`($CC -print-prog-name=ld) 2>&5 | tr -d '\015'` ;;
  *)
    ac_prog=`($CC -print-prog-name=ld) 2>&5` ;;
  esac
  case "$ac_prog" in
    # Accept absolute paths.
changequote(,)dnl
    [\\/]* | [A-Za-z]:[\\/]*)
      re_direlt='/[^/][^/]*/\.\./'
changequote([,])dnl
      # Canonicalize the path of ld
      ac_prog=`echo $ac_prog| sed 's%\\\\%/%g'`
      while echo $ac_prog | grep "$re_direlt" > /dev/null 2>&1; do
        ac_prog=`echo $ac_prog| sed "s%$re_direlt%/%"`
      done
      test -z "$LD" && LD="$ac_prog"
      ;;
  "")
    # If it fails, then pretend we aren't using GCC.
    ac_prog=ld
    ;;
  *)
    # If it is relative, then search for the first ld in PATH.
    pcb_with_gnu_ld=unknown
    ;;
  esac
elif test "$pcb_with_gnu_ld" = yes; then
  AC_MSG_CHECKING([for GNU ld])
else
  AC_MSG_CHECKING([for non-GNU ld])
fi
AC_CACHE_VAL(ac_cv_path_LD,
[if test -z "$LD"; then
  IFS="${IFS=   }"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -f "$ac_dir/$ac_prog" || test -f "$ac_dir/$ac_prog$ac_exeext"; then
      ac_cv_path_LD="$ac_dir/$ac_prog"
      # Check to see if the program is GNU ld.  I'd rather use --version,
      # but apparently some GNU ld's only accept -v.
      # Break only if it was the GNU/non-GNU ld that we prefer.
      if "$ac_cv_path_LD" -v 2>&1 < /dev/null | egrep '(GNU|with BFD)' > /dev/null; then
        test "$pcb_with_gnu_ld" != no && break
      else
        test "$pcb_with_gnu_ld" != yes && break
      fi
    fi
  done
  IFS="$ac_save_ifs"
else
  ac_cv_path_LD="$LD" # Let the user override the test with a path.
fi])
LD="$ac_cv_path_LD"
if test -n "$LD"; then
  AC_MSG_RESULT($LD)
else
  AC_MSG_RESULT(no)
fi
test -z "$LD" && AC_MSG_ERROR([no acceptable ld found in \$PATH])
AC_PROG_LD_GNU
])

AC_DEFUN([AC_PROG_LD_GNU],
[AC_CACHE_CHECK([if the linker ($LD) is GNU ld], ac_cv_prog_gnu_ld,
[# I'd rather use --version here, but apparently some GNU ld's only accept -v.
if $LD -v 2>&1 </dev/null | egrep '(GNU|with BFD)' 1>&5; then
  ac_cv_prog_gnu_ld=yes
else
  ac_cv_prog_gnu_ld=no
fi])
pcb_with_gnu_ld=$ac_cv_prog_gnu_ld
])

AC_PROG_LD

dnl --- Tcl/Tk extension stuff.  Enable Tcl/Tk by default

pcb_with_tcl="yes"
pcb_with_tk="yes"
pcb_with_tcl_includes=""
pcb_with_tk_includes=""
pcb_with_tcl_libraries=""
pbc_with_tk_libraries=""

AC_ARG_WITH(tcl,
[  --with-tcl=DIR          Find tclConfig.sh in DIR], [
  pcb_with_tcl=$withval
  if test "$withval" == "no" -o "$withval" == "NO"; then
      pcb_with_tcl=""
  fi
], ) 

dnl -----------------------------------------------------------------------
dnl If TCL is not disabled, do our best to find it
dnl -----------------------------------------------------------------------

AC_ARG_WITH(tk,		[  --with-tk=DIR           Find tkConfig.sh in DIR],
  pcb_with_tk=$withval)
AC_ARG_WITH(tclincls,   [  --with-tclincls=DIR     Find tcl.h in DIR],
  pcb_with_tcl_includes=$withval)
AC_ARG_WITH(tkincls,	[  --with-tkincls=DIR      Find tk.h in DIR],
  pcb_with_tk_includes=$withval)
AC_ARG_WITH(tcllibs,	[  --with-tcllibs=DIR      Find Tcl library in DIR],
  pcb_with_tcl_libraries=$withval)
AC_ARG_WITH(tklibs,	[  --with-tklibs=DIR       Find Tk library in DIR],
  pcb_with_tk_libraries=$withval)

# -----------------------------------------------------------------------
#
#	Find the Tcl build configuration file "tclConfig.sh"
#
# -----------------------------------------------------------------------

if test "x$pcb_with_tcl" != "x" ; then

  TCL_INC_DIR="."
  TK_INC_DIR="."


  AC_MSG_CHECKING([for tclConfig.sh])
  tcl_config_sh=""

  if test "$pcb_with_tcl" = "no" ; then
    pcb_with_tcl=""
  elif test "$pcb_with_tcl" != "yes" ; then
    #
    # Verify that a tclConfig.sh file exists in the directory specified 
    # by --with-tcl. 
    #
    for dir in \
     $pcb_with_tcl 
    do
      if test -r "$dir/tclConfig.sh" ; then
        tcl_config_sh="$dir/tclConfig.sh"
        break
      elif test -r "$dir/lib/tclConfig.sh" ; then
        tcl_config_sh="$dir/lib/tclConfig.sh"
        break
      elif test -r "$dir/unix/tclConfig.sh" ; then
        tcl_config_sh="$dir/unix/tclConfig.sh"
        break
      fi
    done
  else 
    #
    # Otherwise, search for Tcl configuration file.  
    #

    #  1. Search previously named locations.

    for dir in \
     $prefix \
     $exec_prefix
    do
      if test -r "$dir/tclConfig.sh" ; then
        tcl_config_sh="$dir/tclConfig.sh"
        break
      elif test -r "$dir/lib/tclConfig.sh" ; then
        tcl_config_sh="$dir/lib/tclConfig.sh"
        break
      elif test -r "$dir/unix/tclConfig.sh" ; then
        tcl_config_sh="$dir/unix/tclConfig.sh"
        break
      fi
    done

    #  2. Search standard locations.

    if test "x$tcl_config_sh" = "x" ; then
      for dir in \
       `ls -dr /usr/local/tcl/tcl[[7-9]].[[0-9]]* 2>/dev/null` \
       /usr/local/tcl \
       /usr/local/lib \
       /usr/local \
       /usr/lib \
       /usr/lib64 \
       /sw/lib \
       /usr
      do
        if test -r "$dir/tclConfig.sh" ; then
	  tcl_config_sh="$dir/tclConfig.sh"
	  break
        elif test -r "$dir/lib/tclConfig.sh" ; then
	  tcl_config_sh="$dir/lib/tclConfig.sh"
	  break
        fi
      done
    fi
  fi

  AC_MSG_RESULT([${tcl_config_sh}])

  if test "x$tcl_config_sh" = "x" ; then
    echo "can't find Tcl configuration script \"tclConfig.sh\""
    echo "Reverting to non-Tcl compilation"
    pcb_with_tcl=""
  fi
fi

# -----------------------------------------------------------------------
#
#	Find the Tk build configuration file "tkConfig.sh"
#
# -----------------------------------------------------------------------

if test "x$pcb_with_tcl" != "x" ; then
  AC_MSG_CHECKING([for tkConfig.sh])
  tk_config_sh=""
  if test "$pcb_with_tk" != "yes"; then
    #
    # Verify that a tkConfig.sh file exists in the directory specified 
    # by --with-tcl or --with-tk. 
    #
    for dir in \
     $pcb_with_tk \
     $pcb_with_tcl 
    do
      if test -r "$dir/tkConfig.sh" ; then
        tk_config_sh="$dir/tkConfig.sh"
        break
      elif test -r "$dir/lib/tkConfig.sh" ; then
        tk_config_sh="$dir/lib/tkConfig.sh"
        break
      elif test -r "$dir/unix/tkConfig.sh" ; then
        tk_config_sh="$dir/unix/tkConfig.sh"
        break
      fi
    done
  else 
    #
    # Search for Tk configuration file.  
    #

    #
    #  1. Search previously named locations.
    # 
    for dir in \
     $prefix \
     $exec_prefix
    do
      if test -r "$dir/tkConfig.sh" ; then
        tk_config_sh="$dir/tkConfig.sh"
        break
      elif test -r "$dir/lib/tkConfig.sh" ; then
        tk_config_sh="$dir/lib/tkConfig.sh"
        break
      elif test -r "$dir/unix/tkConfig.sh" ; then
        tk_config_sh="$dir/unix/tkConfig.sh"
        break
      fi
    done
    #
    #  2. Search standard locations.
    # 
    if test "x$tk_config_sh" = "x" ; then
      for dir in \
       /usr/local \
       /usr/lib \
       /usr/local/lib \
       /usr/lib64 \
       /usr \
       `ls -dr /usr/local/lib/tk[[7-9]].[[0-9]]* 2>/dev/null` \
       `ls -dr /usr/lib/tk[[7-9]].[[0-9]]* 2>/dev/null` \
       `ls -dr /usr/lib64/tk[[7-9]].[[0-9]]* 2>/dev/null` \
       `ls -dr /usr/local/tk/tk[[7-9]].[[0-9]]* 2>/dev/null` \
       `ls -dr /usr/local/tk[[7-9]].[[0-9]]* 2>/dev/null` \
	/usr/local/tk \
       `ls -dr /usr/local/lib/tcl[[7-9]].[[0-9]]* 2>/dev/null` \
       `ls -dr /usr/lib/tcl/tk[[7-9]].[[0-9]]* 2>/dev/null` \
       `ls -dr /usr/local/tcl/tcl[[7-9]].[[0-9]]* 2>/dev/null` \
       /sw/lib \
       ${x_libraries}
      do
        if test -r "$dir/tkConfig.sh" ; then
	  tk_config_sh="$dir/tkConfig.sh"
	  break
        elif test -r "$dir/lib/tkConfig.sh" ; then
	  tk_config_sh="$dir/lib/tkConfig.sh"
	  break
        fi
      done
    fi
  fi
  AC_MSG_RESULT([${tk_config_sh}])

  if test "x$tk_config_sh" = "x" ; then
    echo "can't find Tk configuration script \"tkConfig.sh\""
    echo "Reverting to non-Tcl compilation"
    pcb_with_tcl=""
  fi
fi

# -----------------------------------------------------------------------
#
#	Source in the Tcl/Tk configuration scripts.  
#
#	BTW: Does anyone actually use the tclConfig.sh and tkConfig.sh 
#	scripts? They seem to be always missing something.  For example, 
#	why don't they say where to find the Tcl and Tk header files?
#
#       For example, give me the library directory paths separately 
#	(I can't easily parse "-Ldir", but I can always paste a -L 
#	on the front).  If I want to specify equivalent of the -R flag, 
#	I need the build a path of the library directories. Such as
#
#	   "-R $(BLTLIB):$(TKLIB):$(TCLLIB):($XLIB):$(JPEGLIB)"
#
# -----------------------------------------------------------------------

if test "x$pcb_with_tcl" != "x" ; then
  . $tcl_config_sh
  . $tk_config_sh

  if test "$TCL_VERSION" = "7.6" -a "$TK_VERSION" = "4.2" ; then
    :
  elif test "$TCL_VERSION" = "7.5" -a "$TK_VERSION" = "4.1" ; then
    :
  elif test "$TCL_VERSION" = "$TK_VERSION" ; then
    :
  else
    echo "Mismatched Tcl/Tk versions ($TCL_VERSION != $TK_VERSION)"
    echo "Reverting to non-Tcl compilation"
    pcb_with_tcl=""
  fi
fi

if test "x$pcb_with_tcl" != "x" ; then
  if test "x${pcb_with_tcl_includes}" != "x" ; then
    if test -r "${pcb_with_tcl_includes}/tcl.h" ; then
      TCL_INC_DIR=${pcb_with_tcl_includes}
    elif test "x${TCL_INCLUDE_SPEC}" == "x" ; then
      echo "Can't find tcl.h in \"${pcb_with_tcl_includes}\""
      echo "Reverting to non-Tcl compilation"
      pcb_with_tcl=""
    fi
  elif test "x${TCL_INCLUDE_SPEC}" == "x" ; then
    for dir in \
     ${TCL_PREFIX}/include/tcl${TCL_MAJOR_VERSION}.${TCL_MINOR_VERSION} \
     ${TCL_PREFIX}/include \
     ${TCL_SRC_DIR}/generic \
     ${TCL_INC_DIR}
    do
      if test -r "$dir/tcl.h" ; then
        TCL_INC_DIR=$dir
        break
      fi
    done
    if test "x${TCL_INC_DIR}" = "x" ; then
      echo "Can't find tcl.h header file"
      echo "Reverting to non-Tcl compilation"
      pcb_with_tcl=""
    fi
  else
    TCL_INC_DIR=`echo ${TCL_INCLUDE_SPEC} | sed -e '/-I/s/-I//'`
  fi
fi

if test "x$pcb_with_tcl" != "x" ; then
  if test "x${pcb_with_tk_includes}" != "x" ; then
    if test -r "${pcb_with_tk_includes}/tk.h" ; then
      TK_INC_DIR=${pcb_with_tk_includes}
    else
      echo "Can't find tk.h in \"${pcb_with_tk_includes}\""
      echo "Reverting to non-Tcl compilation"
      pcb_with_tcl=""
    fi
  else
    for dir in \
     ${TK_PREFIX}/include/tk${TK_MAJOR_VERSION}.${TK_MINOR_VERSION} \
     ${TK_PREFIX}/include \
     ${TK_SRC_DIR}/generic \
     ${TK_INC_DIR} \
     ${TCL_INC_DIR}
    do
      if test -r "$dir/tk.h" ; then
        TK_INC_DIR=$dir
        break
      fi
    done
    if test "x${TK_INC_DIR}" = "x" ; then
      echo "Can't find tk.h header file"
      echo "Reverting to non-Tcl compilation"
      pcb_with_tcl=""
    fi
  fi
fi

if test "x$pcb_with_tcl" != "x" ; then
  case $host in
    *-sunos4*|*-*-netbsd|NetBSD-*|FreeBSD-*|OpenBSD-*)
      TCL_LIB_NAME="tcl${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION}"
      TK_LIB_NAME="tk${TK_MAJOR_VERSION}${TK_MINOR_VERSION}"
      ;;
    *)
      TCL_LIB_NAME="tcl${TCL_MAJOR_VERSION}.${TCL_MINOR_VERSION}"
      TK_LIB_NAME="tk${TK_MAJOR_VERSION}.${TK_MINOR_VERSION}"
      ;;
  esac

  TCL_LIB_SPEC="-l${TCL_LIB_NAME}"
  TK_LIB_SPEC="-l${TK_LIB_NAME}"

# We really ought to run "ldd" to confirm that the linked libraries match.

  AC_MSG_CHECKING([for wish executable])
  for dir in \
   ${TK_EXEC_PREFIX}/bin \
   ${TK_EXEC_PREFIX}
  do
    for wishexe in \
      wish \
      wish${TK_VERSION} \
      wish.exe \
      wish${TK_VERSION}.exe
    do
      if test -r "$dir/$wishexe" ; then
        WISH_EXE=$dir/$wishexe
        break
      fi
    done
    if test "x${WISH_EXE}" != "x" ; then
      break
    fi
  done
  if test "x${WISH_EXE}" = "x" ; then
    echo "Warning: Can't find executable for \"wish\".  You may have to"
    echo "manually set the value for WISH_EXE in the pcb startup script."
    AC_MSG_RESULT(no)
  else
    AC_MSG_RESULT([${WISH_EXE}])
  fi

# Have to define SHDLIB_EXT here even though we have to do it below, too.
  case $host in
     *-hpux*)
	SHDLIB_EXT=".sl"
	;;
     *cygwin*)
	SHDLIB_EXT=".dll"
	;;
     *-darwin*)
	SHDLIB_EXT=".dylib"
	;;
     *-openbsd*) 
        SHDLIB_EXT=".so.${TCL_LIB_FILE##*.so.}"
        ;;
     *)
	SHDLIB_EXT=".so"
	;;
  esac

  if test "x${pcb_with_tcl_libraries}" != "x" ; then
    for libname in \
     "${pcb_with_tcl_libraries}/lib${TCL_LIB_NAME}${SHDLIB_EXT}"
    do
      if test -r "$libname" ; then
        TCL_LIB_DIR="${pcb_with_tcl_libraries}"
        break
      fi
    done

    if test "x${TCL_LIB_DIR}" = "x" ; then
      echo "Can't find tcl library in \"${pcb_with_tcl_libraries}\""
      echo "Reverting to non-Tcl compile"
      pcb_with_tcl=""
    fi
  else
    for libname in \
     "${TCL_EXEC_PREFIX}/lib/lib${TCL_LIB_NAME}${SHDLIB_EXT}" \
     "${TCL_EXEC_PREFIX}/lib64/lib${TCL_LIB_NAME}${SHDLIB_EXT}"
    do
      if test -r "$libname" ; then
        TCL_LIB_DIR="${TCL_EXEC_PREFIX}/lib"
        break
      fi
    done
    if test "x${TCL_LIB_DIR}" = "x" ; then
      echo "Can't find tcl library"
      echo "Reverting to non-Tcl compile"
      pcb_with_tcl=""
    fi
  fi
fi

if test "x$pcb_with_tcl" != "x" ; then
  if test "x${pcb_with_tk_libraries}" != "x" ; then
    for libname in \
     "${pcb_with_tk_libraries}/lib${TK_LIB_NAME}${SHDLIB_EXT}"
    do
      if test -r "$libname" ; then
        TK_LIB_DIR="${pcb_with_tk_libraries}"
        break
      fi
    done
    if test "x${TK_LIB_DIR}" = "x" ; then
      echo "Can't find tk library in \"${pcb_with_tk_libraries}\""
      echo "Reverting to non-Tcl compile"
      pcb_with_tcl=""
    fi
  else
    for libname in \
     "${TK_EXEC_PREFIX}/lib/lib${TK_LIB_NAME}${SHDLIB_EXT}" \
     "${TK_EXEC_PREFIX}/lib64/lib${TK_LIB_NAME}${SHDLIB_EXT}"
    do
      if test -r "$libname" ; then
        TK_LIB_DIR="${TK_EXEC_PREFIX}/lib"
        break
      fi
    done
    if test "x${TK_LIB_DIR}" = "x" ; then
      echo "Can't find tk library"
      echo "Reverting to non-Tcl compile"
      pcb_with_tcl=""
    fi
  fi
fi

dnl --------------------------------
dnl End of Tcl/Tk search
dnl --------------------------------

# -----------------------------------------------------------------------
# 
# Tk libraries and header files
#
# -----------------------------------------------------------------------

if test "x$pcb_with_tcl" != "x" ; then
  if test "${TK_INC_DIR}" != "/usr/include" -a \
	"${TK_INC_DIR}" != "${TCL_INC_DIR}"; then
    INC_SPECS="${INC_SPECS} -I${TK_INC_DIR}"
    CFLAGS="${INC_SPECS} ${CFLAGS}"
  fi
  if test "${TK_LIB_DIR}" = "/usr/lib" -o \
          "${TK_LIB_DIR}" = "/usr/lib64" ; then
    LIB_SPECS="${TK_LIB_SPEC}"
  else
    LIB_SPECS="-L${TK_LIB_DIR} ${TK_LIB_SPEC}"
    if test "x${loader_run_path}" = "x" ; then
      loader_run_path="${TK_LIB_DIR}"
    else
      loader_run_path="${TK_LIB_DIR}:${loader_run_path}"
    fi
  fi

# -----------------------------------------------------------------------
# 
# Tcl libraries and header files
#
# Add a header file directory specification only if the Tcl headers reside 
# in a different directory from Tk's.  
#
#
# -----------------------------------------------------------------------
  if test "x${TCL_INCLUDE_SPEC}" != "x" ; then
    INC_SPECS="${INC_SPECS} ${TCL_INCLUDE_SPEC}"
  elif test "${TCL_INC_DIR}" != "/usr/include" ; then
    INC_SPECS="${INC_SPECS} -I${TCL_INC_DIR}"
  fi

  if test "${TCL_LIB_DIR}" = "/usr/lib" -o \
          "${TCL_LIB_DIR}" = "/usr/lib64" -o \
          "${TCL_LIB_DIR}" = "${TK_LIB_DIR}" ; then
    LIB_SPECS="${LIB_SPECS} ${TCL_LIB_SPEC}"
  else
    LIB_SPECS="${LIB_SPECS} -L${TCL_LIB_DIR} ${TCL_LIB_SPEC}"
    if test "x${loader_run_path}" = "x" ; then
      loader_run_path="${TCL_LIB_DIR}"
    else
      loader_run_path="${TCL_LIB_DIR}:${loader_run_path}"
    fi
  fi

#--------------------------------------------------------------------
#
# Check if we can generate shared libraries on this system.  Set flags 
# to generate shared libraries for systems that we know about.  Start 
# with the values found in tclConfig.sh, make changes as we know about 
# the different systems.
#
#--------------------------------------------------------------------

# Initialize shared library build variables

  LDDL_FLAGS="-shared"
  SHDLIB_EXT=".so"
  LD_RUN_PATH=""
  EXTRA_LIB_SPECS=""
  TCLSUBDIR="tcltk"

  build_shared="yes"

  case $host in
    *-aix4.[[2-9]]*)
      # No Position-Independent flags needed
    
      # Use the installed export file or the one found in the source directory.

      if test -r "${TCL_LIB_DIR}/lib${TCL_LIB_NAME}.exp" ; then
        tcl_exp="${TCL_LIB_DIR}/lib${TCL_LIB_NAME}.exp"
      else
        tcl_exp="${TCL_SRC_DIR}/unix/lib.exp"
      fi
      if test -r "${TK_LIB_DIR}/lib${TK_LIB_NAME}.exp" ; then
        tk_exp="${TK_LIB_DIR}/lib${TK_LIB_NAME}.exp"
      else
        tk_exp="${TK_SRC_DIR}/unix/lib.exp"
      fi
    
      full_src_path=`cd ${srcdir}; pwd`

      # Use shell-script to link shared library
      SHLIB_LD="${full_src_path}/cf/ldAix /bin/ld -bhalt:4 -bM:SRE -bE:lib.exp -H512 -T512 -bnoentry -bI:${tk_exp} -bI:${tcl_exp}"

      SHLIB_LIB_SPECS="${aix_lib_specs} -lc"

      LDFLAGS="-L${loader_run_path}"
      EXTRA_LIB_SPECS="-ldl"
      ;;
  
    *-aix*)
      # No Position-Independent flags needed
    
      # Use the installed export file or the one found in the source directory.

      if test -r "${TCL_LIB_DIR}/lib${TCL_LIB_NAME}.exp" ; then
        tcl_exp="${TCL_LIB_DIR}/lib${TCL_LIB_NAME}.exp"
      else
        tcl_exp="${TCL_SRC_DIR}/unix/lib.exp"
      fi
      if test -r "${TK_LIB_DIR}/lib${TK_LIB_NAME}.exp" ; then
        tk_exp="${TK_LIB_DIR}/lib${TK_LIB_NAME}.exp"
      else
        tk_exp="${TK_SRC_DIR}/unix/lib.exp"
      fi

      full_src_path=`cd ${srcdir}/cf; pwd`

      # Use shell-script to link shared library

      SHLIB_LD="${full_src_path}/ldAix /bin/ld -bhalt:4 -bM:SRE -bE:lib.exp -H512 -T512 -bnoentry -bI:${tk_exp} -bI:${tcl_exp}"

      SHLIB_LIB_SPECS="${aix_lib_specs} -lc"
    
      LDFLAGS="-L${loader_run_path}"
      EXTRA_LIB_SPECS="-lld"
      ;;
  
    *-bsdi2*|*-bsdi3*)
      SHLIB_LD="shlicc"
      LDDL_FLAGS="-r"

      EXTRA_LIB_SPECS="-ldl"
      ;;

    *-darwin*)
      SHDLIB_EXT=".dylib"
      if test "${pcb_with_gnu_ld}" = "yes" ; then
	 LDDL_FLAGS="-dynamiclib -bind_at_load"
      else
	 LDDL_FLAGS="-dynamiclib -flat_namespace -undefined suppress -noprebind"
	 SHLIB_CFLAGS="-I/sw/include -fno-common"
      fi
      LDFLAGS=${LDFLAGS} ${LIB_SPEC}
      ;;

    *cygwin*)
      AC_DEFINE([CYGWIN], [], [System is Cygwin])
      SHDLIB_EXT=".dll"
      LDDL_FLAGS="-shared -Wl,--enable-auto-image-base"
      SHLIB_LIB_SPECS="${LIB_SPECS}"
      ;;

    *-bsdi4*)
      SHLIB_CFLAGS="-export-dynamic -fPIC"
      SHLIB_LD="${CC}"
      LDDL_FLAGS='-shared -Wl,-E -Wl,-soname,$@'
      ;;

    *-dgux*)
      SHLIB_CFLAGS="-K PIC"
      SHLIB_LD="cc"
      LDDL_FLAGS="-G"
    
      EXTRA_LIB_SPECS="-ldl"
      ;;
  
    *-hpux*)
      if test "$GCC" = "no" ; then
        DEFINES="$DEFINES -D_HPUX_SOURCE"
      fi
      AC_CHECK_LIB(dld, shl_load, [found=yes], [found=no])
      if test "${found}" = "yes" ; then
        SHLIB_CFLAGS="+z"
        SHLIB_LD="ld"
        LDDL_FLAGS="-b -E -n +s +b,${loader_run_path}:."
        SHDLIB_EXT=".sl"

        # The run path is included in both LDFLAGS and LDDL_FLAGS
        # because SHLIB_LD is ld and LD is cc/gcc.

        LDFLAGS="-Wl,-E -Wl,+s,+b,${loader_run_path}:."
        EXTRA_LIB_SPECS="-ldld"
      fi
      ;;
  
    *-irix64-6.5*)
      SHLIB_LD="ld"
      LDDL_FLAGS="-32 -shared -rdata_shared"
      if test "x${loader_run_path}" != "x" ; then
        LD_RUN_PATH="-Wl,-rpath,${loader_run_path}"
      fi
      ;;

    *-irix-[56].*|*-irix64-*)
      SHLIB_LD="ld"
      LDDL_FLAGS="-shared -rdata_shared"
      if test "x${loader_run_path}" != "x" ; then
        LD_RUN_PATH="-Wl,-rpath,${loader_run_path}"
      fi
      LDFLAGS=""
      if test "$GCC" = "yes" ; then
        SHLIB_CFLAGS="-mabi=n32 $SHLIB_CFLAGS"
        LDDL_FLAGS="-mabi=n32 $LDDL_FLAGS"
        LDFLAGS="-mabi=n32 $LDFLAGS"
      else	
        CFLAGS="-n32 $CFLAGS"
        LDFLAGS="-n32 $LDFLAGS"
      fi
      ;;
    
    *-linux*)
      SHLIB_CFLAGS="-fPIC"
      SHLIB_LD="${CC}"
      LDDL_FLAGS='-shared -Wl,-soname,$@'
      if test "x${loader_run_path}" != "x" ; then
        LD_RUN_PATH="-Wl,-rpath,${loader_run_path}"
      fi
      EXTRA_LIB_SPECS="-ldl"

      case $host in
	 x86_64-*)
	    # AMD 64-bit archictecture
	    # Add -fPIC to standard compile switch
	    CFLAGS="${CFLAGS} -fPIC"
	    ;;
      esac
      ;;
  
    *-mp-ras-02*)
      SHLIB_CFLAGS="-G -K PIC"
      SHLIB_LD="${CC}"
      LDDL_FLAGS=""
      ;;

    *-mp-ras-*)
      SHLIB_CFLAGS="-G -K PIC"
      SHLIB_LD="${CC}"
      LDDL_FLAGS="-Wl,-Bexport"
      ;;

    *-ncr-sysv4-*2*)
      SHLIB_CFLAGS="-K PIC"
      SHLIB_LD="cc"
      LDDL_FLAGS="-G"

      EXTRA_LIB_SPECS="-ldl"
      ;;

    *-ncr-sysv4*)
      SHLIB_CFLAGS="-K PIC"
      SHLIB_LD="cc"
      LDDL_FLAGS="-G -Wl,-Bexport"

      LDFLAGS="-Wl,-Bexport"
      EXTRA_LIB_SPECS="-ldl"
      ;;

    *-netbsd*|*-freebsd*|*-openbsd*)
      # Not available on all versions:  check for include file.
      AC_CHECK_HEADER(dlfcn.h, test_ok=yes, test_ok=no)
      if test "$test_ok" = yes; then
        SHLIB_CFLAGS="-fpic"
        SHLIB_LD="ld"
        LDDL_FLAGS="-Bshareable -x ${LIB_SPEC}"
      fi
      ;;

    *-nextstep*)
      SHLIB_LD="cc"
      LDDL_FLAGS="-nostdlib -r"
      ;;

    *-osf1-1.[012]*)
      # OSF/1 1.[012] from OSF, and derivatives, including Paragon OSF/1

      # Warning: Ugly Makefile Hack 
      #	Make package name same as library name

      SHLIB_LD='ld -R -export $@:'
      ;;

    *-osf1-1.*)
      # OSF/1 1.3 from OSF using ELF, and derivatives, including AD2

      SHLIB_CFLAGS="-fpic"
      SHLIB_LD="ld -shared"
      ;;

    *-osf1V*)
      # Digital OSF/1

      SHLIB_LD='ld'
      LDDL_FLAGS='-shared -expect_unresolved "*"'
      if test "x${loader_run_path}" != "x" ; then
        LD_RUN_PATH="-Wl,-rpath,${loader_run_path}"
      fi
      LDFLAGS=""
      ;;

    *-sco*)
      # Note, dlopen is available only on SCO 3.2.5 and greater.  However,
      # this test works, since "uname -s" was non-standard in 3.2.4 and
      # below.

      SHLIB_CFLAGS="-Kpic -belf"
      SHLIB_LD="ld"
      LDDL_FLAGS="-G"
      LDFLAGS="-belf -Wl,-Bexport"
      ;;

    *-sni-sysv*)

      SHLIB_CFLAGS="-K PIC"
      SHLIB_LD="cc"
      LDDL_FLAGS="-G"

      EXTRA_LIB_SPECS="-ldl"
      ;;

    *-sunos4*)

      SHLIB_CFLAGS="-PIC"
      SHLIB_LD="ld"
      LDDL_FLAGS="-assert pure-text"

      EXTRA_LIB_SPECS="-ldl"
      ;;

    *-solaris2*)

      if test "${pcb_with_gnu_ld}" = "yes" ; then
        LDDL_FLAGS='-rdynamic -shared -Wl,-E -Wl,-soname,$@'
        if test "x${loader_run_path}" != "x" ; then
          LD_RUN_PATH="-Wl,-rpath,${loader_run_path}"
        fi
      else
        LDDL_FLAGS="-shared -mimpure-text"
        if test "x${loader_run_path}" != "x" ; then
          LD_RUN_PATH="-R ${loader_run_path}"
        fi
      fi
      EXTRA_LIB_SPECS="-ldl"
      SHLIB_LIB_SPECS="${LIB_SPECS}"
    ;;

    *-mips-dde-sysv*)

      SHLIB_CFLAGS="-KPIC"
      SHLIB_LD="cc"
      LDDL_FLAGS="-G"

      EXTRA_LIB_SPECS="-ldl"
      ;;

    *-pc-sysv4* | *-unixware-5*)
      SHLIB_CFLAGS="-G -KPIC"
      SHLIB_LD="${CC}"
      LDDL_FLAGS=" -Wl,-Bexport"
      ;;
  
  esac

# If we're running gcc, then set SHLIB_CFLAGS flags for compiling
# shared libraries for gcc, instead of those of the vendor's
# compiler.

  if test "$GCC" = "yes" ; then
#   SHLIB_CFLAGS="${SHLIB_CFLAGS} -fPIC"
    CFLAGS="-fPIC ${CFLAGS}"
  fi
  if test "${pcb_with_gnu_ld}" = "yes" ; then
    LDDL_FLAGS="${LDDL_FLAGS} -Wl,--version-script=symbol.map"
  fi

  AC_SUBST(SHDLIB_EXT)
  AC_SUBST(SHLIB_LD)
  AC_SUBST(LDDL_FLAGS)
  AC_SUBST(SHLIB_LIB_SPECS)
  AC_SUBST(EXTRA_LIB_SPECS)
  AC_SUBST(LD_RUN_PATH)
  AC_SUBST(LDFLAGS)
  AC_SUBST(INC_SPECS)
  AC_SUBST(LIB_SPECS)
  AC_SUBST(TCL_LIB_DIR)
  AC_SUBST(WISH_EXE)

  AC_SUBST(TCLSUBDIR)

  AC_DEFINE([TCL_PCB], [], [Compiling PCB as a Tcl extension])
  PCBBIN=""
fi

AC_SUBST(PCBBIN)


# ------------- dmalloc -------------------
dnl dmalloc checks
AC_MSG_CHECKING([if dmalloc debugging should be enabled])
AC_ARG_ENABLE([dmalloc],
[  --enable-dmalloc        Compile and link with dmalloc for malloc debugging [default=no]],
[
if test "X$enable_dmalloc" != "Xno" ; then
	AC_MSG_RESULT([yes])
	AC_CHECK_HEADER(dmalloc.h,,
		AC_ERROR([You have requested dmalloc debugging but dmalloc.h could not be found]))
	AC_CHECK_LIB(dmalloc,main,,
		AC_ERROR([You have requested dmalloc debugging but -ldmalloc could not be found]), $X_LIBS)
	DMALLOC_LIBS="-ldmalloc"
else
	AC_MSG_RESULT([no])
	DMALLOC_LIBS=""
fi
],
[
	AC_MSG_RESULT([no])
	DMALLOC_LIBS=""
])

# ------------- ElectricFence -------------------
dnl ElectricFence checks
AC_MSG_CHECKING([if ElectricFence debugging should be enabled])
AC_ARG_ENABLE([efence],
[  --enable-efence         Link with ElectricFence for malloc debugging [default=no]],
[
if test "X$enable_efence" != "Xno" ; then
	AC_MSG_RESULT([yes])
	AC_CHECK_LIB(efence,main,,
		AC_ERROR([You have requested ElectricFence debugging but -lefence could not be found]), $X_LIBS)
else
	AC_MSG_RESULT([no])
fi
],
[
AC_MSG_RESULT([no])
])

# XXX fix this!!!
#if [ test "x$prefix" = xNONE ]; then
#	datadir=$ac_default_prefix/share
#else
#	datadir=$prefix/share
#fi

# font filename
FONTFILENAME=${FONTFILENAME:-"default_font"}
AC_SUBST(FONTFILENAME)
AC_DEFINE_UNQUOTED(FONTFILENAME,"$FONTFILENAME",[File for default font])

# directory for old-style library and for fonts
PCBLIBDIR=${datadir}/pcb
AC_SUBST(PCBLIBDIR)
#AC_DEFINE_UNQUOTED(PCBLIBDIR,"$PCBLIBDIR",[Library directory])
CPPFLAGS="$CPPFLAGS -DPCBLIBDIR=\\\"$PCBLIBDIR\\\""
CPPFLAGS="$CPPFLAGS -DCAD_DIR=\\\"${datadir}\\\""

AC_SUBST(VERSION)
AC_SUBST(REVISION)
#AC_DEFINE_UNQUOTED(PROG_VERSION, $VERSION, [Program version number])
#AC_DEFINE_UNQUOTED(PROG_REVISION, $REVISION, [Program revision number])
CPPFLAGS="$CPPFLAGS -DPROG_VERSION=\\\"$VERSION\\\""
CPPFLAGS="$CPPFLAGS -DPROG_REVISION=\\\"$REVISION\\\""

# name for old-style library
LIBRARYFILENAME=pcblib
AC_SUBST(LIBRARYFILENAME)
AC_DEFINE_UNQUOTED(LIBRARYFILENAME,"$LIBRARYFILENAME",[library file name])

# name for wish executable
CPPFLAGS="$CPPFLAGS -DWISH_EXE=\\\"$WISH_EXE\\\""

# directory for new library
PCBTREEDIR=${datadir}/pcb/newlib
PCBTREEDIR=${PCBTREEDIR:-"$PCBTREEDIR"}
AC_SUBST(PCBTREEDIR)
#AC_DEFINE_UNQUOTED(PCBTREEDIR,"$PCBLIB",[top directory for new style pcb library])
CPPFLAGS="$CPPFLAGS -DPCBTREEDIR=\\\"$PCBTREEDIR\\\""

# if we have gcc then add -Wall
if test "x$GCC" = "xyes"; then
	if test -z "`echo "$CFLAGS" | grep "\-Wall" 2> /dev/null`" ; then
		CFLAGS="$CFLAGS -Wall"
	fi
fi

CFLAGS="$CFLAGS $X_CFLAGS"
LIBS="$LIBS $X_LIBS $DMALLOC_LIBS"

BTNMOD=${BTNMOD:-"Mod1"}
AC_SUBST(BTNMOD)

AC_OUTPUT(
	Makefile
	README_FILES/Makefile
	doc/Makefile
	example/Makefile
	example/libraries/Makefile
	lib/CreateLibraryContents.sh
	lib/CreateLibrary.sh
	lib/Makefile
	lib/QueryLibrary.sh
	lib/qfp-ui
	newlib/Makefile
	newlib/2_pin_thru-hole_packages/Makefile
	newlib/analog/Makefile
	newlib/connectors/Makefile
	newlib/crystal/Makefile
	newlib/electro-optics/Makefile
	newlib/fuse/Makefile
	newlib/generic_SMD_packages/Makefile
	newlib/headers/Makefile
	newlib/not_vetted/Makefile
	newlib/relays/Makefile
	newlib/switch/Makefile
	newlib/tests/Makefile
	src/Makefile
	src/script/Makefile
	bitmaps/Makefile
	tcltk/Makefile
	tools/Makefile
	tutorial/Makefile
	)
