#include "inline/inline.h"
#include "zfp.h"
#include "zfp/macros.h"
#include "block1.h"
#include "traitsf.h"
#include "template/template.h"
#include "template/codec.h"
#include "inline/bitstream.c"
#include "template/codecf.c"
#include "template/codec1.c"
#include "template/decode.c"
#include "template/decodef.c"
#include "template/decode1.c"
#include "template/revcodecf.c"
#include "template/revdecode.c"
#include "template/revdecodef.c"
#include "template/revdecode1.c"
void zfp_init_decode1f() {
 zfpns.zfp_decode_block_float_1 = zfp_decode_block_float_1;
 zfpns.zfp_decode_block_strided_float_1 = zfp_decode_block_strided_float_1;
 zfpns.zfp_decode_partial_block_strided_float_1 = zfp_decode_partial_block_strided_float_1;
}

