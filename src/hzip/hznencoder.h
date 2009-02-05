#ifndef HZNENCODER_H
#define HZNENCODER_H

#include "hzncodec.h"
#include "obstream.h"

// node encoder
template <typename T>
class HZNencoder : public HZNcodec<T> {
public:
  HZNencoder(OBSTREAM* stream, unsigned nodes, unsigned dims = 3, unsigned perm = 0, int bias = 0) : HZNcodec<T>(nodes, dims, perm, bias), stream(stream) {}

  // encode all nodes of a cell
  inline unsigned encode(const T* node, const int* cell);

  // end encoding
  inline void end();

private:
  using HZNcodec<T>::prepare;
  using HZNcodec<T>::cellsize;
  using HZNcodec<T>::mask;
  using HZNcodec<T>::count;
  using HZNcodec<T>::coded;
  using HZNcodec<T>::pred;
  using HZNcodec<T>::diff;

  void put();
  OBSTREAM* stream;
};

#include "hznencoder.inl"

#endif
