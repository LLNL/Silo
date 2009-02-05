#include <cstdio>
#include <cstdlib>
#include <vector>
#include "local.h"

#include <sys/time.h>

using namespace std;

int main(int argc, char* argv[])
{
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <infile> <outfile>\n", *argv);
    return EXIT_FAILURE;
  }
  char* infile = argv[1];
  char* outfile = argv[2];
  unsigned dim[4];
  
  FILE* file = fopen(infile, "rb");
  if (!file)
    abort();
  vector<double> vertex;
  vector<int> node;
  unsigned perm = 0;
  char line[0x10000];
  unsigned nv;
  for (nv = 0; fgets(line, sizeof(line), file);) {
    float p[3];
    int v[8];
    if (sscanf(line, "v%f%f%f", p+0, p+1, p+2) == 3) {
      for (unsigned i = 0; i < 3; i++)
        vertex.push_back(p[i]);
      nv++;
    }
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

  dim[0]=6; 
  dim[1]=21; 
  dim[2]=21; 
  dim[3]=3; 
  unsigned size = dim[0] * dim[1] * dim[2] * dim[3];

  unsigned outbytes;
  double* data = &*vertex.begin();

  file = fopen(outfile, "wb");
  if (!file)
    abort();
  if (fwrite(data, sizeof(*data), vertex.size(), file) != vertex.size())
    abort();
  fclose(file);

  return 0;
}
