/*
** fpzip version 1.0.1, August 7, 2008
** Part of the LOCAL Toolkit, UCRL-CODE-232243
** Written by Peter Lindstrom, Lawrence Livermore National Laboratory
**
**
**                               SUMMARY
**
** fpzip is a library for lossless or lossy compression of 2D and 3D
** arrays of single- or double-precision floating-point scalars.
** Although linear (1D) streams of scalars may be compressed as well,
** the algorithm has primarily be designed to exploit higher-dimensional
** structure in the data, and may not perform well on 1D data.  The
** library supports compression to either a file or to main memory, and
** allows specifying how many bits of precision to retain.  (Currently
** the precision is limited to 8, 16, 24, or 32 bits for floats and
** 16, 32, 48, or 64 bits for doubles.)  The decompressed data is
** returned in full precision, possibly with the least significant bits
** zeroed.  Because floating-point arithmetic may be affected by factors
** such as register precision, rounding mode, and compiler optimizations,
** precautions have been taken to ensure correctness and portability via
** a set of compile-time switches.  For example, it is possible to specify
** that floating-point operations be emulated via integer arithmetic, or to
** treat the binary representation of floating-point numbers as integers.
** Please consult the Makefile for choosing among these settings.  The
** compressor works correctly on the IEEE 754 floating-point format,
** though no particular assumption is made on the floating-point
** representation other than the most significant bit being the sign bit.
** Special values such as infinities, NaNs, and denormalized numbers
** should be handled correctly by the compressor.
**
** fpzip was developed as part of the LOCAL LDRD project at LLNL, and may
** be freely used and distributed for noncommercial purposes.  The core
** library is written in C++ and applications need to be linked with a
** C++ linker.  The library can, however, be called from C and FORTRAN.
** For further information and bug reports, please e-mail pl@llnl.gov.
**
**
**                                NOTICE
**
** This work was produced at the Lawrence Livermore National Laboratory
** (LLNL) under contract no. DE-AC-52-07NA27344 (Contract 44) between
** the U.S. Department of Energy (DOE) and Lawrence Livermore National
** Security, LLC (LLNS) for the operation of LLNL.  The rights of the
** Federal Government are reserved under Contract 44.
**
**
**                              DISCLAIMER
**
** This work was prepared as an account of work sponsored by an agency of
** the United States government.  Neither the United States government nor
** Lawrence Livermore National Security, LLC, nor any of their employees
** makes any warranty, expressed or implied, or assumes any legal liability
** or responsibility for the accuracy, completeness, or usefulness of any
** information, apparatus, product, or process disclosed, or represents
** that its use would not infringe privately owned rights.  Reference
** herein to any specific commercial product, process, or service by trade
** name, trademark, manufacturer, or otherwise does not necessarily
** constitute or imply its endorsement, recommendation, or favoring by the
** United States government or Lawrence Livermore National Security, LLC.
** The views and opinions of authors expressed herein do not necessarily
** state or reflect those of the United States government or Lawrence
** Livermore National Security, LLC, and shall not be used for advertising
** or product endorsement purposes.
**
**
**                            COMMERCIAL USE
**
** Commercialization of this product is prohibited without notifying the
** Department of Energy (DOE) or Lawrence Livermore National Security.
*/

#ifndef FPZIP_H
#define FPZIP_H

#ifdef __cplusplus
extern "C" {
#endif

/* for error codes */
#include "fpzip_error.h"

unsigned
fpzip_file_read(
  FILE*       file,   /* binary input stream */
  void*       data,   /* array to read */
  int*        prec,   /* returned per field bits of precision (may be null) */
  int         *dp,    /* returned double precision array if nonzero */
  unsigned    *nx,    /* returned number of x samples */
  unsigned    *ny,    /* returned number of y samples */
  unsigned    *nz,    /* returned number of z samples */
  unsigned    *nf     /* returned number of fields */
);

unsigned
fpzip_memory_read(
  const void* buffer, /* pointer to compressed data */
  void*       data,   /* array to read */
  int*        prec,   /* returned per field bits of precision (may be null) */
  int         *dp,    /* returned double precision array if nonzero */
  unsigned    *nx,    /* returned number of x samples */
  unsigned    *ny,    /* returned number of y samples */
  unsigned    *nz,    /* returned number of z samples */
  unsigned    *nf     /* returned number of fields */
);

unsigned
fpzip_file_write(
  FILE*       file,   /* binary output stream */
  const void* data,   /* array to write */
  const int*  prec,   /* per field bits of precision (null = full precision) */
  int         dp,     /* double precision array if nonzero */
  unsigned    nx,     /* number of x samples */
  unsigned    ny,     /* number of y samples */
  unsigned    nz,     /* number of z samples */
  unsigned    nf      /* number of fields */
);

unsigned
fpzip_memory_write(
  void*       buffer, /* pointer to compressed data */
  unsigned    size,   /* size of allocated storage */
  const void* data,   /* array to write */
  const int*  prec,   /* per field bits of precision (null = full precision) */
  int         dp,     /* double precision array if nonzero */
  unsigned    nx,     /* number of x samples */
  unsigned    ny,     /* number of y samples */
  unsigned    nz,     /* number of z samples */
  unsigned    nf      /* number of fields */
);

/*
** Fortran bindings.
*/

void
fpzip_file_read_f(
  const char* path, /* path to input file */
  void*       data, /* array to read */
  int*        prec, /* per field bits of precision */
  int*        dp,   /* double precision array if nonzero */
  int*        nx,   /* number of x samples */
  int*        ny,   /* number of y samples */
  int*        nz,   /* number of z samples */
  int*        nf    /* number of fields */
);

void
fpzip_file_read_f_(
  const char* path, /* path to input file */
  void*       data, /* array to read */
  int*        prec, /* per field bits of precision */
  int*        dp,   /* double precision array if nonzero */
  int*        nx,   /* number of x samples */
  int*        ny,   /* number of y samples */
  int*        nz,   /* number of z samples */
  int*        nf    /* number of fields */
);

void
fpzip_file_write_f(
  const char* path, /* path to output file */
  const void* data, /* array to write */
  const int*  prec, /* per field bits of precision */
  const int*  dp,   /* double precision array if nonzero */
  const int*  nx,   /* number of x samples */
  const int*  ny,   /* number of y samples */
  const int*  nz,   /* number of z samples */
  const int*  nf    /* number of fields */
);

void
fpzip_file_write_f_(
  const char* path, /* path to output file */
  const void* data, /* array to write */
  const int*  prec, /* per field bits of precision */
  const int*  dp,   /* double precision array if nonzero */
  const int*  nx,   /* number of x samples */
  const int*  ny,   /* number of y samples */
  const int*  nz,   /* number of z samples */
  const int*  nf    /* number of fields */
);

#ifdef __cplusplus
}
#endif

#endif
