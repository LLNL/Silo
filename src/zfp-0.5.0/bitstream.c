#include "zfp_bitstream.h"
#include "inline/bitstream.c"

const size_t zfpbs_stream_word_bits = wsize;

#ifdef USE_C_STRUCTSPACE
#define INIT_C_STRUCTSPACE
#include "zfp_bitstream.h"
#endif
