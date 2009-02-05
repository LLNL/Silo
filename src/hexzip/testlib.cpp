#include <cstdio>
#include <cstdlib>
#include <vector>
#include "hexcompressor.h"

#include <sys/time.h>

using namespace std;

int main(int argc, char* argv[])
{
  char* infile = 0;
  char* outfile = 0;
  unsigned bits = 12;
  switch (argc) {
    case 4:
      if (sscanf(argv[3], "%u", &bits) != 1)
        return EXIT_FAILURE;
    case 3:
      outfile = argv[2];
      /*FALLTHROUGH*/
    case 2:
      infile = argv[1];
      break;
    default:
      fprintf(stderr, "Usage: %s <infile> [outfile] [bits]\n", *argv);
      return EXIT_FAILURE;
  }
  HCCODECdefault codec = { 0, bits };

  // read uncompressed hex mesh
  fprintf(stderr, "reading\n");
  FILE* file = fopen(infile, "rb");
  if (!file)
    abort();
  vector<int> node;
  unsigned perm = 0;
  char line[0x10000];
  for (unsigned nv = 0; fgets(line, sizeof(line), file);) {
    float p[3];
    int v[8];
    if (sscanf(line, "v%f%f%f", p+0, p+1, p+2) == 3)
      nv++;
    else if (sscanf(line, "h%i%i%i%i%i%i%i%i", v+0, v+1, v+2, v+3, v+4, v+5, v+6, v+7) == 8) {
      for (unsigned i = 0; i < 8; i++)
        node.push_back(v[i] > 0 ? v[i] - 1 : v[i] + nv);
    }
    else if (strstr(line, "format GAMBIT"))
      perm = 001234567;
    else if (strstr(line, "format CUBIT"))
      perm = 001324576;
  }
  fclose(file);
  unsigned count = node.size() / 8;
  unsigned inbytes = 8 * count * sizeof(int);
  unsigned outbytes;
  int* data = &*node.begin();
  int* copy = new int[8 * count];

  if (outfile) {
    // compress to file
    fprintf(stderr, "compressing to file\n");
    file = fopen(outfile, "wb");
    if (!file)
      abort();
    outbytes = local_hex_file_write(file, data, count, perm, &codec);
    fclose(file);
    if (!outbytes)
      abort();
    fprintf(stderr, "in=%u out=%u ratio=%.2f bph=%.2f\n", inbytes, outbytes, (double)inbytes/outbytes, 8.0 * outbytes / count);

    // decompress from file
    fprintf(stderr, "decompressing from file\n");
    file = fopen(outfile, "rb");
    if (!file)
      abort();
    if (!local_hex_file_read(file, copy, count, perm))
      abort();
    fclose(file);

    // validate data
    fprintf(stderr, "validating\n");
    for (unsigned i = 0; i < 8 * count; i++)
      if (copy[i] != data[i]) {
        fprintf(stderr, "decompression failed\n");
        abort();
      }
  }

  // compress to memory
  fprintf(stderr, "compressing to memory\n");
  unsigned char* buffer = new unsigned char[inbytes];
  struct timeval tv;
  gettimeofday(&tv, 0);
  double t = tv.tv_sec + 1e-6 * tv.tv_usec;
  outbytes = local_hex_memory_write(buffer, inbytes, data, count, perm, &codec);
  if (!outbytes)
    abort();
  gettimeofday(&tv, 0);
  t = tv.tv_sec + 1e-6 * tv.tv_usec - t;
  fprintf(stderr, "in=%u out=%u ratio=%.2f bph=%.2f seconds=%.3f\n", inbytes, outbytes, (double)inbytes/outbytes, 8.0 * outbytes / count, t);

  // decompress from memory
  fprintf(stderr, "decompressing from memory\n");
  if (!local_hex_memory_read(buffer, copy, count, perm))
    abort();

  // validate data
  fprintf(stderr, "validating\n");
  for (unsigned i = 0; i < 8 * count; i++)
    if (copy[i] != data[i]) {
      fprintf(stderr, "decompression failed\n");
      abort();
    }

  fprintf(stderr, "OK\n");

  return 0;
}
