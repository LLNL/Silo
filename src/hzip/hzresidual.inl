// compute (possibly zero) residual from predicted and actual values
template <typename T, class M>
HZresidual<T, M>::HZresidual(T p, T a)
{
  U r = U(map(p) - map(a));
  U c = U(~r);
  value = U(c < r ? ~(2 * r) : 2 * r);
}

// read byte-aligned variable-length residual from stream
template <typename T, class M>
HZresidual<T, M>::HZresidual(IBSTREAM* stream)
{
  U r = 0;
  U c;
  unsigned i = 0;
  do {
    c = U(stream->get());
    r = U(r + (c << i));
    i += 7;
  } while (c & 0x80);
  value = r;
}

// write byte-aligned variable-length residual to stream
template <typename T, class M>
void
HZresidual<T, M>::put(OBSTREAM* stream) const
{
  U r = value;
  do {
    stream->put((r >> 7 ? 0x80 : 0x00) + (r & 0x7f));
    r >>= 7;
  } while (r--);
}

// reconstruct actual value from prediction and residual
template <typename T, class M>
T
HZresidual<T, M>::operator+(T p) const
{
  U r = value;
  U a = U(map(p) - ((r >> 1) ^ (0 - (r & 1))));
  return map(a);
}
