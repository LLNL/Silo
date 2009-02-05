#ifndef FPZIP_READ_H
#define FPZIP_READ_H

#define subsize(T, n) (CHAR_BIT * sizeof(T) * (n) / 4)

// file reader for compressed data
#if FPZIP_BLOCK_SIZE > 1
class RCfiledecoder : public RCdecoder {
public:
  RCfiledecoder(FILE* file) : RCdecoder(), error(false), file(file), count(0), index(0), size(0) {}
  unsigned getbyte()
  {
    if (index == size) {
      size = fread(buffer, 1, FPZIP_BLOCK_SIZE, file);
      if (!size) {
        size = 1;
        error = true;
      }
      else
        count += size;
      index = 0;
    }
    return buffer[index++];
  }
  unsigned bytes() const { return count; }
  bool error;
private:
  FILE* file;
  unsigned count;
  unsigned index;
  unsigned size;
  unsigned char buffer[FPZIP_BLOCK_SIZE];
};
#else
class RCfiledecoder : public RCdecoder {
public:
  RCfiledecoder(FILE* file) : RCdecoder(), error(false), file(file), count(0) {}
  unsigned getbyte()
  {
    int byte = fgetc(file);
    if (byte == EOF)
      error = true;
    else
      count++;
    return byte;
  }
  unsigned bytes() const { return count; }
  bool error;
private:
  FILE* file;
  unsigned count;
};
#endif

class RCmemdecoder : public RCdecoder {
public:
  RCmemdecoder(const void* buffer) : RCdecoder(), error(false), ptr((const unsigned char*)buffer), begin(ptr) {}
  unsigned getbyte() { return *ptr++; }
  unsigned bytes() const { return ptr - begin; }
  bool error;
private:
  const unsigned char* ptr;
  const unsigned char* const begin;
};

#endif
