#!/usr/bin/wish
#-------------------------------------------------------------------------------
# pcb_init.tcl
# Tk-based GUI for PCB
# Copyright 2004, 2005 Paramesh Santanam, MultiGiG, Inc.,
#                      Tim Edwards, MultiGiG, Inc., Nishit Patel
#-------------------------------------------------------------------------------

# Title for the main window
toplevel .pcb
wm title .pcb PCB
wm minsize .pcb 100 75
wm protocol .pcb WM_DELETE_WINDOW {quit}

# font to use for menus and buttons
set buttonFont "-adobe-helvetica-normal-r-*-*-8-*-*-*-*-*-*-*"
set buttonBoldFont "-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*"
set menuFont "-adobe-helvetica-normal-r-*-*-8-*-*-*-*-*-*-*"
set textFont "-misc-fixed-normal-r-*-*-10-*-*-*-*-*-*-*"


# Define menu, commands, shortcuts
#
# file menu
#
set file {{"Save layout" "layout save"}
          {"Save layout as..." "FileDialog {save layout as} {layout save}"}
          {"Load layout" "FileDialog {load file} {layout load}"}
          {"Load element data to paste-buffer" 
	   "FileDialog {load element to buffer} {buffer load element}"}
          {"Load layout data to paste-buffer" 
	   "FileDialog {load file to buffer} {buffer load layout}"}
          {"Load netlist file" "FileDialog {load netlist file} {netlist load}"}
          {"Print layout..." "PrintDialog"}
	  {}
          {"Save connection data of..."}
          {"  A single element" 
	   "GetLocation {Press a button at the element's location}; \
	   FileDialog {save connection to file} \
	   {element save_connections [GetElementName]}"}
          {"  All elements" "FileDialog {save all connection to file} \
	   {element save_connections all}"}
          {"  Unused pins" 
	   "FileDialog {save unused pins to file} {element save_unused_pins}"}
	  {}
	  {"Start new layout" "layout new"}
	  {}
          {"Quit program" quit "Alt-q"}}
#
# edit menu
#
set edit {{"Undo last operation" "undo" u}
          {"Redo last ndone operation" "redo" R}
          {"Clear undo-buffer" "undo clear" Control-U}
          {}
          {"Cut selection to buffer" 
	   "GetLocation {Press a button at the element's location}; \
	   buffer cut" Control-X}
          {"Copy selection to buffer" 
	   "GetLocation {Press a button at the element's location};buffer copy" 
	   Control-x}
          {"Paste buffer to layout" "buffer paste" F7}
          {}
          {"Unselect all" "deselect object all" Alt-A}
          {"Select all" "select object all" Alt-a}
          {}
          {"Edit Names..."}
          {"  Change text on layout" "object name" n}
          {"  Edit name of layout" "GetLayoutName"}
          {"  Edit name of active layer" "GetLayerName"}
          {}
          {"Change board size..." "StyleSizeDialog pcb 0"}}
#
# screen menu
#
set screen {{"Redraw layout" "layout redraw" r}
            {"Center layout" "layout center" c}
            {}
            {"Display grid" "pcb::grid display $gridView" "" 
	     "checkbutton gridView"}
            {"Realign grid" 
	     "GetLocation {Press a button at a grid point}; pcb::grid realign"}
            {"View solder side" 
	     "GetLocation {Press a button at the desired point}; \
	     show solder_side $viewSS" Tab "checkbutton viewSS"}
            {"View soldermask" "show solder_mask $viewSM" "" 
	     "checkbutton viewSM"}
            {}
            {"Grid Setting..."}
            {"0.1 mil" "pcb::grid set 0.1mil" ""
	     "radiobutton gridVal \"0.1 mil\""}
            {"0.5 mil" "pcb::grid set 0.5mil" "" 
	     "radiobutton gridVal \"0.5 mil\""}
            {"1 mil" "pcb::grid set 1mil" "" "radiobutton gridVal \"1.0 mil\""}
            {"0.1 mm" "pcb::grid set 0.1mm" "" "radiobutton gridVal \"0.1 mm\""}
            {"10 mil" "pcb::grid set 10mil" "" 
	     "radiobutton gridVal \"10.0 mil\""}
            {"1 mm" "pcb::grid set 1mm" "" "radiobutton gridVal \"1.0 mm\""}
            {"25 mil" "pcb::grid set 25mil" "" 
	     "radiobutton gridVal \"25.0 mil\""}
            {"50 mil" "pcb::grid set 50mil" "" 
	     "radiobutton gridVal \"50.0 mil\""}
            {"100 mil" "pcb::grid set 100mil" "" 
	     "radiobutton gridVal \"100.0 mil\""}
            {"increment by 5 mil" "pcb::grid set +5mil" g}
            {"decrement by 5 mil" "pcb::grid set -5mil" G}
            {"increment by 1 mm" "pcb::grid set +1mm"}
            {"decrement by 1 mm" "pcb::grid set -1mm"}
            {}
            {"Zoom Setting..."}
            {"4 : 1" "GetLocation {Select a point}; zoom set 4:1" "" 
	     "radiobutton zoomVal \"4:1\""}
            {"2 : 1" "GetLocation {Select a point}; zoom set 2:1" "" 
	     "radiobutton zoomVal \"2:1\""}
            {"1 : 1" "GetLocation {Select a point}; zoom set 1:1" "" 
	     "radiobutton zoomVal \"1:1\""}
            {"1 : 2" "GetLocation {Select a point}; zoom set 1:2" "" 
	     "radiobutton zoomVal \"1:2\""}
            {"1 : 4" "GetLocation {Select a point}; zoom set 1:4" "" 
	     "radiobutton zoomVal \"1:4\""}
            {"1 : 8" "GetLocation {Select a point}; zoom set 1:8" "" 
	     "radiobutton zoomVal \"1:8\""}
            {"1 : 16" "GetLocation {Select a point}; zoom set 1:16" "" 
	     "radiobutton zoomVal \"1:16\""}
	    {}
            {"Displayed element-name..."}
            {"Description" "show element_name description" "" 
	     "radiobutton dispElemName 0"}
            {"Reference designator" "show element_name reference " "" 
	     "radiobutton dispElemName 1"}
            {"Value" "show element_name value" "" "radiobutton dispElemName 2"}
	    {}
            {"Pinout shows number" 
	     "pcb::option pinout_shows_number_not_name $pinShowNum" "" 
	     "checkbutton pinShowNum"}
            {"Open pinout menu" "GetLocation {Select an element}; PinoutBox" 
	     Shift-d}}
#
# sizes menu
#
set sizes {{"Use 'Signal' routing style" {style use 1}
            Control-1 "radiobutton styleNum 1"}
           {"Use 'Power' routing style" {style use 2}
            Control-2 "radiobutton styleNum 2"}
           {"Use 'Fat' routing style" {style use 3}
            Control-3 "radiobutton styleNum 3"}
           {"Use 'Skinny' routing style" {style use 4}
            Control-4 "radiobutton styleNum 4"}
	   {"Adjust 'Signal' sizes ..." "StyleSizeDialog style 1"}
	   {"Adjust 'Power' sizes ..." "StyleSizeDialog style 2"}
	   {"Adjust 'Fat' sizes ..." "StyleSizeDialog style 3"}
	   {"Adjust 'Skinny' sizes ..." "StyleSizeDialog style 4"}
	   {"Adjust active sizes ..." "StyleSizeDialog all 0"}}
#
# settings menu
#
set settings {{"Layer groups"}
              {"Edit layer groupings" "LayerGroupsDialog"}
	      {}
	      {"All-direction lines" 
	       "pcb::option all_direction_lines $allDirLines" "" 
	       "checkbutton allDirLines"}
	      {"Auto swap line start angle" 
	       "pcb::option auto_swap_line_start_angle $autoSwapLSA" "" 
	       "checkbutton autoSwapLSA"}
	      {"Orthogonal moves" "pcb::option orthogonal_moves $orthoMoves" "" 
	       "checkbutton orthoMoves"}
	      {"Crosshair snaps to pins and pads" 
	       "pcb::option crosshair_snaps_to_pins_pads $csSnap" "" 
	       "checkbutton csSnap"}
	      {"Crosshair shows DRC clearance" 
	       "pcb::option crosshair_shows_drc_clearance $csShowDRCC" "" 
	       "checkbutton csShowDRCC"}
	      {"Auto enforce DRC clearance" 
	       "pcb::option auto_enforce_drc_clearance $enfDRCC" "" 
	       "checkbutton enfDRCC"}
	      {}
	      {"Rubber band mode" "pcb::option rubber_band_mode $rubBandMode" 
	       "" "checkbutton rubBandMode"}
	      {"Require unique element names" 
	       "pcb::option unique_element_names $uniqElemNames" "" 
	       "checkbutton uniqElemNames"}
	      {"Auto-zero delta measurements" 
	       "pcb::option auto_zero_delta_measurements $autoZeroDM" "" 
	       "checkbutton autoZeroDM"}
	      {"New lines, arcs clear polygons" 
	       "pcb::option new_lines_arcs_clear_polygons $newClrPoly" "" 
	       "checkbutton newClrPoly"}
	      {"Show autorouter trials" 
	       "pcb::option show_autorouter_trials $showART" "" 
	       "checkbutton showART"}
	      {"Thin draw" "pcb::option thin_draw $thinDraw" "i" 
	       "checkbutton thinDraw"}
	      {"Check polygons" "pcb::option check_polygons $checkPoly" "" 
	       "checkbutton checkPoly"}
             }
#
# select menu
#
set select {{"Select all objects" "select object all"}
            {"Select all connected objects" "select object connected"}
            {}
            {"Unselect all objects" "deselect object all"}
            {"Unselect all connected objects" "deselect object connected"}
	    {}
            {"Select by name"}
            {"All objects" "select object [GetSelectItemName]"}
            {"Elements" "select element [GetSelectItemName]"}
            {"Pads" "select pad [GetSelectItemName]"}
            {"Pins" "select pin [GetSelectItemName]"}
            {"Text" "select text [GetSelectItemName]"}
            {"Vias" "select via [GetSelectItemName]"}
	    {}
            {"Auto-place selected elements" "place" Control-p}
            {"Move selected elements to other side" "element flip_selected" B}
            {"Delete selected objects" "delete selected"}
            {"Convert selection to element" "object convert"}
	    {}
            {"Optimize selected rats" 
	     "netlist delete_rats selected; netlist add_rats selected"}
            {"Auto-route selected rats" "route rats"}
            {"Rip up selected auto-routed tracks" "delete track auto_only"}
	    {}
            {"Change size of selected objects"}
            {"Lines -10 mil" "line selected_size -10mil"}
            {"Lines +10 mil" "line selected_size +10mil"}
            {"Pads -10 mil" "pad size -10mil"}
            {"Pads +10 mil" "pad size +10mil"}
            {"Pins -10 mil" "pin size -10mil"}
            {"Pins +10 mil" "pin size +10mil"}
            {"Text -10 mil" "text selected_size -10mil"}
            {"Text +10 mil" "text selected_size +10mil"}
            {"Vias -10 mil" "via selected_size -10mil"}
            {"Vias +10 mil" "via selected_size +10mil"}
	    {}
            {"Change drilling hole of selected objects"}
            {"Pins -10 mil" "pin hole -10mil"}
            {"Pins +10 mil" "pin hole +10mil"}
            {"Vias -10 mil" "via selected_hole -10mil"}
            {"Vias +10 mil" "via selected_hole +10mil"}
	    {}
            {"Change square-flag of selected objects"}
            {"Elements" "element square"}
            {"Pins" "pin square"}}
#
# buffer menu
#
set buffer {{"Copy selection to buffer" 
             "GetLocation {Press a button at the element's location}; \
	     buffer copy"}
            {"Cut selection to buffer" \
	     "GetLocation {Press a button at the element's location}; \
	     buffer cut"}
            {"Paste buffer to layout" "buffer paste"}
            {}
            {"Rotate buffer 90 deg CCW" "buffer rotate -90"}
            {"Rotate buffer 90 deg CW" "buffer rotate 90"}
            {"Mirror buffer vertically" "buffer mirror Y"}
            {"Mirror buffer horizontally" "buffer mirror X"}
            {}
            {"Clear buffer" "buffer clear"}
            {"Convert buffer to element" "buffer convert"}
            {"Break buffer elements to pieces" "buffer breakup"}
            {"Save buffer elements to file" 
	     "FileDialog {save buffer element as} {buffer save}"}
            {}
            {"Select current buffer"}
            {"#1" "buffer 1 select" Shift-1 "radiobutton bufNum 1"}
            {"#2" "buffer 2 select" Shift-2 "radiobutton bufNum 2"}
            {"#3" "buffer 3 select" Shift-3 "radiobutton bufNum 3"}
            {"#4" "buffer 4 select" Shift-4 "radiobutton bufNum 4"}
            {"#5" "buffer 5 select" Shift-5 "radiobutton bufNum 5"}}

#
# connects menu
#
set connects {{"Lookup connection to object" 
               "GetLocation {Select the object}; object connections" Control-f}
              {"Reset scanned pads/pins/vias" "scan reset pads_pins_vias"}
              {"Reset scanned lines/polygons" "scan reset lines_polygons"}
              {"Reset all connections" "scan reset all" F}
	      {}
              {"Optimize rats-nest" "netlist optimize_rats" o}
              {"Erase rats-nest" "netlist delete_rats all" e}
	      {}
              {"Auto-route selected rats" "route rats"}
              {"Auto-route all rats" "route all"}
              {"Rip up all auto-routed tracks" "delete track all"}
	      {}
              {"Auto-optimize" "route optimize auto" Shift-8}
              {"Debumpify" "route optimize debumpify; layout redraw"}
              {"Unjaggy" "route optimize unjaggy; layout redraw"}
              {"Vianudge" "route optimize vianudge; layout redraw"}
              {"Viatrim" "route optimize viatrim; layout redraw"}
              {"Orthopull" "route optimize orthopull; layout redraw"}
              {"SimpleOpts" "route optimize simple; layout redraw" F8}
              {"Miter" "route optimize miter; layout redraw"}
              {"Only auto-routed nets"
	       "pcb::option only_autorouted_nets $onlyARNets" "" 
	       "checkbutton onlyARNets"}
	      {}
              {"Design Rule Checker" "drc run"}}
#
# info menu
#
set info {{"Generate object report"
           "GetLocation {Select the object}; 
	   DisplayReport Report {object report}" Control-r}
          {"Generate drill summary" 
	   "DisplayReport {Drill Report} {drill summary}" Control-d}
          {"Report found pins/pads" "DisplayReport Report {pin report}"}}
#
# window menu
#
set window {{"Tcl console" "WindowOnOff Console" "" "checkbutton consoleView"}
            {"Library" "WindowOnOff LibraryDialog" ""  "checkbutton libWinView"}
            {"Netlist" "WindowOnOff NetlistDialog" "" 
	     "checkbutton netlistWinView"}}
#
# help menu
#
set help {{"Help" "helpDoc"}
          {"About..." "DisplayReport {About PCB} {pcb::about}"}}

#
# Popup menu -- For third mouse button.
#
set popup {{"Operations on Selections"}
          {"Unselect All" "deselect object all"}
          {"Remove Selected" "delete"}
          {"Copy Selection to buffer" "buffer copy"}
          {"Cut Selection to buffer" "buffer cut"}
          {"Convert Selection to Element" "object convert element"}
          {"Break Element to Pieces" "element breakup"}
          {"Auto-place Selected" "place"}
          {"Auto-route Selected Rats" "route all"}
          {"Rip-up selected auto-routed tracks" "delete track auto_only"}
	  {}
          {"Operations on This Location"}
          {"Toggle Name Visibility" "element display_name selected_elements"}
          {"Edit Name" "object name"}
          {"Object Report" "DisplayReport Report {object report}"}
          {"Rotate Object CCW" "object rotate ccw"}
          {"Rotate Object CW" "object rotate cw"}
          {"Send To Other Side" "object move otherside"}
          {"Toggle Thermal" "mode save;mode thermal;mode notify;mode restore"}
          {"Lookup Connections" "object connections"}
	  {}
	  {"Undo Last Operation" "undo"}
	  {"Redo Last Undone Operation" "redo"}
	  {}
          {"Line Tool" "mode line"}
          {"Via Tool" "mode via"}
          {"Rectangle Tool" "mode rectangle"}
          {"Selection Tool" "mode arrow"}
          {"Text Tool" "mode text"}
          {"Panner Tool" "mode none"}}


#
# mode info
# mode name, pixmap file, x pos, y pos
set DrawingModes \
	 {{"via" "${tcllibdir}/via.gif" 0 0}
          {"line" "${tcllibdir}/line.gif" 0 1}
          {"arc" "${tcllibdir}/arc.gif" 1 0}
          {"text" "${tcllibdir}/text.gif" 1 1}
          {"rectangle" "${tcllibdir}/rect.gif" 2 0}
          {"polygon" "${tcllibdir}/poly.gif" 2 1}
          {"paste_buffer" "${tcllibdir}/buf.gif" 3 0}
          {"remove" "${tcllibdir}/del.gif" 3 1}
          {"rotate" "${tcllibdir}/rotate.gif" 4 0}
          {"insert_point" "${tcllibdir}/insert.gif" 4 1}
          {"thermal" "${tcllibdir}/therm.gif" 5 0}
          {"arrow" "${tcllibdir}/select.gif" 5 1}
          {"pan" "${tcllibdir}/pan.gif" 6 0}
          {"lock" "${tcllibdir}/lock.gif" 6 1}}


# Procedure to create menus
#
# menuBar - parent frame
# menuList - list of options in the menu, associated command and shortcut
# name - menu name
# side - side to pack
proc CreateMenu {menuBar menuList name {side {}}} {
  global menuFont
  set lName [string tolower $name]
  set menuName "$menuBar.$lName"
  menubutton $menuName -text $name -menu "$menuName.menu"
  if {$side != {}} {
    pack $menuName -side $side
  }

  set menuNameList "$menuName.menu"
  menu "$menuName.menu"
  foreach menuItem $menuList {
    set menuVars [llength $menuItem]
    if {$menuVars > 1 } {
      set menucmd [lindex $menuItem 1]
    }

    if {$menuVars == 4} {
      set accelKey [lindex $menuItem 2]
      set buttonType [lindex [lindex $menuItem 3] 0]
      set buttonVar [lindex [lindex $menuItem 3] 1]
      if {$accelKey == ""} {
        if {[string match radio* $buttonType]} {
	  set buttonVal [lindex [lindex $menuItem 3] 2]
          $menuNameList add $buttonType -label [lindex $menuItem 0] \
	  -command "[lindex $menuItem 1]" -font $menuFont -variable $buttonVar \
	  -value $buttonVal -selectcolor red2
	} else {
	  # set on value - "on" and off value - "off"
          $menuNameList add $buttonType -label [lindex $menuItem 0] \
	  -command "[lindex $menuItem 1]" -font $menuFont -variable $buttonVar \
	  -onvalue "on" -offvalue "off" -selectcolor red2
	}
      } else {
        if {[string match radio* $buttonType]} {
	  set buttonVal [lindex [lindex $menuItem 3] 2]
          $menuNameList add $buttonType -label [lindex $menuItem 0] \
	  -command "[lindex $menuItem 1]" -font $menuFont \
	  -accelerator $accelKey -variable $buttonVar -value $buttonVal \
	  -selectcolor red2
	} else {
	  # set on value - "on" and off value - "off"
          $menuNameList add $buttonType -label [lindex $menuItem 0] \
	  -command "[lindex $menuItem 1]" -font $menuFont -variable $buttonVar \
	  -accelerator $accelKey -onvalue "on" -offvalue "off" -selectcolor red2
	}
        #bind .pcb "<$accelKey>" "[lindex $menuItem 1]"
        bind .pcb.layout.pcbFrame.pcbCanvas "<$accelKey>" "[lindex $menuItem 1]"
      }
    } elseif {$menuVars == 3} {
      $menuNameList add command -label "[lindex $menuItem 0]" \
      -command "[lindex $menuItem 1]" -accelerator "[lindex $menuItem 2]" \
      -font $menuFont
      #bind .pcb "<[lindex $menuItem 2]>" "[lindex $menuItem 1]"
      bind .pcb.layout.pcbFrame.pcbCanvas "<[lindex $menuItem 2]>" \
      "[lindex $menuItem 1]"
    } elseif {$menuVars == 2} {
      $menuNameList add command -label [lindex $menuItem 0] \
      -command "[lindex $menuItem 1]" -font $menuFont
    } elseif {$menuVars == 1} {
      $menuNameList add command -label [lindex $menuItem 0] -foreground red \
      -font $menuFont -activeforeground red -state normal
    } else {
      $menuNameList add separator
    }
  }
  return $menuNameList
}

# procedure to toggle colors for layer buttons to indicate on/off status
proc ToggleLayer {layerFrame layerIndex} {
  set fgColor ""
  set bgColor ""

  if {$layerIndex <= 8} {
    layer toggle_visible $layerIndex
  } else {
    switch -exact $layerIndex {
      "9" {show pins_pads}
      "10" {show vias}
      "11" {show far_side}
      "12" {show rats}
    }
    #tag call back for show command handles this
    #set fgColor [lindex [$layerFrame.$layerIndex config -background] end]
    #set bgColor [lindex [$layerFrame.$layerIndex config -foreground] end]
    #$layerFrame.$layerIndex config -foreground $fgColor -background $bgColor
  }
  RefreshLayerMenu
}


# procedure to toggle buttons when show command executed from command line
proc RefreshShow {option {value {}}} {
  global layers
  global layerFrame
  global dispElemName viewSM viewSS
  set fgColor ""
  set bgColor ""
  set layerIndex 0

switch -exact $option {
    "pins_pads" {set layerIndex 9}
    "vias" {set layerIndex 10}
    "far_side" {set layerIndex 11}
    "rats" {set layerIndex 12}
    "solder_mask" {
        if {$value == {}} { return }
        if {$value == "on"} {
           set viewSM "on"
        } elseif {$value == "off"} {
           set viewSM "off"
        }
        return
    }
    "solder_side" {
        if {$value == {}} { return }
        if {$value == "on"} {
           set viewSS "on"
        } elseif {$value == "off"} {
           set viewSS "off"
        } elseif {$value == "toggle"} {
           if {$viewSS  == "on" } {
              set viewSS "off"
           } else {
              set viewSS "on"
           }
        }
        return
    }
    "element_name" {
        if {$value == {}} { return }
        if {$value == "description"} {
         set dispElemName 0
        } elseif {$value == "reference"} {
          set dispElemName 1
        } elseif {$value == "value"} {
          set dispElemName 2
        } else {
          set dispElemName 9
        }
        return
    }
  }
  if {$layerIndex} {
    if {$value == "on"} {
      set fgColor white
      set bgColor [lindex [lindex $layers $layerIndex] 1]
    } elseif {$value == "off"} {
      set fgColor [lindex [lindex $layers $layerIndex] 1]
      set bgColor white
    } else {
      set fgColor [lindex [$layerFrame.$layerIndex config -background] end]
      set bgColor [lindex [$layerFrame.$layerIndex config -foreground] end]
    }
    $layerFrame.$layerIndex config -foreground $fgColor -background $bgColor
  }
}

# Creats popup color window to modify layer color.
proc ModifyLayerColor {layerNum} {
    set orgColor [layer color $layerNum]
    set newColor [tkColorDialog -initialcolor $orgColor]
    if {$newColor != {} } {
        layer color $layerNum $newColor
        RefreshLayerMenu
    }
}

# procedure to create the menu of layers as a tag callback to "start"
proc CreateLayers {frame} {
  global buttonFont
  global layers

  if {![catch {${frame}.0 cget -state}]} {return}

  set layers {}
  for {set i 0} {$i <= 12} {incr i} {
     lappend layers [list [layer name $i] [layer color $i]]
  }

  # create layer toggle buttons and drop down menu for active layer selection

  set idx 0
  foreach layer $layers {
    set layerName [lindex $layer 0]
    set layerColor [lindex $layer 1]
    # create the layer toggle button
    button $frame.$idx -text $layerName -background $layerColor \
	-foreground white -command "ToggleLayer $frame $idx" \
	-activeforeground white -activebackground gray75 -font $buttonFont \
	-borderwidth 0
    # Allow mouse button 2 on the layer to set the active component
    if { $idx != 12 } {
       bind $frame.$idx <Button-2> "layer active $idx"
    } else {
       # Exemption for special case, Netlist layer.
       bind $frame.$idx <Button-2> "layer active netlist"
    }
    # add layer to listbox to choose active layer
    #set lCmd [lindex $layer 2]
    set lCmd "layer active $layerName"
    if {$idx <= 8 || $idx == 12} {
       # Only main 8 layers and Netlist layers.
       $frame.activeLayer.menu add command -label "                $layerName" \
	  -background $layerColor -foreground white -font $buttonFont \
	  -command "$lCmd"
    }
    # For layer color modification
    if { $idx < 8 } {
       bind $frame.$idx <Button-3> "ModifyLayerColor $idx"
    }
    pack $frame.$idx -side top -fill x
    incr idx
  }
  pack $frame.active -side top
  pack $frame.activeLayer -side top -fill x

  # Initial layer update
  #${frame}.activeLayer configure -text [layer active]
  #${frame}.activeLayer configure -background [layer color]

  #Update Layer frame finally. Required to properly update
  #activeLayer menu, especially for Netlist layer.
  RefreshLayerMenu
}

# procedure to re-initialize active layer menu to indicate new choice
proc UpdateLayers {frame option {newlayer {}}} {
  global layers

  if {[info level] <= 1 && $newlayer != {}} {
    if {$option == "active"} {
       ${frame}.activeLayer configure -text [layer active]
       ${frame}.activeLayer configure -background [layer color]
       if { $newlayer == "8"} { layout redraw }
       # Turn on Ratlines if Netlist layer activated
       if { $newlayer == "9" || $newlayer == "netlist" } { show rats on }
       RefreshLayerMenu
     }
    # Update layer frame when layer name, color, status changed.
    if {$option == "name" || $option == "visible" || $option == "invisible" || \
        $option == "toggle_visible" || $option == "color" || $option == "name" } {
       RefreshLayerMenu
    }
  }
}

set lockFlag 0
# procedure to toggle colors for mode buttons to indicate on/off status
proc ShowMode {frame {value {}}} {
  global DrawingModes
  global lockFlag

  if {[info level] <= 1 && $value != {}} {
    foreach locmode $DrawingModes {
      set modename [lindex $locmode 0]
      $frame.$modename config -background white
    }
    # PCB delares "pan" and "none" to be the same mode number,
    # check the passed argument to see when "mode none" was called.
    if {$value != "none"} {
      set newmode [mode]
      # Use "catch" because some modes don't have buttons.
      catch {$frame.$newmode config -background red}
    }

    # Special case : Lock mode. Display object report if
    # LOCK_FLAG is toggled.
    if {$lockFlag == 1 && $value == "notify"} {
       DisplayReport {Report} {object report}
    }
    if {$value == "lock"} {
       # lockFlag will stay set till tool is not changed.
       set lockFlag 1
    } elseif {$value != "notify" && $value != "release"} {
       # Turn off lockFlag to stop displaying report as lock tool is disabled.
       set lockFlag 0
    }
  }
}

proc UpdatePanner {} {
   global viewLoc

   if {[info level] <= 1} {
      set size [layout size]
      set maxx [lindex $size 0]
      set maxy [lindex $size 1]
      set aspect [expr {($maxy + 0.0) / $maxx}]
      set xssize [$viewLoc cget -width]
      set yssize [$viewLoc cget -height]
      set xr [expr {$yssize / $aspect}]
      if {$xr > $xssize} {
         set yssize [expr {$xssize * $aspect}]
         set lscale [expr {$maxx / $xssize}]
      } else {
         set xssize $xr
         set lscale [expr {$maxy / $yssize}]
      }
      $viewLoc coords 1 0 0 $xssize $yssize

      set lcoords [layout view]
      set scoords {}
         foreach l $lcoords {
         lappend scoords [expr {($l + 0.0) / $lscale}]
      }
      $viewLoc coords 2 $scoords
   }
}

proc UpdateLayout {} {
   global viewLoc

   set size [layout size]
   set maxx [lindex $size 0]
   set maxy [lindex $size 1]
   set fullCoords [$viewLoc coords 1]
   set viewCoords [$viewLoc coords 2]
   set fx [lindex $fullCoords 2]
   set fy [lindex $fullCoords 3]
   set vx [lindex $viewCoords 0]
   set vy [lindex $viewCoords 1]
   set rx [expr ($vx + 0.0) / $fx]
   set ry [expr ($vy + 0.0) / $fy]
   set lx [expr $maxx * $rx]
   set ly [expr $maxy * $ry]

   layout view [expr int ($lx)] [expr int ($ly)]
}

proc UpdateScreen {option {fName {}}} {
  if {$option == "load" && $fName != {}} {
    RefreshLayerMenu
    RefreshNetlistDialog
    # Update menus as per PCB database.
    uplevel 1 {InitMenuOptions}
  }
  if {$option == "new"} {
    # Destroy Netlist window if it is there.
    catch {wm withdraw .pcb.netlistView}
    RefreshLayerMenu
    # Update menus as per PCB database.
    uplevel 1 {InitMenuOptions}
  }
  UpdatePanner
}

# initialize the start position for the view box move.
# set the max variance that can be accomodated in ref to the
# size of the original layout.
proc ViewBoxSet {x y} {
  global maxdx maxdy
  global anchorx anchory
  global viewLoc

  set x [$viewLoc canvasx $x]
  set y [$viewLoc canvasx $y]
  set fullCoords [$viewLoc coords 1]
  set viewCoords [$viewLoc coords 2]
  set fx [expr [lindex $fullCoords 2] - [lindex $fullCoords 0]]
  set fy [expr [lindex $fullCoords 3] - [lindex $fullCoords 1]]
  set vx [expr [lindex $viewCoords 2] - [lindex $viewCoords 0]]
  set vy [expr [lindex $viewCoords 3] - [lindex $viewCoords 1]]
  set maxdx [expr $fx - $vx]
  set maxdy [expr $fy - $vy]

  set anchorx $x
  set anchory $y
  #set viewWinId [$viewLoc find withtag viewWin]
}

# update the view box move and the new start location.
# update the layout view to correspon to the view box move
proc ViewBoxMove {x y} {
  global maxdx maxdy
  global anchorx anchory
  global viewLoc

  set x [$viewLoc canvasx $x]
  set y [$viewLoc canvasx $y]
  set dx [expr $x - $anchorx]
  set dy [expr $y - $anchory]

  set viewCoords [$viewLoc coords 2]
  set locx [lindex $viewCoords 0]
  set locy [lindex $viewCoords 1]
  $viewLoc move 2 $dx $dy
  set viewCoords [$viewLoc coords 2]
  set newx [lindex $viewCoords 0]
  set newy [lindex $viewCoords 1]

  # ensure that we remain within the confines of the layout
  set dx 0
  set dy 0
  if {$newx < 0} {
    set dx [expr -$newx]
  }
  if {$newy < 0} {
    set dy [expr -$newy]
  }
  if {$newx > $maxdx} {
    set dx [expr $maxdx - $newx]
  }
  if {$newy > $maxdy} {
    set dy [expr $maxdy - $newy]
  }

  if {$dx || $dy} {
    $viewLoc move 2 $dx $dy
    set viewCoords [$viewLoc coords 2]
  }

  # update the layout view
  UpdateLayout

  set anchorx $x
  set anchory $y
}

# Setup the "tip" mechanism for buttons.  This causes a label to
# pop up with text describing the mode invoked by the button, if
# the pointer stays over the button for more than a second.

proc MakeTip {widget ltext} {
   global tipid
   .pcb.mytip configure -text $ltext
   set tipid [after 1000 UpTip $widget]
}

proc RemoveTip {widget} {
   global tipid
   after cancel $tipid
   place forget .pcb.mytip
}

proc UpTip {widget} {
   place .pcb.mytip -in $widget -x 20 -y 20
}

#
# procedure to create mode buttons
#
proc CreateMode {modeFrame modes} {
  global tcllibdir
  foreach mode $modes {
    set modeName [lindex $mode 0]
    image create photo pm_$modeName -file [subst [lindex $mode 1]]
    button $modeFrame.$modeName -image pm_$modeName -command "mode $modeName" \
	-background white -activebackground red
    grid config $modeFrame.$modeName -row [lindex $mode 2] \
    	-column [lindex $mode 3]
    bind $modeFrame.$modeName <Enter> "MakeTip %W $modeName"
    bind $modeFrame.$modeName <Leave> "RemoveTip %W"
  }
}

#
# Status frame
#
proc MakeStatusBar {} {
  label .pcb.layout.status -text "(status)" -anchor w
  pack .pcb.layout.status -side bottom -fill x
  pack .pcb.layout -side right -fill both -expand true
}


# Style parameters input
# dType - dialog type - pcb, all or style
# sType - style number, used only when dType is style
proc StyleSizeDialog {dType sType} {
   global textScale styleName
   global buttonBoldFont

   if {[catch {wm state .styleSize}]} {
      toplevel .styleSize
      wm title .styleSize "Sizes"

      set minDim 0
      set maxDim 0
      if {$dType == "style"} {
        set styleNum $sType
        set styleName [style $styleNum name]

        label .styleSize.styleNameLabel -text "Routing Style Name" \
		-font $buttonBoldFont
        entry .styleSize.styleNameEntry -textvariable styleName \
		-font $buttonBoldFont

	pack .styleSize.styleNameLabel .styleSize.styleNameEntry \
		-side top -anchor w

	set minDim [expr [lindex [style $styleNum line_width_range] 0]/100]
	set maxDim [expr [lindex [style $styleNum line_width_range] 1]/100]
	scale .styleSize.lineWidth -label "line width" -from $minDim \
		-to $maxDim -length 5c -orient horizontal -variable lineWidth \
		-tickinterval [expr ($maxDim-$minDim)/5] -font $buttonBoldFont
        .styleSize.lineWidth set [expr [style $styleNum line_width]/100]

	set minDim [expr [lindex [style $styleNum via_hole_range] 0]/100]
	set maxDim [expr [lindex [style $styleNum via_hole_range] 1]/100]
        scale .styleSize.viaHole -label "via hole" -from $minDim -to $maxDim \
		-length 5c -orient horizontal -variable viaHole \
		-tickinterval [expr ($maxDim-$minDim)/5] -font $buttonBoldFont \
		-command "AdjustViaSizeDimensions .styleSize.viaSize \
		[lindex [style $styleNum via_size_range] 1]"
        .styleSize.viaHole set [expr [style $styleNum via_hole]/100]

	set minDim [expr [lindex [style $styleNum via_size_range] 0]/100]
	set maxDim [expr [lindex [style $styleNum via_size_range] 1]/100]
        scale .styleSize.viaSize -label "via size" -from $minDim -to $maxDim \
		-length 5c -orient horizontal -variable viaSize \
		-tickinterval [expr ($maxDim-$minDim)/5] -font $buttonBoldFont \
		-command "AdjustViaHoleDimensions .styleSize.viaHole \
		[lindex [style $styleNum via_hole_range] 0]"
        .styleSize.viaSize set [expr [style $styleNum via_size]/100]

	set minDim [expr [lindex [style $styleNum clearance_range] 0]/100]
	set maxDim [expr [lindex [style $styleNum clearance_range] 1]/100]
        scale .styleSize.clearance -label "clearance" -from $minDim \
		-to $maxDim -length 5c -orient horizontal -variable clearance \
		-tickinterval [expr ($maxDim-$minDim)/5] -font $buttonBoldFont
        .styleSize.clearance set [expr [style $styleNum clearance]/100]

        pack .styleSize.lineWidth .styleSize.viaHole .styleSize.viaSize \
	    .styleSize.clearance -side top -fill x
      }


      if {$dType == "all"} {
        # Small fonts..
	set minDim [expr [lindex [line size_range] 0]/100]
	set maxDim [expr [lindex [line size_range] 1]/100]
	scale .styleSize.lineWidth -label "line width" -from $minDim \
		-to $maxDim -length 5c -orient horizontal -variable lineWidth \
		-tickinterval [expr ($maxDim-$minDim)/5] -font $buttonBoldFont
        .styleSize.lineWidth set [expr [line size]/100]

	set minDim [expr [lindex [via hole_range] 0]/100]
	set maxDim [expr [lindex [via hole_range] 1]/100]
	scale .styleSize.viaHole -label "via hole" -from $minDim -to $maxDim \
		-length 5c -orient horizontal -variable viaHole \
		-tickinterval [expr ($maxDim-$minDim)/5] \
		-command "AdjustViaSizeDimensions .styleSize.viaSize \
		[lindex [via size_range] 1]" -font $buttonBoldFont
        .styleSize.viaHole set [expr [via hole]/100]

	set minDim [expr [lindex [via size_range] 0]/100]
	set maxDim [expr [lindex [via size_range] 1]/100]
        scale .styleSize.viaSize -label "via size" -from $minDim -to $maxDim \
		-length 5c -orient horizontal -variable viaSize \
		-tickinterval [expr ($maxDim-$minDim)/5] \
		-command "AdjustViaHoleDimensions .styleSize.viaHole \
		[lindex [via hole_range] 0]" -font $buttonBoldFont
        .styleSize.viaSize set [expr [via size]/100]

	set minDim [expr [lindex [netlist clearance_range] 0]/100]
	set maxDim [expr [lindex [netlist clearance_range] 1]/100]
	scale .styleSize.clearance -label "clearance" -from $minDim \
		-to $maxDim -length 5c -orient horizontal -variable clearance \
		-tickinterval [expr ($maxDim-$minDim)/5] -font $buttonBoldFont
        .styleSize.clearance set [expr [netlist clearance]/100]

	set minDim [expr [lindex [text scale_range] 0]/100]
	set maxDim [expr [lindex [text scale_range] 1]/100]
        scale .styleSize.textScale -label "text scale" -from $minDim \
		-to $maxDim -length 5c -orient horizontal -variable textScale \
		-tickinterval [expr ($maxDim-$minDim)/5] -resolution 0.1 \
		-font $buttonBoldFont
        .styleSize.textScale set [expr [text scale]/100]

        scale .styleSize.minSpaceDRC -label "DRC minimum space" -from 10 \
		-to 500 -length 5c -orient horizontal -variable minSpaceDRC \
		-tickinterval 100 -font $buttonBoldFont
        .styleSize.minSpaceDRC set [expr [drc space]/100]

        scale .styleSize.minWidthDRC -label "DRC minimum width" -from 10 \
		-to 500 -length 5c -orient horizontal -variable minWidthDRC \
		-tickinterval 100 -font $buttonBoldFont
        .styleSize.minWidthDRC set [expr [drc width]/100]

        scale .styleSize.minOverlapDRC -label "DRC minimum overlap" -from 10 \
		-to 500 -length 5c -orient horizontal -variable minOverlapDRC \
		-tickinterval 100 -font $buttonBoldFont
        .styleSize.minOverlapDRC set [expr [drc overlap]/100]

        scale .styleSize.minSilkWidthDRC -label "DRC minimum silkwidth" \
		-from 1 -to 30 -length 5c -orient horizontal \
		-variable minSilkWidthDRC -tickinterval 6  -font $buttonBoldFont
        .styleSize.minSilkWidthDRC set [expr [drc silk]/100]

        pack .styleSize.lineWidth .styleSize.viaHole .styleSize.viaSize \
	    .styleSize.clearance .styleSize.textScale .styleSize.minSpaceDRC \
	    .styleSize.minWidthDRC .styleSize.minOverlapDRC \
	    .styleSize.minSilkWidthDRC -side top -fill x
      }

      if {($dType == "pcb") || ($dType == "all")} {
	set sizeRange [layout size_range]
	set minDim [expr [lindex $sizeRange 0]/100]
	set maxDim [expr [lindex $sizeRange 1]/100]
        scale .styleSize.pcbWidth -label "PCB width" -from $minDim -to $maxDim \
		-length 5c -variable pcbWidth -orient horizontal \
		-tickinterval [expr ($maxDim-$minDim)/4] -font $buttonBoldFont
        .styleSize.pcbWidth set [expr [lindex [layout size] 0]/100]
	
	set minDim [expr [lindex $sizeRange 2]/100]
	set maxDim [expr [lindex $sizeRange 3]/100]
        scale .styleSize.pcbHeight -label "PCB height" -from $minDim \
		-to $maxDim -length 5c -variable pcbHeight -orient horizontal \
		-tickinterval [expr ($maxDim-$minDim)/4] -font $buttonBoldFont
        .styleSize.pcbHeight set [expr [lindex [layout size] 1]/100]

        pack .styleSize.pcbWidth .styleSize.pcbHeight -side top -fill x
      }

      frame .styleSize.okCancel

      button .styleSize.okCancel.ok -text Ok \
      	-command "SetStyleSize $dType $sType; destroy .styleSize"
      button .styleSize.okCancel.cancel -text Cancel \
      	-command "destroy .styleSize"

      pack .styleSize.okCancel.ok .styleSize.okCancel.cancel -side left
      pack .styleSize.okCancel -side top

      bind .styleSize <Key-Escape> {destroy .styleSize}

   } else {
      wm deiconify .styleSize
   }
}

# update new parameters to db
proc SetStyleSize {dType sType} {
  global styleName lineWidth viaSize viaHole clearance textScale
  global minSpaceDRC minWidthDRC minOverlapDRC minSilkWidthDRC
  global pcbWidth pcbHeight

  if {$dType == "style"} {
    style $sType name $styleName
    style $sType line_width [expr $lineWidth * 100]
    style $sType clearance [expr $clearance * 100]
    # Set viaHole first if PCB db contains value greater than scale value.
    set via_hole_before [style $sType via_size]
    set via_hole_after [expr $viaHole * 100]
    if {$via_hole_before > $via_hole_after} {
      style $sType via_hole [expr $viaHole * 100]
      style $sType via_size [expr $viaSize * 100]
     } else {
      style $sType via_size [expr $viaSize * 100]
      style $sType via_hole [expr $viaHole * 100]
    }
    RefreshStyleMenu
  }
  if {$dType == "all"} {
    line size [expr $lineWidth * 100]
    via size [expr $viaSize * 100]
    via hole [expr $viaHole * 100]
    text scale [expr $textScale * 100]
    netlist clearance [expr $clearance * 100]
    drc space [expr $minSpaceDRC * 100]
    drc width [expr $minWidthDRC * 100]
    drc overlap [expr $minOverlapDRC * 100]
    drc silk [expr $minSilkWidthDRC * 100]
    }
  if {$dType == "all" || $dType == "pcb"} {
    layout size [expr $pcbWidth * 100] [expr $pcbHeight * 100]
  }
}

# Adjust via hole dimensions based on via size settings
proc AdjustViaHoleDimensions {wName minDim val} {
  # need to get copper thickness from db, use default for now
  set minDim [expr $minDim/100]
  set maxDim [expr $val-4]
  set tick [expr ($maxDim-$minDim)/4]

  if {$tick == 0} {
    $wName configure -from $minDim -to $maxDim \
  	-tickinterval [expr ($maxDim-$minDim)] 
  } else {
    $wName configure -from $minDim -to $maxDim -tickinterval $tick
  }
}

# Adjust via size dimensions based on via hole settings
proc AdjustViaSizeDimensions {wName maxDim val} {
  # need to get copper thickness from db, use default for now
  set minDim [expr $val+4]
  set maxDim [expr $maxDim/100]
  set tick [expr ($maxDim-$minDim)/4]

  if {$tick == 0} {
    $wName configure -from $minDim -to $maxDim \
  	-tickinterval [expr ($maxDim-$minDim)] 
  } else {
    $wName configure -from $minDim -to $maxDim -tickinterval $tick
  }
}

# initialize default menu options
proc InitMenuOptions {} {
  #puts "InitMenuOptions [info level]"
  uplevel 1 {RefreshStyleMenu}
  uplevel 1 {RefreshBuffer}
  uplevel 1 {RefreshGrid display}
  uplevel 1 {RefreshGrid get}
  uplevel 1 {RefreshZoom}
  uplevel 1 {RefreshOption pinout_shows_number_not_name}
  uplevel 1 {RefreshOption all_direction_lines}
  uplevel 1 {RefreshOption auto_swap_line_start_angle}
  uplevel 1 {RefreshOption orthogonal_moves}
  uplevel 1 {RefreshOption crosshair_snaps_to_pins_pads}
  uplevel 1 {RefreshOption crosshair_shows_drc_clearance}
  uplevel 1 {RefreshOption auto_enforce_drc_clearance}
  uplevel 1 {RefreshOption rubber_band_mode}
  uplevel 1 {RefreshOption unique_element_names}
  uplevel 1 {RefreshOption auto_zero_delta_measurements}
  uplevel 1 {RefreshOption new_lines_arcs_clear_polygons}
  uplevel 1 {RefreshOption show_autorouter_trials}
  uplevel 1 {RefreshOption thin_draw}
  uplevel 1 {RefreshOption check_polygons}
  uplevel 1 {RefreshOption only_autorouted_nets}
  uplevel 1 {RefreshShow element_name [show element_name]}
  uplevel 1 {RefreshShow solder_mask [show solder_mask]}
  uplevel 1 {RefreshShow solder_side [show solder_side]}
  uplevel 1 {RefreshWindowMenu}
}

# Refresh the Window menu according to status of all windows.
proc RefreshWindowMenu {} {
  global  libWinView
  global  netlistWinView
  global  consoleView

  if {![catch {wm state .pcb.libraryView}]} {
    if {[wm state .pcb.libraryView] == "withdrawn"} {
      set libWinView "off"
    } else {
      set libWinView "on"
    }
  } else {
    set libWinView "off"
  }
  if {![catch {wm state .pcb.netlistView}]} {
    if {[wm state .pcb.netlistView] == "withdrawn"} {
      set netlistWinView "off"
    } else {
      set netlistWinView "on"
    }
  } else {
    set netlistWinView "off"
  }
  #TODO : Update status of "consoleView" flag.
}

# Refresh the style menu with new route style information read from layout
# Respond to commands from prompt also!
proc RefreshStyleMenu {} {
    global styleNum
    global sizesMenu
    
    if {[info level] <= 2} {
      for {set sType 1} {$sType <= 4} {incr sType} {
        set sNum $sType
        set sName [style $sNum name]
        $sizesMenu entryconfigure $sNum -label "Use '$sName' routing style"
        $sizesMenu entryconfigure $sNum -command "style use $sName"
        $sizesMenu entryconfigure [expr $sNum+4] \
         	-label "Adjust '$sName' sizes ..."
        $sizesMenu entryconfigure [expr $sNum+4] \
      	         -command "StyleSizeDialog style $sNum"
      }

    # set style in use
    set styleNum [style number]
  }
}

# refresh buffer settings in menu
proc RefreshBuffer {{buf {}} {opt {}}} {
  global bufNum

  if {[info level] <= 1} {
    if {$opt != {} && [string is digit -strict $buf] &&
    	[string equal $opt "select"]} {
       set bufNum $buf
    } else {
      set bufNum [buffer]
    }
  }
}

# highlight current mode button
proc RefreshMode {} {
  global modeFrame

  if {[info level] <= 1} {
    set curmode [mode]
    if {$curmode != "none"} {
      # Use "catch" because some modes don't have buttons.
      catch {$modeFrame.$curmode config -background red}
    }
  }
}

# refresh option settings in menu
proc RefreshOption {opt {val {}}} {
  global pinShowNum
  global allDirLines autoSwapLSA orthoMoves csSnap csShowDRCC
  global enfDRCC rubBandMode uniqElemNames autoZeroDM
  global newClrPoly showART thinDraw checkPoly onlyARNets

  if {[info level] <= 1} {
    if {$opt == "pinout_shows_number_not_name"} {
      if {$val != {}} {
        set pinShowNum $val
      } else {
        set pinShowNum [pcb::option pinout_shows_number_not_name]
      }
    } elseif {$opt == "all_direction_lines"} {
      if {$val != {} && ![string equal $val "toggle"]} {
        set allDirLines $val
      } else {
        set allDirLines [pcb::option all_direction_lines]
      }
    } elseif {$opt == "auto_swap_line_start_angle"} {
      if {$val != {}} {
        set autoSwapLSA $val
      } else {
        set autoSwapLSA [pcb::option auto_swap_line_start_angle]
      }
    } elseif {$opt == "orthogonal_moves"} {
      if {$val != {}} {
        set orthoMoves $val
      } else {
        set orthoMoves [pcb::option orthogonal_moves]
      }
    } elseif {$opt == "crosshair_snaps_to_pins_pads"} {
      if {$val != {}} {
        set csSnap $val
      } else {
        set csSnap [pcb::option crosshair_snaps_to_pins_pads]
      }
    } elseif {$opt == "crosshair_shows_drc_clearance"} {
      if {$val != {}} {
        set csShowDRCC $val
      } else {
        set csShowDRCC [pcb::option crosshair_shows_drc_clearance]
      }
    } elseif {$opt == "auto_enforce_drc_clearance"} {
      if {$val != {}} {
        set enfDRCC $val
      } else {
        set enfDRCC [pcb::option auto_enforce_drc_clearance]
      }
    } elseif {$opt == "rubber_band_mode"} {
      if {$val != {}} {
        set rubBandMode $val
      } else {
        set rubBandMode [pcb::option rubber_band_mode]
      }
    } elseif {$opt == "unique_element_names"} {
      if {$val != {}} {
        set uniqElemNames $val
      } else {
        set uniqElemNames [pcb::option unique_element_names]
      }
    } elseif {$opt == "auto_zero_delta_measurements"} {
      if {$val != {}} {
        set autoZeroDM $val
      } else {
        set autoZeroDM [pcb::option auto_zero_delta_measurements]
      }
    } elseif {$opt == "new_lines_arcs_clear_polygons"} {
      if {$val != {}} {
        set newClrPoly $val
      } else {
        set newClrPoly [pcb::option new_lines_arcs_clear_polygons]
      }
    } elseif {$opt == "show_autorouter_trials"} {
      if {$val != {}} {
        set showART $val
      } else {
        set showART [pcb::option show_autorouter_trials]
      }
    } elseif {$opt == "thin_draw"} {
      if {$val != {}} {
        set thinDraw $val
      } else {
        set thinDraw [pcb::option thin_draw]
      }
    } elseif {$opt == "check_polygons"} {
      if {$val != {}} {
        set checkPoly $val
      } else {
        set checkPoly [pcb::option check_polygons]
      }
    } elseif {$opt == "only_autorouted_nets"} {
      if {$val != {}} {
        set onlyARNets $val
      } else {
        set onlyARNets [pcb::option only_autorouted_nets]
      }
    }
  }
}

# refresh grid settings in menu
proc RefreshGrid {opt {val {}}} {
  global gridVal gridView

  if {[info level] <= 1} {
    if {$opt == "display"} {
      if {$val != {}} {
        set gridView $val
      } else {
        set gridView [grid display]
      }
    } elseif {$opt == "set"} {
      set gridVal [grid get]
    } elseif {$opt == "get"} {
      set gridVal [grid get]
    }
  }
}

# refresh zoom settings in menu
proc RefreshZoom {} {
  global zoomVal

  if {[info level] <= 1} {
    set zVal [zoom]
    switch -exact -- $zVal {
      "-4.0" {set zoomVal 4:1}
      "-2.0" {set zoomVal 2:1}
      "0.0" {set zoomVal 1:1}
      "2.0" {set zoomVal 1:2}
      "4.0" {set zoomVal 1:4}
      "6.0" {set zoomVal 1:8}
      "8.0" {set zoomVal 1:16}
      default {set zoomVal $zVal}
    }
  }
}

# refresh line settings in menu
proc RefreshLine {opt {val {}}} {

  if {[info level] <= 1} {
    if {$opt == "size" && $val != {}} {
        RefreshStyleMenu
    }
  }
}

# refresh netlist settings in menu
proc RefreshNetlist {opt {val {}} {arg3 {}} {arg4 {}}} {

  if {[info level] <= 1} {
    if {$opt == "clearance" && $val != {}} {
        RefreshStyleMenu
    }
    # Update netlist window (if netlist exists).
    if {$opt == "enable" ||  $opt == "disable"} {
        set netInfo [netlist info]
        catch {InitListbox .pcb.netlistView.listBoxes.nets $netInfo }
    }

  }
}

# refresh via settings in menu
proc RefreshVia {opt {val {}}} {

  if {[info level] <= 1 && $val != {}} {
    if {$opt == "size" || $opt == "hole"} {
        RefreshStyleMenu
    }
  }
}

# update layer info when changed
proc RefreshLayerMenu {} {
  global layerFrame
  global layers
  set fgColor ""
  set bgColor ""

  set layers {}
  for {set i 0} {$i <= 12} {incr i} {
    set layerName [layer name $i]
    set layerColor [layer color $i]
    lappend layers [list $layerName $layerColor]
    # set layer name, color not specified in layout file
    $layerFrame.$i configure -text $layerName
  }

  # Update color of popup menu items as well
  # Update layer names in Activelayer dropdown menu.
  for {set i 0} {$i <= 9} {incr i} {
      if { $i == 9 } {
        ${layerFrame}.activeLayer.menu entryconfigure [expr ($i+1)] -label "                netlist"
        ${layerFrame}.activeLayer.menu entryconfigure [expr ($i+1)] \
	    -command "layer active netlist"
        # Sepcial case for Netlist layer
        ${layerFrame}.activeLayer.menu entryconfigure [expr ($i+1)] \
                -background "[lindex [lindex $layers 12] 1]"
     } else {
        ${layerFrame}.activeLayer.menu entryconfigure [expr ($i+1)] -label "                [lindex [lindex $layers $i] 0]"
        ${layerFrame}.activeLayer.menu entryconfigure [expr ($i+1)] \
	    -command "layer active $i"
        ${layerFrame}.activeLayer.menu entryconfigure [expr ($i+1)] \
                -background "[lindex [lindex $layers $i] 1]"
     }
  }
  # Update Active layer name.
  ${layerFrame}.activeLayer configure -text [layer active]
  ${layerFrame}.activeLayer configure -background [layer color]

  # refresh all layers to handle layer groupings in visibility,
  for {set i 0} {$i <= 12} {incr i} {
    if {[layer is_visible $i] == "on"} {
      set bgColor [lindex [lindex $layers $i] 1]
      set fgColor white
    } else {
      set fgColor [lindex [lindex $layers $i] 1]
      set bgColor white
    }
    $layerFrame.$i config -foreground $fgColor -background $bgColor
  }
}

# refresh File->Print and Window->Netlist status.
proc RefreshMenus {{printStat {}} {nListStat {}}} {
  global fileMenu
  global windowMenu

 if {[info level] <= 1} {
    if {$printStat != {}} {
       # deactivate Print menu in File menu until layout changed.
       $fileMenu entryconfigure 7 -state $printStat
    }
    if {$nListStat != {}} {
       # deactivate Netlist menu in Window menu if no Netlist data available.
       $windowMenu entryconfigure 3 -state $nListStat
    }
    RefreshWindowMenu
  }
}

# Dialog to choose file for load/save
# command to be invoked and descriptive text passed from caller
proc FileDialog {text cmd} {
  #set dirFileList [glob -nocomplain *]

  if {[catch {wm state .pcb.fileBrowse}]} {
    toplevel .pcb.fileBrowse
    wm title .pcb.fileBrowse "Files"
    wm resizable .pcb.fileBrowse 0 0

    label .pcb.fileBrowse.dialogLabel -text "${text}:"
    label .pcb.fileBrowse.fileNameLabel -text [pwd]
    entry .pcb.fileBrowse.fileNameEntry -textvariable fileName

    pack .pcb.fileBrowse.dialogLabel .pcb.fileBrowse.fileNameLabel \
    .pcb.fileBrowse.fileNameEntry -side top -anchor w

    frame .pcb.fileBrowse.listBoxes

    listbox .pcb.fileBrowse.listBoxes.dirs -borderwidth 2 -width 30 \
    	-yscrollcommand ".pcb.fileBrowse.listBoxes.dirsScroll set"
    scrollbar .pcb.fileBrowse.listBoxes.dirsScroll \
    	-command ".pcb.fileBrowse.listBoxes.dirs yview"
    listbox .pcb.fileBrowse.listBoxes.files -borderwidth 2 -width 50 \
    	-yscrollcommand ".pcb.fileBrowse.listBoxes.filesScroll set"
    scrollbar .pcb.fileBrowse.listBoxes.filesScroll \
    	-command ".pcb.fileBrowse.listBoxes.files yview"

    pack .pcb.fileBrowse.listBoxes.dirs -side left
    pack .pcb.fileBrowse.listBoxes.dirsScroll -side left -fill y
    pack .pcb.fileBrowse.listBoxes.files -side left
    pack .pcb.fileBrowse.listBoxes.filesScroll -side left -fill y
    pack .pcb.fileBrowse.listBoxes -side top

    frame .pcb.fileBrowse.okCancel

    button .pcb.fileBrowse.okCancel.ok -text Ok -command \
	[subst {eval $cmd \[.pcb.fileBrowse.fileNameEntry get\] ; \
		wm withdraw .pcb.fileBrowse}]
    button .pcb.fileBrowse.okCancel.cancel -text Cancel \
    	-command "wm withdraw .pcb.fileBrowse"

    pack .pcb.fileBrowse.okCancel.ok .pcb.fileBrowse.okCancel.cancel -side left
    pack .pcb.fileBrowse.okCancel -side top

    # bind keys to select and list directories and files
    bind .pcb.fileBrowse.listBoxes.dirs <ButtonRelease-1> {
      set dirItem [.pcb.fileBrowse.listBoxes.dirs curselection]
      set dir [.pcb.fileBrowse.listBoxes.dirs get $dirItem]
      cd $dir
      ReadDir .pcb.fileBrowse.listBoxes.dirs .pcb.fileBrowse.listBoxes.files
      .pcb.fileBrowse.fileNameLabel config -text [pwd]
    }

    bind .pcb.fileBrowse.listBoxes.files <ButtonRelease-1> {
      set fileItem [.pcb.fileBrowse.listBoxes.files curselection]
      set fileItemName [.pcb.fileBrowse.listBoxes.files get $fileItem]
      .pcb.fileBrowse.fileNameEntry delete 0 end
      .pcb.fileBrowse.fileNameEntry insert 0 $fileItemName
    }

    bind .pcb.fileBrowse <Key-Escape> {wm withdraw .pcb.fileBrowse}
    bind .pcb.fileBrowse <Key-Return> {.pcb.fileBrowse.okCancel.ok invoke}

    # populate listboxes with dir info
    ReadDir .pcb.fileBrowse.listBoxes.dirs .pcb.fileBrowse.listBoxes.files

  } else {
     # Update label of FileDialog window.
    .pcb.fileBrowse.dialogLabel configure -text "${text}:"
    .pcb.fileBrowse.okCancel.ok configure -command \
	[subst {eval $cmd \[.pcb.fileBrowse.fileNameEntry get\] ; \
		wm withdraw .pcb.fileBrowse}]
    wm deiconify .pcb.fileBrowse
  }
}

proc ReadDir {dirList fileList} {
  $dirList delete 0 end
  $fileList delete 0 end

  set dirFileList [eval exec ls -a]
  if {$dirFileList != ""} {
    set sortList [lsort $dirFileList]
    foreach fName $sortList {
      if {[file isdirectory $fName] != 0} {
        $dirList insert end "$fName"
      } else {
        $fileList insert end "$fName"
      }
    }
  }
}

# indicate which layers will be turned on/off together
proc LayerGroupsDialog {} {
  global layers
  # array lg contains layer groupings

  if {[catch {wm state .pcb.layerGroups}]} {
    toplevel .pcb.layerGroups
    wm minsize .pcb.layerGroups 325 300
    wm maxsize .pcb.layerGroups 325 300
    wm resizable .pcb.layerGroups 0 0
    wm title .pcb.layerGroups "Layer Groups"
    #label .pcb.layerGroups.lab -text "Layer/Group #"
    #pack .pcb.layerGroups.lab  -side top -anchor nw

    frame .pcb.layerGroups.matrix
    label .pcb.layerGroups.matrix.desc -text "Layer/Group #"
    grid config .pcb.layerGroups.matrix.desc -row 0 -column 0
    for {set i 1} {$i <= 8} {incr i} {
      set lName [lindex [lindex $layers [expr $i-1]] 0]
      # In tcl version 8.3.1 label name starting with uppercase not allowed!
      set lName [string tolower $lName]
      set lColor [lindex [lindex $layers [expr $i-1]] 1]
      label .pcb.layerGroups.matrix.$i -text $i
      label .pcb.layerGroups.matrix.$lName$i -text $lName -foreground $lColor
      grid config .pcb.layerGroups.matrix.$lName$i -row $i -column 0
      grid config .pcb.layerGroups.matrix.$i -row 0 -column $i
      for {set j 1} {$j <= 8} {incr j} {
        radiobutton .pcb.layerGroups.matrix.$i$j -variable lg($i) -value $j \
	    -selectcolor $lColor
        grid config .pcb.layerGroups.matrix.$i$j -row $i -column $j
      }
    }
    label .pcb.layerGroups.matrix.cSide -text "component side"
    grid config .pcb.layerGroups.matrix.cSide -row 9 -column 0
    for {set j 1} {$j <= 8} {incr j} {
      radiobutton .pcb.layerGroups.matrix.9$j -variable cLayer -value $j \
          -selectcolor black
      grid config .pcb.layerGroups.matrix.9$j -row 9 -column $j
    }
    label .pcb.layerGroups.matrix.sSide -text "solder side"
    grid config .pcb.layerGroups.matrix.sSide -row 10 -column 0
    for {set j 1} {$j <= 8} {incr j} {
      radiobutton .pcb.layerGroups.matrix.10$j -variable sLayer -value $j \
          -selectcolor black
      grid config .pcb.layerGroups.matrix.10$j -row 10 -column $j
    }

    frame .pcb.layerGroups.okCancel
    button .pcb.layerGroups.okCancel.ok -text Ok \
    	-command "UpdateLayerGroups; destroy .pcb.layerGroups"
    button .pcb.layerGroups.okCancel.cancel -text Cancel \
    	-command "destroy .pcb.layerGroups"
    pack .pcb.layerGroups.okCancel.ok .pcb.layerGroups.okCancel.cancel \
        -side left
    pack .pcb.layerGroups.matrix .pcb.layerGroups.okCancel -side top

    bind .pcb.layerGroups <Key-Escape> {destroy .pcb.layerGroups}
    bind .pcb.layerGroups <Key-Return> {.pcb.layerGroups.okCancel.ok invoke}

    #initialize current settings
    set lgString [layer group]
    set idx 1
    set lgLen [string length $lgString]
    for {set i 0} {$i < $lgLen} {incr i} {
      set lNum [string index $lgString $i]
      switch -glob $lNum {

        ":" {incr idx}
	"c" {.pcb.layerGroups.matrix.9${idx} select}
	"s" {.pcb.layerGroups.matrix.10${idx} select}
	{[0-9]} {.pcb.layerGroups.matrix.${lNum}${idx} select}
      }
    }
  } else {
    wm deiconify .pcb.layerGroups
  }
}

# create a string with same format as written out in the layout file to reuse
# parse functions in the backend
proc UpdateLayerGroups {} {
  global lg;
  global sLayer
  global cLayer
  set lgString ""

  if {$cLayer == $sLayer} {
    puts "The component and solder layers cannot be in the same group"
    return ""
  }
  for {set j 1} {$j <= 8} {incr j} {
    set lgAdd 0
    for {set i 1} {$i <= 8} {incr i} {
      if {$lg($i) == $j} {
	set lgLen [string length $lgString]
	if {$lgLen != 0 && [string index $lgString end] != ":"} {
	  set lgString "$lgString,"
	}
        set lgString "$lgString$i"
	incr lgAdd
      }
    }
    if {$cLayer == $j} {
      if {[string range $lgString end end] != ":"} {
	set lgString "$lgString,"
      }
      set lgString "${lgString}c"
      if {$lgAdd == 0} {
        puts "There needs to be more than one layer in the group \
	      containing the component or solder layer"
	return ""
      }
    }
    if {$sLayer == $j} {
      if {[string range $lgString end end] != ":"} {
	set lgString "$lgString,"
      }
      set lgString "${lgString}s"
      if {$lgAdd == 0} {
        puts "There needs to be more than one layer in the group \
	      containing the component or solder layer"
	return ""
      }
    }
    if {$lgAdd} {
      set lgString "$lgString:"
    }
  }
  set lgString [string trimright $lgString ":"]
  puts $lgString
  layer group $lgString
}

proc RefreshPrintDialog {} {
    # update all fields.
    .pcb.print.fileNameEntry delete 0 end
    .pcb.print.fileNameEntry insert 0 [print file]
    .pcb.print.scale set [print scale]

    set driver [print driver]
    if {$driver == "ps" } {.pcb.print.driver.ps select}
    if {$driver == "eps" } {.pcb.print.driver.eps select}
    if {$driver == "gbr" } {.pcb.print.driver.gerber select}

    if {$driver == "ps" || $driver == "eps"} {
      if { [print rotate] == "on" } {.pcb.print.rmci.rotate select} \
	else {.pcb.print.rmci.rotate deselect}
      if { [print mirror] == "on" } {.pcb.print.rmci.mirror select} \
    	else {.pcb.print.rmci.mirror deselect}
      if { [print color] == "on" } {.pcb.print.rmci.color select} \
    	else {.pcb.print.rmci.color deselect}
      catch {pack .pcb.print.rmci.rotate .pcb.print.rmci.mirror \
        .pcb.print.rmci.color -in .pcb.print.rmci -side left}
    } else {
      catch {pack forget .pcb.print.rmci.rotate .pcb.print.rmci.mirror \
	  .pcb.print.rmci.color}
    }

    if { [print invert] == "on" } {.pcb.print.rmci.invert select} \
    	else {.pcb.print.rmci.invert deselect}
    if { [print outline] == "on" } {.pcb.print.oad.outline select}  \
    	else {.pcb.print.oad.outline deselect}
    if { [print alignment] == "on" } {.pcb.print.oad.alignment select} \
    	else {.pcb.print.oad.alignment deselect}
    if { [print drill_helper] == "on" } {.pcb.print.oad.drillHelper select} \
    	else {.pcb.print.oad.drillHelper deselect}
    if { [print dos] == "on" } {.pcb.print.dos.dosCompat select} \
    	else {.pcb.print.dos.dosCompat deselect}

    if {$driver == "gbr"} {
      catch {
        pack forget .pcb.print.scaleLabel .pcb.print.scale .pcb.print.scaleSep
      }
    } else {
      catch {
        pack .pcb.print.scaleLabel .pcb.print.scale .pcb.print.scaleSep \
	    -side top -fill x -after .pcb.print.oadSep
      }
    }

    if {$driver == "ps"} {
      set media [print media]		
      switch -exact $media {
        "  A3  " {.pcb.print.paper.a3 select}
        "  A4  " {.pcb.print.paper.a4 select}
        "  A5  " {.pcb.print.paper.a5 select}
        "Letter" {.pcb.print.paper.letter select}
        "Ledger" {.pcb.print.paper.ledger select}
        "Legal" {.pcb.print.paper.legal select}
        "Executive" {.pcb.print.paper.executive select}
        "C-size" {.pcb.print.paper.csize select}
        "D-size" {.pcb.print.paper.dsize select}
        default {.pcb.print.paper.user select}
      }
      catch {
        pack .pcb.print.paperLabel .pcb.print.paper .pcb.print.paperSep \
	    -side top -fill x -after .pcb.print.scaleSep
      }
    } else {
      catch {
        pack forget .pcb.print.paperLabel .pcb.print.paper .pcb.print.paperSep
      }
    }

}

# Set offsets from the print dialog window
proc setoffsets {} {
  set xoffinches [.pcb.print.margins.xentry get]
  set yoffinches [.pcb.print.margins.yentry get]
  if {$xoffinches == ""} {set xoffinches 0}
  if {$yoffinches == ""} {set yoffinches 0}
  if {[catch {set xoffint [expr {100000 * int($xoffinches)}]}]} {return false}
  if {[catch {set yoffint [expr {100000 * int($yoffinches)}]}]} {return false}
  print offset $xoffint $yoffint
  return true
}

# print dialog - screen varies with type of output chosen to
# show only valid options for output type chosen
proc PrintDialog {{dDriver {ps}}} {
  global xMargin yMargin

  set fileName ""
  if {[catch {wm state .pcb.print}]} {
    toplevel .pcb.print
    wm title .pcb.print "Print"
    wm resizable .pcb.print 0 0

    label .pcb.print.driverLabel -text "select device driver:" -anchor w
    frame .pcb.print.driver
    radiobutton .pcb.print.driver.ps -text "Postscript" -variable deviceDriver \
    	-value ps -selectcolor red2 \
	-command "print driver ps; RefreshPrintDialog"
    radiobutton .pcb.print.driver.eps -text "Encapsulated Postscript" \
    	-variable deviceDriver -value eps -selectcolor red2 \
	-command "print driver eps; RefreshPrintDialog"
    radiobutton .pcb.print.driver.gerber -text "Gerber/RS-274X" \
    	-variable deviceDriver -value gbr -selectcolor red2 \
	-command "print driver gbr; RefreshPrintDialog"
    .pcb.print.driver.$dDriver select
    label .pcb.print.driverSep -text "" -anchor w
    pack .pcb.print.driver.ps .pcb.print.driver.eps .pcb.print.driver.gerber \
        -side left
    pack .pcb.print.driverLabel .pcb.print.driver .pcb.print.driverSep \
        -side top -fill x

    label .pcb.print.rmciLabel -text "select options:" -anchor w
    frame .pcb.print.rmci
    checkbutton .pcb.print.rmci.invert -text "invert" -variable rmciInvert \
    	-onvalue on -offvalue off -command {print invert $rmciInvert} \
	-selectcolor red2
    label .pcb.print.rmciSep -text "" -anchor w
    pack .pcb.print.rmci.invert -side left
    checkbutton .pcb.print.rmci.rotate -text "rotate" -variable rmciRotate \
    	-onvalue on -offvalue off -command {print rotate $rmciRotate} \
	-selectcolor red2
    checkbutton .pcb.print.rmci.mirror -text "mirror" -variable rmciMirror \
    	-onvalue on -offvalue off -command {print mirror $rmciMirror} \
	-selectcolor red2
    checkbutton .pcb.print.rmci.color -text "color" -variable rmciColor \
    	-onvalue on -offvalue off -command {print color $rmciColor} \
	-selectcolor red2
    pack .pcb.print.rmci.rotate .pcb.print.rmci.mirror .pcb.print.rmci.color \
        -side left
    pack .pcb.print.rmciLabel .pcb.print.rmci .pcb.print.rmciSep -side top \
        -fill x

    label .pcb.print.oadLabel -text "select outline and alignment targets:" \
    	-anchor w
    frame .pcb.print.oad
    checkbutton .pcb.print.oad.outline -text "outline" -variable oadOutline \
    	-onvalue on -offvalue off -command {print outline $oadOutline} \
	-selectcolor red2
    checkbutton .pcb.print.oad.alignment -text "alignment" -variable \
        oadAlignment -onvalue on -offvalue off \
	-command {print alignment $oadOutline} -selectcolor red2
    checkbutton .pcb.print.oad.drillHelper -text "drill helper" \
    	-variable oadDrillHelper -onvalue on -offvalue off \
	-command {print drill_helper $oadDrillHelper} -selectcolor red2

    label .pcb.print.oadSep -text "" -anchor w
    pack .pcb.print.oad.outline .pcb.print.oad.alignment \
        .pcb.print.oad.drillHelper -side left
    pack .pcb.print.oadLabel .pcb.print.oad .pcb.print.oadSep -side top -fill x

    set printScale 1.0
    label .pcb.print.scaleLabel -text "select output scale here:" -anchor w
    scale .pcb.print.scale -from 0.2 -to 5.0 -length 5c -orient horizontal \
   	-variable printScale -resolution 0.1 -command {print scale }
    #.pcb.print.scale set 1.0

    frame .pcb.print.margins
    set margins [print offset]
    set xMargin [expr [lindex $margins 0] / 100000.0]
    set yMargin [expr [lindex $margins 1] / 100000.0]
    label .pcb.print.margins.xlabel -text "horizontal offset (inches):" -anchor w
    entry .pcb.print.margins.xentry -background white -relief sunken \
	-validate focusout -validatecommand "setoffsets"
    label .pcb.print.margins.ylabel -text "vertical offset (inches):" -anchor w
    entry .pcb.print.margins.yentry -background white -relief sunken \
	-validate focusout -validatecommand "setoffsets"
    pack .pcb.print.margins.xlabel .pcb.print.margins.xentry \
	.pcb.print.margins.ylabel .pcb.print.margins.yentry -side left
    pack .pcb.print.margins -side top

    label .pcb.print.scaleSep -text "" -anchor w
    pack .pcb.print.scaleLabel .pcb.print.scale .pcb.print.scaleSep -side top \
        -fill x
    .pcb.print.margins.xentry delete 0 end
    .pcb.print.margins.yentry delete 0 end
    .pcb.print.margins.xentry insert 0 $xMargin
    .pcb.print.margins.yentry insert 0 $yMargin

    label .pcb.print.paperLabel -text "select paper size here:" -anchor w
    frame .pcb.print.paper
    radiobutton .pcb.print.paper.a3 -text a3 -variable paperSize -value A3 \
    	-selectcolor red2 -command {print media $paperSize}
    radiobutton .pcb.print.paper.a4 -text a4 -variable paperSize -value A4 \
    	-selectcolor red2 -command {print media $paperSize}
    radiobutton .pcb.print.paper.a5 -text a5 -variable paperSize -value A5 \
    	-selectcolor red2 -command {print media $paperSize}
    radiobutton .pcb.print.paper.csize -text C-size -variable paperSize \
    	-value C -selectcolor red2 -command {print media $paperSize}
    radiobutton .pcb.print.paper.dsize -text D-size -variable paperSize \
    	-value D -selectcolor red2 -command {print media $paperSize}
    radiobutton .pcb.print.paper.legal -text legal -variable paperSize \
    	-value legal -selectcolor red2 -command {print media $paperSize}
    radiobutton .pcb.print.paper.letter -text letter -variable paperSize \
    	-value letter -selectcolor red2 -command {print media $paperSize}
    radiobutton .pcb.print.paper.ledger -text ledger -variable paperSize \
    	-value ledger -selectcolor red2 -command {print media $paperSize}
    radiobutton .pcb.print.paper.executive -text executive -variable paperSize \
    	-value executive -selectcolor red2 -command {print media $paperSize}
    radiobutton .pcb.print.paper.user -text user -variable paperSize \
        -value user -selectcolor red2 -command {print media $paperSize}
    .pcb.print.paper.letter select
    grid config .pcb.print.paper.a3 -row 0 -column 0 -sticky ew
    grid config .pcb.print.paper.a4 -row 0 -column 1 -sticky ew
    grid config .pcb.print.paper.a5 -row 0 -column 2 -sticky ew
    grid config .pcb.print.paper.csize -row 0 -column 3 -sticky ew
    grid config .pcb.print.paper.dsize -row 0 -column 4 -sticky ew
    grid config .pcb.print.paper.legal -row 1 -column 0 -sticky ew
    grid config .pcb.print.paper.letter -row 1 -column 1 -sticky ew
    grid config .pcb.print.paper.ledger -row 1 -column 2 -sticky ew
    grid config .pcb.print.paper.executive -row 1 -column 3 -sticky ew
    grid config .pcb.print.paper.user -row 1 -column 4 -sticky ew
    label .pcb.print.paperSep -text "" -anchor w
    pack .pcb.print.paperLabel .pcb.print.paper .pcb.print.paperSep -side top \
        -fill x

    label .pcb.print.dosLabel -text "select for DOS compatible names:" -anchor w
    frame .pcb.print.dos
    checkbutton .pcb.print.dos.dosCompat -text "DOS(8.3) names" -variable \
        printDOS -onvalue on -offvalue off -command {print dos $printDOS}
    label .pcb.print.dosSep -text "" -anchor w
    pack .pcb.print.dos.dosCompat -side left
    pack .pcb.print.dosLabel .pcb.print.dos .pcb.print.dosSep -side top -fill x

    label .pcb.print.fileLabel -text "enter filename:" -anchor w
    entry .pcb.print.fileNameEntry -textvariable printFile -text $fileName

    pack .pcb.print.fileLabel .pcb.print.fileNameEntry -side top -fill x

    frame .pcb.print.okCancel
    button .pcb.print.okCancel.ok -text Ok \
    	-command {eval print file [.pcb.print.fileNameEntry get]; \
	wm withdraw .pcb.print}
    button .pcb.print.okCancel.cancel -text Cancel \
        -command "wm withdraw .pcb.print"
    pack .pcb.print.okCancel.ok .pcb.print.okCancel.cancel -side left
    pack .pcb.print.okCancel -side top

    bind .pcb.print  <Key-Escape> {wm withdraw .pcb.print}
    bind .pcb.print <Key-Return> {.pcb.print.okCancel.ok invoke}

    RefreshPrintDialog
  } else {
    RefreshPrintDialog
    wm deiconify .pcb.print
  }
}

proc NetlistDialog {args} {
  global netInfo
  global net

  if {[catch {wm state .pcb.netlistView}]} {
    toplevel .pcb.netlistView
    wm title .pcb.netlistView "Netlist"
    wm resizable .pcb.netlistView 0 0

    frame .pcb.netlistView.netInfo

    label .pcb.netlistView.netInfo.netLabel -text "Nodes displayed for net: "
    label .pcb.netlistView.netInfo.netName -text " " -foreground red
    label .pcb.netlistView.netInfo.styleLabel -text "Style: "
    label .pcb.netlistView.netInfo.styleName -text "" -foreground red

    frame .pcb.netlistView.buttonFrame

    button .pcb.netlistView.buttonFrame.selectNet -text "Select Net On Layout" \
    	-command {set scrollPos \
	[lindex [.pcb.netlistView.listBoxes.nets yview] 0];\
        netlist show_net $net; \
        .pcb.netlistView.listBoxes.nets yview moveto $scrollPos }

    button .pcb.netlistView.buttonFrame.disableNets -text "Disable All Nets" \
    	-command {netlist disable all; set netInfo [netlist info]; \
	InitListbox .pcb.netlistView.listBoxes.nets $netInfo}
    button .pcb.netlistView.buttonFrame.enableNets -text "Enable All Nets" \
    	-command {netlist enable all; set netInfo [netlist info]; \
	InitListbox .pcb.netlistView.listBoxes.nets $netInfo}

    pack .pcb.netlistView.buttonFrame.selectNet \
    	.pcb.netlistView.buttonFrame.disableNets \
	.pcb.netlistView.buttonFrame.enableNets -side left
    pack .pcb.netlistView.buttonFrame -side top

    pack .pcb.netlistView.netInfo.netLabel .pcb.netlistView.netInfo.netName \
    	.pcb.netlistView.netInfo.styleLabel .pcb.netlistView.netInfo.styleName \
	-side left -fill y
    pack .pcb.netlistView.netInfo -side top

    frame .pcb.netlistView.listBoxes

    listbox .pcb.netlistView.listBoxes.nets -borderwidth 2 \
    	-yscrollcommand ".pcb.netlistView.listBoxes.netsScroll set" -width 25 \
	-selectmode single
    scrollbar .pcb.netlistView.listBoxes.netsScroll \
    	-command ".pcb.netlistView.listBoxes.nets yview"
    listbox .pcb.netlistView.listBoxes.connections -borderwidth 2 \
    	-yscrollcommand ".pcb.netlistView.listBoxes.connectionsScroll set" \
	-width 25 -selectmode single
    scrollbar .pcb.netlistView.listBoxes.connectionsScroll \
    	-command ".pcb.netlistView.listBoxes.connections yview"

    pack .pcb.netlistView.listBoxes.nets -side left -fill both -expand true
    pack .pcb.netlistView.listBoxes.netsScroll -side left -fill y
    pack .pcb.netlistView.listBoxes.connections -side left -fill both \
        -expand true
    pack .pcb.netlistView.listBoxes.connectionsScroll -side left -fill y
    pack .pcb.netlistView.listBoxes -side top -fill both -expand true


    # bind keys to select and list directories and files
    bind .pcb.netlistView.listBoxes.nets <ButtonRelease-1> {
      set netItem [.pcb.netlistView.listBoxes.nets curselection]
      set net [string range \
          [.pcb.netlistView.listBoxes.nets get $netItem] 0 end]
      .pcb.netlistView.netInfo.netName configure -text [string range $net 2 end]
      set netStyle [netlist style $net]
      .pcb.netlistView.netInfo.styleName configure -text $netStyle
      set connInfo [netlist connections $net]
      InitListbox .pcb.netlistView.listBoxes.connections $connInfo
    }

    # bind Double Click-Button1 Enable/Disable Single Net.
    bind .pcb.netlistView.listBoxes.nets <Double-ButtonRelease-1> {
      # Don't allow window to scroll while net being enabled/disabled.
      set scrollPos [lindex [.pcb.netlistView.listBoxes.nets yview] 0]
      set netItem [.pcb.netlistView.listBoxes.nets curselection]
      set net [string range \
          [.pcb.netlistView.listBoxes.nets get $netItem] 0 end]
      .pcb.netlistView.netInfo.netName configure -text [string range $net 2 end]
      set netStyle [netlist style $net]
      .pcb.netlistView.netInfo.styleName configure -text $netStyle
      set connInfo [netlist connections $net]
      InitListbox .pcb.netlistView.listBoxes.connections $connInfo
      if {[string index $net 0] == "*"} {netlist enable $net} \
	else {netlist disable $net}
      set netInfo [netlist info]
      InitListbox .pcb.netlistView.listBoxes.nets $netInfo
      .pcb.netlistView.listBoxes.nets yview moveto $scrollPos
    }

    bind .pcb.netlistView.listBoxes.connections <ButtonRelease-1> {
      set nodeItem [.pcb.netlistView.listBoxes.connections curselection]
      set node [.pcb.netlistView.listBoxes.connections get $nodeItem]
      netlist show_pin $node
    }

    # populate listboxes with nets info
    set netInfo [netlist info]
    InitListbox .pcb.netlistView.listBoxes.nets $netInfo

  } else {
    wm deiconify .pcb.netlistView
  }
}

proc InitListbox {lBox lArg} {
  set bigLen 0
  $lBox delete 0 end
  foreach lName $lArg {
    $lBox insert end "$lName"
    set lNameLen [string length $lName]
    if {$bigLen < $lNameLen} {
      set bigLen $lNameLen
    }
  }
  return $bigLen
}

#Highlight current net when rats lines are drawing on layout.
proc HighlightNet {hNet} {
  global net

  if {$hNet == {} } return
  set index 0
  set netInfo [netlist info]
  set net $hNet

  #Get the real name of net(when called from C routine).
  if {[string index $hNet 0] == "*"  || [string index $hNet 0] == " "} {
     set hNet [string range $hNet 2 end]
  }

  #deselect all nets
  .pcb.netlistView.listBoxes.nets selection clear 0 \
      [.pcb.netlistView.listBoxes.nets index end]
  foreach lName $netInfo {
    set lnet [string range $lName 2 end]
    if {[string equal $lnet $hNet] } {
       #Higlight and scroll Netlist box to current net
       .pcb.netlistView.listBoxes.nets selection set $index
       .pcb.netlistView.listBoxes.nets yview $index

       .pcb.netlistView.netInfo.netName configure -text $hNet
       set netStyle [netlist style $hNet]
       .pcb.netlistView.netInfo.styleName configure -text $netStyle
       set connInfo [netlist connections $hNet]
       InitListbox .pcb.netlistView.listBoxes.connections $connInfo
    }
    incr index
  }
}


#TODO : No more required.
proc OrgInitListbox {lBox lArg {strArg ""}} {
  set bigLen 0
  $lBox delete 0 end
  set lArgSorted [lsort $lArg]
  foreach lName $lArgSorted {
    $lBox insert end "$strArg$lName"
    set lNameLen [string length $lName]
    if {$bigLen < $lNameLen} {
      set bigLen $lNameLen
    }
  }
  return $bigLen
}

proc RefreshNetlistDialog { } {
  global windowMenu

  if {![catch {wm state .pcb.netlistView}]} {
    set netInfo [netlist info]
    # destroy Netlist window if no netlist is there.
    if { [string bytelength $netInfo] == 0} {
       wm withdraw .pcb.netlistView
       # deactivate Netlist menu in Window menu if no Netlist data available.
       $windowMenu entryconfigure 3 -state disabled
       return
    } else {
      InitListbox .pcb.netlistView.listBoxes.nets $netInfo
      catch {InitListbox .pcb.netlistView.listBoxes.connections " "}
      .pcb.netlistView.netInfo.netName configure -text " "
      .pcb.netlistView.netInfo.styleName configure -text " "
    }
  }
}

proc LibraryDialog {args} {

  if {[catch {wm state .pcb.libraryView}]} {
    toplevel .pcb.libraryView
    wm title .pcb.libraryView "Library"
    wm resizable .pcb.libraryView 0 0

    frame .pcb.libraryView.libInfo

    label .pcb.libraryView.libInfo.libLabel -text "Library: "
    label .pcb.libraryView.libInfo.libName -text " " -foreground red
    label .pcb.libraryView.libInfo.elemLabel -text "Element: "
    label .pcb.libraryView.libInfo.elemName -text " " -foreground red

    pack .pcb.libraryView.libInfo.libLabel .pcb.libraryView.libInfo.libName \
    .pcb.libraryView.libInfo.elemLabel .pcb.libraryView.libInfo.elemName \
    -side left -fill y
    pack .pcb.libraryView.libInfo -side top

    frame .pcb.libraryView.listBoxes

    listbox .pcb.libraryView.listBoxes.library -borderwidth 2 \
    	-yscrollcommand ".pcb.libraryView.listBoxes.libraryScroll set" -width 25
    scrollbar .pcb.libraryView.listBoxes.libraryScroll \
    	-command ".pcb.libraryView.listBoxes.library yview"
    listbox .pcb.libraryView.listBoxes.components -borderwidth 2 \
    	-yscrollcommand ".pcb.libraryView.listBoxes.componentsScroll set" \
	-width 50
    scrollbar .pcb.libraryView.listBoxes.componentsScroll \
    	-command ".pcb.libraryView.listBoxes.components yview"

    pack .pcb.libraryView.listBoxes.library -side left -fill both -expand true
    pack .pcb.libraryView.listBoxes.libraryScroll -side left -fill y
    pack .pcb.libraryView.listBoxes.components -side left -fill both \
        -expand true
    pack .pcb.libraryView.listBoxes.componentsScroll -side left -fill y
    pack .pcb.libraryView.listBoxes -side top -fill both -expand true


    # bind keys to select and list directories and files
    bind .pcb.libraryView.listBoxes.library <ButtonRelease-1> {
      set libraryItem [.pcb.libraryView.listBoxes.library curselection]
      set library [.pcb.libraryView.listBoxes.library get $libraryItem]
      .pcb.libraryView.libInfo.libName configure -text $library
      .pcb.libraryView.listBoxes.components delete 0 end
      set libElements [library elements $library]
      set elemLenMin 50
      set elemLenMax 80
      set elemLen \
          [InitListbox .pcb.libraryView.listBoxes.components $libElements]
      if {$elemLen < $elemLenMin} {
        .pcb.libraryView.listBoxes.components configure -width $elemLenMin
      } elseif {$elemLen > $elemLenMax} {
        .pcb.libraryView.listBoxes.components configure -width $elemLenMax
      } else {
        .pcb.libraryView.listBoxes.components configure -width $elemLen
      }
      .pcb.libraryView.libInfo.elemName configure -text " "
    }

    bind .pcb.libraryView.listBoxes.components <ButtonRelease-1> {
      set componentItem [.pcb.libraryView.listBoxes.components curselection]
      set component [.pcb.libraryView.listBoxes.components get $componentItem]
      .pcb.libraryView.libInfo.elemName configure -text $component
      library pick $component
    }

    # populate listboxes with dir info
    set libInfo [library info]
    InitListbox .pcb.libraryView.listBoxes.library $libInfo

  } else {
    wm deiconify .pcb.libraryView
  }
}


#----------------------------------------------------------
# Procedure that prompts the user for selecting any location
# on layout window to continue command.
#----------------------------------------------------------
proc GetLocation {prompt_text} {
  global dprompt
  set dprompt(ok) 0
  if {[catch {.pcb.layout.nameFrame cget -width}]} {
    pack forget .pcb.layout.status
    frame .pcb.layout.nameFrame
    label .pcb.layout.nameFrame.label -text "$prompt_text" -anchor w \
                    -foreground red
    pack .pcb.layout.nameFrame.label  -side left
    pack .pcb.layout.nameFrame -side top -fill x
  }
  # Wait for Button1 press event.
  set canvasBindings [bindtags .pcb.layout.pcbFrame.pcbCanvas]
  bind set1 <Button-1> {set dprompt(ok) 1}
  bindtags .pcb.layout.pcbFrame.pcbCanvas {set1}
  tkwait variable dprompt(ok)

  pack forget .pcb.layout.nameFrame
  pack .pcb.layout.status -side bottom -fill x
  destroy .pcb.layout.nameFrame
  # Restore key/button bindings.
  bindtags .pcb.layout.pcbFrame.pcbCanvas $canvasBindings
}

#----------------------------------------------------------
# Procedure that prompts the user for input on the message
# line, and returns the result.
#----------------------------------------------------------

proc GetUserInput {prompt_text var_default action_proc args} {
  global dprompt
  set dprompt(ok) 0
  if {[catch {.pcb.layout.nameFrame cget -width}]} {
    pack forget .pcb.layout.status
    frame .pcb.layout.nameFrame
    label .pcb.layout.nameFrame.label -text "$prompt_text" -anchor w
    entry .pcb.layout.nameFrame.entry -textvariable $var_default

    pack .pcb.layout.nameFrame.label .pcb.layout.nameFrame.entry -side left
    pack .pcb.layout.nameFrame -side top -fill x
  }

  catch {focus .pcb.layout.nameFrame.entry}
  catch {grab .pcb.layout.nameFrame.entry}
  bind .pcb.layout.nameFrame.entry <Return> {set dprompt(ok) 1}
  bind .pcb.layout.nameFrame.entry <Escape> {pack forget .pcb.layout.nameFrame;\
       pack .pcb.layout.status -side bottom -fill x;\
       destroy .pcb.layout.nameFrame;\
       focus -force .pcb.layout.pcbFrame.pcbCanvas}
  tkwait variable dprompt(ok)

  catch {grab release .pcb.layout.nameFrame.entry}
  catch {$action_proc $args}
  pack forget .pcb.layout.nameFrame
  pack .pcb.layout.status -side bottom -fill x
  destroy .pcb.layout.nameFrame
  focus -force .pcb.layout.pcbFrame.pcbCanvas
}

proc GetLayoutName {} {
  global layoutName
  GetUserInput "enter the layout name: " layoutName {}
  layout name $layoutName
  return $layoutName
}

proc GetLayerName {} {
  global activeLayerName
  set activeLayerName [layer name]
  GetUserInput "enter the layer name: " activeLayerName {}
  layer name [layer name] $activeLayerName
  RefreshLayerMenu
  return $activeLayerName
}

proc GetSelectItemName {} {
  global selectname
  #set activeLayerName [layer name]
  GetUserInput "pattern: " selectname {}
  return $selectname
}

proc GetElementName {} {
  global elementName
  GetUserInput "enter the element name: " elementName {}
  return $elementName
}


proc PinoutBox {} {
  global PCBOpts
  if {[catch {pinout}]} {return }
  if {[catch {wm state .pinout}]} {
    toplevel .pinout

    simple .pinout.draw -height 300 -width 300
    #pack .pinout.draw -side top
    pack .pinout.draw -side top -fill both -expand true

    set PCBOpts(pinout) .pinout.draw
    bind .pinout.draw <Expose> {pinout redraw}
    bind .pinout.draw <Destroy> {pinout delete}

    frame .pinout.buttonFrame
    button .pinout.buttonFrame.dismiss -text "Dismiss" \
    	-command {wm withdraw .pinout}
    button .pinout.buttonFrame.shrink -text "Shrink" \
    	-command {pinout zoom out; pinout redraw}
    button .pinout.buttonFrame.enlarge -text "Enlarge" \
    	-command {pinout zoom in; pinout redraw}

    pack .pinout.buttonFrame.dismiss .pinout.buttonFrame.shrink \
    	.pinout.buttonFrame.enlarge  -side left
    pack .pinout.buttonFrame -side top
  } else {
    wm deiconify .pinout
  }
  pinout create
  pinout redraw
  wm title .pinout [pinout title]
}


# Popup menu for third mouse button
proc PCBPopupMenu {X Y} {
  global PopupMenu
  tk_popup $PopupMenu $X $Y
}

proc WindowOnOff {winName} {
  global  libWinView
  global  netlistWinView
  global  consoleView

  if {$winName == {}} { return }
  if {$winName == "LibraryDialog"} {
    if {$libWinView == "on"} {
      LibraryDialog
    } else {
      catch {wm withdraw .pcb.libraryView}
    }
  } elseif {$winName == "NetlistDialog"} {
    if {$netlistWinView == "on"} {
      NetlistDialog
    } else {
      catch {wm withdraw .pcb.netlistView}
    }
  } else {
    if {$consoleView == "on"} {
      catch {pcb::consoleup}
    } else {
      catch {pcb::consoledown}
    }
  }
}

#----------------------------------------------------------
# Create text window
#----------------------------------------------------------

proc DisplayReport {title cmd} {
  global textFont
  global fieldtext

  if {[catch {wm state .pcb.textdialog}]} {
     toplevel .pcb.textdialog
     wm withdraw .pcb.textdialog
     wm title .pcb.textdialog " "
     wm resizable .pcb.textdialog 0 0
     frame .pcb.textdialog.text
     label .pcb.textdialog.text.field -text "" -justify center -font $textFont
     pack .pcb.textdialog.text.field -side left
     pack .pcb.textdialog.text -side top -padx 4 -pady 4 -fill x

     frame .pcb.textdialog.okButton
     button .pcb.textdialog.okButton.ok -text "Ok" \
       -command {wm withdraw .pcb.textdialog}
     pack .pcb.textdialog.okButton.ok
     pack .pcb.textdialog.okButton
     bind .pcb.textdialog <Key-Escape> {wm withdraw .pcb.textdialog}
     bind .pcb.textdialog <Key-Return> {.pcb.textdialog.okButton.ok invoke}
  }

  wm title .pcb.textdialog $title
  set fieldtext [eval $cmd]

  if {$fieldtext != {}} {
      .pcb.textdialog.text.field configure -text $fieldtext
      wm deiconify .pcb.textdialog
  }
  # Set Text alignment.
  if {[string match Drill* $title]} {
      .pcb.textdialog.text.field configure -justify center
  } else {
      .pcb.textdialog.text.field configure -justify left
  }
  wm geometry .pcb.textdialog +250+250
}

proc helpDoc {} {
  global PCB_LIB_DIR

  if {![catch {which acroread}]} {
     set result [eval exec {which acroread}]
     if {[file executable $result]} {
      eval exec {$result $PCB_LIB_DIR/pcb.pdf &}
    }
  } elseif {![catch {which xpdf}]} {
    set result [eval exec {which xpdf}]
    if {[file executable $result]} {
      eval exec {$result $PCB_LIB_DIR/pcb.pdf &}
    }
  } elseif {![catch {which gpdf}]} {
    set result [eval exec {which gpdf}]
    if {[file executable $result]} {
      eval exec {$result $PCB_LIB_DIR/pcb.pdf &}
    }
  } elseif {![catch {which ggv}]} {
    set result [eval exec {which ggv}]
    if {[file executable $result]} {
      eval exec {$result $PCB_LIB_DIR/pcb.ps &}
    }
  }
}

# Create subwindow for pcb layout
#label $menuFrame.canvasCursorLoc -text "0, 0" -relief sunken
frame .pcb.layout
frame .pcb.layout.pcbFrame -background red -border 1
simple .pcb.layout.pcbFrame.pcbCanvas -background white
pack .pcb.layout.pcbFrame.pcbCanvas -fill both -expand true

# Create frame for the menu bar
#
set menuFrame [frame .pcb.menuFrame -relief raised -borderwidth 1]

#pack .pcb.menuBar -side top -fill x -expand true
pack $menuFrame -side top -fill x
pack .pcb.layout.pcbFrame -ipadx 300 -ipady 100 -side top \
	-fill both -expand true

# File Menu
#
set fileMenu [CreateMenu $menuFrame $file "File" "left"]

# Edit Menu
#
set editMenu [CreateMenu $menuFrame $edit "Edit" "left"]

# Screen Menu
#
set screenMenu [CreateMenu $menuFrame $screen "Screen" "left"]

# Sizes Menu
#
set sizesMenu [CreateMenu $menuFrame $sizes "Sizes" "left"]

# Settings Menu
#
set settingsMenu [CreateMenu $menuFrame $settings "Settings" "left"]

# Select Menu
#
set selectMenu [CreateMenu $menuFrame $select "Select" "left"]

# Buffer Menu
#
set bufferMenu [CreateMenu $menuFrame $buffer "Buffer" "left"]

# Connects Menu
#
set connectsMenu [CreateMenu $menuFrame $connects "Connects" "left"]

# Info Menu
#
set infoMenu [CreateMenu $menuFrame $info "Info" "left"]

# Window Menu
#
set windowMenu [CreateMenu $menuFrame $window "Window" "left"]

# Help Menu
#
set helpMenu [CreateMenu $menuFrame $help "Help" "right"]

# Popup Menu, for third mouse button
#
set PopupMenu [CreateMenu $menuFrame $popup ""]

# Display for cursor location in canvas
label $menuFrame.canvasCursorLoc -text "0, 0" -relief sunken
pack $menuFrame.canvasCursorLoc -side right

# Create frame for the layer bar
#
set sideFrame [frame .pcb.sideFrame]
pack $sideFrame -side left -fill y

# panner (view locator)
set viewLoc [canvas $sideFrame.viewLayout -width 100 -height 100 -borderwidth 2]
$viewLoc create rect 0 0 100 100 -fill grey	;# tag 1
$viewLoc create rect 0 0 50 50 -fill black 	;# tag 2
pack $viewLoc -side top
bind $viewLoc <Button-1> {ViewBoxSet %x %y}
bind $viewLoc <B1-Motion> {ViewBoxMove %x %y}

# layer frame for layer toggle windows and mode buttons
set layerFrame [frame $sideFrame.layerFrame -relief raised -borderwidth 1]

label $layerFrame.visible -text "on/off"

pack $layerFrame -side top -fill x
pack $layerFrame.visible -side top

menubutton $layerFrame.activeLayer -text "none" -menu \
	$layerFrame.activeLayer.menu -background gray50 \
	-foreground white -font $buttonFont
menu $layerFrame.activeLayer.menu
label $layerFrame.active -text "active"

# Create frame for the mode bar
#
set modeFrame [frame $sideFrame.modeFrame -relief flat]

label .pcb.mytip -background yellow -relief groove -borderwidth 2

#
# Mode buttons
#
CreateMode $modeFrame $DrawingModes
pack $modeFrame -side top -fill x -ipady 10

# Status bar
MakeStatusBar

#
# Set global variables to tell the C code where to find various windows
#
set PCBOpts(drawing) .pcb.layout.pcbFrame.pcbCanvas
set PCBOpts(status) .pcb.layout.status
set PCBOpts(cursorpos) $menuFrame.canvasCursorLoc
set PCBOpts(panner) $viewLoc
set PCBOpts(menu) $menuFrame

# deactivate Netlist menu in Window menu if no Netlist data available.
$windowMenu entryconfigure 3 -state disabled

# Allow keystrokes in the drawing window by setting the focus on
# any enter event.  This must be appended ("+") to the existing
# event handler, which handles the auto-scroll start and stop.

bind .pcb.layout.pcbFrame.pcbCanvas <Enter> {+focus %W}


# Key translations (from Pcb.masterForm*output.baseTranslations in Pcb.ad)
bind .pcb.layout.pcbFrame.pcbCanvas <Key-Escape> {mode none}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-space> {mode arrow}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-Delete> {mode save; mode remove; \
						mode notify; mode restore}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-BackSpace> {mode save; mode remove; \
						mode notify; mode restore}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-Insert> {mode insertpoint}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-Tab> {show solder_side toggle}

bind .pcb.layout.pcbFrame.pcbCanvas <Key-exclam> {buffer 1 select}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-at>  {buffer 2 select}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-numbersign> {buffer 3 select}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-dollar>  {buffer 4 select}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-percent>  {buffer 5 select}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-bracketleft> \
	{mode save; mode move; mode notify}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-bracketright> \
	{mode notify; mode restore}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-period> \
	{option all_direction_lines toggle}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-slash> {option cycle_clip}

bind .pcb.layout.pcbFrame.pcbCanvas <Key-1> {layer active 0}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-2> {layer active 1}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-3> {layer active 2}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-4> {layer active 3}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-5> {layer active 4}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-6> {layer active 5}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-7> {layer active 6}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-8> {layer active 7}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-9> {layer active 8}

bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-1> {style use 1}
bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-2> {style use 2}
bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-3> {style use 3}
bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-4> {style use 4}

bind .pcb.layout.pcbFrame.pcbCanvas <ButtonPress-1> {mode notify}
bind .pcb.layout.pcbFrame.pcbCanvas <ButtonRelease-1> {mode release}
bind .pcb.layout.pcbFrame.pcbCanvas <ButtonPress-2> {mode save; mode stroke}
bind .pcb.layout.pcbFrame.pcbCanvas <ButtonRelease-2> \
	{mode release; mode restore}
bind .pcb.layout.pcbFrame.pcbCanvas <ButtonPress-3> {PCBPopupMenu %X %Y}
bind .pcb.layout.pcbFrame.pcbCanvas <Shift-ButtonPress-1> {select object}
bind .pcb.layout.pcbFrame.pcbCanvas <Shift-ButtonRelease-1> { }

bind .pcb.layout.pcbFrame.pcbCanvas <Key-F1> {mode via}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-F2> {mode line}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-F3> {mode paste_buffer}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-F4> {mode rectangle}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-F5> {mode text}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-F6> {mode polygon}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-F7> {mode thermal}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-F8> {mode arc}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-F9> {mode rotate}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-F10> {mode arrow}
bind .pcb.layout.pcbFrame.pcbCanvas <Shift-Key-F3> {buffer rotate 270}

bind .pcb.layout.pcbFrame.pcbCanvas <Key-Up> {pointer 0 -1}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-Down> {pointer 0 1}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-Left> {pointer -1 0}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-Right> {pointer 1 0}
bind .pcb.layout.pcbFrame.pcbCanvas <Shift-Key-Up> {pointer 0 -10}
bind .pcb.layout.pcbFrame.pcbCanvas <Shift-Key-Down> {pointer 0 10}
bind .pcb.layout.pcbFrame.pcbCanvas <Shift-Key-Left> {pointer -10 0}
bind .pcb.layout.pcbFrame.pcbCanvas <Shift-Key-Right> {pointer 10 0}


#bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-a> {select object all}  #Already bound.
#bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-A> {deselect object all}  #Already bound.
bind .pcb.layout.pcbFrame.pcbCanvas <Key-b> {object move otherside}
#bind .pcb.layout.pcbFrame.pcbCanvas <Key-B> {element flip_selected}  #Already bound.
#bind .pcb.layout.pcbFrame.pcbCanvas <Key-c> {layout center}  #Already bound.
bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-c> \
	{buffer clear; buffer add; deselect object all}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-d> {object display_name}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-D> {PinoutBox}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-E> {netlist delete_rats selected}
#bind .pcb.layout.pcbFrame.pcbCanvas <Key-e> {netlist delete_rats all}  #Already bound.
bind .pcb.layout.pcbFrame.pcbCanvas <Key-f> {scan reset all; scan connection}
#bind .pcb.layout.pcbFrame.pcbCanvas <Key-F> {scan reset all}  #Already bound.
bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-f> {scan connection}
#bind .pcb.layout.pcbFrame.pcbCanvas <Key-g> {pcb::grid set +5mil}  #Already bound.
#bind .pcb.layout.pcbFrame.pcbCanvas <Key-G> {pcb::grid set -5mil}  #Already bound.
bind .pcb.layout.pcbFrame.pcbCanvas <Key-h> {element display_name object}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-H> \
	{element display_name selected_elements}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-j> {polygon toggle_clearance object}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-k> {object clearance +200}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-K> {object clearance -200}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-l> {line size +500}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-L> {line size -500}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-m> {object move current}
bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-m> {pointer mark}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-p> {polygon previous}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-P> {polygon close}
bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-q> {quit}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-q> {object square}
# Override macro created for Menu.
bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-r> {DisplayReport Report {object report}}
#bind .pcb.layout.pcbFrame.pcbCanvas <Key-R> {redo}                          # Already Bound.
#bind .pcb.layout.pcbFrame.pcbCanvas <Key-r> {layout redraw}               # Already Bound.
bind .pcb.layout.pcbFrame.pcbCanvas <Key-s> {object size +500}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-S> {object size -500}
bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-s> {object hole +500}
bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-S> {object hole -500}
#bind .pcb.layout.pcbFrame.pcbCanvas <Key-u> {undo}                            # Already Bound.
#bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-U> {undo clear}        # Already Bound.
bind .pcb.layout.pcbFrame.pcbCanvas <Key-v> {via size +500}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-V> {via size -500}
bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-v> {via hole +500}
bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-V> {via hole -500}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-w> {netlist add_rats all}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-W> {netlist add_rats selected}
#bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-x> {buffer copy}         # Already Bound.
#bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-X> {buffer cut}           # Already Bound.
bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-x> \
	{buffer clear; buffer add; delete selected}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-z> {zoom in}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-Z> {zoom out}
bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-z> {undo}
bind .pcb.layout.pcbFrame.pcbCanvas <Control-Key-Z> {undo clear}

#Not avail in menu.c file.
bind .pcb.layout.pcbFrame.pcbCanvas <Key-t> {text scale +20}
bind .pcb.layout.pcbFrame.pcbCanvas <Key-T> {text scale -20}
bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-k>  {netlist clearance +200}
bind .pcb.layout.pcbFrame.pcbCanvas <Alt-Key-K>  {netlist clearance -200}

# call backs to refresh menus and buttons when commands invoked from
# the command line interface
pcb::tag mode "ShowMode $modeFrame %1"
pcb::tag layer "UpdateLayers $layerFrame %1 %2"
pcb::tag layout "UpdateScreen %1 %2"
pcb::tag show "RefreshShow %1 %2"
pcb::tag grid "RefreshGrid %1 %2"
pcb::tag buffer "RefreshBuffer %1 %2"
pcb::tag zoom "RefreshZoom"
pcb::tag option "RefreshOption %1 %2"
pcb::tag style "RefreshStyleMenu"
pcb::tag line "RefreshLine %1 %2"
pcb::tag via "RefreshVia %1 %2"
pcb::tag netlist "RefreshNetlist %1 %2"
# Close all windows except layout window. Windows can be
# made on/off from Window menu.
pcb::tag start "CreateLayers $layerFrame; RefreshMode; \
	tkwait visibility .pcb.layout.pcbFrame.pcbCanvas; \
	focus -force .pcb.layout.pcbFrame.pcbCanvas;  \
	InitMenuOptions; catch {wm withdraw .pcb.libraryView}; \
	catch {wm withdraw .pcb.netlistView}; pcb::consoledown;"


