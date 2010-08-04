/* config.h.  Generated from config.h.in by configure.  */
/* config/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to dummy `main' function (if any) required to link to the Fortran
   libraries. */
/* #undef FC_DUMMY_MAIN */

/* Define if F77 and FC dummy `main' functions are identical. */
/* #undef FC_DUMMY_MAIN_EQ_F77 */

/* Define to a macro mangling the given C identifier (in lower and upper
   case), which must not contain underscores, for linking with Fortran. */
#define FC_FUNC(name,NAME) NAME

/* As FC_FUNC, but for C identifiers containing underscores. */
/*#define SILO_FC_FUNC_(name,NAME) name ## _*/

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #define HAVE_DLFCN_H 1 */

/* System provides fclose prototypes */
#define HAVE_FCLOSE_POINTER 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* System provides fflush prototypes */
#define HAVE_FFLUSH_POINTER 1

/* Define to 1 if you have the `fnmatch' function. */
/* #define HAVE_FNMATCH 1 */

/* Define to 1 if you have the <fnmatch.h> header file. */
/* #define HAVE_FNMATCH_H 1 */

/* System provides fopen prototypes */
#define HAVE_FOPEN_POINTER 1

/* Define to 1 if you have the `fpclass' function. */
#define HAVE_FPCLASS 1

/* System provides fprintf prototypes */
#define HAVE_FPRINTF_POINTER 1

/* System provides fread prototypes */
#define HAVE_FREAD_POINTER 1

/* System provides fseek prototypes */
#define HAVE_FSEEK_POINTER 1

/* System provides ftell prototypes */
#define HAVE_FTELL_POINTER 1

/* System provides fwrite prototypes */
#define HAVE_FWRITE_POINTER 1


/* Define to 1 if you have the <history.h> header file. */
/* #undef HAVE_HISTORY_H */

/* Define to 1 if you have the <ieeefp.h> header file. */
/* #undef HAVE_IEEEFP_H */

/* Define to 1 if you have the <inttypes.h> header file. */
/* #define HAVE_INTTYPES_H 1 */

/* Define to 1 if you have the `isnan' function. */
#define HAVE_ISNAN 1

/* Define to 1 if you have the `hdf5' library (-lhdf5). */
#define HAVE_LIBHDF5 1

/* Define if you have a readline compatible library */
#define HAVE_LIBREADLINE 1

/* Define to 1 if you have the `sz' library (-lsz). */
#define HAVE_LIBSZ 1

/* Define to 1 if you have the `z' library (-lz). */
#define HAVE_LIBZ 1

/* Define to 1 if you have the `memmove' function. */
#ifndef HAVE_MEMMOVE
#define HAVE_MEMMOVE 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Support for NetCDF */
/* #define HAVE_NETCDF_DRIVER 1 */

/* Support for PDB */
#define HAVE_PDB_DRIVER 1

/* Define to 1 if you have the <readline.h> header file. */
/* #undef HAVE_READLINE_H */

/* Define if your readline library has \`add_history' */
/* #define HAVE_READLINE_HISTORY 1 */

/* Define to 1 if you have the <readline/history.h> header file. */
/* #define HAVE_READLINE_HISTORY_H 1 */

/* Define to 1 if you have the <readline/readline.h> header file. */
/* #define HAVE_READLINE_READLINE_H 1 */

/* System provides setvbuf prototypes */
#define HAVE_SETVBUF_POINTER 1

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stdint.h> header file. */
/* #define HAVE_STDINT_H 1 */

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
/* #define HAVE_STRINGS_H 1 */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/fcntl.h> header file. */
/* #define HAVE_SYS_FCNTL_H 1 */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
/* #define HAVE_SYS_TIME_H 1 */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Support for Taurus */
#define HAVE_TAURUS_DRIVER 1

/* Define to 1 if you have the <unistd.h> header file. */
/* #define HAVE_UNISTD_H 1 */

/* Define to 1 if you have the <zlib.h> header file. */
#define HAVE_ZLIB_H 1

#define SILO_LONG_LONG __int64

/* Name of package */
#define PACKAGE "silo"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "visit-help@llnl.gov"

/* Define to the full name of this package. */
#define PACKAGE_NAME "silo"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "silo"

/* include the file that has the version defines */
#include "siloversion.h"

/* The size of `off64_t', as computed by sizeof. */
#define SIZEOF_OFF64_T 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* Override longjmp */
/* #undef longjmp */

/* Override setjmp */
/* #undef setjmp */
