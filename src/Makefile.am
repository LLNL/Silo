#
# Makefile.am --- automake input file for Silo
#
## Process this file with automake to create Makefile.in

DRIVER_DIRS =
DRIVER_LIBS =
if NETCDF_NEEDED
DRIVER_DIRS += netcdf
DRIVER_LIBS += netcdf/libsilo_netcdf.la
endif
if PDB_DRV_NEEDED
DRIVER_DIRS += pdb_drv
DRIVER_LIBS += pdb_drv/libsilo_pdb_drv.la
endif
if HDF5_DRV_NEEDED
DRIVER_DIRS += hdf5_drv
DRIVER_LIBS += hdf5_drv/libsilo_hdf5_drv.la
endif
if TAURUS_NEEDED
DRIVER_DIRS += taurus
DRIVER_LIBS += taurus/libsilo_taurus.la
endif
SUBDIRS = score pdb swat silo debug \
$(DRIVER_DIRS) unknown filters .
DIST_SUBDIRS = . debug filters hdf5_drv netcdf \
pdb pdb_drv score silo swat taurus unknown

if HDF5_DRV_NEEDED
  lib_LTLIBRARIES = libsiloh5.la
  libsiloh5_la_SOURCES =
  libsiloh5_la_LIBADD = \
   score/libsilo_score.la \
   pdb/libsilo_pdb.la \
   swat/libsilo_swat.la \
   silo/libsilo_silo.la \
   debug/libsilo_debug.la \
   $(DRIVER_LIBS) \
   unknown/libsilo_unknown.la \
   filters/libsilo_filters.la 
  libsiloh5_la_LDFLAGS = -avoid-version
else
  lib_LTLIBRARIES = libsilo.la
  libsilo_la_SOURCES =
  libsilo_la_LIBADD = \
   score/libsilo_score.la \
   pdb/libsilo_pdb.la \
   swat/libsilo_swat.la \
   silo/libsilo_silo.la \
   debug/libsilo_debug.la \
   $(DRIVER_LIBS) \
   unknown/libsilo_unknown.la \
   filters/libsilo_filters.la 
  libsilo_la_LDFLAGS = -avoid-version
endif
