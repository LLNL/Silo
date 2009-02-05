/*
** hzip version 1.0.1, August 6, 2008
** Part of the LOCAL Toolkit, UCRL-CODE-232243
** Written by Peter Lindstrom, Lawrence Livermore National Laboratory
**
**
**                               SUMMARY
**
** hzip is a library for lossless compression of connectivity, geometry,
** and related data in structured and unstructured meshes composed of
** cells with hypercube topology, i.e. line segments in 1D, quadrilaterals
** in 2D, and hexahedra in 3D.  This space of data sets includes images
** and voxel grids.  hzip currently supports compression of node-centered
** floating-point and integer scalars.  Cell-centered data in structured
** grids can be handled by treating cells as nodes.  (Full support for
** cell-centered data as well as controlled lossy compression will be
** available in later releases.)  Though not optimized for, hzip supports
** mixed and hybrid meshes with a small number of secondary element types
** such as triangles and wedges through the use of degenerate cells.
** For best results, the nodes and cells as well as cell orientations
** should be organized coherently in some nonspecific but easy-to-predict
** pattern since the compressor must encode and fully preserve this
** information.  Please see the accompanying documentation and example
** code for additional details and usage.
**
** hzip was developed as part of the LOCAL LDRD project at LLNL, and may
** be freely used and distributed for noncommercial purposes.  The core
** library is written in C++ and applications need to be linked with a
** C++ linker.  The library can, however, be called from C.  For further
** information and bug reports, please e-mail pl@llnl.gov.
**
**
**                                NOTICE
**
** This work was produced at the Lawrence Livermore National Laboratory
** (LLNL) under contract no. DE-AC-52-07NA27344 (Contract 44) between
** the U.S. Department of Energy (DOE) and Lawrence Livermore National
** Security, LLC (LLNS) for the operation of LLNL.  The rights of the
** Federal Government are reserved under Contract 44.
**
**
**                              DISCLAIMER
**
** This work was prepared as an account of work sponsored by an agency of
** the United States government.  Neither the United States government nor
** Lawrence Livermore National Security, LLC, nor any of their employees
** makes any warranty, expressed or implied, or assumes any legal liability
** or responsibility for the accuracy, completeness, or usefulness of any
** information, apparatus, product, or process disclosed, or represents
** that its use would not infringe privately owned rights.  Reference
** herein to any specific commercial product, process, or service by trade
** name, trademark, manufacturer, or otherwise does not necessarily
** constitute or imply its endorsement, recommendation, or favoring by the
** United States government or Lawrence Livermore National Security, LLC.
** The views and opinions of authors expressed herein do not necessarily
** state or reflect those of the United States government or Lawrence
** Livermore National Security, LLC, and shall not be used for advertising
** or product endorsement purposes.
**
**
**                            COMMERCIAL USE
**
** Commercialization of this product is prohibited without notifying the
** Department of Energy (DOE) or Lawrence Livermore National Security.
*/

#ifndef HZIP_H
#define HZIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/*@p-u-b-l-i-c---m-a-c-r-o-s-------------------------------------------------*/

#define HZ_VER_MAJOR    1
#define HZ_VER_MINOR    0
#define HZ_VER_RELEASE  1
#define HZ_VER_STRING   "1.0.1"

#define HZ_DIMS_MAX     3                  /* max number of dimensions */
#define HZ_CELLSIZE_MAX (1 << HZ_DIMS_MAX) /* max number of nodes per cell */

#define HZM_CODEC_BASE  0 /* base mesh codec */
#define HZM_CODEC_ZLIB  1 /* zlib mesh codec */
#define HZN_CODEC_BASE  0 /* base node codec */
#define HZN_CODEC_ZLIB  1 /* zlib node codec */

/*@p-u-b-l-i-c---t-y-p-e-s---------------------------------------------------*/

typedef struct HZMstream HZMstream; /* mesh I/O stream */
typedef struct HZNstream HZNstream; /* node I/O stream */

typedef enum {
  hzUCHAR  = 1, /* unsigned char C type */
  hzUSHORT = 2, /* unsigned short C type */
  hzINT    = 3, /* int C type */
  hzFLOAT  = 4, /* float C type */
  hzDOUBLE = 5  /* double C type */
} HZtype;

/*@p-u-b-l-i-c---d-a-t-a-----------------------------------------------------*/

/*
** The baseline codec supports one parameter, "bits," that indirectly
** specifies how much memory to use both during compression and
** decompression for hash-based prediction.  The memory usage is
** 33 * 2^(dims + bits - 3) bytes, or 132 KB for hexahedral meshes with
** the default setting bits = 12.  Larger values of "bits" generally
** improve compression.
*/

static const struct HZMCODECbase {
  unsigned bits; /* number of hash table bits */
} hzm_codec_base = { 12 };

/*
** The zlib codec applies zlib compression on top of the baseline codec,
** which generally improves compression significantly.  The additional
** parameters "insize" and "outsize" specify the size of memory buffers
** used by the zlib compressor.  Larger buffers generally improve speed.
** The parameter "level" specifies the zlib compression level from -1 to 9:
** -1 for the default level; 0 for no compression; 1 for maximum speed;
** and 9 for maximum compression.  NOTE: The HZzlib parameters are shared
** among codecs, and should not directly be passed to the compressor.
** Instead, use the HZ*CODECzlib structures.
*/

struct HZzlib {   /* NOTE: not to be used in isolation */
  int    level;   /* zlib compression level */
  size_t insize;  /* byte size of zlib deflate input (uncompressed) buffer */
  size_t outsize; /* byte size of zlib deflate output (compressed) buffer */
};

/*
** zlib codec for mesh connectivity.
*/

static const struct HZMCODECzlib {
  unsigned      bits; /* number of hash table bits */
  struct HZzlib zlib; /* zlib parameters */
} hzm_codec_zlib = { 12, { -1, 0x6000, 0x2000 } };

/*
** zlib codec for node-centered data.
*/

static const struct HZNCODECzlib {
  struct HZzlib zlib; /* zlib parameters */
} hzn_codec_zlib = { { -1, 0x6000, 0x2000 } };

/*@p-u-b-l-i-c---f-u-n-c-t-i-o-n-s-------------------------------------------*/

/*
** The nodes within each cell can be permuted by specifying hexadecimal
** digits "perm" that map the source ordering to the canonical ordering
** assumed by the compressor.  For example, the permutation 0x54672310
** maps the Gray code ordering below to the canonical binary code ordering,
** i.e. the i'th hexadecimal digit j (with i = 0 being the rightmost digit)
** specifies for node #i in the canonical ordering the corresponding
** node #j in the source ordering.  In the canonical ordering, the i'th
** bit in the index specifies the "coordinate" (0 or 1) along the i'th
** dimension (thus the ordering for a quadrilateral follows the ordering
** for face {0, 1, 2, 3} in the hexahedron shown below).
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
** Although compression is still possible without permuting nodes, the
** prediction and compression of node-centered data relies heavily on
** having the nodes correctly ordered.  While less dependent on ordering,
** compression of mesh connectivity usually benefits as well from correct
** node ordering.  NOTE: Since the compressor stores the data in the
** canonical ordering, the same "perm" value must be passed to the
** decompressor if the original ordering is to be recovered.
*/

/*@mesh.connectivity.functions...............................................*/

HZMstream*                /* input stream */
hzip_mesh_open_file(
  FILE*       file,       /* compressed input file */
  unsigned    perm        /* node permutation (0 for default) */
);

HZMstream*                /* input stream */
hzip_mesh_open_mem(
  const void* buffer,     /* pointer to compressed data */
  size_t      size,       /* byte size of compressed data */
  unsigned    perm        /* node permutation (0 for default) */
);

unsigned                  /* mesh dimensionality */
hzip_mesh_dimensions(
  HZMstream*  stream      /* input stream */
);

unsigned                  /* number of cells in input (0 if unknown) */
hzip_mesh_cells(
  HZMstream*  stream      /* input stream */
);

int                       /* number of cells read from stream or error code */
hzip_mesh_read(
  HZMstream*  stream,     /* input stream */
  int*        mesh,       /* array of node indices to read */
  unsigned    count       /* number of cells to read */
);

HZMstream*                /* output stream */
hzip_mesh_create_file(
  FILE*       file,       /* compressed output file */
  unsigned    perm,       /* node permutation (0 for default) */
  unsigned    dims,       /* topological mesh dimensionality (1-3) */
  unsigned    count,      /* total number of cells */
  unsigned    codec,      /* codec number */
  const void* param       /* codec parameters (NULL for default) */
);

HZMstream*                /* output stream */
hzip_mesh_create_mem(
  void*       buffer,     /* pointer to compressed data */
  size_t      size,       /* byte size of allocated storage */
  unsigned    perm,       /* node permutation (0 for default) */
  unsigned    dims,       /* topological mesh dimensionality (1-3) */
  unsigned    count,      /* total number of cells */
  unsigned    codec,      /* codec number */
  const void* param       /* codec parameters (NULL for default) */
);

int                       /* number of bytes written so far or error code */
hzip_mesh_write(
  HZMstream*  stream,     /* output stream */
  const int*  mesh,       /* array of node indices to write */
  unsigned    count       /* number of cells to write */
);

int                       /* number of bytes written so far or error code */
hzip_mesh_flush(
  HZMstream*  stream      /* output stream */
);

int                       /* number of bytes read/written or error code */
hzip_mesh_close(
  HZMstream*  stream      /* I/O stream */
);

unsigned                  /* node permutation */
hzip_mesh_construct(
  int*           mesh,    /* array of node indices to construct */
  unsigned       dims,    /* topological mesh dimensionality (1-3) */
  const unsigned count[], /* structured grid dimensions (node counts) */
  int            bias     /* node index bias */
);

/*@node-centered.data.functions..............................................*/

HZNstream*                /* input stream */
hzip_node_open_file(
  FILE*       file,       /* compressed input file */
  unsigned    perm,       /* node permutation (0 for default) */
  int         bias        /* node index bias */
);

HZNstream*                /* input stream */
hzip_node_open_mem(
  const void* buffer,     /* pointer to compressed data */
  size_t      size,       /* byte size of compressed data */
  unsigned    perm,       /* node permutation (0 for default) */
  int         bias        /* node index bias */
);

int                       /* number of nodes read from stream or error code */
hzip_node_read(
  HZNstream*  stream,     /* input stream */
  void*       node,       /* node data array to read to */
  const int*  mesh,       /* array of node indices */
  unsigned    count       /* number of cells to visit */
);

unsigned                  /* mesh dimensionality */
hzip_node_dimensions(
  HZNstream*  stream      /* input stream */
);

unsigned                  /* number of nodes in input (0 if unknown) */
hzip_node_count(
  HZNstream*  stream      /* input stream */
);

HZtype                    /* node data type */
hzip_node_type(
  HZNstream*  stream      /* input stream */
);

HZNstream*                /* output stream */
hzip_node_create_file(
  FILE*       file,       /* compressed output file */
  unsigned    perm,       /* node permutation */
  int         bias,       /* node index bias */
  unsigned    dims,       /* topological mesh dimensionality */
  unsigned    count,      /* total number of nodes */
  HZtype      type,       /* node data type */
  unsigned    codec,      /* codec number */
  const void* param       /* codec parameters */
);

HZNstream*                /* output stream */
hzip_node_create_mem(
  void*       buffer,     /* pointer to compressed data */
  size_t      size,       /* byte size of allocated storage */
  unsigned    perm,       /* node permutation */
  int         bias,       /* node index bias */
  unsigned    dims,       /* mesh dimensionality */
  unsigned    count,      /* total number of nodes */
  HZtype      type,       /* node data type */
  unsigned    codec,      /* codec number */
  const void* param       /* codec parameters */
);

int
hzip_node_write(
  HZNstream*  stream,     /* output stream */
  const void* node,       /* node data array to write from */
  const int*  mesh,       /* array of node indices */
  unsigned    count       /* number of cells to visit */
);

int                       /* number of bytes written so far or error code */
hzip_node_flush(
  HZNstream*  stream      /* output stream */
);

int                       /* number of bytes read/written or error code */
hzip_node_close(
  HZNstream*  stream      /* I/O stream */
);

#ifdef __cplusplus
}
#endif

#endif
