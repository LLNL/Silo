#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ioperf.h>

/*
 * Purpose: Implement ioperf's I/O interface using stdio functions
 */

static const char *filename;
static FILE *file; 

static int Open_stdio(ioflags_t iopflags)
{
    char *mode;

    mode = "a+";
    if ((iopflags&IO_WRITE) && !(iopflags&IO_READ)) mode="w";
    if (!(iopflags&IO_WRITE) && (iopflags&IO_READ)) mode="r";

    file = fopen(filename, mode);
    if (!file) return 0;
    return 1;
}

static int Write_stdio(void *buf, size_t nbytes)
{
    return fwrite(buf, 1, nbytes, file);
}

static int Read_stdio(void *buf, size_t nbytes)
{
    return fread(buf, 1, nbytes, file);
}

static int Close_stdio()
{
    int n = fclose(file);
    if (n == 0) return 1;
    return n;
}

iointerface_t *CreateInterface(int argc, char *argv[], const char *_filename)
{
    iointerface_t *retval;

    filename = strdup(_filename);

    retval = (iointerface_t*) calloc(sizeof(iointerface_t),1);
    retval->Open = Open_stdio;
    retval->Write = Write_stdio;
    retval->Read = Read_stdio;
    retval->Close = Close_stdio;

    return retval;
}
