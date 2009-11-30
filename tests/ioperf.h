typedef enum _ioflags_t
{
    IO_WRITE    = 0x00000001,
    IO_READ     = 0x00000002,
    IO_APPEND   = 0x00000004,
    IO_TRUNCATE = 0x00000008
} ioflags_t;

typedef enum _ioop_t
{
    OP_WRITE,
    OP_READ,
    OP_OPEN,
    OP_CLOSE,
    OP_SEEK,
    OP_ERROR,
    OP_OUTPUT_TIMINGS,
    OP_OUTPUT_SUMMARY
} ioop_t;

typedef int (*OpenFunc)(ioflags_t flags);
typedef int (*WriteFunc)(void *buf, size_t nbytes);
typedef int (*ReadFunc)(void *buf, size_t nbytes);
typedef int (*CloseFunc)(void);
typedef int (*SeekFunc)(size_t offset);
typedef double (*TimeFunc)(void);

typedef struct _iointerface_t
{
    TimeFunc Time; /* A default is provided but can be overridden by plugin */
    OpenFunc Open;
    WriteFunc Write;
    ReadFunc Read;
    CloseFunc Close;
    SeekFunc Seek;
    void *dlhandle;
} iointerface_t;

typedef iointerface_t* (*CreateInterfaceFunc)(int argc, char *argv[], const char *filename);
