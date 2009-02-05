#ifndef HZNCODEC_H
#define HZNCODEC_H

#include <climits>
#include "hzip.h"
#include "hzresidual.h"
#include "hznpredictor.h"
#include "bitvector.h"

#define HZN_VERSION 0x10

// base class for node encoder/decoder
template <typename T>
class HZNcodec {
public:
  inline HZNcodec(unsigned nodes, unsigned dims = 3, unsigned perm = 0, int bias = 0);
  virtual ~HZNcodec() { delete pred; }

  // number of encoded/decoded nodes
  unsigned nodes() const { return count; }

protected:
  // zero-based index of i'th node in cell
  unsigned index(const int* cell, unsigned i) const { return cell[perm[i]] - bias; }

  // prepare for encoding/decoding nodes in cell
  inline unsigned prepare(unsigned* v, const int* cell) const;

  const unsigned cellsize;        // number of nodes per cell
  const int bias;                 // index offset
  unsigned mask;                  // prediction mask
  unsigned count;                 // number of encoded/decoded nodes
  BITVECTOR coded;                // per-node encoded/decoded flag
  HZNpredictor* pred;             // spectral predictor
  HZresidual<T> diff[CHAR_BIT];   // residuals for last eight nodes
  unsigned perm[HZ_CELLSIZE_MAX]; // in-cell permutation of nodes
};

#include "hzncodec.inl"

#endif
