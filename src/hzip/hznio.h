#ifndef HZNIO_H
#define HZNIO_H

#include "hzio.h"
#include "hznencoder.h"
#include "hzndecoder.h"

// node input/output stream
struct HZNstream {
  HZNstream(HZaccess access) : access(access) {}
  HZaccess access; // read or write access
  HZtype   type;   // node data type
  union {
    OBSTREAM* out; // output stream
    IBSTREAM* in;  // input stream
  };
  union {
    HZNencoder<unsigned char>* cencoder;
    HZNencoder<unsigned short>* sencoder;
    HZNencoder<int>* iencoder;
    HZNencoder<float>* fencoder;
    HZNencoder<double>* dencoder;
    HZNdecoder<unsigned char>* cdecoder;
    HZNdecoder<unsigned short>* sdecoder;
    HZNdecoder<int>* idecoder;
    HZNdecoder<float>* fdecoder;
    HZNdecoder<double>* ddecoder;
  };
  unsigned dims;  // mesh dimensionality
  unsigned count; // total number of nodes (0 if unknown)
};

#endif
