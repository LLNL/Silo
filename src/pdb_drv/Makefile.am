## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_pdb_drv.la
libsilo_pdb_drv_la_SOURCES = $(FILES)

AM_CPPFLAGS = -I$(srcdir)/../pdb -I$(builddir)/../silo -I$(srcdir)/../silo -I$(srcdir)/../score

noinst_HEADERS = \
 silo_pdb_private.h
FILES = \
 pjgroup.c \
 pjjacket.c \
 pjobj.c \
 silo_pdb.c
