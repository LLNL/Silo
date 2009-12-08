## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_silo.la
libsilo_silo_la_SOURCES = $(FILES)

EXTRAFILES =
EXTRAINCS =
if FORTRAN_NEEDED
 EXTRAFILES += silo_f.c
 EXTRAINCS += silo_f.h
endif

EXTRA_DIST = \
 silo_fcb.c \
 silo_f.h \
 silo_f.c \
 silo.h.in

include_HEADERS = \
 silo.inc \
 pmpio.h

nodist_include_HEADERS = \
 silo.h

noinst_HEADERS = \
 silo_drivers.h \
 silo_private.h \
 silo_win32_compatibility.h \
 $(EXTRAINCS)

FILES = \
 alloc.c \
 extface.c \
 silo.c  \
 silo_ns.c \
 $(EXTRAFILES)

BUILT_SOURCES = silo.inc
DISTCLEANFILES = silo_version.h

# built if it doesn't exit, could cause problem with VPATH
# be extra careful about check-in time-stamps! silo.inc MUST be the oldest.
silo.inc: silo.h.in silo_f.h
	if test ! -e $@; then \
	  cat ./silo.h $(srcdir)/silo_f.h | $(top_srcdir)/config/mkinc > $@; fi
