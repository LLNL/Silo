#include "inline/inline.h"
#include "zfp.h"
#include "zfp/macros.h"
#include "block1.h"
#include "traitsd.h"
#include "template/template.h"
#include "template/codec.h"
#include "inline/bitstream.c"
#include "template/codecf.c"
#include "template/codec1.c"
#include "template/encode.c"
#include "template/encodef.c"
#include "template/encode1.c"
#include "template/revcodecf.c"
#include "template/revencode.c"
#include "template/revencodef.c"
#include "template/revencode1.c"
void zfp_init_encode1d() {
 zfpns.zfp_encode_block_double_1 = zfp_encode_block_double_1;
 zfpns.zfp_encode_block_strided_double_1 = zfp_encode_block_strided_double_1;
 zfpns.zfp_encode_partial_block_strided_double_1 = zfp_encode_partial_block_strided_double_1;
}

