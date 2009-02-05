#ifndef HZNPREDICTOR_H
#define HZNPREDICTOR_H

// spectral predictor for node-centered data
class HZNpredictor {
public:
  HZNpredictor(unsigned dims) : cellsize(1 << dims) {}

  // predict node with index cell[i] from known in-cell neighbors given by mask
  template <typename T>
  inline T predict(const T* node, const unsigned* cell, unsigned i, unsigned mask);

private:
  static const int weight[][8];
  const unsigned cellsize;
};

#include "hznpredictor.inl"

#endif
