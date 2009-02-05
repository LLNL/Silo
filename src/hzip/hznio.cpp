#include "hzip.h"
#include "hznio.h"

/*@p-r-i-v-a-t-e---f-u-n-c-t-i-o-n-s-----------------------------------------*/

// close stream for writing and deallocate
static int
read_close(
  HZNstream* stream // output stream
)
{
  switch (stream->type) {
    case hzUCHAR:
      delete stream->cdecoder;
      break;
    case hzUSHORT:
      delete stream->sdecoder;
      break;
    case hzINT:
      delete stream->idecoder;
      break;
    case hzFLOAT:
      delete stream->fdecoder;
      break;
    case hzDOUBLE:
      delete stream->ddecoder;
      break;
  }

  IBSTREAM* in = stream->in;
  int status = bytes(in);

  delete stream;
  delete in;

  return status;
}

// close stream for writing and deallocate
static int
write_close(
  HZNstream* stream // output stream
)
{
  switch (stream->type) {
    case hzUCHAR:
      stream->cencoder->end();
      delete stream->cencoder;
      break;
    case hzUSHORT:
      stream->sencoder->end();
      delete stream->sencoder;
      break;
    case hzINT:
      stream->iencoder->end();
      delete stream->iencoder;
      break;
    case hzFLOAT:
      stream->fencoder->end();
      delete stream->fencoder;
      break;
    case hzDOUBLE:
      stream->dencoder->end();
      delete stream->dencoder;
      break;
  }

  OBSTREAM* out = stream->out;
  out->close();
  int status = bytes(out);

  delete stream;
  delete out;

  return status;
}

/*@p-u-b-l-i-c---f-u-n-c-t-i-o-n-s-------------------------------------------*/

// return mesh dimensionality
unsigned
hzip_node_dimensions(
  HZNstream* stream // I/O stream
)
{
  return stream->dims;
}

// return number of nodes in stream
unsigned
hzip_node_count(
  HZNstream* stream // input stream
)
{
  return stream->count;
}

// return node data type
HZtype
hzip_node_type(
  HZNstream* stream // input stream
)
{
  return stream->type;
}

// close stream and deallocate
int
hzip_node_close(
  HZNstream* stream // I/O stream
)
{
  return stream->access == hzREAD ? read_close(stream) : write_close(stream);
}
