#ifndef HZM_DECODER_H
#define HZM_DECODER_H

#include "hzmcodec.h"
#include "ibstream.h"

// mesh decoder
class HZMdecoder : public HZMcodec {
public:
  HZMdecoder(IBSTREAM* stream, unsigned dims = 3, unsigned perm = 0, unsigned bits = HZM_HASH_BITS) : HZMcodec(dims, perm, bits), stream(stream) { count = CHAR_BIT; }

  // decode node indices of a cell
  unsigned decode(int* v);

private:
  void get();
  IBSTREAM* stream;
};

#endif
