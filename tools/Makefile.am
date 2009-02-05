#
# Makefile.am --- automake input file for Silo
#
## Process this file with automake to create Makefile.in

TOOLS_DIR =
if BROWSER_NEEDED
TOOLS_DIR += browser silock
endif
if PYTHON_NEEDED
TOOLS_DIR += python
endif
SUBDIRS = \
 . \
 $(TOOLS_DIR)
DIST_SUBDIRS = \
 . \
 browser \
 python \
 silock
