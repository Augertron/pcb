#--------------------------------
# PCB Resource setup for TCL
#--------------------------------

option add *pcb.alignmentDistance      20000
option add *pcb.allDirectionLines      Off
option add *pcb.backupInterval         60
option add *pcb.charactersPerLine      78
option add *pcb.connectedColor         green
option add *pcb.crosshairColor         red
option add *pcb.crossColor             yellow
option add *pcb.elementColor           black
option add *pcb.elementSelectedColor   cyan
option add *pcb.elementCommand         "M4PATH='%p';export M4PATH;echo 'include(%f)' | m4"
option add *pcb.elementPath             ".:/usr/local/share/pcb/newlib:packages:circuits:/usr/local/share/pcb:/usr/local/share/pcb/circuits:/usr/local/share/pcb/packages"
option add *pcb.fileCommand            "cat '%f'"
option add *pcb.filePath               "."
option add *pcb.fontCommand            "M4PATH='%p';export M4PATH;echo 'include(%f)' | m4"
option add *pcb.fontFile               default_font
option add *pcb.fontPath               ".:/usr/local/share/pcb"
option add *pcb.grid                   2000
option add *pcb.gridColor	       red
option add *pcb.warnColor              hotpink
option add *pcb.maskColor              red
option add *pcb.invertedFrameWidth     500
option add *pcb.invisibleObjectsColor  gray65
option add *pcb.invisibleMarkColor     gray70
option add *pcb.layerColor1            brown4
option add *pcb.layerColor2            RoyalBlue3
option add *pcb.layerColor3            khaki3
option add *pcb.layerColor4            OrangeRed3
option add *pcb.layerColor5            PaleGreen4
option add *pcb.layerColor6            burlywood4
option add *pcb.layerColor7            turquoise4
option add *pcb.layerColor8            forestgreen
option add *pcb.layerGroups            "1,c:2,s:3:4:5:6:7:8"
option add *pcb.layerName1             component
option add *pcb.layerName2             solder
option add *pcb.layerName3             ground
option add *pcb.layerName4             power
option add *pcb.layerName5             signal1
option add *pcb.layerName6             signal2
option add *pcb.layerName7             unused
option add *pcb.layerName8             unused
option add *pcb.layerSelectedColor1    cyan
option add *pcb.layerSelectedColor2    cyan
option add *pcb.layerSelectedColor3    cyan
option add *pcb.layerSelectedColor4    cyan
option add *pcb.layerSelectedColor5    cyan
option add *pcb.layerSelectedColor6    cyan
option add *pcb.layerSelectedColor7    cyan
option add *pcb.layerSelectedColor8    cyan
option add *pcb.libraryCommand         "/usr/local/share/pcb/QueryLibrary.sh '%p' '%f' %a"
option add *pcb.libraryContentsCommand "/usr/local/share/pcb/ListLibraryContents.sh '%p' '%f'"
option add *pcb.libraryFilename        pcblib
option add *pcb.libraryPath            ".:/usr/local/share/pcb"
option add *pcb.libraryTree            /usr/local/share/pcb/newlib
option add *pcb.lineThickness          1000
option add *pcb.media                  letter
option add *pcb.menuFile		      /usr/local/share/pcb/pcb-menu.res
option add *pcb.offLimitColor          gray80
option add *pcb.pinColor               gray30
option add *pcb.pinSelectedColor       cyan
option add *pcb.pinoutFont0            -*-courier-bold-r-*-*-24-*-*-*-*-*-*-*
option add *pcb.pinoutFont1            -*-courier-bold-r-*-*-12-*-*-*-*-*-*-*
option add *pcb.pinoutFont2            -*-courier-bold-r-*-*-8-*-*-*-*-*-*-*
option add *pcb.pinoutFont3            -*-courier-bold-r-*-*-4-*-*-*-*-*-*-*
option add *pcb.pinoutFont4            -*-courier-bold-r-*-*-2-*-*-*-*-*-*-*
option add *pcb.pinoutNameLength       8
option add *pcb.pinoutOffsetX          100
option add *pcb.pinoutOffsetY          100
option add *pcb.pinoutTextOffsetX      8
option add *pcb.pinoutTextOffsetY      8
option add *pcb.pinoutZoom             2
option add *pcb.printFile              "%f.output"
option add *pcb.raiseLogWindow         On
option add *pcb.ratColor               DarkGoldenrod
option add *pcb.ratSelectedColor       cyan
option add *pcb.ratThickness           1000
option add *pcb.resetAfterElement      On
option add *pcb.ringBellWhenFinished   Off
option add *pcb.routeStyles	      "Signal,1000,3600,2000,1000:Power,2500,6000,3500,1000:Fat,4000,6000,3500,1000:Skinny,600,2402,1181,600"
option add *pcb.saveCommand            "cat - > '%f'"
option add *pcb.saveInTMP              Off
option add *pcb.saveLastCommand        Off
option add *pcb.size                   6000x5000
option add *pcb.stipplePolygons        Off
option add *pcb.textScale              100
# useLogWindow disabled for now, until converted to Tk (Tim, 1/21/05)
option add *pcb.useLogWindow           Off
option add *pcb.uniqueNames            On
option add *pcb.snapPin                On
option add *pcb.clearLines             On
option add *pcb.viaColor               gray50
option add *pcb.viaSelectedColor       cyan
option add *pcb.viaThickness           6000
option add *pcb.viaDrillingHole        2800
option add *pcb.volume                 100
option add *pcb.zoom                   3
option add *pcb.Bloat		       699
option add *pcb.minWid		       800
option add *pcb.Shrink		       400
option add *pcb.minSlk		       800
option add *pcb*beNiceToColormap       false
option add *pcb*background	       gray90
option add *pcb*horizDistance          2

# use same font used for drill summary, object report etc in popup dialogs
option add *Dialog.msg.font            {Fixed 10}

# End of Resource configuration file.
