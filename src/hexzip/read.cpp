#include <cstdio>
#include "hexcompressor.h"
#include "read.h"

using namespace std;

static unsigned
getuint(
  HCdecoder* hd
)
{
  unsigned x = 0;
  for (unsigned i = 0; i < 4; i++)
    x += hd->getbyte() << (8 * i);
  return x;
}

static bool
read_header(
  HCdecoder* hd,
  unsigned&  count,
  unsigned&  bits
)
{
  // magic
  if (hd->getbyte() != 's' ||
      hd->getbyte() != 'h' ||
      hd->getbyte() != 'c' ||
      hd->getbyte() != '\0')
    return false;

  // format version and codec
  if (hd->getbyte() != SHC_VERSION ||
      hd->getbyte() != 0)
    return false;

  // number of hexahedra and codec parameters
  unsigned cells = getuint(hd);
  if (!count)
    count = cells;
  else if (count != cells)
    return false;
  bits = hd->getbyte();

  return true;
}

static unsigned
read_hex_stream(
  HCdecoder* hd,   // hex decoder
  int*       data, // array of node indices to read
  unsigned   count // number of hexahedra
)
{
  unsigned bits;
  if (!read_header(hd, count, bits))
    return 0;
  VTXpredictor* pred[8];
  for (unsigned i = 0; i < 8; i++)
    pred[i] = new VTXpredictor(bits);
  hd->init(pred);
  for (unsigned i = 0; i < count; i++, data += 8) {
    unsigned v[8];
    hd->decode(v);
    for (unsigned j = 0; j < 8; j++)
      data[hd->perm[j]] = v[j];
  }
  for (unsigned i = 0; i < 8; i++)
    delete pred[i];
  return count;
}

// decompress hex mesh connectivity from file
unsigned
local_hex_file_read(
  FILE*     file,  // binary output stream
  int*      data,  // array of node indices to read
  unsigned  count, // number of hexahedra
  unsigned  perm   // vertex permutation
)
{
  HCfiledecoder* hd = new HCfiledecoder(file, perm);
  count = read_hex_stream(hd, data, count);
  if (hd->error)
    count = 0;
  delete hd;
  return count;
}

// decompress hex mesh connectivity from memory
unsigned
local_hex_memory_read(
  const void* buffer, // pointer to compressed data
  int*        data,   // array of node indices to read
  unsigned    count,  // number of hexahedra
  unsigned    perm    // vertex permutation
)
{
  HCmemdecoder* hd = new HCmemdecoder(buffer, perm);
  count = read_hex_stream(hd, data, count);
  if (hd->error)
    count = 0;
  delete hd;
  return count;
}
