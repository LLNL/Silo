#include "hzip.h"
#include "hznio.h"

/*@p-r-i-v-a-t-e---f-u-n-c-t-i-o-n-s-----------------------------------------*/

// write header data
static HZNstream*
write_header(
  OBSTREAM*   out,   // output stream
  unsigned    perm,  // node permutation
  int         bias,  // node index bias
  unsigned    dims,  // mesh dimensionality
  unsigned    count, // total number of nodes
  HZtype      type,  // node data type
  unsigned    codec, // codec number
  const void* param  // codec parameters
)
{
  HZNstream* stream = new HZNstream(hzWRITE);

  switch (codec) {
    case HZN_CODEC_BASE:
      stream->out = out;
      break;
#ifndef WITHOUT_ZLIB
    case HZN_CODEC_ZLIB: {
        const HZNCODECzlib* p = param ? (const HZNCODECzlib*)param : &hzn_codec_zlib;
        stream->out = new OBSTREAMzlib(out, p->zlib.level, p->zlib.insize, p->zlib.outsize);
      }
      break;
#endif
    default:
      delete stream;
      delete out;
      return 0;
  }
  stream->type = type;
  stream->dims = dims;
  stream->count = count;

#ifndef HZ_WITHOUT_MAGIC
  // magic and format
  out->put('h');
  out->put('z');
  out->put('n');
  out->put(HZN_VERSION);
#endif

  // cell count and codec parameters
  out->put(dims);
  out->put(0);
  out->put(codec);
  out->put(type);
  putuint(out, count);

  // check for errors
  if (stream->out->error()) {
    delete stream->out;
    delete stream;
    return 0;
  }

  // initialize encoder
  switch (type) {
    case hzUCHAR:
      stream->cencoder = new HZNencoder<unsigned char>(stream->out, count, dims, perm, bias);
      break;
    case hzUSHORT:
      stream->sencoder = new HZNencoder<unsigned short>(stream->out, count, dims, perm, bias);
      break;
    case hzINT:
      stream->iencoder = new HZNencoder<int>(stream->out, count, dims, perm, bias);
      break;
    case hzFLOAT:
      stream->fencoder = new HZNencoder<float>(stream->out, count, dims, perm, bias);
      break;
    case hzDOUBLE:
      stream->dencoder = new HZNencoder<double>(stream->out, count, dims, perm, bias);
      break;
  }

  return stream;
}

// compress and write node data
template <typename T>
static void
write(
  HZNencoder<T>* encoder, // node encoder
  const void*    node,    // node data array
  const int*     mesh,    // array of node indices
  unsigned       count    // number of cells to visit
)
{
  for (unsigned i = 0; i < count; i++)
    mesh += encoder->encode((const T*)node, mesh);
}

/*@p-u-b-l-i-c---f-u-n-c-t-i-o-n-s-------------------------------------------*/

// create stream for writing node data to file
HZNstream*
hzip_node_create_file(
  FILE*       file,  // compressed output file
  unsigned    perm,  // node permutation
  int         bias,  // node index bias
  unsigned    dims,  // mesh dimensionality
  unsigned    count, // total number of nodes
  HZtype      type,  // node data type
  unsigned    codec, // codec number
  const void* param  // codec parameters
)
{
  return write_header(new OBSTREAMfile(file), perm, bias, dims, count, type, codec, param);
}

// create stream for writing node data to memory
HZNstream*
hzip_node_create_mem(
  void*       buffer, // pointer to compressed data
  size_t      size,   // byte size of allocated storage
  unsigned    perm,   // node permutation
  int         bias,   // node index bias
  unsigned    dims,   // mesh dimensionality
  unsigned    count,  // total number of nodes
  HZtype      type,   // node data type
  unsigned    codec,  // codec number
  const void* param   // codec parameters
)
{
  return write_header(new OBSTREAMmem(buffer, size), perm, bias, dims, count, type, codec, param);
}

// compress node data for count cells
int
hzip_node_write(
  HZNstream*  stream, // output stream
  const void* node,   // node data array to write from
  const int*  mesh,   // array of node indices
  unsigned    count   // number of cells to visit
)
{
  switch (stream->type) {
    case hzUCHAR:
      write(stream->cencoder, node, mesh, count);
      break;
    case hzUSHORT:
      write(stream->sencoder, node, mesh, count);
      break;
    case hzINT:
      write(stream->iencoder, node, mesh, count);
      break;
    case hzFLOAT:
      write(stream->fencoder, node, mesh, count);
      break;
    case hzDOUBLE:
      write(stream->dencoder, node, mesh, count);
      break;
  }

  return bytes(stream->out);
}

// flush all buffers
int
hzip_node_flush(
  HZNstream* stream // output stream
)
{
  OBSTREAM* out = stream->out;
  out->flush();
  return bytes(out);
}
