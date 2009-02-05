#ifndef HZRESIDUAL_H
#define HZRESIDUAL_H

#include "ibstream.h"
#include "obstream.h"
#include "hzmap.h"

// variable-length residual computation and I/O
template < typename T, class M = HZmap<T> >
class HZresidual {
public:
  HZresidual() : value() {}

  // construct residual from predicted and actual values
  HZresidual(T p, T a);

  // read and construct residual from stream
  HZresidual(IBSTREAM* stream);

  // write residual to stream
  void put(OBSTREAM* stream) const;

  // compute actual value from prediction and residual
  T operator+(T p) const;

  // is residual zero?
  bool zero() const { return !value; }

private:
  typedef typename M::RANGE U;
  U value; // value of residual
  M map;   // functors for mapping between value type T and integer type U
};

template <typename T, class M>
inline T operator+(T p, HZresidual<T, M> r) { return r + p; }

template <typename T, class M>
inline T& operator+=(T& v, HZresidual<T, M> r) { v = r + v; return v; }

// specialization for fixed-length unsigned char residuals
template <class M>
class HZresidual<unsigned char, M> {
private:
  typedef unsigned char U;
public:
  HZresidual() : value() {}
  HZresidual(U p, U a) : value(U(p - a)) {}
  HZresidual(IBSTREAM* stream) : value(U(stream->get())) {}
  void put(OBSTREAM* stream) const { stream->put(value); }
  U operator+(U p) const { return U(p - value); }
  bool zero() const { return !value; }
private:
  U value; // value of residual
};

#include "hzresidual.inl"

#endif
