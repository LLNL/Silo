template <typename T>
void
HZNencoder<T>::put()
{
  // write compressed data for eight nodes
  stream->put(mask);
  for (unsigned i = 0; i < CHAR_BIT; i++)
    if (mask & (1 << i))
      diff[i].put(stream);
  mask = 0;
}

template <typename T>
unsigned
HZNencoder<T>::encode(
  const T*   node,
  const int* cell
)
{
  // compute mask of already encoded nodes in cell
  unsigned v[HZ_CELLSIZE_MAX];
  unsigned m = prepare(v, cell);

  // encode all unencoded nodes in cell
  for (unsigned i = 0; i < cellsize; i++)
    if (!(m & (1 << i))) {
      // predict, record difference, and mark node as encoded
      T p = pred->predict(node, v, i, m);
      m += 1 << i;
      coded[v[i]] = true;
      unsigned j = count++ % CHAR_BIT;
      diff[j] = HZresidual<T>(p, node[v[i]]);
      if (!diff[j].zero())
        mask += 1 << j;
      if (j == CHAR_BIT - 1)
        put();
    }

  return cellsize;
}

template <typename T>
void
HZNencoder<T>::end()
{
  if (count % CHAR_BIT)
    put();
}
