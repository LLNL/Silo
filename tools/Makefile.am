#
# Makefile.am --- automake input file for Silo
#
## Process this file with automake to create Makefile.in

BROWSER_DIR =
if BROWSER_NEEDED
BROWSER_DIR += browser python silock
endif
SUBDIRS = \
 . \
 $(BROWSER_DIR)
DIST_SUBDIRS = \
 . \
 browser \
 python \
 silock
