#include "inline/inline.h"
#include "zfp.h"
#include "zfp/macros.h"
#include "block4.h"
#include "traitsd.h"
#include "template/template.h"
#include "template/codec.h"
#include "inline/bitstream.c"
#include "template/codecf.c"
#include "template/codec4.c"
#include "template/decode.c"
#include "template/decodef.c"
#include "template/decode4.c"
#include "template/revcodecf.c"
#include "template/revdecode.c"
#include "template/revdecodef.c"
#include "template/revdecode4.c"
void zfp_init_decode4d() {
 zfpns.zfp_decode_block_double_4 = zfp_decode_block_double_4;
 zfpns.zfp_decode_block_strided_double_4 = zfp_decode_block_strided_double_4;
 zfpns.zfp_decode_partial_block_strided_double_4 = zfp_decode_partial_block_strided_double_4;
}

