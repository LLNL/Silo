template <unsigned width>
unsigned
PCmap<float, width, void>::forward(float d) const
{
  RANGE r = (RANGE&)d;
  r = ~r;
  r >>= shift;
  r ^= -(r >> (bits - 1)) >> (shift + 1);
  return r;
}

template <unsigned width>
float
PCmap<float, width, void>::inverse(unsigned r) const
{
  r ^= -(r >> (bits - 1)) >> (shift + 1);
  r = ~r;
  r <<= shift;
  return (DOMAIN&)r;
}

template <unsigned width>
float
PCmap<float, width, void>::identity(float d) const
{
  RANGE r = (RANGE&)d;
  r >>= shift;
  r <<= shift;
  return (DOMAIN&)r;
}

template <unsigned width>
unsigned long long
PCmap<double, width, void>::forward(double d) const
{
  RANGE r = (RANGE&)d;
  r = ~r;
  r >>= shift;
  r ^= -(r >> (bits - 1)) >> (shift + 1);
  return r;
}

template <unsigned width>
double
PCmap<double, width, void>::inverse(unsigned long long r) const
{
  r ^= -(r >> (bits - 1)) >> (shift + 1);
  r = ~r;
  r <<= shift;
  return (DOMAIN&)r;
}

template <unsigned width>
double
PCmap<double, width, void>::identity(double d) const
{
  RANGE r = (RANGE&)d;
  r >>= shift;
  r <<= shift;
  return (DOMAIN&)r;
}
