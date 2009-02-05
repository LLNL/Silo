#include "hzmencoder.h"
#include "hzresidual.h"

void
HZMencoder::put()
{
  // write compressed data for last cell(s)
  stream->put(mask);
  for (unsigned i = 0; i < CHAR_BIT; i++)
    if (mask & (1 << i))
      diff[i].put(stream);
  mask = count = 0;
}

unsigned
HZMencoder::encode(const int* v)
{
  // encode cell
  for (unsigned i = 0; i < cellsize; i++, count++) {
    node[i] = v[perm[i]];
    unsigned p = pred[i]->predict(node, i);
    if (p != node[i]) {
      unsigned q = pred[i]->repredict(node, i);
      diff[count] = HZresidual<unsigned>(q, node[i]);
      mask |= 1 << count;
    }
  }
  if (count == CHAR_BIT)
    put();
  advance();
  return cellsize;
}

void
HZMencoder::end()
{
  if (count)
    put();
}
