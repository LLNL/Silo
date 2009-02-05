#include <cstdio>
#include "hexcompressor.h"
#include "write.h"

using namespace std;

static bool
initialize(
  const void* codec,
  unsigned&   bits
)
{
  // only one codec supported
  if (*(const unsigned*)codec != 0)
    return false;
  const HCCODECdefault* cp = (const HCCODECdefault*)codec;
  bits = cp->bits;
  return true;
}

static void
putuint(
  HCencoder* he,
  unsigned   x
)
{
  for (unsigned i = 0; i < 4; i++, x >>= 8)
    he->putbyte((unsigned char)(x & 0xff));
}

static void
write_header(
  HCencoder* he,
  unsigned   count,
  unsigned   bits
)
{
  // magic
  he->putbyte('s');
  he->putbyte('h');
  he->putbyte('c');
  he->putbyte('\0');

  // format and codec
  he->putbyte(SHC_VERSION);
  he->putbyte(0);

  // number of hexahedra and codec parameters
  putuint(he, count);
  he->putbyte((unsigned char)bits);
}

static bool
write_hex_stream(
  HCencoder*  he,    // hex encoder
  const int*  data,  // array of node indices to write
  unsigned    count, // number of hexahedra
  const void* codec  // compression codec
)
{
  unsigned bits;
  if (!initialize(codec ? codec : &local_hex_codec_default, bits))
    return false;
  write_header(he, count, bits);
  VTXpredictor* pred[8];
  for (unsigned i = 0; i < 8; i++)
    pred[i] = new VTXpredictor(bits);
  he->init(pred);
  for (unsigned i = 0; i < count; i++, data += 8) {
    unsigned v[8];
    for (unsigned j = 0; j < 8; j++)
      v[j] = data[he->perm[j]];
    he->encode(v);
  }
  he->finish();
  for (unsigned i = 0; i < 8; i++)
    delete pred[i];
  return true;
}

// compress hex mesh connectivity to file
unsigned
local_hex_file_write(
  FILE*       file,  // binary output stream
  const int*  data,  // array of node indices to write
  unsigned    count, // number of hexahedra
  unsigned    perm,  // vertex permutation
  const void* codec  // compression codec
)
{
  unsigned bytes = 0;
  HCfileencoder* he = new HCfileencoder(file, perm);
  if (write_hex_stream(he, data, count, codec) && !he->error)
    bytes = he->bytes();
  delete he;
  return bytes;
}

// compress hex mesh connectivity to memory
unsigned
local_hex_memory_write(
  void*       buffer, // pointer to compressed data
  unsigned    size,   // size of allocated storage */
  const int*  data,   // array of node indices to write
  unsigned    count,  // number of hexahedra
  unsigned    perm,   // vertex permutation
  const void* codec   // compression codec
)
{
  unsigned bytes = 0;
  HCmemencoder* he = new HCmemencoder(buffer, size, perm);
  if (write_hex_stream(he, data, count, codec) && !he->error)
    bytes = he->bytes();
  delete he;
  return bytes;
}
