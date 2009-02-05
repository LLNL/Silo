#include <cstdio>
#include <cstdlib>
#include "pcencoder.h"
#include "rcqsmodel.h"
#include "front.h"
#include "local.h"
#include "codec.h"
#include "write.h"

#if LOCAL_FP == LOCAL_FP_FAST || LOCAL_FP == LOCAL_FP_SAFE
// compress 3D array at specified precision using floating-point arithmetic
template <typename T, unsigned bits>
static void
compress3d(
  RCencoder* re,   // entropy encoder
  const T*   data, // flattened 3D array to compress
  unsigned   nx,   // number of x samples
  unsigned   ny,   // number of y samples
  unsigned   nz    // number of z samples
)
{
  // initialize compressor
  typedef PCmap<T, bits> MAP;
  RCmodel* rm = new RCqsmodel(true, PCencoder<T, MAP>::symbols);
  PCencoder<T, MAP>* fe = new PCencoder<T, MAP>(re, &rm);
  FRONT<T> f(nx, ny);

  // encode difference between predicted (p) and actual (a) value
  unsigned x, y, z;
  for (z = 0, f.advance(0, 0, 1); z < nz; z++)
    for (y = 0, f.advance(0, 1, 0); y < ny; y++)
      for (x = 0, f.advance(1, 0, 0); x < nx; x++) {
        #if LOCAL_FP == LOCAL_FP_SAFE
        volatile T p = f(1, 1, 1);
        p += f(1, 0, 0);
        p -= f(0, 1, 1);
        p += f(0, 1, 0);
        p -= f(1, 0, 1);
        p += f(0, 0, 1);
        p -= f(1, 1, 0);
        #else
        T p = f(1, 0, 0) - f(0, 1, 1) +
              f(0, 1, 0) - f(1, 0, 1) +
              f(0, 0, 1) - f(1, 1, 0) +
              f(1, 1, 1);
        #endif
        T a = *data++;
        a = fe->encode(a, p);
        f.push(a);
      }

  delete fe;
  delete rm;
}
#elif LOCAL_FP == LOCAL_FP_EMUL
#include "fpe.h"
// compress 3D array at specified precision using floating-point emulation
template <typename T, unsigned bits>
static void
compress3d(
  RCencoder* re,   // entropy encoder
  const T*   data, // flattened 3D array to compress
  unsigned   nx,   // number of x samples
  unsigned   ny,   // number of y samples
  unsigned   nz    // number of z samples
)
{
  // initialize compressor
  typedef PCmap<T, bits> MAP;
  typedef FPE<T> FLOAT;
  RCmodel* rm = new RCqsmodel(true, PCencoder<T, MAP>::symbols);
  PCencoder<T, MAP>* fe = new PCencoder<T, MAP>(re, &rm);
  FRONT<FLOAT> f(nx, ny);

  // encode difference between predicted (p) and actual (a) value
  unsigned x, y, z;
  for (z = 0, f.advance(0, 0, 1); z < nz; z++)
    for (y = 0, f.advance(0, 1, 0); y < ny; y++)
      for (x = 0, f.advance(1, 0, 0); x < nx; x++) {
        FLOAT p = f(1, 0, 0) - f(0, 1, 1) +
                  f(0, 1, 0) - f(1, 0, 1) +
                  f(0, 0, 1) - f(1, 1, 0) +
                  f(1, 1, 1);
        T a = *data++;
        a = fe->encode(a, T(p));
        f.push(a);
      }

  delete fe;
  delete rm;
}
#else // LOCAL_FP_INT
// compress 3D array at specified precision using integer arithmetic
template <typename T, unsigned bits>
static void
compress3d(
  RCencoder* re,   // entropy encoder
  const T*   data, // flattened 3D array to compress
  unsigned   nx,   // number of x samples
  unsigned   ny,   // number of y samples
  unsigned   nz    // number of z samples
)
{
  // initialize compressor
  typedef PCmap<T, bits> TMAP;
  typedef typename TMAP::RANGE U;
  typedef PCmap<U, bits, U> UMAP;
  RCmodel* rm = new RCqsmodel(true, PCencoder<U, UMAP>::symbols);
  PCencoder<U, UMAP>* fe = new PCencoder<U, UMAP>(re, &rm);
  TMAP map;
  FRONT<U> f(nx, ny, map.forward(0));

  // encode difference between predicted (p) and actual (a) value
  unsigned x, y, z;
  for (z = 0, f.advance(0, 0, 1); z < nz; z++)
    for (y = 0, f.advance(0, 1, 0); y < ny; y++)
      for (x = 0, f.advance(1, 0, 0); x < nx; x++) {
        U p = f(1, 0, 0) - f(0, 1, 1) +
              f(0, 1, 0) - f(1, 0, 1) +
              f(0, 0, 1) - f(1, 1, 0) +
              f(1, 1, 1);
        U a = map.forward(*data++);
        a = fe->encode(a, p);
        f.push(a);
      }

  delete fe;
  delete rm;
}
#endif

// compress 4D array
template <typename T>
static void
compress4d(
  RCencoder* re,   // entropy encoder
  const T*   data, // flattened 4D array to compress
  const int* prec, // per field desired precision
  unsigned   nx,   // number of x samples
  unsigned   ny,   // number of y samples
  unsigned   nz,   // number of z samples
  unsigned   nf    // number of fields
)
{
  // compress one field at a time
  for (unsigned i = 0; i < nf; i++) {
    int bits = prec ? prec[i] : CHAR_BIT * (int)sizeof(T);
    re->encode(bits, 32);
    switch (bits) {
      case subsize(T, 1): //  8-bit float, 16-bit double
        compress3d<T, subsize(T, 1)>(re, data, nx, ny, nz);
        break;
      case subsize(T, 2): // 16-bit float, 32-bit double
        compress3d<T, subsize(T, 2)>(re, data, nx, ny, nz);
        break;
      case subsize(T, 3): // 24-bit float, 48-bit double
        compress3d<T, subsize(T, 3)>(re, data, nx, ny, nz);
        break;
      case subsize(T, 4): // 32-bit float, 64-bit double
        compress3d<T, subsize(T, 4)>(re, data, nx, ny, nz);
        break;
      default:
        fprintf(stderr, "LOCAL: precision %d not supported\n", bits);
        abort();
        break;
    }
    data += nx * ny * nz;
  }
}

static void
write_header(
  RCencoder* re,
  unsigned   nx,
  unsigned   ny,
  unsigned   nz,
  unsigned   nf,
  int        dp
)
{
  // magic
  re->encode('s', 8);
  re->encode('f', 8);
  re->encode('c', 8);
  re->encode('\0', 8);

  // format version
  re->encode(SFC_MAJ_VERSION, 16);
  re->encode(SFC_MIN_VERSION, 16);

  // array dimensions
  re->encode(nf, 32);
  re->encode(nz, 32);
  re->encode(ny, 32);
  re->encode(nx, 32);

  // single or double precision
  re->encode(!!dp);
}

static void
local_stream_write(
  RCencoder*  re,   // entropy encoder
  const void* data, // array to write
  const int*  prec, // per field bits of precision
  int         dp,   // double precision array if nonzero
  unsigned    nx,   // number of x samples
  unsigned    ny,   // number of y samples
  unsigned    nz,   // number of z samples
  unsigned    nf    // number of fields
)
{
  #if LOCAL_VERBOSE
  printf(" Writing (%u,%u,%u,%u)\n", nx, ny, nz, nf);
  #endif
  write_header(re, nx, ny, nz, nf, dp);
  if (dp)
    compress4d(re, (const double*)data, prec, nx, ny, nz, nf);
  else
    compress4d(re, (const float*)data, prec, nx, ny, nz, nf);
  re->finish();
}

// compress and write a single or double precision 4D array to file
unsigned
local_file_write(
  FILE*       file, // binary output stream
  const void* data, // array to write
  const int*  prec, // per field bits of precision
  int         dp,   // double precision array if nonzero
  unsigned    nx,   // number of x samples
  unsigned    ny,   // number of y samples
  unsigned    nz,   // number of z samples
  unsigned    nf    // number of fields
)
{
  RCfileencoder* re = new RCfileencoder(file);
  local_stream_write(re, data, prec, dp, nx, ny, nz, nf);
  re->flush();
  unsigned bytes = re->error ? 0 : re->bytes();
  delete re;
  return bytes;
}

// compress and write a single or double precision 4D array to file
unsigned
local_memory_write(
  void*       buffer, // pointer to compressed data
  unsigned    size,   // size of allocated storage
  const void* data,   // array to write
  const int*  prec,   // per field bits of precision
  int         dp,     // double precision array if nonzero
  unsigned    nx,     // number of x samples
  unsigned    ny,     // number of y samples
  unsigned    nz,     // number of z samples
  unsigned    nf      // number of fields
)
{
  RCmemencoder* re = new RCmemencoder(buffer, size);
  local_stream_write(re, data, prec, dp, nx, ny, nz, nf);
  unsigned bytes = re->error ? 0 : re->bytes();
  delete re;
  return bytes;
}

// wrappers for fortran calls
void
local_file_write_f(
  const char* path, // path to output file
  const void* data, // array to write
  const int*  prec, // per field bits of precision
  const int*  dp,   // double precision array if nonzero
  const int*  nx,   // number of x samples
  const int*  ny,   // number of y samples
  const int*  nz,   // number of z samples
  const int*  nf    // number of fields
)
{
  FILE* file = fopen(path, "wb");
  if (!file) {
    fprintf(stderr, "LOCAL: cannot create file %s\n", path);
    abort();
  }
  if (!local_file_write(file, data, prec, *dp, *nx, *ny, *nz, *nf)) {
    fprintf(stderr, "LOCAL: cannot write file %s\n", path);
    abort();
  }
  fclose(file);
}

void
local_file_write_f_(
  const char* path, // path to output file
  const void* data, // array to write
  const int*  prec, // per field bits of precision
  const int*  dp,   // double precision array if nonzero
  const int*  nx,   // number of x samples
  const int*  ny,   // number of y samples
  const int*  nz,   // number of z samples
  const int*  nf    // number of fields
)
{
  local_file_write_f(path, data, dp, prec, nx, ny, nz, nf);
}
