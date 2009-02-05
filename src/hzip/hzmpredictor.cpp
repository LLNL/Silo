#include "hzmpredictor.h"

#define HZM_ORDER 2

HZMpredictor::HZMpredictor(unsigned dims, unsigned bits) :
  dims(dims), size(1 << bits), shift(bits / HZM_ORDER), conf(size, false)
{
  hash = new unsigned[size];
  for (unsigned i = 0; i < size; i++)
    hash[i] = 0;
}

HZMpredictor::~HZMpredictor()
{
  delete[] hash;
}

unsigned
HZMpredictor::predict(const unsigned* v, unsigned i)
{
  unsigned a = node(v, -1, i - 0) - node(v, -2, i - 0);
  unsigned b = node(v,  0, i - 1) - node(v, -1, i - 1);
  unsigned h = index((b << shift) + a);
  unsigned p = hash[h] + node(v, -1, i) + a;
  if (p == v[i])
    conf[h] = true;
  else {
    if (conf[h])
      conf[h] = false;
    else
      hash[h] += v[i] - p;
  }
  return p;
}

unsigned
HZMpredictor::retrodict(const unsigned* v, unsigned i, bool correct)
{
  unsigned a = node(v, -1, i - 0) - node(v, -2, i - 0);
  unsigned b = node(v,  0, i - 1) - node(v, -1, i - 1);
  unsigned h = index((b << shift) + a);
  unsigned p = hash[h] + node(v, -1, i) + a;
  if (correct)
    conf[h] = true;
  else {
    if (conf[h])
      conf[h] = false;
    else
      hash[h] += v[i] - p;
  }
  return p;
}

// Predict v[i] as one of the already encoded neighbors along the up to
// four axes (up to three within this cell plus the corresponding node
// in the previous cell).  The chosen axis is the one that minimizes the
// prediction error (difference) among the past eight nodes (some of
// which may belong to the previous cell).
unsigned
HZMpredictor::repredict(const unsigned* v, unsigned i) const
{
  unsigned l = dims;
  if (i) {
    unsigned n = 1 << dims;
    v -= n;
    i += n;
    unsigned d[4] = { 0, 0, 0, 0 };
    unsigned c[4] = { 0, 0, 0, 0 };
    for (unsigned j = i - n; j < i; j++)
      for (unsigned k = 0, m = 1; k <= dims; k++, m <<= 1)
        if (i & j & m) {
          d[k] += length(v[j], v[j - m]);
          c[k]++;
        }
    for (unsigned k = 0; k < dims; k++)
      if (c[l] * d[k] < c[k] * d[l])
        l = k;
  }
  return v[(int)i - (1 << l)];
}
