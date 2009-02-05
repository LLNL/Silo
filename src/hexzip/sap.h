#ifndef SAP_H
#define SAP_H

// stratified adaptive prefix code
class SAPCODE {
public:
  SAPCODE(unsigned m = 7, unsigned n = 14);
  ~SAPCODE();

  // return prefix code for symbol
  unsigned short encode(unsigned short symbol);

  // return symbol corresponding to prefix code
  unsigned short decode(unsigned short codeword);

private:
  void update(unsigned short symbol, unsigned short codeword);

  struct SAPCODEnode {
    unsigned char  prev; // previous in linked list
    unsigned char  next; // next in linked list
  };
  unsigned short* code;  // mapping from symbol to codeword
  unsigned short* sym;   // mapping from codeword to symbol
  SAPCODEnode*    list;  // linked list of highest level symbols
  unsigned char   head;  // head of linked list
  unsigned char   tail;  // tail of linked list
  const unsigned  lsize; // size of highest level table
  const unsigned  csize; // size of lowest level table
};

#endif
