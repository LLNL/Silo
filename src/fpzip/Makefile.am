## Procss this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsilo_fpzip.la
libsilo_fpzip_la_SOURCES = $(FILES)

FPZIP_FP =              FPZIP_FP_SAFE
#FPZIP_FP =             FPZIP_FP_FAST
#FPZIP_FP =             FPZIP_FP_EMUL
#FPZIP_FP =             FPZIP_FP_INT
FPZIP_BLOCK_SIZE =      0x1000
FPZIP_VERBOSE =         0
AM_CPPFLAGS = -DFPZIP_BLOCK_SIZE=$(FPZIP_BLOCK_SIZE) \
              -DFPZIP_FP=$(FPZIP_FP) \
              -DFPZIP_VERBOSE=$(FPZIP_VERBOSE)

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
 write.h \
 fpzip.h \
 fpzip_error.h

FILES = \
 rcdecoder.cpp \
 rcencoder.cpp \
 rcqsmodel.cpp \
 read.cpp \
 write.cpp
