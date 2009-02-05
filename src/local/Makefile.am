## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_local.la
libsilo_local_la_SOURCES = $(FILES)

LOCAL_FP =              LOCAL_FP_SAFE
#LOCAL_FP =             LOCAL_FP_FAST
#LOCAL_FP =             LOCAL_FP_EMUL
#LOCAL_FP =             LOCAL_FP_INT
LOCAL_BLOCK_SIZE =      0x1000
LOCAL_VERBOSE =         0
AM_CPPFLAGS = -DLOCAL_BLOCK_SIZE=$(LOCAL_BLOCK_SIZE) \
              -DLOCAL_FP=$(LOCAL_FP) \
              -DLOCAL_VERBOSE=$(LOCAL_VERBOSE)

EXTRA_PROGRAMS = testlocal
testlocal_SOURCES = testlocal.cpp
testlocal_LDADD = -L. -lsilo_local

include_HEADERS = \
 local.h
noinst_HEADERS = \
 codec.h \
 fpe.h \
 fpe.inl \
 front.h \
 pccodec.h \
 pccodec.inl \
 pcdecoder.h \
 pcdecoder.inl \
 pcencoder.h \
 pcencoder.inl \
 pcmap.h \
 pcmap.inl \
 rcdecoder.h \
 rcdecoder.inl \
 rcencoder.h \
 rcencoder.inl \
 rcmodel.h \
 rcqsmodel.h \
 rcqsmodel.inl \
 read.h \
 write.h

FILES = \
 rcdecoder.cpp \
 rcencoder.cpp \
 rcqsmodel.cpp \
 read.cpp \
 write.cpp
