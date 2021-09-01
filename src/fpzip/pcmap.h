#ifndef PC_MAP_H
#define PC_MAP_H

#include <climits>
#if !defined WITH_REINTERPRET_CAST && !defined WITH_UNION
#include <cstring>
#endif

#define bitsizeof(t) ((unsigned)(CHAR_BIT * sizeof(t)))

template <typename T, unsigned width = bitsizeof(T), typename U = void>
struct PCmap;

// specialized for integer-to-integer map
template <typename T, unsigned width>
struct PCmap<T, width, void> {
  typedef T FPZIP_Domain_t;
  typedef T FPZIP_Range_t;
  static const unsigned bits = width;                    // FPZIP_Range_t bits
  static const unsigned shift = bitsizeof(FPZIP_Range_t) - bits; // FPZIP_Domain_t\FPZIP_Range_t bits
  FPZIP_Range_t forward(FPZIP_Domain_t d) const { return d >> shift; }
  FPZIP_Domain_t inverse(FPZIP_Range_t r) const { return r << shift; }
  FPZIP_Domain_t identity(FPZIP_Domain_t d) const { return inverse(forward(d)); }
};

// specialized for float type
template <unsigned width>
struct PCmap<float, width, void> {
  typedef float    FPZIP_Domain_t;
  typedef unsigned FPZIP_Range_t;
  union UNION {
    UNION(FPZIP_Domain_t d) : d(d) {}
    UNION(FPZIP_Range_t r) : r(r) {}
    FPZIP_Domain_t d;
    FPZIP_Range_t r;
  };
  static const unsigned bits = width;                    // FPZIP_Range_t bits
  static const unsigned shift = bitsizeof(FPZIP_Range_t) - bits; // FPZIP_Domain_t\FPZIP_Range_t bits
  FPZIP_Range_t fcast(FPZIP_Domain_t d) const;
  FPZIP_Domain_t icast(FPZIP_Range_t r) const;
  FPZIP_Range_t forward(FPZIP_Domain_t d) const;
  FPZIP_Domain_t inverse(FPZIP_Range_t r) const;
  FPZIP_Domain_t identity(FPZIP_Domain_t d) const;
};

// specialized for double type
template <unsigned width>
struct PCmap<double, width, void> {
  typedef double             FPZIP_Domain_t;
  typedef unsigned long long FPZIP_Range_t;
  union UNION {
    UNION(FPZIP_Domain_t d) : d(d) {}
    UNION(FPZIP_Range_t r) : r(r) {}
    FPZIP_Domain_t d;
    FPZIP_Range_t r;
  };
  static const unsigned bits = width;                    // FPZIP_Range_t bits
  static const unsigned shift = bitsizeof(FPZIP_Range_t) - bits; // FPZIP_Domain_t\FPZIP_Range_t bits
  FPZIP_Range_t fcast(FPZIP_Domain_t d) const;
  FPZIP_Domain_t icast(FPZIP_Range_t r) const;
  FPZIP_Range_t forward(FPZIP_Domain_t d) const;
  FPZIP_Domain_t inverse(FPZIP_Range_t r) const;
  FPZIP_Domain_t identity(FPZIP_Domain_t d) const;
};

#include "pcmap.inl"

#endif
