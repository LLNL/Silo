#ifdef WIN32
#ifndef SILO_WIN32_COMPATIBILITY
#define SILO_WIN32_COMPATIBILITY
#include <io.h>  /* Include Windows IO */
#include <errno.h>
#include <sys\types.h>
#include <sys\stat.h>


#define access    _access
#define isatty    _isatty
#define pclose    _pclose
#define popen     _popen
#define read      _read
#define snprintf  _snprintf
#define stat      _stat
#define write     _write

#ifndef S_IWUSR
  #ifdef S_IWRITE
    #define S_IWUSR S_IWRITE
  #else
    #define S_IWUSR _S_IWRITE
  #endif
#endif
#ifndef S_ISREG
  #define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
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


/* It may require some experimentation to get these defines correct */
#define WTERMSIG(x)    ((x) & 0xff) 
#define WEXITSTATUS(x) (((x) >> 8) & 0xff)
#define WIFSIGNALED(x) (WTERMSIG(x) != 0) 
#define WIFEXITED(x)   (WTERMSIG(x) == 0)


#endif
#endif
