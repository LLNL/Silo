#ifndef FPZIP_ERROR_H
#define FPZIP_ERROR_H

#define FPZIP_BAD_MAGIC                 1
#define FPZIP_BAD_FORMAT_VERSION        2
#define FPZIP_BITS_TOO_LARGE            3
#define FPZIP_TARGETSCALE_TOO_LARGE     4
#define FPZIP_BAD_PRECISION             5

#ifdef __cplusplus
extern "C" {
#endif
extern int fpzip_errno;
#ifdef __cplusplus
}
#endif


#endif
