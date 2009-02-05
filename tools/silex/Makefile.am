## Procss this file with automake to create Makefile.in

bin_PROGRAMS = silex
silex_SOURCES = $(FILES)
nodist_silex_SOURCES = $(FILES_MOC)
if HDF5_DRV_NEEDED
  silex_LDADD = ../../src/libsiloh5.la $(QT_LIBS)
else
  silex_LDADD = ../../src/libsilo.la $(QT_LIBS)
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
 -I$(includedir) \
 -I$(top_srcdir)/src/silo

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
