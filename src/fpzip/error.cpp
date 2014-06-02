#include "fpzip.h"

fpzipError fpzip_errno;

const char* fpzip_errstr[] = {
  "success",
  "cannot open file for reading",
  "cannot create file for writing",
  "cannot read stream",
  "cannot write stream",
  "not an fpz stream",
  "fpz format version not supported",
  "array dimensions do not match",
  "floating-point types do not match",
  "precision not supported",
  "buffer overflow",
};
