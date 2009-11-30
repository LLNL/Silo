#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ioperf.h>

/*
 * Implement ioperf's I/O interface using section 2 functions
 */

static const char *filename;
static int fd;

static int Open_sec2(ioflags_t iopflags)
{
    int flags = 0x0;

    flags = O_CREAT|O_RDWR;
    if ((iopflags&IO_WRITE) && !(iopflags&IO_READ)) flags=O_WRONLY;
    if (!(iopflags&IO_WRITE) && (iopflags&IO_READ)) flags=O_RDONLY;
    if (iopflags&IO_TRUNCATE) flags|=O_TRUNC;
    if (iopflags&IO_APPEND) flags|=O_APPEND;

    fd = open(filename, flags, S_IRUSR|S_IWUSR); 

    return fd;
}

static int Write_sec2(void *buf, size_t nbytes)
{
    return write(fd, buf, nbytes);
}

static int Read_sec2(void *buf, size_t nbytes)
{
    return read(fd, buf, nbytes);
}

static int Close_sec2()
{
    int n = close(fd);
    if (n == 0) return 1;
    return n;
}

iointerface_t *CreateInterface(int argc, char *argv[], const char *_filename)
{
    iointerface_t *retval;

    filename = strdup(_filename);

    retval = (iointerface_t*) calloc(sizeof(iointerface_t),1);
    retval->Open = Open_sec2;
    retval->Write = Write_sec2;
    retval->Read = Read_sec2;
    retval->Close = Close_sec2;

    return retval;
}
