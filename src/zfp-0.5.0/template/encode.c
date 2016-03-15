/* encode block of integers */
static uint
_T2(encode_block, Int, DIMS)(bitstream* stream, int minbits, int maxbits, int maxprec, Int* iblock)
{
  int bits;
  _cache_align(UInt ublock[BLOCK_SIZE]);
  /* perform decorrelating transform */
  _T2(fwd_xform, Int, DIMS)(iblock);
  /* reorder signed coefficients and convert to unsigned integer */
  _T1(fwd_order, Int)(ublock, iblock, PERM, BLOCK_SIZE);
  /* encode integer coefficients */
  bits = _T1(encode_ints, UInt)(stream, maxbits, maxprec, ublock, BLOCK_SIZE);
  /* write at least minbits bits by padding with zeros */
  if (bits < minbits) {
    stream_pad(stream, minbits - bits);
    bits = minbits;
  }
  return bits;
}

/* public functions -------------------------------------------------------- */

/* encode contiguous integer block */
C_STRUCTSPACE_STATIC uint
_T2(zfp_encode_block, Int, DIMS)(zfp_stream* zfp, const Int* iblock)
{
  _cache_align(Int block[BLOCK_SIZE]);
  uint i;
  /* copy block */
  for (i = 0; i < BLOCK_SIZE; i++)
    block[i] = iblock[i];
  return _T2(encode_block, Int, DIMS)(zfp->stream, zfp->minbits, zfp->maxbits, zfp->maxprec, block);
}

/* encode contiguous floating-point block */
C_STRUCTSPACE_STATIC uint
_T2(zfp_encode_block, Scalar, DIMS)(zfp_stream* zfp, const Scalar* fblock)
{
  /* compute maximum exponent */
  int emax = _T1(exponent_block, Scalar)(fblock, BLOCK_SIZE);
  int maxprec = _T2(precision, Scalar, DIMS)(emax, zfp->maxprec, zfp->minexp);
  uint e = maxprec ? emax + EBIAS : 0;
  /* encode block only if biased exponent is nonzero */
  if (e) {
    _cache_align(Int iblock[BLOCK_SIZE]);
    /* encode common exponent; LSB indicates that exponent is nonzero */
    int ebits = EBITS + 1;
    stream_write_bits(zfp->stream, 2 * e + 1, ebits);
    /* perform forward block-floating-point transform */
    _T1(fwd_cast, Scalar)(iblock, fblock, BLOCK_SIZE, emax);
    /* encode integer block */
    return ebits + _T2(encode_block, Int, DIMS)(zfp->stream, zfp->minbits - ebits, zfp->maxbits - ebits, maxprec, iblock);
  }
  else {
    /* write single zero-bit to indicate that all values are zero */
    stream_write_bit(zfp->stream, 0);
    if (zfp->minbits > 1) {
      stream_pad(zfp->stream, zfp->minbits - 1);
      return zfp->minbits;
    }
    else
      return 1;
  }
}
