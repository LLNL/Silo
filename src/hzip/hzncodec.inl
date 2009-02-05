template <typename T>
HZNcodec<T>::HZNcodec(unsigned nodes, unsigned dims, unsigned pcode, int bias) :
  cellsize(1 << dims),
  bias(bias),
  mask(0),
  count(0),
  coded(nodes, false),
  pred(new HZNpredictor(dims))
{
  if (!pcode)
    pcode = 0x76543210;
  for (unsigned i = 0; i < cellsize; i++, pcode >>= 4)
    perm[i] = pcode & 0xf;
}

template <typename T>
unsigned
HZNcodec<T>::prepare(unsigned* v, const int* cell) const
{
  unsigned m = 0;
  for (unsigned i = 0; i < cellsize; i++) {
    v[i] = index(cell, i);
    m += (coded[v[i]] ? 1 : 0) << i;
  }
  return m;
}
