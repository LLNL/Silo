#ifndef HZM_ENCODER_H
#define HZM_ENCODER_H

#include "hzmcodec.h"
#include "obstream.h"

// mesh encoder
class HZMencoder : public HZMcodec {
public:
  HZMencoder(OBSTREAM* stream, unsigned dims = 3, unsigned perm = 0, unsigned bits = HZM_HASH_BITS) : HZMcodec(dims, perm, bits), stream(stream) {}

  // encode node indices of a cell
  unsigned encode(const int* v);

  // end encoding
  void end();

private:
  void put();
  OBSTREAM* stream;
};

#endif
