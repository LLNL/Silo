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
 SiloFile.h \
 silexstream.h

FILES = \
 Explorer.C \
 SiloView.C \
 SiloDirView.C \
 SiloDirTreeView.C \
 SiloArrayView.C \
 SiloValueView.C \
 SiloObjectView.C \
 SiloFile.C \
 main.C 

FILES_MOC = \
 Explorer_moc.C \
 SiloView_moc.C \
 SiloDirView_moc.C \
 SiloDirTreeView_moc.C \
 SiloArrayView_moc.C \
 SiloValueView_moc.C \
 SiloObjectView_moc.C
 
%_moc.C: %.h
	$(QT_MOC) -o $@ $<

BUILT_SOURCES = $(FILES_MOC)

CLEANFILES = $(FILES_MOC)
