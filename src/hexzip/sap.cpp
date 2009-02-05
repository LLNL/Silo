#include "sap.h"

SAPCODE::SAPCODE(unsigned m, unsigned n) : lsize(1 << m), csize(1 << n)
{
  // initialize code with 2^m highest and 2^n lowest strata entries
  sym = new unsigned short[csize];
  code = new unsigned short[csize];
  for (unsigned i = 0; i < csize; i++)
    sym[i] = code[i] = (unsigned short)i;
  list = new SAPCODEnode[lsize];
  for (unsigned i = 0; i < lsize; i++) {
    list[i].prev = (unsigned char)(i - 1);
    list[i].next = (unsigned char)(i + 1);
  }
  head = 0;
  tail = (unsigned char)(lsize - 1);
  list[head].prev = head;
  list[tail].next = tail;
}

SAPCODE::~SAPCODE()
{
  delete[] sym;
  delete[] code;
  delete[] list;
}

void
SAPCODE::update(unsigned short symbol, unsigned short codeword)
{
  unsigned char c;
  if (codeword < lsize) {
    // symbol is cached
    c = (unsigned char)codeword;
  }
  else {
    // not in cache; replace with least recently used symbol
    c = tail;
    code[symbol] = c;
    code[sym[c]] = codeword;
    sym[codeword] = sym[c];
    sym[c] = symbol;
  }
  if (head != c) {
    // move symbol to front of list
    unsigned char p = list[c].prev;
    unsigned char n = list[c].next;
    if (tail == c) {
      // remove from end of list
      list[p].next = p;
      tail = p;
    }
    else {
      // remove from middle of list
      list[p].next = n;
      list[n].prev = p;
    }
    // insert at front of list
    list[c].prev = c;
    list[c].next = head;
    list[head].prev = c;
    head = c;
  }
}

unsigned short
SAPCODE::encode(unsigned short symbol)
{
  unsigned short codeword = code[symbol];
  update(symbol, codeword);
  return codeword;
}

unsigned short
SAPCODE::decode(unsigned short codeword)
{
  unsigned short symbol = sym[codeword];
  update(symbol, codeword);
  return symbol;
}
