## Procss this file with automake to create Makefile.in
##
##   Mark C. Miller, Wed Sep  2 15:25:10 PDT 2009
##   Added READLINE_LIBS

bin_PROGRAMS = browser
dist_bin_SCRIPTS = \
 silodiff \
 silofile \
 silofile.magic

browser_SOURCES = $(FILES)
if HDF5_DRV_NEEDED
if HZIP_NEEDED
  # Dummy C++ source to cause C++ linking.
  nodist_EXTRA_browser_SOURCES = dummy.cxx
endif
if FPZIP_NEEDED
  # Dummy C++ source to cause C++ linking.
  nodist_EXTRA_browser_SOURCES = dummy.cxx
endif
  browser_LDADD = ../../src/libsiloh5.la
else
  browser_LDADD = ../../src/libsilo.la
endif
browser_LDADD += $(READLINE_LIBS)
info_TEXINFOS = browser.texi

AM_CPPFLAGS = \
 -I$(top_builddir)/src/silo \
 -I$(includedir) \
 -DPUBLIC_INIT_FILE=\"$(includedir)/browser_rc\"

EXTRA_DIST = \
 browser.html \
 browser.init \
 sample.init \
 silodiff.init

noinst_HEADERS = browser.h
FILES = \
 array.c \
 bif.c \
 browser.c \
 cons.c \
 file.c \
 func.c \
 lex.c \
 num.c \
 obj.c \
 output.c \
 parse.c \
 prim.c \
 ptr.c \
 range.c \
 sdo.c \
 stc.c \
 str.c \
 switch.c \
 sym.c
