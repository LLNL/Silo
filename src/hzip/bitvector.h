#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <cstring>

// replacement for std::vector<bool>
class BITVECTOR {
private:
  // helper class for bit assignment
  typedef unsigned char BYTE;
  class BIT {
  public:
    BIT(BYTE& byte, BYTE mask) : byte(byte), mask(mask) {}
    bool operator=(bool value)
    {
      byte = BYTE(value ? byte | mask : byte & ~mask);
      return value;
    }
    operator bool() const { return (byte & mask) != 0; }
  private:
    BYTE& byte; // reference to target byte
    BYTE mask;  // mask for target bit
  };

public:
  BITVECTOR() : a(0) {}
  BITVECTOR(size_t size) : a(new BYTE[count(size)]) {}
  BITVECTOR(size_t size, bool value) : a(new BYTE[count(size)])
  {
    memset(a, value ? ~0 : 0, count(size));
  }
  ~BITVECTOR() { delete[] a; }
  BIT operator[](unsigned i) { return BIT(byte(i), mask(i)); }
  bool operator[](unsigned i) const { return (byte(i) & mask(i)) != 0; }

private:
  size_t count(size_t size) const { return (size + 7) >> 3; }
  BYTE mask(unsigned i) const { return BYTE(1 << (i & 0x7)); }
  BYTE& byte(unsigned i) { return a[i >> 3]; }
  const BYTE& byte(unsigned i) const { return a[i >> 3]; }
  BYTE* const a;
};

#endif
