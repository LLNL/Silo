#include "hzip.h"
#include "hznio.h"

/*@p-r-i-v-a-t-e---f-u-n-c-t-i-o-n-s-----------------------------------------*/

// read header data
static HZNstream*
read_header(
  IBSTREAM* in,   // input stream
  unsigned  perm, // node permutation
  int       bias  // node index bias
)
{
#ifndef HZ_WITHOUT_MAGIC
  // magic and format
  if (in->get() != 'h' ||
      in->get() != 'z' ||
      in->get() != 'n' ||
      in->get() != HZN_VERSION) {
    delete in;
    return 0;
  }
#endif

  // codec parameters and number of cells
  HZNstream* stream = new HZNstream(hzREAD);
  stream->dims = in->get();
  in->get();
  unsigned codec = in->get();
  switch (codec) {
    case HZN_CODEC_BASE:
      stream->in = in;
      break;
#ifndef WITHOUT_ZLIB
    case HZN_CODEC_ZLIB:
      stream->in = new IBSTREAMzlib(in);
      break;
#endif
    default:
      delete stream;
      delete in;
      return 0;
  }
  stream->type = HZtype(in->get());
  stream->count = getuint(in);

  // check for errors
  if (stream->in->error()) {
    delete stream->in;
    delete stream;
    return 0;
  }

  // initialize decoder
  switch (stream->type) {
    case hzUCHAR:
      stream->cdecoder = new HZNdecoder<unsigned char>(stream->in, stream->count, stream->dims, perm, bias);
      break;
    case hzUSHORT:
      stream->sdecoder = new HZNdecoder<unsigned short>(stream->in, stream->count, stream->dims, perm, bias);
      break;
    case hzINT:
      stream->idecoder = new HZNdecoder<int>(stream->in, stream->count, stream->dims, perm, bias);
      break;
    case hzFLOAT:
      stream->fdecoder = new HZNdecoder<float>(stream->in, stream->count, stream->dims, perm, bias);
      break;
    case hzDOUBLE:
      stream->ddecoder = new HZNdecoder<double>(stream->in, stream->count, stream->dims, perm, bias);
      break;
  }

  return stream;
}

// read and decompress node data
template <typename T>
static unsigned
read(
  HZNdecoder<T>* decoder, // node decoder
  void*          node,    // node data array to read to
  const int*     mesh,    // array of node indices
  unsigned       count    // number of cells to visit
)
{
  unsigned nodes = decoder->nodes();
  for (unsigned i = 0; i < count; i++)
    mesh += decoder->decode((T*)node, mesh);
  return decoder->nodes() - nodes;
}

/*@p-u-b-l-i-c---f-u-n-c-t-i-o-n-s-------------------------------------------*/

// open stream for reading node-centered data from file
HZNstream*
hzip_node_open_file(
  FILE*    file, // compressed input file
  unsigned perm, // node permutation
  int      bias  // node index bias
)
{
  return read_header(new IBSTREAMfile(file), perm, bias);
}

// open stream for reading node-centered data from memory
HZNstream*
hzip_node_open_mem(
  const void* buffer, // pointer to compressed data
  size_t      size,   // byte size of compressed data
  unsigned    perm,   // node permutation
  int         bias    // node index bias
)
{
  return read_header(new IBSTREAMmem(buffer, size), perm, bias);
}

// read and decompress node-centered data from stream
int
hzip_node_read(
  HZNstream* stream, // input stream
  void*      node,   // node data array to read to
  const int* mesh,   // array of node indices
  unsigned   count   // number of cells to visit
)
{
  IBSTREAM* in = stream->in;
  int nodes = -1;

  switch (stream->type) {
    case hzUCHAR:
      nodes = read(stream->cdecoder, node, mesh, count);
      break;
    case hzUSHORT:
      nodes = read(stream->sdecoder, node, mesh, count);
      break;
    case hzINT:
      nodes = read(stream->idecoder, node, mesh, count);
      break;
    case hzFLOAT:
      nodes = read(stream->fdecoder, node, mesh, count);
      break;
    case hzDOUBLE:
      nodes = read(stream->ddecoder, node, mesh, count);
      break;
  }

  return in->error() ? in->error() : nodes;
}
