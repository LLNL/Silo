#ifndef PREDICTOR_H
#define PREDICTOR_H

#define HASH_BITS 12

class VTXpredictor {
public:
  VTXpredictor(unsigned bits = HASH_BITS);
  ~VTXpredictor();

  // primary index prediction for vertex v[i]
  unsigned predict(const unsigned* v, unsigned i, bool retrodict);

  // secondary index prediction for vertex v[i]
  unsigned repredict(const unsigned* v, unsigned i) const;

private:
  unsigned index(unsigned x) const { return x & (size - 1); }
  unsigned length(unsigned u, unsigned v) const { return u > v ? u - v : v - u; }

  const unsigned size;  // hash table size
  const unsigned shift; // hash key shift
  unsigned*      hash;  // hash table
  bool*          conf;  // confidence flags for hash entries
};

#endif
