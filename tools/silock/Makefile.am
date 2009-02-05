## Procss this file with automake to create Makefile.in

bin_PROGRAMS = silock
silock_SOURCES = silock.c
if HDF5_DRV_NEEDED
  silock_LDADD = ../../src/libsiloh5.la
else
  silock_LDADD = ../../src/libsilo.la
endif
AM_CPPFLAGS = -I$(includedir) -I$(top_srcdir)/src/silo
