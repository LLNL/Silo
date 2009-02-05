## Procss this file with automake to create Makefile.in

bin_PROGRAMS = browser
dist_bin_SCRIPTS = silodiff
browser_SOURCES = $(FILES)
if HDF5_DRV_NEEDED
  browser_LDADD = -L$(libdir) -L../../src -lsiloh5
else
  browser_LDADD = -L$(libdir) -L../../src -lsilo
endif
info_TEXINFOS = browser.texi

AM_CPPFLAGS = \
 -I$(includedir) \
 -I$(top_srcdir)/src/swat \
 -I$(top_srcdir)/src/silo \
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
