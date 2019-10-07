#include "inline/inline.h"
#include "zfp.h"
#include "zfp/macros.h"
#include "block3.h"
#include "traitsi.h"
#include "template/template.h"
#include "template/codec.h"
#include "inline/bitstream.c"
#include "template/codec3.c"
#include "template/encode.c"
#include "template/encodei.c"
#include "template/encode3.c"
#include "template/revencode.c"
#include "template/revencode3.c"
void zfp_init_encode3i() {
 zfpns.zfp_encode_block_int32_3 = zfp_encode_block_int32_3;
 zfpns.zfp_encode_block_strided_int32_3 = zfp_encode_block_strided_int32_3;
 zfpns.zfp_encode_partial_block_strided_int32_3 = zfp_encode_partial_block_strided_int32_3;
}

