## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_hexzip.la
libsilo_hexzip_la_SOURCES = $(FILES)

EXTRA_PROGRAMS = testlib
testlib_SOURCES = testlib.cpp
testlib_LDADD = -L. -lsilo_hexzip

include_HEADERS = \
 hexcompressor.h
noinst_HEADERS = \
 hccodec.h \
 hcdecoder.h \
 hcencoder.h \
 predictor.h \
 read.h \
 sap.h \
 write.h

FILES = \
 hcdecoder.cpp \
 hcencoder.cpp \
 predictor.cpp \
 read.cpp \
 sap.cpp \
 write.cpp
