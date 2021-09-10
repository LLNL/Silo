# Copyright (C) 1994-2016 Lawrence Livermore National Security, LLC.
# LLNL-CODE-425250.
# All rights reserved.
# 
# This file is part of Silo. For details, see silo.llnl.gov.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the disclaimer below.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the disclaimer (as noted
#      below) in the documentation and/or other materials provided with
#      the distribution.
#    * Neither the name of the LLNS/LLNL nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
# 
# THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
# "AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
# LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
# LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
# CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
# PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
# NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# This work was produced at Lawrence Livermore National Laboratory under
# Contract No.  DE-AC52-07NA27344 with the DOE.
# 
# Neither the  United States Government nor  Lawrence Livermore National
# Security, LLC nor any of  their employees, makes any warranty, express
# or  implied,  or  assumes  any  liability or  responsibility  for  the
# accuracy, completeness,  or usefulness of  any information, apparatus,
# product, or  process disclosed, or  represents that its use  would not
# infringe privately-owned rights.
# 
# Any reference herein to  any specific commercial products, process, or
# services by trade name,  trademark, manufacturer or otherwise does not
# necessarily  constitute or imply  its endorsement,  recommendation, or
# favoring  by  the  United  States  Government  or  Lawrence  Livermore
# National Security,  LLC. The views  and opinions of  authors expressed
# herein do not necessarily state  or reflect those of the United States
# Government or Lawrence Livermore National Security, LLC, and shall not
# be used for advertising or product endorsement purposes.
#
## Procss this file with automake to create Makefile.in
AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -DBIT_STREAM_WORD_TYPE=uint8 -I$(srcdir)/include

noinst_LTLIBRARIES = libsilo_zfp.la
libsilo_zfp_la_SOURCES = $(FILES)

EXTRA_DIST = \
  LICENSE \
  README.md \
  VERSIONS.md

noinst_HEADERS = \
  src/block1.h \
  src/block2.h \
  src/block3.h \
  src/block4.h \
  src/traitsd.h \
  src/traitsf.h \
  src/traitsi.h \
  src/traitsl.h \
  src/undefall.h \
  include/zfp.h \
  include/bitstream.h \
  include/zfp/macros.h \
  include/zfp/system.h \
  include/zfp/types.h \
  src/inline/inline.h \
  src/inline/bitstream.c \
  src/template/codec.h \
  src/template/template.h \
  src/template/codec1.c \
  src/template/codec2.c \
  src/template/codec3.c \
  src/template/codec4.c \
  src/template/codecf.c \
  src/template/compress.c \
  src/template/cudacompress.c \
  src/template/cudadecompress.c \
  src/template/decode.c \
  src/template/decode1.c \
  src/template/decode2.c \
  src/template/decode3.c \
  src/template/decode4.c \
  src/template/decodef.c \
  src/template/decodei.c \
  src/template/decompress.c \
  src/template/encode.c \
  src/template/encode1.c \
  src/template/encode2.c \
  src/template/encode3.c \
  src/template/encode4.c \
  src/template/encodef.c \
  src/template/encodei.c \
  src/template/ompcompress.c \
  src/template/revcodecf.c \
  src/template/revdecode.c \
  src/template/revdecode1.c \
  src/template/revdecode2.c \
  src/template/revdecode3.c \
  src/template/revdecode4.c \
  src/template/revdecodef.c \
  src/template/revencode.c \
  src/template/revencode1.c \
  src/template/revencode2.c \
  src/template/revencode3.c \
  src/template/revencode4.c \
  src/template/revencodef.c

FILES = \
  src/bitstream.c \
  src/decode1d.c \
  src/decode1f.c \
  src/decode1i.c \
  src/decode1l.c \
  src/decode2d.c \
  src/decode2f.c \
  src/decode2i.c \
  src/decode2l.c \
  src/decode3d.c \
  src/decode3f.c \
  src/decode3i.c \
  src/decode3l.c \
  src/decode4d.c \
  src/decode4f.c \
  src/decode4i.c \
  src/decode4l.c \
  src/encode1d.c \
  src/encode1f.c \
  src/encode1i.c \
  src/encode1l.c \
  src/encode2d.c \
  src/encode2f.c \
  src/encode2i.c \
  src/encode2l.c \
  src/encode3d.c \
  src/encode3f.c \
  src/encode3i.c \
  src/encode3l.c \
  src/encode4d.c \
  src/encode4f.c \
  src/encode4i.c \
  src/encode4l.c \
  src/zfp.c
