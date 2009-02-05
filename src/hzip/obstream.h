#ifndef OBSTREAM_H
#define OBSTREAM_H

#include <cerrno>
#include <cstdio>
#include <cstring>

// error codes
enum OBSTREAMerror {
  OBSTREAM_OK = 0,
  OBSTREAM_ERR_UNKNOWN = -1,
  OBSTREAM_ERR_NO_SPACE = -2,
  OBSTREAM_ERR_ZLIB = -3
};

//@OBSTREAM....................................................................

// abstract base class for output byte stream
class OBSTREAM {
public:
  virtual ~OBSTREAM() {}

  // next stream object in pipeline (or 0 if none)
  virtual OBSTREAM* next() { return 0; }

  // tail stream object in pipeline (the terminal object that outputs bytes)
  virtual OBSTREAM* tail() { return this; }

  // detach this stream object from the next one in the pipeline
  virtual OBSTREAM* detach() { return 0; }

  // output a single byte
  virtual void put(unsigned char byte) = 0;

  // output a sequence of bytes
  virtual void put(const void* byte, size_t size)
  {
    for (const unsigned char* p = (const unsigned char*)byte; size--; p++)
      put(*p);
  }

  // flush any buffered data
  virtual void flush() {}

  // flush and close stream for writing
  virtual void close() {}

  // number of bytes written to this stream object
  virtual size_t bytes() const = 0;

  // error code (0 is returned if no error)
  virtual int error() const = 0;
};

//@OBSTREAMmem.................................................................

// write stream to a fixed-size memory buffer
class OBSTREAMmem : public OBSTREAM {
public:
  OBSTREAMmem(void* buffer, size_t size) :
    begin((unsigned char*)buffer),
    end(begin + size),
    ptr(begin),
    status(OBSTREAM_OK)
  {}
  void put(unsigned char byte)
  {
    if (ptr == end)
      status = OBSTREAM_ERR_NO_SPACE;
    else
      *ptr++ = byte;
  }
  void put(const void* byte, size_t size)
  {
    if (ptr + size > end)
      status = OBSTREAM_ERR_NO_SPACE;
    else {
      memcpy(ptr, byte, size);
      ptr += size;
    }
  }
  size_t bytes() const { return (size_t)(ptr - begin); }
  int error() const { return status; }
private:
  unsigned char* const begin;
  unsigned char* const end;
  unsigned char* ptr;
  int status;
};

//@OBSTREAMfile................................................................

// write stream to a binary file
class OBSTREAMfile : public OBSTREAM {
public:
  OBSTREAMfile(FILE* out) : file(out), written(0), status(OBSTREAM_OK) {}
  void put(unsigned char byte)
  {
    if (fputc(byte, file) == EOF)
      status = (errno == ENOMEM ? OBSTREAM_ERR_NO_SPACE : OBSTREAM_ERR_UNKNOWN);
    else
      written++;
  }
  void put(const void* byte, size_t size)
  {
    if (fwrite(byte, 1, size, file) != size)
      status = (errno == ENOMEM ? OBSTREAM_ERR_NO_SPACE : OBSTREAM_ERR_UNKNOWN);
    else
      written += size;
  }
  void flush() { fflush(file); }
  void close() { flush(); }
  size_t bytes() const { return written; }
  int error() const { return status; }
private:
  FILE* file;
  size_t written;
  int status;
};

//@OBSTREAMzlib................................................................

#ifndef WITHOUT_ZLIB
#include <zlib.h>

#define OBSTREAM_ZLIB_IBUF_SIZE 0x6000
#define OBSTREAM_ZLIB_OBUF_SIZE 0x2000

// zlib compressed stream
class OBSTREAMzlib : public OBSTREAM {
public:
  OBSTREAMzlib(OBSTREAM* stream, int level = Z_DEFAULT_COMPRESSION, size_t insize = OBSTREAM_ZLIB_IBUF_SIZE, size_t outsize = OBSTREAM_ZLIB_OBUF_SIZE) :
    obstream(stream),
    insize(insize),
    outsize(outsize),
    in(new unsigned char[insize]),
    out(new unsigned char[outsize]),
    end(in + insize),
    ptr(in),
    written(0),
    status(OBSTREAM_OK)
  {
    zstream.zalloc = Z_NULL;
    zstream.zfree = Z_NULL;
    zstream.opaque = Z_NULL;
    if (deflateInit(&zstream, level) != Z_OK)
      status = OBSTREAM_ERR_ZLIB;
  }
  ~OBSTREAMzlib()
  {
    delete[] in;
    delete[] out;
    if (obstream)
      delete obstream;
  }
  OBSTREAM* next() { return obstream; }
  OBSTREAM* tail() { return obstream->tail(); }
  OBSTREAM* detach() { OBSTREAM* s = obstream; obstream = 0; return s; }
  void put(unsigned char byte)
  {
    if (ptr == end)
      encode(Z_NO_FLUSH);
    *ptr++ = byte;
  }
  using OBSTREAM::put;
  void flush() { encode(Z_SYNC_FLUSH); obstream->flush(); }
  void close()
  {
    encode(Z_FINISH);
    if (deflateEnd(&zstream) != Z_OK)
      status = OBSTREAM_ERR_ZLIB;
    obstream->close();
  }
  size_t bytes() const { return written; }
  int error() const { return status ? status : obstream->error(); }

private:
  // encode buffer
  void encode(int mode)
  {
    zstream.next_in = in;
    zstream.avail_in = (unsigned)(ptr - in);
    written += zstream.avail_in;
    do {
      zstream.next_out = out;
      zstream.avail_out = outsize;
      deflate(&zstream, mode);
      obstream->put(out, outsize - zstream.avail_out);
    } while (!zstream.avail_out);
    ptr = in;
  }

  OBSTREAM* obstream;
  z_stream zstream;
  const size_t insize;
  const size_t outsize;
  unsigned char* const in;
  unsigned char* const out;
  unsigned char* const end;
  unsigned char* ptr;
  size_t written;
  int status;
};
#endif

#endif
