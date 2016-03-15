#include <limits.h>
#include <math.h>
#include <stdio.h>

/* private functions ------------------------------------------------------- */

/* return normalized floating-point exponent for x >= 0 */
static int
_T1(exponent, Scalar)(Scalar x)
{
  if (x > 0) {
    int e;
    FREXP(x, &e);
    /* clamp exponent in case x is denormal */
    return MAX(e, 1 - EBIAS);
  }
  return -EBIAS;
}

/* compute maximum exponent in block of n values */
static int
_T1(exponent_block, Scalar)(const Scalar* p, uint n)
{
  Scalar max = 0;
  do {
    Scalar f = FABS(*p++);
    if (max < f)
      max = f;
  } while (--n);
  return _T1(exponent, Scalar)(max);
}

/* map floating-point number x to integer relative to exponent e */
static Scalar
_T1(quantize, Scalar)(Scalar x, int e)
{
  return LDEXP(x, (CHAR_BIT * (int)sizeof(Scalar) - 2) - e);
}

/* forward block-floating-point transform to signed integers */
static void
_T1(fwd_cast, Scalar)(Int* iblock, const Scalar* fblock, uint n, int emax)
{
  /* compute power-of-two scale factor s */
  Scalar s = _T1(quantize, Scalar)(1, emax);
  /* compute p-bit int y = s*x where x is floating and |y| <= 2^(p-2) - 1 */
  do
    *iblock++ = (Int)(s * *fblock++);
  while (--n);
}

/* pad partial block of width n <= 4 and stride s */
static void
_T1(pad_block, Scalar)(Scalar* p, uint n, uint s)
{
  switch (n) {
    case 0:
      p[0 * s] = 0;
      /* FALLTHROUGH */
    case 1:
      p[1 * s] = p[0 * s];
      /* FALLTHROUGH */
    case 2:
      p[2 * s] = p[1 * s];
      /* FALLTHROUGH */
    case 3:
      p[3 * s] = p[0 * s];
      /* FALLTHROUGH */
    default:
      break;
  }
}

/* forward lifting transform of 4-vector */
static void
_T1(fwd_lift, Int)(Int* p, uint s)
{
  Int x, y, z, w;
  x = *p; p += s;
  y = *p; p += s;
  z = *p; p += s;
  w = *p; p += s;

  /*
  ** non-orthogonal transform
  **        ( 4  4  4  4) (x)
  ** 1/16 * ( 5  1 -1 -5) (y)
  **        (-4  4  4 -4) (z)
  **        (-2  6 -6  2) (w)
  */
  x += w; x >>= 1; w -= x;
  z += y; z >>= 1; y -= z;
  x += z; x >>= 1; z -= x;
  w += y; w >>= 1; y -= w;
  w += y >> 1; y -= w >> 1;

  p -= s; *p = w;
  p -= s; *p = z;
  p -= s; *p = y;
  p -= s; *p = x;
}

/* map two's complement signed integer to negabinary unsigned integer */
static UInt
_T1(int2uint, Int)(Int x)
{
  return ((UInt)x + NBMASK) ^ NBMASK;
}

/* reorder signed coefficients and convert to unsigned integer */
static void
_T1(fwd_order, Int)(UInt* ublock, const Int* iblock, const uchar* perm, uint n)
{
  do
    *ublock++ = _T1(int2uint, Int)(iblock[*perm++]);
  while (--n);
}

/* compress sequence of size unsigned integers */
static uint
_T1(encode_ints, UInt)(bitstream* _restrict stream, uint maxbits, uint maxprec, const UInt* _restrict data, uint size)
{
  /* make a copy of bit stream to avoid aliasing */
  bitstream s = *stream;
  uint intprec = CHAR_BIT * (uint)sizeof(UInt);
  uint kmin = intprec > maxprec ? intprec - maxprec : 0;
  uint bits = maxbits;
  uint i, k, m, n;
  uint64 x;

  /* encode one bit plane at a time from MSB to LSB */
  for (k = intprec, n = 0; bits && k-- > kmin;) {
    /* step 1: extract bit plane #k to x */
    x = 0;
    for (i = 0; i < size; i++)
      x += (uint64)((data[i] >> k) & 1u) << i;
    /* step 2: encode first n bits of bit plane */
    m = MIN(n, bits);
    bits -= m;
    x = stream_write_bits(&s, x, m);
    /* step 3: unary run-length encode remainder of bit plane */
    for (; n < size && bits && (bits--, stream_write_bit(&s, !!x)); x >>= 1, n++)
      for (; n < size - 1 && bits && (bits--, !stream_write_bit(&s, x & 1u)); x >>= 1, n++)
        ;
  }

  *stream = s;
  return maxbits - bits;
}

