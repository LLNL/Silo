template <typename T>
T
HZNpredictor::predict(
  const T*        node,
  const unsigned* cell,
  unsigned        i,
  unsigned        mask
)
{
  // renumber nodes so that predicted node i is first in cell
  unsigned j = mask;
  j = ((j << (i & 1)) & 0xaa) + ((j >> (i & 1)) & 0x55);
  j = ((j << (i & 2)) & 0xcc) + ((j >> (i & 2)) & 0x33);
  j = ((j << (i & 4)) & 0xf0) + ((j >> (i & 4)) & 0x0f);
  j >>= 1;

  // predict node as weighted combination of its neighbors
  volatile T p = 0;
  for (unsigned k = 1; k < cellsize; k++)
    if (weight[j][k])
      p = T(p + weight[j][k] * node[cell[i ^ k]]);
  p = T(p / weight[j][0]);

  return p;
}
