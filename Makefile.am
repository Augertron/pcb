## $Id: Makefile.am,v 1.2 2006/06/14 08:04:41 eric Exp $
##
## Top level automake file for PCB

AUTOMAKE_OPTIONS = no-dist

SUBDIRS= src bitmaps @TCLSUBDIR@ lib newlib doc \
    example tools tutorial README_FILES

EXTRA_DIST=	autogen.sh globalconst.h README.cvs

FULL_VERSION = @VERSION@.@REVISION@

DISTCLEANFILES=	configure.lineno \
    pcb.spec pcb-$(FULL_VERSION) pcb-$(FULL_VERSION).tgz

dist:
	$(RM) pcb.spec pcb-$(FULL_VERSION) pcb-$(FULL_VERSION).tgz
	sed -e /%VERSION%/s/%VERSION%/$(FULL_VERSION)/ \
	    pcb.spec.in > pcb.spec
	ln -nsf . pcb-$(FULL_VERSION)
	tar zchvf pcb-$(FULL_VERSION).tgz --exclude CVS \
	    --exclude pcb-$(FULL_VERSION)/pcb-$(FULL_VERSION) \
	    --exclude pcb-$(FULL_VERSION)/pcb-$(FULL_VERSION).tgz \
	    pcb-$(FULL_VERSION)

