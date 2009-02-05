#include "hzip.h"
#include "hzmio.h"

/*@p-r-i-v-a-t-e---f-u-n-c-t-i-o-n-s-----------------------------------------*/

// read header data
static HZMstream*
read_header(
  IBSTREAM* in,  // input stream
  unsigned  perm // node permutation
)
{
#ifndef HZ_WITHOUT_MAGIC
  // magic and format
  if (in->get() != 'h' ||
      in->get() != 'z' ||
      in->get() != 'm' ||
      in->get() != HZM_VERSION) {
    delete in;
    return 0;
  }
#endif

  // codec parameters and number of cells
  HZMstream* stream = new HZMstream(hzREAD);
  stream->dims = in->get();
  in->get();
  unsigned codec = in->get();
  switch (codec) {
    case HZM_CODEC_BASE:
      stream->in = in;
      break;
#ifndef WITHOUT_ZLIB
    case HZM_CODEC_ZLIB:
      stream->in = new IBSTREAMzlib(in);
      break;
#endif
    default:
      delete stream;
      delete in;
      return 0;
  }
  unsigned bits = in->get();
  stream->count = getuint(in);
  stream->avail = stream->count ? stream->count : getuint(in);

  // check for errors
  if (stream->in->error()) {
    delete stream->in;
    delete stream;
    return 0;
  }

  // initialize decoder
  stream->decoder = new HZMdecoder(stream->in, stream->dims, perm, bits);

  return stream;
}

/*@p-u-b-l-i-c---f-u-n-c-t-i-o-n-s-------------------------------------------*/

// open stream for reading mesh from file
HZMstream*
hzip_mesh_open_file(
  FILE*    file, // compressed input file
  unsigned perm  // node permutation
)
{
  return read_header(new IBSTREAMfile(file), perm);
}

// open stream for reading mesh from memory
HZMstream*
hzip_mesh_open_mem(
  const void* buffer, // pointer to compressed data
  size_t      size,   // byte size of compressed data
  unsigned    perm    // node permutation
)
{
  return read_header(new IBSTREAMmem(buffer, size), perm);
}

// read and decompress mesh from stream
int
hzip_mesh_read(
  HZMstream* stream, // input stream
  int*       mesh,   // array of node indices to read
  unsigned   count   // number of cells to read
)
{
  IBSTREAM* in = stream->in;

  // decompress mesh connectivity
  unsigned left = count;
  for (unsigned read; left && stream->avail; left -= read) {
    read = left < stream->avail ? left : stream->avail;
    for (unsigned i = 0; i < read; i++)
      mesh += stream->decoder->decode(mesh);
    if (stream->count)
      stream->avail -= read;
    else
      stream->avail = getuint(in->head());
  }

  return in->error() ? in->error() : (int)(count - left);
}
