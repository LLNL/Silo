#ifndef HC_DECODER_H
#define HC_DECODER_H

#include "hccodec.h"

class HCdecoder : public HCcodec {
public:
  HCdecoder(unsigned perm = 0) : HCcodec(perm) { hptr = 8; }
  virtual ~HCdecoder() {}

  // decode eight vertex indices of a hexahedron
  void decode(unsigned* v);

  // virtual function for reading byte stream
  virtual unsigned char getbyte() = 0;

protected:
  void get();
  unsigned getresidual();
  unsigned actual(unsigned p, unsigned r) const;
};

#endif
