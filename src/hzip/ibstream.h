#ifndef IBSTREAM_H
#define IBSTREAM_H

#include <cstdio>
#include <cstring>

// error codes
enum IBSTREAMerror {
  IBSTREAM_OK = 0,
  IBSTREAM_ERR_UNKNOWN = -1,
  IBSTREAM_ERR_INVALID = -2,
  IBSTREAM_ERR_ZLIB = -3
};

//@IBSTREAM....................................................................

// abstract base class for input byte stream
class IBSTREAM {
public:
  virtual ~IBSTREAM() {}

  // previous stream object in pipeline (or 0 if none)
  virtual IBSTREAM* prev() { return 0; }

  // head stream object in pipeline (the source object that generates bytes)
  virtual IBSTREAM* head() { return this; }

  // detach this stream object from the previous one in the pipeline
  virtual IBSTREAM* detach() { return 0; }

  // input a single byte (EOF is returned if no more input)
  virtual int get() = 0;

  // input a sequence of bytes (the actual number of bytes read is returned)
  virtual size_t get(void *byte, size_t size)
  {
    unsigned char* p = (unsigned char*)byte;
    for (int c; size-- && (c = get()) != EOF; p++)
      *p = (unsigned char)c;
    return (size_t)(p - (unsigned char*)byte);
  }

  // rewind stream a number of bytes
  virtual bool unget(size_t size) = 0;

  // number of bytes read from this stream object
  virtual size_t bytes() const = 0;

  // error code (0 is returned if no error)
  virtual int error() const = 0;
};

//@IBSTREAMmem.................................................................

// read stream from a fixed-size memory buffer
class IBSTREAMmem : public IBSTREAM {
public:
  IBSTREAMmem(const void* buffer, size_t size) :
    begin((const unsigned char*)buffer),
    end(begin + size),
    ptr(begin),
    status(IBSTREAM_OK)
  {}
  int get()
  {
    return ptr == end ? EOF : *ptr++;
  }
  size_t get(void* byte, size_t size)
  {
    if (size > (size_t)(end - ptr))
      size = (size_t)(end - ptr);
    memcpy(byte, ptr, size);
    ptr += size;
    return size;
  }
  bool unget(size_t size)
  {
    if ((size_t)(ptr - begin) < size) {
      status = IBSTREAM_ERR_INVALID;
      return false;
    }
    else {
      ptr -= size;
      return true;
    }
  }
  size_t bytes() const { return (size_t)(ptr - begin); }
  int error() const { return status; }
private:
  const unsigned char* const begin;
  const unsigned char* const end;
  const unsigned char* ptr;
  int status;
};

//@IBSTREAMfile................................................................

// read stream from a binary file
class IBSTREAMfile : public IBSTREAM {
public:
  IBSTREAMfile(FILE* in) : file(in), read(0), status(IBSTREAM_OK) {}
  int get()
  {
    int c = fgetc(file);
    if (c == EOF) {
      if (ferror(file))
        status = IBSTREAM_ERR_UNKNOWN;
    }
    else
      read++;
    return c;
  }
  size_t get(void* byte, size_t size)
  {
    size = fread(byte, 1, size, file);
    if (ferror(file))
      status = IBSTREAM_ERR_UNKNOWN;
    read += size;
    return size;
  }
  bool unget(size_t size)
  {
    if (fseek(file, -(long)size, SEEK_CUR)) {
      status = IBSTREAM_ERR_INVALID;
      return false;
    }
    else {
      read -= size;
      return true;
    }
  }
  size_t bytes() const { return read; }
  int error() const { return status; }
private:
  FILE* file;
  size_t read;
  int status;
};

//@IBSTREAMzlib................................................................

#ifndef WITHOUT_ZLIB
#include <zlib.h>

#define IBSTREAM_ZLIB_IBUF_SIZE 0x2000
#define IBSTREAM_ZLIB_OBUF_SIZE 0x6000

// zlib compressed stream
class IBSTREAMzlib : public IBSTREAM {
public:
  IBSTREAMzlib(IBSTREAM* stream, size_t insize = IBSTREAM_ZLIB_IBUF_SIZE, size_t outsize = IBSTREAM_ZLIB_OBUF_SIZE) :
    ibstream(stream), insize(insize), outsize(outsize),
    in(new unsigned char[insize]),
    out(new unsigned char[outsize]),
    end(0),
    ptr(0),
    read(0),
    status(IBSTREAM_OK)
  {
    zstream.zalloc = Z_NULL;
    zstream.zfree = Z_NULL;
    zstream.opaque = Z_NULL;
    zstream.next_in = Z_NULL;
    zstream.avail_in = 0;
    if (inflateInit(&zstream) != Z_OK)
      status = IBSTREAM_ERR_ZLIB;
  }
  ~IBSTREAMzlib()
  {
    delete[] in;
    delete[] out;
    if (ibstream)
      delete ibstream;
  }
  IBSTREAM* prev() { return ibstream; }
  IBSTREAM* head() { return ibstream->head(); }
  IBSTREAM* detach() { IBSTREAM* s = ibstream; ibstream = 0; return s; }
  int get()
  {
    return ptr == end && !decode() ? EOF : *ptr++;
  }
  bool unget(size_t size)
  {
    if (size) {
      status = IBSTREAM_ERR_INVALID;
      return false;
    }
    else
      return true;
  }
  using IBSTREAM::get;
  size_t bytes() const { return read; }
  int error() const { return status ? status : ibstream->error(); }

private:
  // decode buffer
  bool decode()
  {
    do {
      if (!zstream.avail_in) {
        zstream.next_in = in;
        zstream.avail_in = ibstream->get(in, insize);
        if (!zstream.avail_in) {
          status = IBSTREAM_ERR_ZLIB;
          return false;
        }
      }
      zstream.next_out = out;
      zstream.avail_out = outsize;
      switch (inflate(&zstream, Z_NO_FLUSH)) {
        case Z_OK:
          break;
        case Z_STREAM_END:
          if (inflateEnd(&zstream) == Z_OK) {
            if (ibstream->unget(zstream.avail_in))
              zstream.avail_in = 0;
            break;
          }
          /*FALLTHROUGH*/
        default:
          status = IBSTREAM_ERR_ZLIB;
          return false;
      }
      end = out + outsize - zstream.avail_out;
    } while (end == out);
    read += (unsigned)(end - out);
    ptr = out;
    return true;
  }

  IBSTREAM* ibstream;
  z_stream zstream;
  const size_t insize;
  const size_t outsize;
  unsigned char* const in;
  unsigned char* const out;
  const unsigned char* end;
  const unsigned char* ptr;
  size_t read;
  int status;
};
#endif

#endif
