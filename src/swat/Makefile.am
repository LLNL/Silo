## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_swat.la
libsilo_swat_la_SOURCES = $(FILES)

AM_CPPFLAGS = -I$(srcdir)/../score

EXTRA_PROGRAMS = swtest
swtest_SOURCES = swtest.c
swtest_LDADD = -L. -lsilo_swat

noinst_HEADERS = \
 swat.h

FILES = \
 randf.c \
 sw_alias.c \
 sw_array.c \
 sw_data.c \
 sw_error.c \
 sw_etc.c \
 sw_getargs.c \
 sw_history.c \
 sw_math.c \
 sw_optval.c \
 sw_print.c \
 sw_string.c
