#include "inline/inline.h"
#include "zfp.h"
#include "zfp/macros.h"
#include "block2.h"
#include "traitsl.h"
#include "template/template.h"
#include "template/codec.h"
#include "inline/bitstream.c"
#include "template/codec2.c"
#include "template/encode.c"
#include "template/encodei.c"
#include "template/encode2.c"
#include "template/revencode.c"
#include "template/revencode2.c"
void zfp_init_encode2l() {
 zfpns.zfp_encode_block_int64_2 = zfp_encode_block_int64_2;
 zfpns.zfp_encode_block_strided_int64_2 = zfp_encode_block_strided_int64_2;
 zfpns.zfp_encode_partial_block_strided_int64_2 = zfp_encode_partial_block_strided_int64_2;
}

