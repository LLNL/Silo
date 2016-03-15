#if defined(INIT_C_STRUCTSPACE) || !defined(ZFP_BITSTREAM_H)
#define ZFP_BITSTREAM_H

#include <stddef.h>
#include "zfp_types.h"

/* forward declaration of opaque type */
#ifndef INIT_C_STRUCTSPACE
typedef struct bitstream bitstream;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_C_STRUCTSPACE /* fake a C++-like namespace { */
#define C_STRUCTSPACE_STATIC static
#  ifdef INIT_C_STRUCTSPACE
#    define DEF_FUNC_BS(Ret, Func, Args) (Ret(*)Args) Func,
#    define DEF_VAR_BS(Typ, Var, Const)  Const,
     const struct zfpbs_structspace zfpbs = {
#  else
#    define DEF_FUNC_BS(Ret,Func,Args) Ret (*Func) Args;
#    define DEF_VAR_BS(Typ, Var, Const)  Typ Var;
     struct zfpbs_structspace {
#  endif
#else
#define C_STRUCTSPACE_STATIC
#define DEF_FUNC_BS(Ret,Func,Args) Ret Func Args;
#define DEF_VAR_BS(Typ, Var, Const)  Typ Var;
#endif

/* bit stream granularity */
DEF_VAR_BS(size_t, stream_word_bits, wsize)

/* allocate and initialize bit stream */
DEF_FUNC_BS(bitstream*, stream_open,(void* buffer, size_t bytes))

/* close and deallocate bit stream */
DEF_FUNC_BS(void, stream_close,(bitstream* stream))

/* pointer to beginning of stream */
DEF_FUNC_BS(void*, stream_data,(const bitstream* stream))

/* current byte size of stream (if flushed) */
DEF_FUNC_BS(size_t, stream_size,(const bitstream* stream))

/* byte capacity of stream */
DEF_FUNC_BS(size_t, stream_capacity,(const bitstream* stream))

/* number of blocks between consecutive blocks */
DEF_FUNC_BS(int, stream_delta,(const bitstream* stream))

/* read single bit (0 or 1) */
DEF_FUNC_BS(uint, stream_read_bit,(bitstream* stream))

/* write single bit */
DEF_FUNC_BS(uint, stream_write_bit,(bitstream* stream, uint bit))

/* read 0 <= n <= 64 bits */
DEF_FUNC_BS(uint64, stream_read_bits,(bitstream* stream, uint n))

/* write 0 <= n <= 64 low bits of value and return remaining bits */
DEF_FUNC_BS(uint64, stream_write_bits,(bitstream* stream, uint64 value, uint n))

/* return bit offset to next bit to be read */
DEF_FUNC_BS(size_t, stream_rtell,(const bitstream* stream))

/* return bit offset to next bit to be written */
DEF_FUNC_BS(size_t, stream_wtell,(const bitstream* stream))

/* rewind stream to beginning */
DEF_FUNC_BS(void, stream_rewind,(bitstream* stream))

/* position stream for reading at given bit offset */
DEF_FUNC_BS(void, stream_rseek,(bitstream* stream, size_t offset))

/* position stream for writing at given bit offset */
DEF_FUNC_BS(void, stream_wseek,(bitstream* stream, size_t offset))

/* skip over the next n bits */
DEF_FUNC_BS(void, stream_skip,(bitstream* stream, uint n))

/* append n zero bits to stream */
DEF_FUNC_BS(void, stream_pad,(bitstream* stream, uint n))

/* align stream on next word boundary */
DEF_FUNC_BS(void, stream_align,(bitstream* stream))

/* flush out any remaining buffered bits */
DEF_FUNC_BS(void, stream_flush,(bitstream* stream))

#ifdef BITSTREAM_STRIDED
/* set block size in number of words and spacing in number of blocks */
DEF_FUNC_BS(int, stream_set_stride,(bitstream* stream, uint block, int delta))
#endif

#ifdef USE_C_STRUCTSPACE /* } fake a C++-like namespace */
};
extern const struct zfpbs_structspace zfpbs;
#endif
#undef DEF_FUNC_BS
#undef DEF_VAR_BS


#ifdef __cplusplus
}
#endif

#endif
