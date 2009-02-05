#ifndef LOCAL_H
#define LOCAL_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned
local_file_read(
  FILE*       file,   /* binary input stream */
  void*       data,   /* array to read */
  int*        prec,   /* per field bits of precision (may be null) */
  int         dp,     /* double precision array if nonzero */
  unsigned    nx,     /* number of x samples */
  unsigned    ny,     /* number of y samples */
  unsigned    nz,     /* number of z samples */
  unsigned    nf      /* number of fields */
);

unsigned
local_memory_read(
  const void* buffer, /* pointer to compressed data */
  void*       data,   /* array to read */
  int*        prec,   /* per field bits of precision (may be null) */
  int         dp,     /* double precision array if nonzero */
  unsigned    nx,     /* number of x samples */
  unsigned    ny,     /* number of y samples */
  unsigned    nz,     /* number of z samples */
  unsigned    nf      /* number of fields */
);

unsigned
local_file_write(
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
local_memory_write(
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
local_file_read_f(
  const char* path, /* path to input file */
  void*       data, /* array to read */
  int*        prec, /* per field bits of precision */
  const int*  dp,   /* double precision array if nonzero */
  const int*  nx,   /* number of x samples */
  const int*  ny,   /* number of y samples */
  const int*  nz,   /* number of z samples */
  const int*  nf    /* number of fields */
);

void
local_file_read_f_(
  const char* path, /* path to input file */
  void*       data, /* array to read */
  int*        prec, /* per field bits of precision */
  const int*  dp,   /* double precision array if nonzero */
  const int*  nx,   /* number of x samples */
  const int*  ny,   /* number of y samples */
  const int*  nz,   /* number of z samples */
  const int*  nf    /* number of fields */
);

void
local_file_write_f(
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
local_file_write_f_(
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
