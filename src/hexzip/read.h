#ifndef HC_READ_H
#define HC_READ_H

#include "hcdecoder.h"

// file reader for compressed data
class HCfiledecoder : public HCdecoder {
public:
  HCfiledecoder(FILE* file, unsigned perm = 0) : HCdecoder(perm), error(false), file(file), count(0) {}
  unsigned char getbyte()
  {
    int byte = fgetc(file);
    if (byte == EOF)
      error = true;
    else
      count++;
    return (unsigned char)byte;
  }
  unsigned bytes() const { return count; }
  bool error;
private:
  FILE* file;
  unsigned count;
};

// memory writer for compressed data
class HCmemdecoder : public HCdecoder {
public:
  HCmemdecoder(const void* buffer, unsigned perm = 0) : HCdecoder(perm), error(false), ptr((const unsigned char*)buffer), begin(ptr) {}
  unsigned char getbyte() { return *ptr++; }
  unsigned bytes() const { return ptr - begin; }
  bool error;
private:
  const unsigned char* ptr;
  const unsigned char* const begin;
};

#endif
