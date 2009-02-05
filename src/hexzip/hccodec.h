#ifndef HC_CODEC_H
#define HC_CODEC_H

#include "predictor.h"
#include "sap.h"

#define SHC_VERSION 0x10
#define VBUF_SIZE   (030 + 8 * 8)

class HCcodec {
public:
  inline HCcodec(unsigned q);
  virtual ~HCcodec() {}

  // initialize vertex predictors
  inline void init(VTXpredictor* p[8]);

  unsigned perm[8];        // in-hex permutation of vertices

protected:
  struct HCcode {
    unsigned char mask;    // vertex mask flagging which residuals needed
    unsigned residual[8];  // residuals for mispredicted vertex indices
  };
  VTXpredictor* pred[8];   // vertex index predictors
  unsigned vtx[VBUF_SIZE]; // recent vertex index history
  unsigned vptr;           // index into buffer
  unsigned char mask;      // hex mask flagging nonzero vertex masks
  HCcode hex[8];           // compression data for last eight hexahedra
  unsigned hptr;           // index of current hex among eight
  SAPCODE sap;             // stratified adaptive prefix code for residuals
};

void
HCcodec::init(VTXpredictor* p[8])
{
  for (unsigned i = 0; i < 8; i++)
    pred[i] = p[i];
}

HCcodec::HCcodec(unsigned p) : vptr(030), hptr(0)
{
  if (!p)
    p = 001234567;
  for (unsigned i = 0; i < 8; i++, p >>= 3) {
    pred[i] = 0;
    perm[7 - i] = p & 7;
  }
  for (unsigned i = 0; i < VBUF_SIZE; i++)
    vtx[i] = 0;
}

#endif
