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


bin_PROGRAMS = silex
silex_SOURCES = $(FILES)
nodist_silex_SOURCES = $(FILES_MOC)
if HDF5_DRV_NEEDED
  silex_LDADD = ../../src/libsiloh5.la $(QT_LDFLAGS) $(QT_LIBS)
else
  silex_LDADD = ../../src/libsilo.la $(QT_LDFLAGS) $(QT_LIBS)
endif

EXTRA_DIST = \
 array.xpm \
 curve.xpm \
 folder.xpm \
 mat.xpm \
 mesh.xpm \
 object.xpm \
 silovar.xpm \
 species.xpm \
 var.xpm

AM_CXXFLAGS=$(QT_CXXFLAGS)
AM_CPPFLAGS = \
 -I$(top_srcdir)/src/silo \
 -I$(includedir)

noinst_HEADERS = \
 Explorer.h \
 SiloView.h \
 SiloDirView.h \
 SiloDirTreeView.h \
 SiloArrayView.h \
 SiloValueView.h \
 SiloObjectView.h \
 SiloFile.h 

FILES = \
 Explorer.cpp \
 SiloView.cpp \
 SiloDirView.cpp \
 SiloDirTreeView.cpp \
 SiloArrayView.cpp \
 SiloValueView.cpp \
 SiloObjectView.cpp \
 SiloFile.cpp \
 main.cpp 

FILES_MOC = \
 Explorer_moc.cpp \
 SiloView_moc.cpp \
 SiloDirView_moc.cpp \
 SiloDirTreeView_moc.cpp \
 SiloArrayView_moc.cpp \
 SiloValueView_moc.cpp \
 SiloObjectView_moc.cpp
 
%_moc.cpp: %.h
	$(QT_MOC) -o $@ $<

BUILT_SOURCES = $(FILES_MOC)

CLEANFILES = $(FILES_MOC)

install-exec-hook: $(BUNDLE_TARGET)

silex.app:
	@if test -e $@; then \
	  rm -f $@/Contents/{bin,lib}; \
	  rm -rf $@; \
	fi; \
	mkdir $@; \
	mkdir $@/Contents; \
	mkdir $@/Contents/MacOS; \
	mkdir $@/Contents/Resources; \
	{
	  echo '<?xml version="1.0" encoding="UTF-8"?>'; \
	  echo '<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">'; \
	  echo '<plist version="1.0">'; \
	  echo '<dict>'; \
	  echo '        <key>CFBundleName</key>'; \
	  echo '        <string>@PACKAGE_TARNAME@</string>'; \
	  echo '        <key>CFBundlePackageType</key>'; \
	  echo '        <string>APPL</string>'; \
	  echo '        <key>CFBundleGetInfoString</key>'; \
	  echo '        <string>Created by @CONFIG_USER@ on @CONFIG_DATE@</string>'; \
	  echo '        <key>CFBundleSignature</key>'; \
	  echo '        <string>SILX</string>'; \
	  echo '</dict>'; \
	  echo '</plist>'; \
	} > $@/Contents/Info.plist
	echo 'APPLSILX' > $@/Contents/PkgInfo

.PHONY: silex.app
