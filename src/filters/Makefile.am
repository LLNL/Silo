## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_filters.la
libsilo_filters_la_SOURCES = $(FILES)

AM_CPPFLAGS = -I$(builddir)/../silo -I$(srcdir)/../silo

noinst_HEADERS = \
 filter.h
FILES = \
 f_ale3d.c \
 f_sample.c
