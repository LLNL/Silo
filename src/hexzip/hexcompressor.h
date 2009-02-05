#ifndef HEXCOMPRESSOR_H
#define HEXCOMPRESSOR_H

#ifdef __cplusplus
extern "C" {
#endif

/*@p-u-b-l-i-c---d-a-t-a-----------------------------------------------------*/

/*
** The default codec supports one parameter, "bits," that indirectly
** specifies how much memory to use both during compression and
** decompression for hash-based prediction.  The memory usage is
** (64K + 40 * 2^bits) bytes, or 224 KB using the default setting
** bits = 12.  Larger values of "bits" generally improve compression.
*/

static const struct HCCODECdefault {
  unsigned codec; /* codec identifier */
  unsigned bits;  /* number of hash table bits */
} local_hex_codec_default = { 0, 12 };

/*@p-u-b-l-i-c---f-u-n-c-t-i-o-n-s-------------------------------------------*/

/*
** The vertices within each hexahedron can be permuted by specifying
** eight octal digits "perm" that map the source ordering to the canonical
** ordering assumed by the compressor.  For example, the permutation
** 01327645 maps the Gray code ordering below to the canonical binary
** code ordering, i.e. the i'th octal digit j (with i = 0 being the
** leftmost digit) specifies for vertex #i in the canonical ordering the
** corresponding vertex #j in the source ordering.
**
**     3-------2       2-------3
**    /|      /|      /|      /|
**   4-------5 |     6-------7 |
**   | |     | | --> | |     | |
**   | 0-----|-1     | 0-----|-1
**   |/      |/      |/      |/
**   7-------6       4-------5
**
**   source data     canonical
**   ordering        ordering
**
** It is not necessary to permute vertices if the orderings do not match,
** but experimental evidence suggests that this often improves compression.
** Since the compressor stores the data in the canonical ordering, the
** same "perm" value must be passed to the decompressor if the original
** ordering is to be recovered.
*/

unsigned              /* number of hexahedra read */
local_hex_file_read(
  FILE*    file,      /* binary output stream */
  int*     data,      /* array of node indices to read */
  unsigned count,     /* number of hexahedra */
  unsigned perm       /* vertex permutation (or 0 for default ordering) */
);

unsigned              /* number of hexahedra read */
local_hex_memory_read(
  const void* buffer, /* pointer to compressed data */
  int*        data,   /* array of node indices to read */
  unsigned    count,  /* number of hexahedra */
  unsigned    perm    /* vertex permutation (or 0 for default ordering) */
);

unsigned              /* number of bytes written */
local_hex_file_write(
  FILE*       file,   /* binary output stream */
  const int*  data,   /* array of node indices to write */
  unsigned    count,  /* number of hexahedra */
  unsigned    perm,   /* vertex permutation (or 0 for default ordering) */
  const void* codec   /* compression codec (or NULL for default codec) */
);

unsigned              /* number of bytes written */
local_hex_memory_write(
  void*       buffer, /* pointer to compressed data */
  unsigned    size,   /* size of allocated storage */
  const int*  data,   /* array of node indices to write */
  unsigned    count,  /* number of hexahedra */
  unsigned    perm,   /* vertex permutation (or 0 for default ordering) */
  const void* codec   /* compression codec (or NULL for default codec) */
);

#ifdef __cplusplus
}
#endif

#endif
