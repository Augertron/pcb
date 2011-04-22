#!/bin/sh
#
# This script starts pcb under the Tcl interpreter,
# reading commands from a special .wishrc script which
# launches magic and retains the Tcl interactive interpreter.
#

loclibdir=${PCB_LIB_DIR:=PCBLIBDIR}
export PCB_LIB_DIR
PCB_WISH=WISH_EXE
export PCB_WISH

# Hacks for Cygwin
if [ ${TERM:=""} = "cygwin" ]; then
   export PATH=$PATH:TCLLIBDIR
   export DISPLAY=${DISPLAY:=":0"}
fi

TKCON=true
for i in $@ ; do
   case $i in
      -noc*) TKCON=;;
      --help)
	echo "Standard usage:"
	echo "    pcb [filename]"
	echo "Online documentation:"
	echo "    http://opencircuitdesign.com/pcb"
	exit 0
	;;
      --version)
	echo "PCB version PROG_VERSION revision PROG_REVISION"
	exit 0
	;;
   esac
done

if [ $TKCON ]; then

   if [ ! -f ${loclibdir}/console.tcl ]; then
      loclibdir=${loclibdir}/tcl
   fi

   exec ${loclibdir}/tkcon.tcl \
	-`eval' "source ${loclibdir}/console.tcl" \
        -slave "package require Tk; set argc $#; set argv [list $*]; \
        source ${loclibdir}/pcb.tcl"
else

#
# Run the stand-in for wish (pcbexec), which acts exactly like "wish"
# except that it replaces ~/.wishrc with pcb.tcl.  This executable is
# *only* needed when running without the console; the console itself is
# capable of sourcing the startup script.
#
   exec ${loclibdir}/pcbexec -- $@

fi
