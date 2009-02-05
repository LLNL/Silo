#ifndef LOCAL_WRITE_H
#define LOCAL_WRITE_H

#define subsize(T, n) (CHAR_BIT * sizeof(T) * (n) / 4)

namespace { // anonymous namespace

// file writer for compressed data
#if LOCAL_BLOCK_SIZE > 1
class RCfileencoder : public RCencoder {
public:
  RCfileencoder(FILE* file) : error(false), file(file), count(0), size(0), RCencoder() {}
  ~RCfileencoder() { flush(); }
  void putbyte(unsigned byte)
  {
    if (size == LOCAL_BLOCK_SIZE)
      flush();
    buffer[size++] = (unsigned char)byte;
  }
  void flush()
  {
    if (fwrite(buffer, 1, size, file) != size)
      error = true;
    else
      count += size;
    size = 0;
  }
  unsigned bytes() const { return count; }
  bool error;
private:
  FILE* file;
  unsigned count;
  unsigned size;
  unsigned char buffer[LOCAL_BLOCK_SIZE];
};
#else
class RCfileencoder : public RCencoder {
public:
  RCfileencoder(FILE* file) : error(false), file(file), count(0), RCencoder() {}
  void putbyte(unsigned byte)
  {
    if (fputc(byte, file) == EOF)
      error = true;
    else
      count++;
  }
  void flush() {}
  unsigned bytes() const { return count; }
  bool error;
private:
  FILE* file;
  unsigned count;
};
#endif

// memory writer for compressed data
class RCmemencoder : public RCencoder {
public:
  RCmemencoder(void* buffer, unsigned size) : error(false), ptr((unsigned char*)buffer), begin(ptr), end(ptr + size), RCencoder() {}
  void putbyte(unsigned byte)
  {
    if (ptr == end)
      error = true;
    else
      *ptr++ = (unsigned char)byte;
  }
  unsigned bytes() const { return ptr - begin; }
  bool error;
private:
  unsigned char* ptr;
  const unsigned char* const begin;
  const unsigned char* const end;
};

} // anonymous namespace

#endif
