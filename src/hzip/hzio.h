#ifndef HZIO_H
#define HZIO_H

#include <climits>
#include "ibstream.h"
#include "obstream.h"

/*@p-u-b-l-i-c---m-a-c-r-o-s-------------------------------------------------*/

#define UCHAR_MASK ((1 << CHAR_BIT) - 1)

/*@p-u-b-l-i-c---t-y-p-e-s---------------------------------------------------*/

// stream access type
enum HZaccess {
  hzREAD  = 0,
  hzWRITE = 1
};

#include "hzio.inl"

#endif
