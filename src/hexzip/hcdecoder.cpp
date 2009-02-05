#include "hcdecoder.h"

void
HCdecoder::get()
{
  // read compressed data for next eight hexahedra
  mask = getbyte();
  for (unsigned i = 0; i < 8; i++)
    if (mask & (1 << i)) {
      hex[i].mask = getbyte();
      for (unsigned j = 0; j < 8; j++)
        if (hex[i].mask & (1 << j))
          hex[i].residual[j] = getresidual();
    }
}

unsigned
HCdecoder::getresidual()
{
  // fetch variable-length residual
  unsigned r = 0;
  for (unsigned i = 0, c = 0x80; c & 0x80; i += 7) {
    c = getbyte();
    r += (c & 0x7f) << i;
  }

  // use stratified adaptive prefix code for small differences
  if (!(r >> 14))
    r = sap.decode((unsigned short)r);

  return r;
}

unsigned
HCdecoder::actual(unsigned p, unsigned r) const
{
  unsigned d = r >> 1;
  return p + (r & 1 ? ~d : d);
}

void
HCdecoder::decode(unsigned* v)
{
  if (hptr == 8) {
    get();
    hptr = 0;
    vptr = 030;
    for (unsigned i = 0; i < 030; i++)
      vtx[i] = vtx[i + 0x40];
  }
  for (unsigned i = 0; i < 8; i++)
    if ((mask & (1 << hptr)) && (hex[hptr].mask & (1 << i))) {
      unsigned p = pred[i]->repredict(vtx + vptr, i);
      vtx[vptr + i] = v[i] = actual(p, hex[hptr].residual[i]);
      pred[i]->predict(vtx + vptr, i, false);
    }
    else
      vtx[vptr + i] = v[i] = pred[i]->predict(vtx + vptr, i, true);
  vptr += 8;
  hptr++;
}
