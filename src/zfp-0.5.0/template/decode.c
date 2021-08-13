/* decode block of integers */
static uint
_T2(decode_block, Int, DIMS)(bitstream* stream, int minbits, int maxbits, int maxprec, Int* iblock)
{
  int bits;
  _cache_align(UInt ublock[BLOCK_SIZE]);
  /* decode integer coefficients */
  bits = _T1(decode_ints, UInt)(stream, maxbits, maxprec, ublock, BLOCK_SIZE);
  /* read at least minbits bits */
  if (bits < minbits) {
    stream_skip(stream, minbits - bits);
    bits = minbits;
  }
  /* reorder unsigned coefficients and convert to signed integer */
  _T1(inv_order, Int)(ublock, iblock, PERM, BLOCK_SIZE);
  /* perform decorrelating transform */
  _T2(inv_xform, Int, DIMS)(iblock);
  return bits;
}

/* decode contiguous floating-point block */
C_STRUCTSPACE_STATIC uint
_T2(zfp_decode_block, Int, DIMS)(zfp_stream* zfp, Int* iblock)
{
  return _T2(decode_block, Int, DIMS)(zfp->stream, zfp->minbits, zfp->maxbits, zfp->maxprec, iblock);
}

/* decode contiguous floating-point block */
C_STRUCTSPACE_STATIC uint
_T2(zfp_decode_block, Scalar, DIMS)(zfp_stream* zfp, Scalar* fblock)
{
  /* test if block has nonzero values */
  if (stream_read_bit(zfp->stream)) {
    _cache_align(Int iblock[BLOCK_SIZE]);
    /* decode common exponent */
    uint ebits = EBITS + 1;
    int emax = stream_read_bits(zfp->stream, ebits - 1) - EBIAS;
    int maxprec = _T2(precision, Scalar, DIMS)(emax, zfp->maxprec, zfp->minexp);
    /* decode integer block */
    uint bits = _T2(decode_block, Int, DIMS)(zfp->stream, zfp->minbits - ebits, zfp->maxbits - ebits, maxprec, iblock);
    /* perform inverse block-floating-point transform */
    _T1(inv_cast, Scalar)(iblock, fblock, BLOCK_SIZE, emax);
    return ebits + bits;
  }
  else {
    /* set all values to zero */
    uint i;
    for (i = 0; i < BLOCK_SIZE; i++)
      *fblock++ = 0;
    if (zfp->minbits > 1) {
      stream_skip(zfp->stream, zfp->minbits - 1);
      return zfp->minbits;
    }
    else
      return 1;
  }
}
