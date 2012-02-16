# Wishrc startup for ToolScript (pcb)
#
# load /usr/local/share/pcb/tclpcb.so

set envlibdir [array get env "PCB_LIB_DIR"]
if {$envlibdir != {}} {
   set path [lindex $envlibdir 1]
} else {
   set path "/home/tim/cad/share/pcb"
   set PCB_LIB_DIR $path
}
unset envlibdir

# Rename commands that conflict between PCB and Tcl/Tk.  These MUST be
# handled by the PCB command callback functions!

if {[lsearch [info commands] tcl_grid] < 0} {rename grid tcl_grid}
if {[lsearch [info commands] tcl_option] < 0} {rename option tcl_option}
if {[lsearch [info commands] tcl_text] < 0} {rename text tcl_text}
if {[lsearch [info commands] tcl_scan] < 0} {rename scan tcl_scan}
if {[lsearch [info commands] tcl_place] < 0} {rename place tcl_place}

set tcllibdir $path
if {![file exists ${tcllibdir}/tclpcb.so]} {
   set tcllibdir ${path}/tcl
}
if {![file exists ${tcllibdir}/tclpcb.so]} {
   puts stderr "Error:  Cannot find file tclpcb.so"
}
load ${tcllibdir}/tclpcb.so

package require Pcb

# It is important to make sure no magic commands overlap with Tcl built-in
# commands, because otherwise the namespace import will fail.

proc pushnamespace { name } {

   set y [namespace eval ${name} info commands ::${name}::*]
   set z [info commands]

   foreach v $y {
      regsub -all {\*} $v {\\*} i
      set x [namespace tail $i]
      if {[lsearch $z $x] < 0} { 
	 namespace import $i                       
      } else {
	 puts "Warning: ${name} command '$x' use fully-qualified name '$v'"
      }
   }
}

proc popnamespace { name } {
   set z [info commands]
   set l [expr [string length ${name}] + 5]

   while {[set v [lsearch $z ${name}_tcl_*]] >= 0} {
      set y [lindex $z $v]
      set w [string range $y $l end]
      interp alias {} ::$w {}
      rename ::$y ::$w
      puts "Info: replacing ::$w with ::$y"
   }
   namespace forget ::${name}::*
}

set auto_noexec 1	;# don't EVER call UNIX commands w/o "shell" in front

# Have we called pcb from tkcon?

if {[lsearch [interp aliases] tkcon] != -1} {

   # Rename conflicting TkCon commands before pushing pcb's namespace
   # (None for PCB. . .)

} else {

   # Extend the "unknown" command-line handler to be compatible with the
   # way we have renamed certain core Tcl/Tk functions.

   rename unknown tcl_unknown
   proc unknown { args } {
      # CAD tools special:
      # Check for commands which were renamed to tcl_(command)

      set cmd [lindex $args 0]
      if {[lsearch [info commands] tcl_$cmd] >= 0} {
         set arglist [concat tcl_$cmd [lrange $args 1 end]]
         set ret [catch {eval $arglist} result]
         if {$ret == 0} {
            return $result
         } else {
            return -code $ret -errorcode $errorCode $result
         }
      }
      return [eval [concat tcl_unknown $args]]
   }
}

# Allow commands in the pcb namespace to be called from the toplevel namespace

pushnamespace pcb

# Source the default resources file

source ${tcllibdir}/resource.tcl

# Sourcing of individual resource overrides

set envhomedir [array get env "HOME"]
if {$envhomedir != {}} {
   set homepath [lindex $envhomedir 1]
   set homedefaults ${homepath}/.Xdefaults
   if {[file exists $homedefaults]} {option readfile $homedefaults userDefault}
   unset homedefaults
   unset homepath
}
unset envhomedir

# Source the GUI-generating script

source ${tcllibdir}/pcb_init.tcl
unset tcllibdir

# Parse the argument list for "-noc[console]" and remove it from the args list

set argafter {}

for {set i 0} {$i < $argc} {incr i 1} {
   set x [lindex $argv $i]
   switch -regexp -- $x {
      ^-n* {
      }
      default {
	 lappend argafter $x
      }
   }
}

set argc [llength $argafter]

# Run the pcb start function

if {[string length $argafter] == 0} {
   pcb::start
} else {
   pcb::start ${argafter}
}

unset argafter

# Pcb start function drops back to interpreter after initialization & setup

if {[string range [wm title .] 0 3] == "wish"} {
   wm withdraw .
}
if {[string range [wm title .] 0 4] == "tkcon"} {
   wm withdraw .
}
if {[string range [wm title .] 0 6] == "pcbexec"} {
   wm withdraw .
}

