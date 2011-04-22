# Commands to run in the console prior to launching the
# slave interpreter for PCB
#
# Since the TkCon window is not a necessity in PCB, we withdraw it
# but set up commands which can be executed from the slave interpreter
# that can be used to reinstate the console when necessary (such as
# when the "%" key macro is typed).
#

wm protocol . WM_DELETE_WINDOW {tkcon slave slave set consoleView off; \
    wm withdraw .}

proc replaceprompt { ch } {
    tkcon set ::tkcon::OPT(prompt1) "$ch "
    .text delete prompt.last-2c
    .text insert [.text index prompt.last-1c] $ch {prompt}
    .text mark set output [.text index output-1c]
}

proc gettext {{prompt {}} {response {}}} {

   # Differentiate between console command-line and button execution
   # (Determines whether to prompt before or after execution)
   if {[.text index output] == [.text index end]} {
      ::tkcon::Prompt
      set mode 1
   } else {
      set mode 0
   }

   replaceprompt "?"
   if {$prompt != {}} {
      .text insert end $prompt {stdout}
   }
   if {$response != {}} {
      set result [tkcon congets $response]
   } else {
      set result [tkcon congets]
   }
   tkcon set ::tkcon::OPT(prompt1) "% "
   if {$mode == 0} {
      ::tkcon::Prompt
   } else {
      .text mark set output end
      tkcon console see output  ;# adjust view to encompass newline
      update idletasks
   }
   return $result
}

slave alias pcb::consoledown wm withdraw .
slave alias pcb::consoleup wm deiconify .
slave alias pcb::dialog gettext
