#include "hzip.h"
#include "hzmio.h"

/*@p-r-i-v-a-t-e---f-u-n-c-t-i-o-n-s-----------------------------------------*/

// close stream for reading and deallocate
static int
read_close(
  HZMstream* stream // input stream
)
{
  IBSTREAM* in = stream->in;
  int status = bytes(in);

  delete stream->decoder;
  delete stream;
  delete in;

  return status;
}

// close stream for writing and deallocate
static int
write_close(
  HZMstream* stream // output stream
)
{
  stream->encoder->end();
  OBSTREAM* out = stream->out;
  if (!stream->count)
    putuint(out->tail(), 0);
  out->close();
  int status = bytes(out);

  delete stream->encoder;
  delete stream;
  delete out;

  return status;
}

/*@p-u-b-l-i-c---f-u-n-c-t-i-o-n-s-------------------------------------------*/

// return mesh dimensionality
unsigned
hzip_mesh_dimensions(
  HZMstream* stream // I/O stream
)
{
  return stream->dims;
}

// return number of cells in stream
unsigned
hzip_mesh_cells(
  HZMstream* stream // I/O stream
)
{
  return stream->count;
}

// close stream and deallocate
int
hzip_mesh_close(
  HZMstream* stream // I/O stream
)
{
  return stream->access == hzREAD ? read_close(stream) : write_close(stream);
}
