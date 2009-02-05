template <typename T>
void
HZNdecoder<T>::get()
{
  // read compressed data for eight nodes
  mask = stream->get();
  for (unsigned i = 0; i < CHAR_BIT; i++)
    if (mask & (1 << i))
      diff[i] = HZresidual<T>(stream);
}

template <typename T>
unsigned
HZNdecoder<T>::decode(
  T*         node,
  const int* cell
)
{
  // compute mask of already decoded nodes in cell
  unsigned v[HZ_CELLSIZE_MAX];
  unsigned m = prepare(v, cell);

  // encode all unencoded nodes in cell
  for (unsigned i = 0; i < cellsize; i++)
    if (!(m & (1 << i))) {
      unsigned j = count++ % CHAR_BIT;
      if (!j)
        get();
      // predict, record difference, and mark node as encoded
      T p = pred->predict(node, v, i, m);
      m += 1 << i;
      coded[v[i]] = true;
      node[v[i]] = p;
      if (mask & (1 << j))
        node[v[i]] += diff[j];
    }

  return cellsize;
}
