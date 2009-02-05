## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_silo.la
libsilo_silo_la_SOURCES = $(FILES)

EXTRA_DIST = \
 silo_fcb.c 

include_HEADERS = \
 silo.h \
 silo.inc 
noinst_HEADERS = \
 silo_drivers.h \
 silo_f.h \
 silo_private.h \
 silo_win32_compatibility.h

FILES = \
 alloc.c \
 extface.c \
 silo.c \
 silo_f.c

BUILT_SOURCES = silo.inc
DISTCLEANFILES = silo_version.h silo.inc

# built if it doesn't exit, could cause problem with VPATH
silo.inc: silo.h silo_f.h
	if test ! -e silo.inc; then \
	  cat $(srcdir)/silo.h $(srcdir)/silo_f.h | $(top_srcdir)/config/mkinc > $@; \
	fi
