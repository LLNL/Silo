#include "hzip.h"
#include "hzmio.h"

/*@p-r-i-v-a-t-e---f-u-n-c-t-i-o-n-s-----------------------------------------*/

// write header data
static HZMstream*
write_header(
  OBSTREAM*   out,   // output stream
  unsigned    perm,  // node permutation
  unsigned    dims,  // mesh dimensionality
  unsigned    count, // total number of cells
  unsigned    codec, // codec number
  const void* param  // codec parameters
)
{
  HZMstream* stream = new HZMstream(hzWRITE);
  unsigned bits;

  switch (codec) {
    case HZM_CODEC_BASE: {
        const HZMCODECbase* p = param ? (const HZMCODECbase*)param : &hzm_codec_base;
        stream->out = out;
        bits = p->bits;
      }
      break;
#ifndef WITHOUT_ZLIB
    case HZM_CODEC_ZLIB: {
        const HZMCODECzlib* p = param ? (const HZMCODECzlib*)param : &hzm_codec_zlib;
        stream->out = new OBSTREAMzlib(out, p->zlib.level, p->zlib.insize, p->zlib.outsize);
        bits = p->bits;
      }
      break;
#endif
    default:
      delete stream;
      delete out;
      return 0;
  }
  stream->dims = dims;
  stream->count = count;

#ifndef HZ_WITHOUT_MAGIC
  // magic and format
  out->put('h');
  out->put('z');
  out->put('m');
  out->put(HZM_VERSION);
#endif

  // cell count and codec parameters
  out->put(dims);
  out->put(0);
  out->put(codec);
  out->put(bits);
  putuint(out, count);

  // check for errors
  if (stream->out->error()) {
    delete stream->out;
    delete stream;
    return 0;
  }

  // initialize encoder
  stream->encoder = new HZMencoder(stream->out, dims, perm, bits);

  return stream;
}

/*@p-u-b-l-i-c---f-u-n-c-t-i-o-n-s-------------------------------------------*/

// create stream for writing mesh to file
HZMstream*
hzip_mesh_create_file(
  FILE*       file,  // compressed output file
  unsigned    perm,  // node permutation
  unsigned    dims,  // mesh dimensionality
  unsigned    count, // total number of cells
  unsigned    codec, // codec number
  const void* param  // codec parameters
)
{
  return write_header(new OBSTREAMfile(file), perm, dims, count, codec, param);
}

// create stream for writing mesh to memory
HZMstream*
hzip_mesh_create_mem(
  void*       buffer, // pointer to compressed data
  size_t      size,   // byte size of allocated storage
  unsigned    perm,   // node permutation
  unsigned    dims,   // mesh dimensionality
  unsigned    count,  // total number of cells
  unsigned    codec,  // codec number
  const void* param   // codec parameters
)
{
  return write_header(new OBSTREAMmem(buffer, size), perm, dims, count, codec, param);
}

// compress and write mesh to stream
int
hzip_mesh_write(
  HZMstream* stream, // output stream
  const int* mesh,   // array of node indices to write
  unsigned   count   // number of cells to write
)
{
  OBSTREAM* out = stream->out;

  // if total cell count is not known, write size of this chunk
  if (!stream->count)
    putuint(out->tail(), count);

  // compress mesh connectivity
  for (unsigned i = 0; i < count; i++)
    mesh += stream->encoder->encode(mesh);

  // flush all buffers in case more chunks follow
  if (!stream->count)
    out->flush();

  return bytes(out);
}

// flush all buffers
int
hzip_mesh_flush(
  HZMstream* stream // output stream
)
{
  OBSTREAM* out = stream->out;
  if (stream->count)
    out->flush();
  return bytes(out);
}
