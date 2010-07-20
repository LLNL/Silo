# Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
# LLNL-CODE-425250.
# All rights reserved.
# 
# This file is part of Silo. For details, see silo.llnl.gov.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the disclaimer below.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the disclaimer (as noted
#      below) in the documentation and/or other materials provided with
#      the distribution.
#    * Neither the name of the LLNS/LLNL nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
# 
# THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
# "AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
# LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
# LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
# CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
# PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
# NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# This work was produced at Lawrence Livermore National Laboratory under
# Contract No.  DE-AC52-07NA27344 with the DOE.
# 
# Neither the  United States Government nor  Lawrence Livermore National
# Security, LLC nor any of  their employees, makes any warranty, express
# or  implied,  or  assumes  any  liability or  responsibility  for  the
# accuracy, completeness,  or usefulness of  any information, apparatus,
# product, or  process disclosed, or  represents that its use  would not
# infringe privately-owned rights.
# 
# Any reference herein to  any specific commercial products, process, or
# services by trade name,  trademark, manufacturer or otherwise does not
# necessarily  constitute or imply  its endorsement,  recommendation, or
# favoring  by  the  United  States  Government  or  Lawrence  Livermore
# National Security,  LLC. The views  and opinions of  authors expressed
# herein do not necessarily state  or reflect those of the United States
# Government or Lawrence Livermore National Security, LLC, and shall not
# be used for advertising or product endorsement purposes.
#
## Procss this file with automake to create Makefile.in


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
if PDBP_DRV_NEEDED
DRIVER_DIRS += pdbp_drv
DRIVER_LIBS += pdbp_drv/libsilo_pdbp_drv.la
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
pdb pdb_drv pdbp_drv score silo taurus unknown
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
