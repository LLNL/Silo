#include "inline/inline.h"
#include "zfp.h"
#include "zfp/macros.h"
#include "block2.h"
#include "traitsl.h"
#include "template/template.h"
#include "template/codec.h"
#include "inline/bitstream.c"
#include "template/codec2.c"
#include "template/decode.c"
#include "template/decodei.c"
#include "template/decode2.c"
#include "template/revdecode.c"
#include "template/revdecode2.c"
void zfp_init_decode2l() {
 zfpns.zfp_decode_block_int64_2 = zfp_decode_block_int64_2;
 zfpns.zfp_decode_block_strided_int64_2 = zfp_decode_block_strided_int64_2;
 zfpns.zfp_decode_partial_block_strided_int64_2 = zfp_decode_partial_block_strided_int64_2;
}

