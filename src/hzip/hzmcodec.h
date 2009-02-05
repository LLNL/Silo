#ifndef HZM_CODEC_H
#define HZM_CODEC_H

#include <climits>
#include "hzip.h"
#include "hzresidual.h"
#include "hzmpredictor.h"

#define HZM_VERSION  0x10
#define HZM_BUF_HIST (2 * HZ_CELLSIZE_MAX)
#define HZM_BUF_SIZE (2 * HZM_BUF_HIST)

// base class for mesh encoder/decoder
class HZMcodec {
public:
  inline HZMcodec(unsigned dims = 3, unsigned perm = 0, unsigned bits = HZM_HASH_BITS);
  inline virtual ~HZMcodec();

protected:
  // advance node pointer
  inline void advance();

  // node buffer methods
  unsigned* begin() { return buffer + HZM_BUF_HIST; }
  unsigned* end() { return buffer + HZM_BUF_SIZE; }
  void copy(unsigned* p) { *(p - HZM_BUF_HIST) = *p; }

  const unsigned cellsize;             // number of nodes per cell
  unsigned mask;                       // prediction mask
  unsigned count;                      // number of nodes buffered
  unsigned* node;                      // pointer into buffer
  unsigned buffer[HZM_BUF_SIZE];       // recent node index history
  HZresidual<unsigned> diff[CHAR_BIT]; // residuals for last eight nodes
  unsigned perm[HZ_CELLSIZE_MAX];      // in-cell permutation of nodes
  HZMpredictor* pred[HZ_CELLSIZE_MAX]; // node index predictors
};

#include "hzmcodec.inl"

#endif
