#ifndef HC_ENCODER_H
#define HC_ENCODER_H

#include "hccodec.h"

class HCencoder : public HCcodec {
public:
  HCencoder(unsigned perm = 0) : HCcodec(perm) { mask = 0; }
  virtual ~HCencoder() {}

  // finish encoding
  void finish();

  // encode eight vertex indices of a hexahedron
  void encode(const unsigned* v);

  // virtual function for writing byte stream
  virtual void putbyte(unsigned char byte) = 0;

protected:
  void put();
  void putresidual(unsigned r);
  unsigned residual(unsigned a, unsigned p) const;
};

#endif
