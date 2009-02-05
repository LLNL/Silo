#ifndef HZMIO_H
#define HZMIO_H

#include "hzio.h"
#include "hzmencoder.h"
#include "hzmdecoder.h"

// mesh input/output stream
struct HZMstream {
  HZMstream(HZaccess access) : access(access) {}
  HZaccess      access;  // read or write access
  union {
    OBSTREAM*   out;     // output stream
    IBSTREAM*   in;      // input stream
  };
  union {
    HZMencoder* encoder; // mesh compressor
    HZMdecoder* decoder; // mesh decompressor
  };
  unsigned      dims;    // mesh dimensionality
  unsigned      count;   // total number of cells (0 if unknown)
  unsigned      avail;   // number of cells left in chunk
};

#endif
