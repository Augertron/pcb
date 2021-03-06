#
#                             COPYRIGHT
# 
#   PCB, interactive printed circuit board design
#   Copyright (C) 1994,1995,1996 Thomas Nau
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# 
#   Contact addresses for paper mail and Email:
#   Thomas Nau, Schlehenweg 15, 88471 Baustetten, Germany
#   Thomas.Nau@rz.uni-ulm.de
# 
#   RCS: $Id: to.inc,v 1.1.1.1 2006/04/10 22:05:22 tim Exp $
#
# TO packages

# -------------------------------------------------------------------
# a TO3 housing
# 
# $1: canonical name
# $2: name on PCB
# $3: value
#
# by Olaf Kaluza <criseis!olaf@ruhrgebiet.individual.net>
define(`PKG_TO3_90',
`Element(0x00 "$1" "$2" "$3" 400 800 0 100 0x00)
(
	PIN(650, 1000, 120, 40, 1)
	PIN(650, 550, 120, 40, 2)
	PIN(1320, 775, 250, 120, 3)
	PIN(125, 775, 250, 120, 4)

	ElementArc(700 775 500 500 70 40 20)
	ElementArc(700 775 500 500 250 40 20)
	ElementArc(1320 775 180 180 125 110 20)
	ElementArc(125 775 180 180 305 110 20)
	ElementLine(25 925 530 1245 20)
	ElementLine(25 625 530 305 20)
	ElementLine(870 305 1430 630 20)
	ElementLine(870 1245 1430 920 20)

	Mark(650 775)
)')

# by Volker Bosch, 45 degree, TO3
define(`PKG_TO3_45',
`Element(0x00 "$1" "$2" "$3" 480 510 0 100 0x00)
(
	PIN(750, 750, 120, 30, 1)
	PIN(960, 380, 120, 30, 2)
	PIN(1300, 830, 250, 120, 3)
	PIN(280, 210, 250, 120, 4)

	ElementLine(660 1010 1300 1010 20)
	ElementLine(1210 230 1470 770 20)
	ElementLine(110 270 375 810 20)
	ElementLine(280 30 920 30 20)

	ElementArc(790 520 420 420 0 360 20)
	ElementArc(790 520 510 510 215 40 20)
	ElementArc(790 520 510 510 35 40 20)

	ElementArc(1300 830 180 180 90 110 20)
	ElementArc(280 210 180 180 270 110 20)

	Mark(750 750)
)')

# -------------------------------------------------------------------
# a TO5 housing
# 
# $1: canonical name
# $2: name on PCB
# $3: value
#
# by Volker Bosch (bosch@iema.e-technik.uni-stuttgart.de)
define(`PKG_TO5',
`Element(0x00 "$1" "$2" "$3" 110 110 0 100 0x00)
(
	PIN(100, 200, 80, 20, 1)
	PIN(200, 300, 80, 20, 2)
	PIN(300, 200, 80, 20, 3)

	ElementArc(200 200 150 150 0 360 10)
	ElementArc(200 200 170 170 0 360 20)
	ElementLine(65 95 35 65 20)
	ElementLine(35 65 65 35 20)
	ElementLine(65 35 95 65 20)

	Mark(100 200)
)')

# -------------------------------------------------------------------
# a TO92 housing
# 
# $1: canonical name
# $2: name on PCB
# $3: value
#
# by Volker Bosch (bosch@iema.e-technik.uni-stuttgart.de)
# lineare Anordnung der Pins
define(`PKG_TO92',
`Element(0x00 "$1" "$2" "$3" 60 70 0 100 0x00)
(
	PIN(50, 200, 80, 20, 1)
	PIN(150, 200, 80, 20, 2)
	PIN(250, 200, 80, 20, 3)

	ElementArc(150 200 100 100 315 270 10)
	ElementLine( 80 130 220 130 10)

	Mark(50 200)
)')

# -------------------------------------------------------------------
# a TO126 housing
# 
# $1: canonical name
# $2: name on PCB
# $3: value
#
# by Volker Bosch (bosch@iema.e-technik.uni-stuttgart.de)
define(`PKG_TO126LAY',
`Element(0x00 "$1" "$2" "$3" 80 480 1 100 0x00)
(
	PIN(100, 600, 80, 20, 1)
	PIN(200, 600, 80, 20, 2)
	PIN(300, 600, 80, 20, 3) 

	# Befestigungsbohrung
	PIN(200, 170, 130, 110, 4)

	# Anschlussdraehte
	ElementLine(100 600 100 500 30)
	ElementLine(200 600 200 500 30)
	ElementLine(300 600 300 500 30)

	# Gehaeuse
	ElementLine( 50 500 350 500 20)
	ElementLine(350 500 350  70 20)
	ElementLine(350  70  50  70 20)
	ElementLine( 50  70  50 500 20)

	Mark(100 600)
)')

define(`PKG_TO126LAY-WIDE',
`Element(0x00 "$1" "$2" "$3" 80 480 1 100 0x00)
(
	PIN(100, 600, 80, 20, 1)
	PIN(200, 700, 80, 20, 2)
	PIN(300, 600, 80, 20, 3)

	# Befestigungsbohrung
	PIN(200, 170, 130, 110, 4)

	# Anschlussdraehte
	ElementLine(100 600 100 500 30)
	ElementLine(200 700 200 500 30)
	ElementLine(300 600 300 500 30)

	# Gehaeuse
	ElementLine( 50 500 350 500 20)
	ElementLine(350 500 350  70 20)
	ElementLine(350  70  50  70 20)
	ElementLine( 50  70  50 500 20)

	Mark(100 600)
)')

define(`PKG_TO126STAND',
`Element(0x00 "$1" "$2" "$3" 60 170 0 100 0x00)
(
	PIN(100, 100, 80, 20, 1)
	PIN(200, 100, 80, 20, 2)
	PIN(300, 100, 80, 20, 3)

	ElementLine(50 50 350 50 20)
	ElementLine(350 50 350 150 20)
	ElementLine(350 150 50 150 20)
	ElementLine(50 150 50 50 20)

	Mark(100 100)
)')

define(`PKG_TO126STAND-WIDE',
`Element(0x00 "$1" "$2" "$3" 270 170 0 100 0x00)
(
	PIN(100, 100, 80, 20, 1)
	PIN(200, 200, 80, 20, 2)
	PIN(300, 100, 80, 20, 3)

	# Gehaeuse
	ElementLine(200 200 200 150 30)
	ElementLine( 50  50 350  50 20)
	ElementLine(350  50 350 150 20)
	ElementLine(350 150  50 150 20)
	ElementLine( 50 150  50  50 20) 

	# Bohrung
	ElementLine(150 50 150 150 10)
	ElementLine(250 50 250 150 10)

	Mark(100 100)
)')

# -------------------------------------------------------------------
# a TO220 housing
# 
# $1: canonical name
# $2: name on PCB
# $3: value
#
# by Volker Bosch (bosch@iema.e-technik.uni-stuttgart.de)
define(`PKG_TO220LAY',
`Element(0x00 "$1" "$2" "$3" 50 570 1 100 0x00)
(
	PIN(100, 800, 80, 20, 1)
	PIN(200, 800, 80, 20, 2)
	PIN(300, 800, 80, 20, 3)

	# Befestigungsbohrung
	PIN(200, 130, 150, 130, 4)

	# Anschlussdraehte
	ElementLine(100 800 100 620 30)
	ElementLine(200 800 200 620 30)
	ElementLine(300 800 300 620 30)

	# Gehaeuse
	ElementLine(  0 620 400 620 20)
	ElementLine(400 620 400 245 20)
	ElementLine(400 245   0 245 20)
	ElementLine(  0 245   0 620 20)

	# Kuehlfahne mit Kerben
	ElementLine(  0 245 400 245 20)
	ElementLine(400 245 400 120 20)
	ElementLine(400 120 385 120 20)
	ElementLine(385 120 385  50 20)
	ElementLine(385  50 400  50 20)
	ElementLine(400  50 400  10 20)
	ElementLine(400  10   0  10 20)
	ElementLine(  0  10   0  50 20)
	ElementLine(  0  50  15  50 20)
	ElementLine( 15  50  15 120 20)
	ElementLine( 15 120   0 120 20)
	ElementLine(  0 120   0 245 20)

	Mark(200 800)
)')

define(`PKG_TO220LAY-WIDE',
`Element(0x00 "$1" "$2" "$3" 50 570 1 100 0x00)
(
	PIN(100, 800, 80, 20, 1)
	PIN(200, 900, 80, 20, 2)
	PIN(300, 800, 80, 20, 3)
		  
	# Befestigungsbohrung  
	PIN(200, 130, 150, 130, 4)

	# Anschlussdraehte  
	ElementLine(100 800 100 620 30)
	ElementLine(200 900 200 620 30)
	ElementLine(300 800 300 620 30)

	# Gehaeuse
	ElementLine(  0 620 400 620 20)
	ElementLine(400 620 400 245 20)
	ElementLine(400 245   0 245 20)
	ElementLine(  0 245   0 620 20)

	# Kuehlfahne mit Kerben
	ElementLine(  0 245 400 245 20)
	ElementLine(400 245 400 120 20)
	ElementLine(400 120 385 120 20)
	ElementLine(385 120 385  50 20)
	ElementLine(385  50 400  50 20)
	ElementLine(400  50 400  10 20)
	ElementLine(400  10   0  10 20)
	ElementLine(  0  10   0  50 20)
	ElementLine(  0  50  15  50 20)
	ElementLine( 15  50  15 120 20)
	ElementLine( 15 120   0 120 20)
	ElementLine(  0 120   0 245 20)

	Mark(200 800)
)')

define(`PKG_TO220STAND',
`Element(0x00 "$1" "$2" "$3" 0 10 0 100 0x00)
(
	PIN(100, 200, 80, 20, 1)
	PIN(200, 200, 80, 20, 2)
	PIN(300, 200, 80, 20, 3)

	# Gehaeuse
	ElementLine(  0  80 400  80 20)
	ElementLine(400  80 400 260 20)
	ElementLine(400 260   0 260 20) 
	ElementLine(  0 260   0  80 20) 

	# Kuehlfahne icl. Bohrung
	ElementLine(  0  80 400  80 20)
	ElementLine(400  80 400 140 20)
	ElementLine(400 140   0 140 20)
	ElementLine(  0 140   0  80 20)

	ElementLine(130 80 130 140 10)
	ElementLine(270 80 270 140 10)

	Mark(100 200)
)')

define(`PKG_TO220STAND-WIDE',
`Element(0x00 "$1" "$2" "$3" 0 10 0 100 0x00)
(
	PIN(100, 200, 80, 20, 1)
	PIN(200, 300, 80, 20, 2)
	PIN(300, 200, 80, 20, 3)
		  
	# Gehaeuse
	ElementLine(  0  80 400  80 20)
	ElementLine(400  80 400 260 20)
	ElementLine(400 260   0 260 20) 
	ElementLine(  0 260   0  80 20) 
			
	# Kuehlfahne icl. Bohrung
	ElementLine(  0  80 400  80 20)
	ElementLine(400  80 400 140 20)
	ElementLine(400 140   0 140 20)
	ElementLine(  0 140   0  80 20)

	ElementLine(130 80 130 140 10)
	ElementLine(270 80 270 140 10)
		
	# Anschlussdraht
	ElementLine(200 300 200 260 30)
		
	Mark(100 200)
)')

# Small outline transistors, taken from some nice data sheets
# by NEC/CEL for the NE688 series.
# WX, WY are width of the pad footprint
# DX, DY are center-to-center pad spacing
# OX, OY are outline size for drawing
# There is good reason to make OY match the size of plastic package,
# but I suggest making OX small enough to not get ink on the pad,
# even if that understates the size of the real package.
# Jan 6, 2000  Larry Doolittle  <LRDoolittle@lbl.gov>
# Status: Untested, but looks pretty :-)

define(`PKG_BASE_SOT',
`Element(0x00 "$1" "$2" "$3" 0 eval(DY+WY/2+10) 0 100 0x00)
(
	define(`WID', WY)
	define(`RX',  `eval((WX-WID)/2)')
	define(`X1', 0)
	define(`Y1', DY)
	define(`X2', 0)
	define(`Y2', 0)
	define(`X3', DX)
	define(`Y3', `eval(DY/2)')
	# Use Pad instead of PAD so all pads come out square
	Pad(eval(X1-RX)  Y1  eval(X1+RX)  Y1  WID  "1"  0x100)
	Pad(eval(X2-RX)  Y2  eval(X2+RX)  Y2  WID  "2"  0x100)
	Pad(eval(X3-RX)  Y3  eval(X3+RX)  Y3  WID  "3"  0x100)

	define(`LX', `eval((DX-OX)/2)')
	define(`LY', `eval((DY-OY)/2)')
	define(`MX', `eval((DX+OX)/2)')
	define(`MY', `eval((DY+OY)/2)')
	ElementLine(LX LY LX MY 6)
	ElementLine(LX MY MX MY 6)
	ElementLine(MX MY MX LY 6)
	ElementLine(MX LY LX LY 6)

)')

define(`PKG_SOT23_CEL',`
	define(`WX', 40)
	define(`WY', 32)
	define(`DX', 95)
	define(`DY', 78)
	define(`OX', 46)
	define(`OY', 114)
	PKG_BASE_SOT(`$1',`$2',`$3')
')

define(`PKG_SOT323_CEL',`
	define(`WX', 32)
	define(`WY', 24)
	define(`DX', 67)
	define(`DY', 51)
	define(`OX', 26)
	define(`OY', 79)
	PKG_BASE_SOT(`$1',`$2',`$3')
')

define(`PKG_NEC19',`
	define(`WX', 24)
	define(`WY', 24)
	define(`DX', 50)
	define(`DY', 40)
	define(`OX', 18)
	define(`OY', 62)
	PKG_BASE_SOT(`$1',`$2',`$3')
')
