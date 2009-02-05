inline int
bytes(
  IBSTREAM* in // input stream
)
{
  return in->error() ? in->error() : (int)in->head()->bytes();
}

inline int
bytes(
  OBSTREAM* out // output stream
)
{
  return out->error() ? out->error() : (int)out->tail()->bytes();
}

inline unsigned
getuint(
  IBSTREAM* in // input stream
)
{
  unsigned x = 0;
  for (unsigned i = 0; i < sizeof(x); i++)
    x += (unsigned)in->get() << (CHAR_BIT * i);
  return x;
}

inline void
putuint(
  OBSTREAM* out, // output stream
  unsigned  x    // value to output
)
{
  for (unsigned i = 0; i < sizeof(x); i++, x >>= CHAR_BIT)
    out->put(x & UCHAR_MASK);
}
