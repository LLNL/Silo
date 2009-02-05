#ifndef SWAT_H
#define SWAT_H

/*

                           Copyright 1991 - 2002
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/

#include <stdio.h>
#include <stdlib.h>

#ifdef THINK_C
#include <string.h>
#include <console.h>
#endif

/*
 * <float.h> deficiencies on the SUN.
 */
#ifdef __STDC__
#include <float.h>
#else
#ifndef FLT_MAX
#define FLT_MAX 3.40282347E+38
#endif
#endif

/* Generic function return values */
#define  OOPS       -1
#define  OKAY        0

#ifdef   MAXNAME
#undef   MAXNAME
#endif
#define  MAXNAME  256

#ifdef   MAXLINE
#undef   MAXLINE
#endif
#define  MAXLINE  1024

/* Datatypes */
#define  SWAT_INT               16
#define  SWAT_SHORT             17
#define  SWAT_LONG              18
#define  SWAT_FLOAT             19
#define  SWAT_DOUBLE            20
#define  SWAT_CHAR              21
#define  SWAT_UNKNOWN           25

/* Macros used for exporting symbols on Win32 systems. */
#ifndef SILO_API
#ifdef _WIN32
/* Make Silo a DLL by default. */
#ifdef SILO_STATIC_LIBRARY
#define SILO_API
#else
#ifdef SILO_EXPORTS
#define SILO_API __declspec(dllexport)
#else
#define SILO_API __declspec(dllimport)
#endif
#endif
#else
#define SILO_API
#endif
#endif

/* Useful macros */
#define  WHITESPACE(c)  (((c) == ' ') || ((c) == '\t'))
#define  STR_EQUAL(a,b) ((int)strcmp(a,b) == 0)
#define  STR_EMPTY(s)   ((strlen(s) == 0) || (strcspn(s," \t") == 0))
#define  DIGIT(c)       ((c) >= '0' && (c) <= '9')
#define  STR_BEGINSWITH(s,p)  ((strstr(s,p) == s) ? 1 : 0)
#define  STR_HASCHAR(s,c)  ((strchr(s,c) == NULL) ? 0 : 1)
#define  STR_LASTCHAR(s)   (s[strlen(s)-1])

#define  INDEX(col,row,ncol) (((row) * (ncol)) + col)  /* Zero - origin ! */
#define  INDEX3(i,j,k,stride) (i)*stride[0] + (j)*stride[1] + (k)*stride[2]

/*
 *  Standard memory management procedures.
 */
#define  ALLOC(x)         (x *) malloc(sizeof(x))
#define  ALLOC_N(x,n)     (x *) calloc (n, sizeof (x))
#define  REALLOC(p,x,n)   (x *) realloc (p, (n)*sizeof(x))
#define  REALLOC_N(p,x,n) (x *) realloc (p, (n)*sizeof(x))
#define  FREE(x)          if ( (x) != NULL) {free(x);(x)=NULL;}

/*
 *  SCORE-dependent memory management procedures.
 */
#define  SCALLOC(x)     ((x *) lite_SC_alloc(1L, (long) sizeof(x),NULL))
#define  SCALLOC_N(x,n) ((x *) lite_SC_alloc((long) n, (long) sizeof(x),NULL))
#define  SCREALLOC(p,x) (p=(x*)lite_SC_realloc((byte*)p,1L,(long)sizeof(x)))
#define  SCREALLOC_N(p,x,n)     \
                (p=(x*)lite_SC_realloc((byte*)p,(long)(n),(long)sizeof(x)))
#define  SCFREE(x)      if((x) != NULL) lite_SC_free((byte *)x)

#ifndef  TRUE
#define  TRUE  1
#endif

#ifndef  FALSE
#define  FALSE 0
#endif

#ifdef   MMAKE
#undef   MMAKE
#endif
#define  MMAKE(x)        (x *) malloc(sizeof(x))

#ifdef   MMAKE_N
#undef   MMAKE_N
#endif
#define  MMAKE_N(x, n)   (x *) calloc (n, sizeof (x))

#define  ASSIGN(a,b,what)    (a)->what = (b)->what
#define  DEG_TO_RAD(deg)     (deg) * 0.017453292

#ifdef   DEREF
#undef   DEREF
#endif
#define  DEREF(type,x)       (* ((type *) (x)))

#ifdef   PI
#undef   PI
#endif
#define  PI             3.141592654

#ifdef   MAX
#undef   MAX
#endif
#define  MAX(a, b)   ( (a) > (b) ? (a) : (b) )

#ifdef   MIN
#undef   MIN
#endif
#define  MIN(a, b)   ( (a) < (b) ? (a) : (b) )

#ifdef   ABS
#undef   ABS
#endif
#define  ABS(a)         (((a) > 0) ? (a) : -(a))

#ifdef DECLARE
#undef DECLARE
#endif
#ifdef ANSI
#define DECLARE(x,y) x y
#else
#define DECLARE(x,y) x()
#endif

#define  MAX_ARRAY(a,len)    a[max_index(a,len)]
#define  MIN_ARRAY(a,len)    a[min_index(a,len)]

#define  MEM_SAVE(type,p,n)  (type *) memsave((char *) p, n * sizeof(type))

#ifndef byte
#define byte void
#ifdef MIPS
#undef byte
#define byte  char
#endif
#ifdef IBMRISC
#undef byte
#define byte char
#endif
#endif /* ifndef byte */

/*--------------------------------------------------
 * Useful typedefs
 *-------------------------------------------------*/

typedef struct {
    char         **options;
    byte         **values;
    int            noptions;
    int            list_len;
} Optlist;

typedef struct {
    float          extmin;
    float          extmax;
} Extent;


/* Function prototypes */
SILO_API void alias_init (void);
SILO_API void alias_purge (void);
SILO_API int alias_set  (char *, char *);
SILO_API int alias_get  (char *, char *);
SILO_API int alias_undo (char *);
SILO_API int alias_exists (char *);
SILO_API int alias_show (char *);
SILO_API int al_index   (char *);

SILO_API void CenterArray (float *, int *, int *, int);
SILO_API int MoveArray  (float *, int *, float *, int *, int *);
SILO_API float         *ExpandArray1to2 (int, float *, int, int);
SILO_API int           *ReduceIArray (int *, int, int *, int *, int *, int *, int *, int *, int *);
SILO_API float         *ReduceArray (float *, int, int *, int *, int *, int *, int *, int *, int *);
SILO_API int SubsetMinMax2 (float *, int, float *, float *, int, int, int, int, int);
SILO_API int tab_lookup (double, float *, int);
SILO_API int xy_to_kj   (double, float *, int, int);
SILO_API void UpdateStride (int *, int *, int *, int *, int);
SILO_API float         *d2f (double *, int);
SILO_API double        *f2d (float *, int);
SILO_API float         *TransposeArray2 (float *, int *, int);
SILO_API int ReduceVector (float *, int *, int, double);
SILO_API int ReduceIVector (int *, int *, int, int);
SILO_API int SW_prtarray (void *, int, int);

SILO_API double randf (double rmin, double rmax);

SILO_API char          *SW_GetDatatypeString (int);
SILO_API int SW_GetDatatypeID (char *);
SILO_API int SW_GetMachineDataSize (int);

SILO_API void ReportError (char *);
SILO_API void SetErrorReporting (int);
SILO_API void fatality  (int);

SILO_API char **FileList (char *, char *, int *);
SILO_API void copy_var (char *, char *, int);
SILO_API void *memsave (char *, int);
SILO_API void myfree (int *);
SILO_API int  search (int *, int, int);
SILO_API int  SW_file_readable (char *);
SILO_API int  SW_file_exists (char *);

SILO_API void hist_init (int);
SILO_API void hist_purge (void);
SILO_API int  hist_add (char *, int);
SILO_API int  hist_get_last (char *);
SILO_API int  hist_get_nth (char *, int);
SILO_API int  hist_get_matching (char *, char *);
SILO_API int  hist_expand (char *, char *);
SILO_API void hist_show (int);

SILO_API int  GetNextInt (void);
SILO_API int  min_index (float *, int);
SILO_API int  max_index (float *, int);
SILO_API void aminmax (float *, int, int, int, float *, float *);
SILO_API int  arrminmax (float *, int, float *, float *);
SILO_API int  iarrminmax (int *, int, int *, int *);
SILO_API int  darrminmax (double *, int, double *, double *);
SILO_API void sw_printarrminmax (float *, int);

SILO_API Optlist *MakeOptlist (int);
SILO_API int     SetOption (Optlist *, char *, void *);
SILO_API int     DelOption (Optlist *, char *);
SILO_API int     FreeOptlist (Optlist *);

SILO_API int inarow (void *, int, int);

SILO_API char *safe_strdup(const char *s);
SILO_API int  safe_strlen (char *);
SILO_API int  strsuf     (char *, char *);
SILO_API char *blank_fill (char *, int);
SILO_API char *str_f2c (char *);
SILO_API void sort_list (char **, int);
SILO_API char *SW_strndup (char *, int);
SILO_API char *SC_strndup (char *, int);
SILO_API char *SW_dirname (char *);
SILO_API char *SW_basename (char *);

SILO_API void strprint  (FILE *, char **, int, int, int, int, int);
SILO_API int  SWPrintData (FILE *, char *, int *, int, int, int, int, int);

SILO_API int sw_getarg(void *arg,int *argc,char ***argv,char *optstring);
SILO_API int sw_getarg_noremove(void *arg,int *argc,char ***argv,char *optstring);

#endif /* SWAT_H */
