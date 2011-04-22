## $Id: Makefile.am,v 1.2 2006/10/23 18:11:36 tim Exp $
##

SUBDIRS=
CP=		@CP@

pcbexecdir=	@PCBLIBDIR@

bin_PROGRAMS= pcb

# BITMAPS= arc_mode.xbm buffer_mode.xbm insert_mode.xbm line_mode.xbm \
# 	lock_mode.xbm pan_mode.xbm polygon_mode.xbm rectangle_mode.xbm \
# 	remove_mode.xbm rotate_mode.xbm select_mode.xbm text_mode.xbm \
# 	thermal_mode.xbm via_mode.xbm

PIXMAPS= arc.gif buf.gif insert.gif line.gif lock.gif pan.gif poly.gif \
	rect.gif del.gif rotate.gif select.gif text.gif therm.gif via.gif

# SRC_BITMAPS=$(BITMAPS:%=$(top_srcdir)/bitmaps/%)

SRC_PIXMAPS=$(PIXMAPS:%=$(top_srcdir)/bitmaps/%)

TCL_SCRIPT_FILES= pcb.tcl pcb_init.tcl console.tcl resource.tcl tkcon.tcl

pcbexec_DATA= tclpcb${SHDLIB_EXT} ${SRC_PIXMAPS}
pcbexec_SCRIPTS= pcbexec pcb ${TCL_SCRIPT_FILES}

AM_CFLAGS = -DNDEBUG

pcb_object_files = tclpcb.o tclcommandAF.o tclcommandGM.o tclcommandNO.o tclcommandPR.o tclcommandSZ.o tkSimple.o
link_OBJECTS = ../src/libpcb.a

# for globalconst.h and autoconf's config.h
INCLUDES=	-I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/bitmaps

all-am: tclpcb${SHDLIB_EXT} pcb.tcl pcbexec pcb

clean-generic:
	$(RM) pcb pcb.tcl tclpcb${SHDLIB_EXT} 

tclpcb${SHDLIB_EXT}: ${pcb_object_files} ${link_OBJECTS}
	$(RM) tclpcb${SHDLiB_EXT}
	$(CC) ${CFLAGS} ${SHLIB_CFLAGS} -o $@ ${LDDL_FLAGS} ${pcb_object_files} \
		${link_OBJECTS} ${SHLIB_LIB_SPECS} ${LDFLAGS} ${X_EXTRA_LIBS} \
		${EXTRA_LIBS} -lc ${LIBS}

pcbexec$(EXEEXT): pcbexec.o
	$(RM) pcbexec$(EXEEXT)
	$(CC) ${CFLAGS} ${CPPFLAGS} ${DEFS} $(INCLUDES) \
		pcbexec.c -o pcbexec$(EXEEXT) ${LIB_SPECS} \
		${LDFLAGS} ${LIBS}

pcb: pcb.sh.m4
	$(M4) $(M4_DEFS) ${CPPFLAGS} pcb.sh.m4 > $@
	chmod 0755 $@

pcb.tcl: pcb.tcl.m4
	$(M4) $(M4_DEFS) ${CPPFLAGS} pcb.tcl.m4 > $@
	chmod 0755 $@

DISTCLEANFILES= pcb.tcl pcb
