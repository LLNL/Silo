#include "hzmdecoder.h"

void
HZMdecoder::get()
{
  // read compressed data for next cell
  mask = stream->get();
  for (unsigned i = 0; i < CHAR_BIT; i++)
    if (mask & (1 << i))
      diff[i] = HZresidual<unsigned>(stream);
  count = 0;
}

unsigned
HZMdecoder::decode(int* v)
{
  // decode cell
  if (count == CHAR_BIT)
    get();
  for (unsigned i = 0; i < cellsize; i++, count++) {
    bool correct = !(mask & (1 << count));
    if (!correct) {
      unsigned q = pred[i]->repredict(node, i);
      node[i] = q + diff[count];
    }
    unsigned p = pred[i]->retrodict(node, i, correct);
    if (correct)
      node[i] = p;
    v[perm[i]] = (int)node[i];
  }
  advance();
  return cellsize;
}
