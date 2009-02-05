#ifndef HZNDECODER_H
#define HZNDECODER_H

#include "hzncodec.h"
#include "ibstream.h"

// node decoder
template <typename T>
class HZNdecoder : public HZNcodec<T> {
public:
  HZNdecoder(IBSTREAM* stream, unsigned nodes, unsigned dims = 3, unsigned perm = 0, int bias = 0) : HZNcodec<T>(nodes, dims, perm, bias), stream(stream) {}

  // encode all nodes of a cell
  inline unsigned decode(T* node, const int* cell);

private:
  using HZNcodec<T>::prepare;
  using HZNcodec<T>::cellsize;
  using HZNcodec<T>::mask;
  using HZNcodec<T>::count;
  using HZNcodec<T>::coded;
  using HZNcodec<T>::pred;
  using HZNcodec<T>::diff;

  void get();
  IBSTREAM* stream;
};

#include "hzndecoder.inl"

#endif
