## Procss this file with automake to create Makefile.in

bin_PROGRAMS = silock
silock_SOURCES = silock.c
if HDF5_DRV_NEEDED
  silock_LDADD = -L$(libdir) -L../../src -lsiloh5
else
  silock_LDADD = -L$(libdir) -L../../src -lsilo
endif
AM_CPPFLAGS = -I$(includedir) -I$(top_srcdir)/src/silo
