## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_debug.la
libsilo_debug_la_SOURCES = $(FILES)

AM_CPPFLAGS = -I$(builddir)/../silo -I$(srcdir)/../silo

noinst_HEADERS = \
 silo_debug_private.h
FILES = \
 silo_debug.c
