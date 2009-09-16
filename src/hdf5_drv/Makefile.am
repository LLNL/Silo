## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_hdf5_drv.la
libsilo_hdf5_drv_la_SOURCES = $(FILES)
AM_CPPFLAGS = -I$(builddir)/../silo -I$(srcdir)/../silo

if HZIP_NEEDED
AM_CPPFLAGS += -I$(srcdir)/../hzip
endif

if FPZIP_NEEDED
AM_CPPFLAGS += -I$(srcdir)/../fpzip 
endif

noinst_HEADERS = \
 silo_hdf5_private.h
FILES = \
 silo_hdf5.c
