#include "hcencoder.h"

void
HCencoder::finish()
{
  // flush buffered data if any
  if (hptr)
    put();
}

void
HCencoder::put()
{
  // write compressed data for last eight hexahedra
  putbyte(mask);
  for (unsigned i = 0; i < 8; i++)
    if (mask & (1 << i)) {
      putbyte(hex[i].mask);
      for (unsigned j = 0; j < 8; j++)
        if (hex[i].mask & (1 << j))
          putresidual(hex[i].residual[j]);
    }
  mask = 0;
}

void
HCencoder::putresidual(unsigned r)
{
  // use stratified adaptive prefix code for small differences
  if (!(r >> 14))
    r = sap.encode((unsigned short)r);

  // output variable-length residual interleaved with termination bits
  while (r > 0x7f) {
    putbyte((unsigned char)((r & 0x7f) + 0x80));
    r >>= 7;
  }
  putbyte((unsigned char)r);
}

// Compute (unsigned) residual difference between actual and predicted values.
// Note that residuals can be zero, e.g. the ith vertices in two consecutive
// hexes could be the same, or a hex may be degenerate.
unsigned
HCencoder::residual(unsigned a, unsigned p) const
{
  unsigned r = 2 * (a - p);
  if (a < p)
    r = ~r;
  return r;
}

void
HCencoder::encode(const unsigned* v)
{
  hex[hptr].mask = 0;
  for (unsigned i = 0; i < 8; i++) {
    vtx[vptr + i] = v[i];
    unsigned p = pred[i]->predict(vtx + vptr, i, false);
    if (p != v[i]) {
      hex[hptr].mask |= 1 << i;
      hex[hptr].residual[i] = residual(v[i], pred[i]->repredict(vtx + vptr, i));
    }
  }
  if (hex[hptr].mask)
    mask |= 1 << hptr;
  vptr += 8;
  if (++hptr == 8) {
    hptr = 0;
    vptr = 030;
    for (unsigned i = 0; i < 030; i++)
      vtx[i] = vtx[i + 0x40];
    put();
  }
}
