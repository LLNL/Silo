#ifndef HZMAP_H
#define HZMAP_H

#if !defined WITH_REINTERPRET_CAST && !defined WITH_UNION
#include <cstring>
#endif

// identity map
template <typename T>
class HZmap {
public:
  typedef T DOMAIN;
  typedef T RANGE;
  T operator()(T x) const { return x; }
};

// monotonic map between signed and unsigned int
template <>
class HZmap<int> {
public:
  typedef int DOMAIN;
  typedef unsigned RANGE;
  union UNION {
    UNION(DOMAIN d) : d(d) {}
    UNION(RANGE r) : r(r) {}
    DOMAIN d;
    RANGE r;
  };
  int operator()(unsigned u) const
  {
    u = ~u > u ? ~u : u - 0x80000000u;
#ifdef WITH_REINTERPRET_CAST
    return reinterpret_cast<const int&>(u);
#elif defined WITH_UNION
    UNION shared(u);
    return shared.d;
#else
    int i;
    memcpy(&i, &u, sizeof(i));
    return i;
#endif
  }
  unsigned operator()(int i) const
  {
#ifdef WITH_REINTERPRET_CAST
    unsigned u = reinterpret_cast<const unsigned&>(i);
#elif defined WITH_UNION
    UNION shared(i);
    unsigned u = shared.r;
#else
    unsigned u;
    memcpy(&u, &i, sizeof(i));
#endif
    return ~u < u ? ~u : u + 0x80000000u;
  }
};

// monotonic map between float and its binary unsigned integer representation
template <>
class HZmap<float> {
public:
  typedef float DOMAIN;
  typedef unsigned RANGE;
  union UNION {
    UNION(DOMAIN d) : d(d) {}
    UNION(RANGE r) : r(r) {}
    DOMAIN d;
    RANGE r;
  };
  float operator()(unsigned u) const
  {
    u = ~u > u ? ~u : u - 0x80000000u;
#ifdef WITH_REINTERPRET_CAST
    return reinterpret_cast<const float&>(u);
#elif defined WITH_UNION
    UNION shared(u);
    return shared.d;
#else
    float f;
    memcpy(&f, &u, sizeof(f));
    return f;
#endif
  }
  unsigned operator()(float f) const
  {
#ifdef WITH_REINTERPRET_CAST
    unsigned u = reinterpret_cast<const unsigned&>(f);
#elif defined WITH_UNION
    UNION shared(f);
    unsigned u = shared.r;
#else
    unsigned u;
    memcpy(&u, &f, sizeof(u));
#endif
    return ~u < u ? ~u : u + 0x80000000u;
  }
};

// monotonic map between double and its binary unsigned integer representation
template <>
class HZmap<double> {
public:
  typedef double DOMAIN;
  typedef unsigned long long RANGE;
  union UNION {
    UNION(DOMAIN d) : d(d) {}
    UNION(RANGE r) : r(r) {}
    DOMAIN d;
    RANGE r;
  };
  double operator()(unsigned long long u) const
  {
    u = ~u > u ? ~u : u - 0x8000000000000000ull;
#ifdef WITH_REINTERPRET_CAST
    return reinterpret_cast<const double&>(u);
#elif defined WITH_UNION
    UNION shared(u);
    return shared.d;
#else
    double f;
    memcpy(&f, &u, sizeof(f));
    return f;
#endif
  }
  unsigned long long operator()(double f) const
  {
#ifdef WITH_REINTERPRET_CAST
    unsigned long long u = reinterpret_cast<const unsigned long long&>(f);
#elif defined WITH_UNION
    UNION shared(f);
    unsigned long long u = shared.r;
#else
    unsigned long long u;
    memcpy(&u, &f, sizeof(u));
#endif
    return ~u < u ? ~u : u + 0x8000000000000000ull;
  }
};

#endif
