## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_netcdf.la
libsilo_netcdf_la_SOURCES = $(FILES)

AM_CPPFLAGS = -I$(srcdir)/../pdb -I$(builddir)/../silo -I$(srcdir)/../silo  -I$(srcdir)/../score 

noinst_HEADERS = \
 silo_netcdf_private.h \
 table.h
FILES = \
 api.c \
 ent.c \
 netcdf.c \
 obj.c \
 silo_netcdf.c \
 table.c 
