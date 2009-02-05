#ifndef CODEC_H
#define CODEC_H

#define LOCAL_FP_FAST 1
#define LOCAL_FP_SAFE 2
#define LOCAL_FP_EMUL 3
#define LOCAL_FP_INT  4

#ifndef LOCAL_FP
  #error "floating-point mode LOCAL_FP not defined"
#elif LOCAL_FP < 1 || LOCAL_FP > 4
  #error "invalid floating-point mode LOCAL_FP"
#endif

#if LOCAL_FP == LOCAL_FP_INT
// identity map for integer arithmetic
template <typename T, unsigned width>
struct PCmap<T, width, T> {
  typedef T DOMAIN;
  typedef T RANGE;
  static const unsigned bits = width;
  static const T        mask = ~T(0) >> (bitsizeof(T) - bits);
  RANGE forward(DOMAIN d) const { return d & mask; }
  DOMAIN inverse(RANGE r) const { return r & mask; }
  DOMAIN identity(DOMAIN d) const { return d & mask; }
};
#endif

#define SFC_MAJ_VERSION 0x0101
#define SFC_MIN_VERSION LOCAL_FP

#endif
