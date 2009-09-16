## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_taurus.la
libsilo_taurus_la_SOURCES = $(FILES)

AM_CPPFLAGS = -I$(builddir)/../silo -I$(srcdir)/../silo

EXTRA_DIST = \
 README

noinst_HEADERS = \
 silo_taurus_private.h \
 taurus.h
FILES = \
 silo_taurus.c \
 taurus.c
