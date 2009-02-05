#ifndef HZM_PREDICTOR_H
#define HZM_PREDICTOR_H

#include "bitvector.h"

#define HZM_HASH_BITS 12

// mesh predictor of node indices
class HZMpredictor {
public:
  HZMpredictor(unsigned dims = 3, unsigned bits = HZM_HASH_BITS);
  ~HZMpredictor();

  // primary index prediction for node v[i]
  unsigned predict(const unsigned* v, unsigned i);

  // primary index prediction for node v[i]
  unsigned retrodict(const unsigned* v, unsigned i, bool correct);

  // secondary index prediction for node v[i]
  unsigned repredict(const unsigned* v, unsigned i) const;

private:
  unsigned index(unsigned x) const { return x & (size - 1); }
  unsigned length(unsigned u, unsigned v) const { return u > v ? u - v : v - u; }
  unsigned node(const unsigned* v, int i, int j) const { return v[(i << dims) + j]; }

  const unsigned dims;  // number of dimensions
  const unsigned size;  // hash table size
  const unsigned shift; // hash key shift
  unsigned*      hash;  // hash table
  BITVECTOR      conf;  // confidence flags for hash entries
};

#endif
