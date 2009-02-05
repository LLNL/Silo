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
if HZIP_NEEDED
DRIVER_DIRS += hzip
DRIVER_LIBS += hzip/libsilo_hzip.la
endif
if FPZIP_NEEDED
DRIVER_DIRS += fpzip
DRIVER_LIBS += fpzip/libsilo_fpzip.la
endif
SUBDIRS = score pdb silo debug \
$(DRIVER_DIRS) unknown filters .
DIST_SUBDIRS = . debug filters hdf5_drv hzip fpzip netcdf \
pdb pdb_drv score silo taurus unknown
MAKESETTINGS = make.settings

if HDF5_DRV_NEEDED
  SETTINGSFILE=libsiloh5.settings
  lib_LTLIBRARIES = libsiloh5.la
  libsiloh5_la_SOURCES = siloh5_version.c
  libsiloh5_la_LIBADD = \
   score/libsilo_score.la \
   pdb/libsilo_pdb.la \
   silo/libsilo_silo.la \
   debug/libsilo_debug.la \
   $(DRIVER_LIBS) \
   unknown/libsilo_unknown.la \
   filters/libsilo_filters.la 
  libsiloh5_la_LDFLAGS = -avoid-version
else
  SETTINGSFILE=libsilo.settings
  lib_LTLIBRARIES = libsilo.la
  libsilo_la_SOURCES = silo_version.c
  libsilo_la_LIBADD = \
   score/libsilo_score.la \
   pdb/libsilo_pdb.la \
   silo/libsilo_silo.la \
   debug/libsilo_debug.la \
   $(DRIVER_LIBS) \
   unknown/libsilo_unknown.la \
   filters/libsilo_filters.la 
  libsilo_la_LDFLAGS = -avoid-version
endif

BUILT_SOURCES = $(MAKESETTINGS)
$(MAKESETTINGS): Makefile
	@echo "\"Makefile Summary\\\\n\"" | \
		sed 's|\\\\|\\|'> $(MAKESETTINGS); \
	echo "\"Compile command:         $(COMPILE) \\\\n\"" | \
		sed 's|\\\\|\\|'>> $(MAKESETTINGS); \
	echo "\"Libtool command:         $(LTCOMPILE) \\\\n\"" | \
		sed 's|\\\\|\\|'>> $(MAKESETTINGS);\
	echo "\"Link command:            $(LINK) $(LIBS) \\\\n\"" | \
		sed 's|\\\\|\\|'>> $(MAKESETTINGS); \
	if test -f $(SETTINGSFILE); then \
	  cat $(MAKESETTINGS) >> $(SETTINGSFILE); fi

install-data-local:
	@test -z "$(libdir)" || $(mkdir_p) "$(DESTDIR)$(libdir)"; \
	if test -f $(SETTINGSFILE); then \
	  cat $(SETTINGSFILE) | sed 's|\\n||' | \
		sed 's|"||g' > $(SETTINGSFILE).tmp; \
	  $(INSTALL) $(SETTINGSFILE).tmp $(DESTDIR)$(libdir)/$(SETTINGSFILE); \
          rm -f $(SETTINGSFILE).tmp; \
        fi

uninstall-local:
	@rm -f $(DESTDIR)$(libdir)/$(SETTINGSFILE)

clean-local:
	-rm -f $(MAKESETTINGS)
