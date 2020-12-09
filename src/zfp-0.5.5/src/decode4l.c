#include "inline/inline.h"
#include "zfp.h"
#include "zfp/macros.h"
#include "block4.h"
#include "traitsl.h"
#include "template/template.h"
#include "template/codec.h"
#include "inline/bitstream.c"
#include "template/codec4.c"
#include "template/decode.c"
#include "template/decodei.c"
#include "template/decode4.c"
#include "template/revdecode.c"
#include "template/revdecode4.c"
void zfp_init_decode4l() {
 zfpns.zfp_decode_block_int64_4 = zfp_decode_block_int64_4;
 zfpns.zfp_decode_block_strided_int64_4 = zfp_decode_block_strided_int64_4;
 zfpns.zfp_decode_partial_block_strided_int64_4 = zfp_decode_partial_block_strided_int64_4;
}

