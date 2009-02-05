## Procss this file with automake to create Makefile.in

if !ZLIB_AVAILABLE
AM_CPPFLAGS = -DWITHOUT_ZLIB
endif

noinst_LTLIBRARIES = libsilo_hzip.la
libsilo_hzip_la_SOURCES = $(FILES)

noinst_HEADERS = \
 bitvector.h \
 hzio.h \
 hzip.h \
 hzmap.h \
 hzmcodec.h \
 hzmdecoder.h \
 hzmencoder.h \
 hzmio.h \
 hzmpredictor.h \
 hzncodec.h \
 hzndecoder.h \
 hznencoder.h \
 hznio.h \
 hznpredictor.h \
 hzresidual.h \
 ibstream.h \
 obstream.h \
 hzio.inl \
 hzmcodec.inl \
 hzncodec.inl \
 hzndecoder.inl \
 hznencoder.inl \
 hznpredictor.inl \
 hzresidual.inl

FILES = \
 hzmdecoder.cpp \
 hzmencoder.cpp \
 hzmio.cpp \
 hzmpredictor.cpp \
 hzmread.cpp \
 hzmwrite.cpp \
 hznio.cpp \
 hznpredictor.cpp \
 hznread.cpp \
 hznwrite.cpp \
 hzutil.cpp
