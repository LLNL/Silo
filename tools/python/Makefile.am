## Procss this file with automake to create Makefile.in

lib_LTLIBRARIES = Silo.la
Silo_la_SOURCES = $(FILES)
Silo_la_LDFLAGS = -module -avoid-version

AM_CPPFLAGS = $(PYTHON_CPPFLAGS) -I$(includedir) -I$(top_srcdir)/src/silo

EXTRA_DIST = \
 test_error.py \
 test_read.py \
 test_write.py

noinst_HEADERS = \
 pydbfile.h \
 pydbtoc.h \
 pysilo.h 
FILES = \
 pydbfile.C \
 pydbtoc.C \
 pysilo.C \
 pydbfile.h \
 pydbtoc.h \
 pysilo.h 
