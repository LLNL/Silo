## Procss this file with automake to create Makefile.in

lib_LTLIBRARIES = Silo.la
Silo_la_SOURCES = $(FILES)
Silo_la_LDFLAGS = -module -avoid-version
if HDF5_DRV_NEEDED
  Silo_la_LIBADD = ../../src/libsiloh5.la
else
  Silo_la_LIBADD = ../../src/libsilo.la
endif

AM_CPPFLAGS = $(PYTHON_CPPFLAGS) -I$(top_builddir)/src/silo -I$(includedir)

noinst_HEADERS = \
 pydbfile.h \
 pydbtoc.h \
 pysilo.h 

FILES = \
 pydbfile.cpp \
 pydbtoc.cpp \
 pysilo.cpp \
 pydbfile.h \
 pydbtoc.h \
 pysilo.h 
