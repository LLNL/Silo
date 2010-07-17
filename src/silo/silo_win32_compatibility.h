#ifdef WIN32
#ifndef SILO_WIN32_COMPATIBILITY
#define SILO_WIN32_COMPATIBILITY
#include <io.h>  /* Include Windows IO */
#include <errno.h>
#include <sys\types.h>
#include <sys\stat.h>

#define snprintf _snprintf

#define write  _write
#define read   _read

#define stat   _stat
#define access _access

#ifndef S_IWUSR
#ifdef S_IWRITE
#define S_IWUSR S_IWRITE
#else
#define S_IWUSR _S_IWRITE
#endif
#endif


/* Define modes for the call to _access if they are not already defined. */
#ifndef F_OK
#define F_OK   0
#endif

#ifndef W_OK
#define W_OK   2
#endif

#ifndef R_OK
#define R_OK   4
#endif

#endif
#endif
