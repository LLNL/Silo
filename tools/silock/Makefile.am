## Procss this file with automake to create Makefile.in

bin_PROGRAMS = silock
silock_SOURCES = silock.c
if HDF5_DRV_NEEDED
if HZIP_NEEDED
  # Dummy C++ source to cause C++ linking.
  nodist_EXTRA_silock_SOURCES = dummy.cxx
endif
if FPZIP_NEEDED
  # Dummy C++ source to cause C++ linking.
  nodist_EXTRA_silock_SOURCES = dummy.cxx
endif
  silock_LDADD = ../../src/libsiloh5.la
else
  silock_LDADD = ../../src/libsilo.la
endif
AM_CPPFLAGS = -I$(top_builddir)/src/silo -I$(includedir)
