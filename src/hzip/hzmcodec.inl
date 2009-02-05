HZMcodec::HZMcodec(unsigned dims, unsigned pcode, unsigned bits) :
  cellsize(1 << dims),
  mask(0),
  count(0),
  node(begin())
{
  for (unsigned i = 0; i < HZM_BUF_SIZE; i++)
    buffer[i] = 0;
  if (!pcode)
    pcode = 0x76543210;
  for (unsigned i = 0; i < cellsize; i++, pcode >>= 4) {
    pred[i] = new HZMpredictor(dims, bits);
    perm[i] = pcode & 0xf;
  }
}

HZMcodec::~HZMcodec()
{
  for (unsigned i = 0; i < cellsize; i++)
    delete pred[i];
}

void
HZMcodec::advance()
{
  // make shadow copy and reset pointer at end of buffer
  for (unsigned i = 0; i < cellsize; i++)
    copy(node++);
  if (node == end())
    node = begin();
}
