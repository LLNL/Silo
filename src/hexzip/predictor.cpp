#include "predictor.h"

#define ORDER 2

VTXpredictor::VTXpredictor(unsigned bits) : size(1 << bits), shift(bits / ORDER)
{
  hash = new unsigned[size];
  for (unsigned i = 0; i < size; i++)
    hash[i] = 0;
  conf = new bool[size];
  for (unsigned i = 0; i < size; i++)
    conf[i] = false;
}

VTXpredictor::~VTXpredictor()
{
  delete[] hash;
  delete[] conf;
}

unsigned
VTXpredictor::predict(const unsigned* v, unsigned i, bool retrodict)
{
  unsigned a = v[i - 010] - v[i - 020];
  unsigned b = v[i - 020] - v[i - 030];
  unsigned c = v[i - 001] - v[i - 011];
  unsigned h = index((c << shift) + (b << (shift / 2)) + a);
  unsigned p = v[i - 010] + a + hash[h];
  if (retrodict || p == v[i])
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
// four axes (up to three within this hexahedron plus the corresponding
// vertex in the previous hexahedron).  The chosen axis is the one that
// minimizes the prediction error (difference) among the past eight
// vertices (some of which may belong to the previous hexahedron).
unsigned
VTXpredictor::repredict(const unsigned* v, unsigned i) const
{
  unsigned l = 3;
  if (i) {
    v -= 8;
    i += 8;
    unsigned d[4] = { 0, 0, 0, 0 };
    unsigned n[4] = { 0, 0, 0, 0 };
    for (unsigned j = i - 8; j < i; j++)
      for (unsigned k = 0, m = 1; k < 4; k++, m <<= 1)
        if (i & j & m) {
          d[k] += length(v[j], v[j - m]);
          n[k]++;
        }
    for (unsigned k = 0; k < 3; k++)
      if (n[l] * d[k] < n[k] * d[l])
        l = k;
  }
  return v[i - (1 << l)];
}
