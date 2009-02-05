#ifndef HC_WRITE_H
#define HC_WRITE_H

#include "hcencoder.h"

// file writer for compressed data
class HCfileencoder : public HCencoder {
public:
  HCfileencoder(FILE* file, unsigned perm = 0) : HCencoder(perm), error(false), file(file), count(0) {}
  void putbyte(unsigned char byte)
  {
    if (fputc(byte, file) == EOF)
      error = true;
    else
      count++;
  }
  unsigned bytes() const { return count; }
  bool error;
private:
  FILE* file;
  unsigned count;
};

// memory writer for compressed data
class HCmemencoder : public HCencoder {
public:
  HCmemencoder(void* buffer, unsigned size, unsigned perm = 0) : HCencoder(perm), error(false), ptr((unsigned char*)buffer), begin(ptr), end(ptr + size) {}
  void putbyte(unsigned char byte)
  {
    if (ptr == end)
      error = true;
    else
      *ptr++ = byte;
  }
  unsigned bytes() const { return ptr - begin; }
  bool error;
private:
  unsigned char* ptr;
  const unsigned char* const begin;
  const unsigned char* const end;
};

#endif
