#!/bin/sh

# Adjust Config
sed -i .orig \
    -e 's/^# DEFS += -DBIT_STREAM_WORD_TYPE=uint8/DEFS += -DBIT_STREAM_WORD_TYPE=uint8/'\
    -e 's/^BUILD_UTILITIES = \(.*\)/BUILD_UTILITIES = 0/'\
    -e 's/BUILD_TESTING = \(.*\)/BUILD_TESTING = 0/' Config

#
# For each file in templates, add 'static' in front of _t2 that are missing them
#
tfiles=$(ls src/template/*.c)
for tf in $tfiles; do
    cp $tf $tf.orig
done
for tf in $tfiles; do
    cat $tf | tr '\n' '@' | sed -e 's/@\([a-z0-9]*\)@_t2(/@static \1@_t2(/g' | tr '@' '\n' > $tf.tmp
    mv $tf.tmp $tf
done

#
# For each file in templates, add zfpns. to _t2(zfp_encode_* and _t2(zfp_decode* calls
#
tfiles=$(ls src/template/*compress.c)
for tf in $tfiles; do
    cat $tf | sed -e 's/\(^ *\)_t2(zfp_\(..\)code_\(.*\),/\1_t2(zfpns.zfp_\2code_\3,/' > $tf.tmp
    mv $tf.tmp $tf
done

# For each file in src
#    1) Add partial initialization of namespace struct
sfiles=$(ls src/*.c)
for sf in $sfiles; do
    cp $sf $sf.orig
    fsig_lines=$(gcc -E -I./src -I./include $sf | grep '^zfp_.*_block' | cut -d'(' -f1)
    if [[ -z "$fsig_lines" ]]; then
        continue
    fi
    append="extern struct _zfp_namespace_struct zfpns;"
    append="$append\nstruct _zfp_namespace_struct zfpns = {"
    first=1
    for line in $fsig_lines; do
        if [[ $first -ne 1 ]]; then
            append="$append,"
        fi
        first=0
        append="$append\n    .$line = $line"
    done
    append="$append\n};\n"
    echo $append >> $sf
done

#
# Adjust zfp.c for all static methods
#
cp src/zfp.c src/zfp.c.orig
cat src/zfp.c | tr '\n' '@' | sed -e 's/@@\([a-z0-9_\*]*\)@zfp_\([a-z0-9_]*\)(\([a-z0-9_, \*]*\))@{@/@@static \1 zfp_\2( \3 )@{@/g' | tr '@' '\n' > src/zfp.c.tmp
mv src/zfp.c.tmp src/zfp.c
# Adjust all ZFP function call instances with zfpns. prepending them
cat src/zfp.c | sed '/^static .* zfp_.*(/!s/zfp_\([a-z0-9_]*\)(/zfpns.zfp_\1(/' > src/zfp.c.tmp
mv src/zfp.c.tmp src/zfp.c
# Gather ZFP constant definitions and remove from top
const_list=$(cat src/zfp.c | grep '^export_ const' | sed 's/export_ const \(.*\) zfp_\(.*\) = \(.*\);/    .zfp_\2 = \3,@/')
cat src/zfp.c | grep -v '^export_ const' > src/zfp.c.tmp
mv src/zfp.c.tmp src/zfp.c
# Adjust refs to zfp_codec_version
cat src/zfp.c | sed 's/zfp_codec_version/zfpns.zfp_codec_version/g' > src/zfp.c.tmp
mv src/zfp.c.tmp src/zfp.c
# Define and populate the zfpns struct with constants and functions defined here
echo "@struct _zfp_namespace_struct zfpns = {@" | tr '@' '\n' >> src/zfp.c
echo $const_list | tr '@' '\n' >> src/zfp.c
func_list=$(cat src/zfp.c | grep '^static \([a-z0-9_\*]*\) zfp_\([a-z0-9_]*\)(' | cut -d'(' -f1 | cut -d' ' -f3)
last_func=$(echo $func_list | rev | cut -d' ' -f1 | rev)
for f in $func_list; do
    if [[ $f = $last_func ]]; then
        break
    fi
    echo "    .$f = $f," >> src/zfp.c
done
echo "    .$last_func = $last_func" >> src/zfp.c
echo "@};@" | tr '@' '\n' >> src/zfp.c

# fix up bitstream funcs in zfp.c
sed -e 's/stream_word_bits/bsns.stream_word_bits/g' -e 's/ stream_flush(/ bsns.stream_flush(/g' -e 's/ stream_align/ bsns.stream_align/g' -e 's/ stream_rewind/ bsns.stream_rewind/g' -e 's/ stream_write_bits/ bsns.stream_write_bits/g' -e 's/\([( ]\)stream_read_bits/\1bsns.stream_read_bits/g' -e 's/ stream_size/ bsns.stream_size/g' src/zfp.c > src/zfp.c.tmp
mv src/zfp.c.tmp src/zfp.c

#
# Adjust zfp.h to create namspace struct
#
cat include/zfp.h | tr '\n' '@' | sed -e 's/@zfp_field_alloc();@/@zfp_field_alloc(@);@/' | sed -e 's/@\([a-zA-Z0-9_\*]*\)\([^@]*\)@zfp_\([^@]*\)(@/@\1@(*zfp_\3)(@/g' -e 's/@\([uintvoid]\{4\}\) zfp_\([^@]*\)(/@\1 (\*zfp_\2)(/g' -e 's/@#ifdef __cplusplus@extern "C" {@#endif@/@#ifdef __cplusplus@extern "C" {@#endif@@typedef struct _zfp_namespace_struct {@#undef extern_@#define extern_ @/' -e 's/@#ifdef __cplusplus@}@#endif@/@} zfp_namespace_struct_t;@@extern zfp_namespace_struct_t zfpns;@@#ifdef __cplusplus@}@#endif@@/' | tr '@' '\n' > include/zfp.h.tmp
mv include/zfp.h include/zfp.h.orig
mv include/zfp.h.tmp include/zfp.h

#
# Adjust bitstream.h
#
cp include/bitstream.h include/bitstream.h.orig
sed -e 's/^\(.*\) stream_\(.*\)(\(.*\));/\1 (\*stream_\2)(\3);/' -e '/^extern_ const/d' include/bitstream.h | tr '\n' '@' | sed -e 's/@#ifndef inline_@/@#if 1@/' -e 's/@extern "C" {@#endif@/@extern "C" {@#endif@typedef struct _bs_namespace_struct {@@const size_t stream_word_bits;@/' -e 's/@#ifdef __cplusplus@}@/@} bs_namespace_struct_t;@extern bs_namespace_struct_t bsns;@@#ifdef _cplusplus@}@/' | tr '@' '\n' > include/bitstream.h.tmp
mv include/bitstream.h.tmp include/bitstream.h
# Fix src/inline/bitstream.c
cat src/inline/bitstream.c | tr '\n' '@' | sed -e 's/@#ifndef inline_@/@#undef inline_@#define inline_ static@#ifndef inline_@/' | tr '@' '\n' > src/inline/bitstream.c.tmp
mv src/inline/bitstream.c.tmp src/inline/bitstream.c
echo "@struct _bs_namespace_struct bsns = {@" | tr '@' '\n' >> src/inline/bitstream.c
func_list=$(grep ^stream_ src/inline/bitstream.c |grep -v '_word(' | grep -v stream_set_stride | cut -d'(' -f1)
for f in $func_list; do
    echo "    .$f = $f," >> src/inline/bitstream.c
done
echo "    .stream_word_bits = wsize" >> src/inline/bitstream.c
echo "@};@" | tr '@' '\n' >> src/inline/bitstream.c
# Fix src/bitstream.c
sed -i .orig '/export_ const size_t stream_word_bits/d' src/bitstream.c
