#include "inline/inline.h"
#include "zfp.h"
#include "zfp/macros.h"
#include "block3.h"
#include "traitsf.h"
#include "template/template.h"
#include "template/codec.h"
#include "inline/bitstream.c"
#include "template/codecf.c"
#include "template/codec3.c"
#include "template/decode.c"
#include "template/decodef.c"
#include "template/decode3.c"
#include "template/revcodecf.c"
#include "template/revdecode.c"
#include "template/revdecodef.c"
#include "template/revdecode3.c"
void zfp_init_decode3f() {
 zfpns.zfp_decode_block_float_3 = zfp_decode_block_float_3;
 zfpns.zfp_decode_block_strided_float_3 = zfp_decode_block_strided_float_3;
 zfpns.zfp_decode_partial_block_strided_float_3 = zfp_decode_partial_block_strided_float_3;
}

