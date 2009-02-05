#include <cstdio>
#include <cstdlib>
#include "local.h"

using namespace std;

int main(int argc, char* argv[])
{
  if (argc != 7) {
    fprintf(stderr, "Usage: %s <infile> <outfile> <nx> <ny> <nz> <nf>\n", *argv);
    return EXIT_FAILURE;
  }
  char* infile = argv[1];
  char* outfile = argv[2];
  unsigned dim[4];
  for (unsigned i = 0; i < 4; i++)
    sscanf(argv[i + 3], "%u", &dim[i]);
  unsigned size = dim[0] * dim[1] * dim[2] * dim[3];
  unsigned inbytes = size * sizeof(double);
  int dp = 1;
  int* prec = new int[dim[3]];
  for (unsigned i = 0; i < dim[3]; i++)
    prec[i] = 64;

  // read raw data
  fprintf(stderr, "reading\n");
  FILE* file = fopen(infile, "rb");
  if (!file)
    abort();
  double* data = new double[size];
  if (fread(data, sizeof(*data), size, file) != size)
    abort();
  fclose(file);

  // compress to file
  fprintf(stderr, "compressing to file\n");
  file = fopen(outfile, "wb");
  if (!file)
    abort();
  unsigned outbytes = local_file_write(file, data, prec, dp, dim[0], dim[1], dim[2], dim[3]);
  fclose(file);
  if (!outbytes)
    abort();
  fprintf(stderr, "in=%u out=%u\n", inbytes, outbytes);

  // decompress from file
  fprintf(stderr, "decompressing from file\n");
  file = fopen(outfile, "rb");
  if (!file)
    abort();
  double* copy = new double[size];
  if (!local_file_read(file, copy, prec, dp, dim[0], dim[1], dim[2], dim[3]))
    abort();
  fclose(file);

  // validate data
  fprintf(stderr, "validating\n");
  for (unsigned i = 0; i < size; i++)
    if (data[i] != copy[i]) {
      fprintf(stderr, "decompression failed\n");
      abort();
    }

  // compress to memory
  fprintf(stderr, "compressing to memory\n");
  unsigned char* buffer = new unsigned char[size * sizeof(*data)];
  outbytes = local_memory_write(buffer, size * sizeof(*data), data, prec, dp, dim[0], dim[1], dim[2], dim[3]);
  if (!outbytes)
    abort();
  fprintf(stderr, "in=%u out=%u\n", inbytes, outbytes);

  // decompress from memory
  fprintf(stderr, "decompressing from memory\n");
  if (!local_memory_read(buffer, copy, prec, dp, dim[0], dim[1], dim[2], dim[3]))
    abort();

  // validate data
  fprintf(stderr, "validating\n");
  for (unsigned i = 0; i < size; i++)
    if (data[i] != copy[i]) {
      fprintf(stderr, "decompression failed\n");
      abort();
    }

  return 0;
}
