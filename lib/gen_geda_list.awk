#!/usr/bin/awk -f
#
# $Id: gen_geda_list.awk,v 1.1.1.1 2006/04/10 22:05:22 tim Exp $
#
# Script to regenerate geda.list from geda.inc
#
# Usage:
#
#  awk -f gen_geda_list.awk geda.inc > geda.list
#

BEGIN {
	printf("#\n");
	printf("# NOTE: Auto-generated. Do not change.\n");
	printf("#");
}

/^\#\#/ {
	printf("#\n");
	next;
}


/^[ \t]*define/ {
	pkg = $1;
	ind = index(pkg, "PKG");
	pkg = substr(pkg, ind+4);
	ind = index(pkg, "'");
	pkg = substr(pkg, 1, ind-1);
	printf("geda_%s:%s:%s\n", pkg, pkg, pkg);
}

