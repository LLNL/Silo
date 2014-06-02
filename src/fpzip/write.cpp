#include <cstdio>
#include <cstdlib>
#include "pcencoder.h"
#include "rcqsmodel.h"
#include "front.h"
#include "fpzip.h"
#include "codec.h"
#include "write.h"

#if FPZIP_FP == FPZIP_FP_FAST || FPZIP_FP == FPZIP_FP_SAFE
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
        #if FPZIP_FP == FPZIP_FP_SAFE
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
#elif FPZIP_FP == FPZIP_FP_EMUL
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
#else // FPZIP_FP_INT
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

// compress p-bit float, 2p-bit double
#define compress_case(p)\
  case subsize(T, p):\
    compress3d<T, subsize(T, p)>(re, data, nx, ny, nz);\
    break

// compress 4D array
template <typename T>
static bool
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
      compress_case( 2);
      compress_case( 3);
      compress_case( 4);
      compress_case( 5);
      compress_case( 6);
      compress_case( 7);
      compress_case( 8);
      compress_case( 9);
      compress_case(10);
      compress_case(11);
      compress_case(12);
      compress_case(13);
      compress_case(14);
      compress_case(15);
      compress_case(16);
      compress_case(17);
      compress_case(18);
      compress_case(19);
      compress_case(20);
      compress_case(21);
      compress_case(22);
      compress_case(23);
      compress_case(24);
      compress_case(25);
      compress_case(26);
      compress_case(27);
      compress_case(28);
      compress_case(29);
      compress_case(30);
      compress_case(31);
      compress_case(32);
      default:
        fpzip_errno = fpzipErrorBadPrecision;
        return false;
    }
    data += nx * ny * nz;
  }
  return true;
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
  re->encode((unsigned)'f', 8);
  re->encode((unsigned)'p', 8);
  re->encode((unsigned)'z', 8);
  re->encode((unsigned)'\0', 8);

  // format version
  re->encode(FPZ_MAJ_VERSION, 16);
  re->encode(FPZ_MIN_VERSION, 16);

  // array dimensions
  re->encode(nf, 32);
  re->encode(nz, 32);
  re->encode(ny, 32);
  re->encode(nx, 32);

  // single or double precision
  re->encode(!!dp);
}

static bool
fpzip_stream_write(
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
  write_header(re, nx, ny, nz, nf, dp);
  bool status;
  if (dp)
    status = compress4d(re, (const double*)data, prec, nx, ny, nz, nf);
  else
    status = compress4d(re, (const float*)data, prec, nx, ny, nz, nf);
  re->finish();
  return status;
}

// compress and write a single or double precision 4D array to file
size_t
fpzip_file_write(
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
  fpzip_errno = fpzipSuccess;
  size_t bytes = 0;
  RCfileencoder* re = new RCfileencoder(file);
  if (fpzip_stream_write(re, data, prec, dp, nx, ny, nz, nf)) {
    re->flush();
    if (re->error)
      fpzip_errno = fpzipErrorWriteStream;
    else
      bytes = re->bytes();
  }
  delete re;
  return bytes;
}

// compress and write a single or double precision 4D array to file
size_t
fpzip_memory_write(
  void*       buffer, // pointer to compressed data
  size_t      size,   // size of allocated storage
  const void* data,   // array to write
  const int*  prec,   // per field bits of precision
  int         dp,     // double precision array if nonzero
  unsigned    nx,     // number of x samples
  unsigned    ny,     // number of y samples
  unsigned    nz,     // number of z samples
  unsigned    nf      // number of fields
)
{
  fpzip_errno = fpzipSuccess;
  size_t bytes = 0;
  RCmemencoder* re = new RCmemencoder(buffer, size);
  if (fpzip_stream_write(re, data, prec, dp, nx, ny, nz, nf)) {
    if (re->error) {
      if (!fpzip_errno)
        fpzip_errno = fpzipErrorWriteStream;
    }
    else
      bytes = re->bytes();
  }
  delete re;
  return bytes;
}

// wrappers for fortran calls
void
fpzip_file_write_f(
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
    fpzip_errno = fpzipErrorCreateFile;
    return;
  }
  fpzip_file_write(file, data, prec, *dp, *nx, *ny, *nz, *nf);
  fclose(file);
}

void
fpzip_file_write_f_(
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
  fpzip_file_write_f(path, data, dp, prec, nx, ny, nz, nf);
}
