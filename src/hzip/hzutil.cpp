#include "hzip.h"

/*@p-r-i-v-a-t-e---f-u-n-c-t-i-o-n-s-----------------------------------------*/

// recursively construct mesh
static int*
mesh_construct(
  int*           mesh,     // array of node indices to construct
  unsigned       dims,     // topological mesh dimensionality (1-3)
  const unsigned count[],  // structured grid dimensions (node counts)
  unsigned       dim,      // current dimension
  const int      offset[], // node offset along each dimension
  int            index     // current node index
)
{
  if (dim--)
    for (unsigned i = 0; i < count[dim] - 1; i++, index += offset[dim])
      mesh = mesh_construct(mesh, dims, count, dim, offset, index);
  else {
    unsigned cellsize = 1 << dims;
    for (unsigned i = 0; i < cellsize; i++) {
      int k = index;
      for (unsigned j = 0; j < dims; j++)
        if (i & (1 << j))
          k += offset[j];
      *mesh++ = k;
    }
  }
  return mesh;
}

/*@p-u-b-l-i-c---f-u-n-c-t-i-o-n-s-------------------------------------------*/

// construct mesh connectivity for regular grid
unsigned
hzip_mesh_construct(
  int*           mesh,    // array of node indices to construct
  unsigned       dims,    // topological mesh dimensionality (1-3)
  const unsigned count[], // structured grid dimensions (node counts)
  int            bias     // node index bias
)
{
  int offset[HZ_DIMS_MAX] = { 1 };
  for (unsigned i = 1; i < dims; i++)
    offset[i] = count[i - 1] * offset[i - 1];
  mesh_construct(mesh, dims, count, dims, offset, bias);
  return 0;
}
