/*
Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
LLNL-CODE-425250.
All rights reserved.

This file is part of Silo. For details, see silo.llnl.gov.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the disclaimer below.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the disclaimer (as noted
     below) in the documentation and/or other materials provided with
     the distribution.
   * Neither the name of the LLNS/LLNL nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
"AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This work was produced at Lawrence Livermore National Laboratory under
Contract No.  DE-AC52-07NA27344 with the DOE.

Neither the  United States Government nor  Lawrence Livermore National
Security, LLC nor any of  their employees, makes any warranty, express
or  implied,  or  assumes  any  liability or  responsibility  for  the
accuracy, completeness,  or usefulness of  any information, apparatus,
product, or  process disclosed, or  represents that its use  would not
infringe privately-owned rights.

Any reference herein to  any specific commercial products, process, or
services by trade name,  trademark, manufacturer or otherwise does not
necessarily  constitute or imply  its endorsement,  recommendation, or
favoring  by  the  United  States  Government  or  Lawrence  Livermore
National Security,  LLC. The views  and opinions of  authors expressed
herein do not necessarily state  or reflect those of the United States
Government or Lawrence Livermore National Security, LLC, and shall not
be used for advertising or product endorsement purposes.
*/
/*-------------------------------------------------------------------------
 *
 * Created:             browser.h
 *                      Dec  4 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             Browser header file
 *
 * Modifications:
 *
 *      Robb Matzke, 10 Feb 1997
 *      The Makefile can override INIT_FILE, HISTORY_FILE,
 *      and HISTORY_STIFLE by defining any of those constants on the
 *      compiler command-line.
 *
 *      Robb Matzke, 5 Mar 1997
 *      Changed the name of the browser startup file to `.browser_rc'.
 *
 *      Robb Matzke, 11 Jun 1997
 *      Changed the documentation URL to something that probably won't
 *      be found (but won't be found quickly) until we find a permanent
 *      home for the documentation.  The documentation URL in this file
 *      is the initial value for the `doc_url' variable which can be
 *      (re)set from initialization files without recompiling.
 *
 *      Jeremy Meredith, Fri Nov 19 09:47:37 PST 1999
 *      Added code to replace strdup calls with safe_strdup calls.
 *
 *      Sean Ahern, Wed May 10 14:22:50 PDT 2000
 *      Removed tabs.
 *
 *      Robb Matzke, 2000-06-02
 *      Incremented browser major version number because names of internal
 *      variables changed, making older init files no longer work.
 *
 *      Eric Brugger, Mon Aug  7 13:49:15 PDT 2000
 *      I changed the patch number for release 4.1.
 *
 *      Eric Brugger, Fri Sep  1 17:35:18 PDT 2000
 *      I changed the patch number for release 4.1.1.
 *
 *      Eric Brugger, Thu Oct 19 14:38:00 PDT 2000
 *      I changed the patch number for release 4.1.2.
 *
 *      Eric Brugger, Fri Apr  6 11:43:43 PDT 2001
 *      I changed the patch number for release 4.2.
 *
 *      Eric Brugger, Fri Nov  1 10:47:02 PDT 2001
 *      I changed the patch number for release 4.2.1.
 *
 *      Eric Brugger, Mon Mar 11 15:03:10 PST 2002
 *      I changed the patch number for release 4.3.
 *
 *      Eric Brugger, Mon May  6 11:14:32 PDT 2002
 *      I changed the patch number for release 4.3.1.
 *
 *      Eric Brugger, Tue Aug 27 08:58:40 PDT 2002
 *      I changed the patch number for release 4.3.2.
 *
 *      Eric Brugger, Fri Sep 12 10:52:23 PDT 2003
 *      I changed the patch number for release 4.4.
 *
 *      Eric Brugger, Tue Sep 28 11:50:10 PDT 2004
 *      I changed the patch number for release 4.4.1.
 *
 *      Mark C. Miller, Tue Feb 15 11:11:22 PST 2005
 *      I changed the patch number for release 4.4.2
 *
 *      Mark C. Miller, Tue Mar 10 15:41:50 PDT 2009
 *      Removed VERSION symbols. No need for browser to maintain
 *      version information apart from the Silo library it is linked with.
 *
 *      Kathleen Bonnell, Thu Dec 9 09:28:57 PST 2010
 *      Added declaration of diferentll, couple of Win32-specifics.
 *-------------------------------------------------------------------------
 */
#ifndef _BROWSER_H
#define _BROWSER_H

#include <silo.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#  include <silo_win32_compatibility.h>
#endif

#define false           0
#define true            1
#define CAUGHT_SIGINT   999999
#define SHALLOW         0
#define DEEP            1
#define NIL             ((obj_t)0)
#define NDIMS           10
#define NELMTS(X)       (sizeof(X)/sizeof(*(X)))

#ifndef _WIN32
char *safe_strdup(const char *);
#endif
#undef strdup
#define strdup(s) safe_strdup(s)

/*
 * The Makefile can override these constants.
 */
#ifndef INIT_FILE
#  define INIT_FILE     ".browser_rc"           /*relative to $HOME*/
#endif
#ifndef HISTORY_FILE
#  define HISTORY_FILE  ".browser_history"      /*relative to $HOME*/
#endif
#ifndef HISTORY_STIFLE
#  define HISTORY_STIFLE 500
#endif

/* Command-line switches */
typedef struct switch_t switch_t;
typedef struct switches_t switches_t;
typedef int (*switch_handler_t)(switch_t*, const char*, const char*);

struct switches_t {
    size_t      nused;          /*number of switches defined    */
    size_t      nalloc;         /*number of slots allocated     */
    switch_t    *sw;            /*array of switches             */
};

struct switch_t {
    char        *short_name;    /*single letter switch          */
    char        *long_name;     /*full name of switch           */
    char        *arg_spec;      /*argument specification        */
    char        *doc_string;    /*documentation string          */
    void        *info;          /*extra info to pass through    */
    switches_t  *all;           /*containing switch list        */
    switch_handler_t handler;   /*callback function             */
    int         seen;           /*number of times seen          */
    const char  *lexeme;        /*lexeme of latest occurrence   */
    union {
        int     d;
        double  g;
        const char *s;
    } value;                    /*value of latest occurrence    */
};

/*
 * These are often already defined.
 */
#ifndef MAX
#  define MAX(X,Y)      ((X)>(Y)?(X):(Y))
#endif
#ifndef MIN
#  define MIN(X,Y)      ((X)<(Y)?(X):(Y))
#endif
                
typedef struct obj_t *obj_t;
typedef struct class_t *class_t;

/*
 * The walk functions take an extra argument to hold state information.
 */
typedef struct walk_t {

#define WALK_PRINT      0                       /*print typed memory    */
   struct out_t         *f;                     /*output file           */

#define WALK_RETRIEVE   1                       /*retrieve integer vals */
   int                  nvals;                  /*number retrieved vals */
   int                  maxvals;                /*max vals to retreive  */
   int                  *vals;                  /*retrieved values      */

#define WALK_DIFF       2
   obj_t                a_sdo;                  /*SDO for a_mem         */
   obj_t                b_sdo;                  /*SDO for b_mem         */

} walk_t;

/*
 * I/O association tables map integer values to string constants and
 * vice versa.
 */
#define NASSOCS         32

typedef struct prim_assoc_t {
   int          n ;                     /*integer part                  */
   char         *s ;                    /*string part                   */
} prim_assoc_t;

/*
 * Class methods and variables.
 */
struct class_t {
   char                 *name ;                 /*class name            */
   obj_t                (*new)(va_list);        /*constructor           */
   obj_t                (*dest)(obj_t);         /*destructor            */
   obj_t                (*copy)(obj_t,int);     /*copy constructor      */
   void                 (*print)(obj_t,struct out_t*);  /*print object  */
   obj_t                (*eval)(obj_t);         /*evaluate an object    */
   obj_t                (*feval)(obj_t);        /*evaluate to a function*/
   obj_t                (*apply)(obj_t,obj_t);  /*apply func to args    */
   char                 *(*objname)(obj_t);     /*get object name       */
   void                 (*walk1)(obj_t,void*,int,walk_t*); /*traversal  */
   int                  (*walk2)(obj_t,void*,obj_t,void*,walk_t*);/*diff*/
   int                  (*walk3)(void*,obj_t,obj_t);/*assignements      */
   int                  (*size_of)(obj_t);      /*number of bytes in obj*/
   obj_t                (*deref)(obj_t,int,obj_t*); /*dereference an obj*/
   obj_t                (*bind)(obj_t,void*);   /*binds array dimensions*/
   int                  (*diff)(obj_t,obj_t);   /*object differencer    */
};


/*
 * Public object methods and variables...
 */
typedef struct obj_pub_t {
   class_t              cls ;                   /*my class              */
   int                  ref ;                   /*reference count       */
} obj_pub_t;

struct obj_t {
   obj_pub_t            pub;
   /*private stuff here*/
};

/*
 * Types of tokens returned by the lexical analysis functions.
 */
#define TOK_EOF         EOF             /*end of file                   */
#define TOK_EOL         256             /*end of line                   */
#define TOK_INVALID     257             /*invalid character on input    */
#define TOK_SYM         258             /*symbol                        */
#define TOK_NUM         259             /*integer or floating pt const  */
#define TOK_RT          '>'             /*file redirection              */
#define TOK_RTRT        260             /*file appending                */
#define TOK_PIPE        '|'             /*redirection to a command      */
#define TOK_STR         261             /*string constant               */
#define TOK_DOT         '.'             /*dot                           */
#define TOK_LTPAREN     '('
#define TOK_RTPAREN     ')'
#define TOK_COLON       ':'
#define TOK_COMMA       ','
#define TOK_EQ          '='
#define TOK_LTCURLY     '{'
#define TOK_RTCURLY     '}'

typedef struct lex_t {                  /*lexer input stream            */
    FILE                *f;             /*input file                    */
    char                *s;             /*input string                  */
    int                 at;             /*offset into input string      */
    int                 tok;            /*current token                 */
    char                lexeme[8192];   /*current lexeme                */
    char                *prompt;        /*prompt string                 */
    struct lex_t        *stack[32];     /*input stack                   */
    int                 nstack;         /*items on input stack          */
} lex_t;

#define OUT_NFIELDS     20              /*max output fields             */

/* Pager flags */
typedef enum {
    PAGER_OKAY=0,                       /*normal (default) pager mode   */
    PAGER_INTERRUPT,                    /*got a SIGINT                  */
    PAGER_PIPE,                         /*got a SIGPIPE                 */
    PAGER_NEXT_SECTION,                 /*turn off until next section   */
    PAGER_NEXT_CMD                      /*turn off pager until next cmd */
} pflags_t;

#define PAGER_ACTIVE(F) ((F)->paged && PAGER_OKAY==(F)->pflags)

typedef struct out_t {                  /*output stream                 */
    FILE                *f;             /*output file                   */
    int                 row, col;       /*current position              */
    int                 paged;          /*should output be paged?       */
    pflags_t            pflags;         /*how should output be paged?   */
    int                 rtmargin;       /*columns for right margin      */
    int                 indent;         /*indentation level             */
    int                 literal;        /*output literally              */
    int                 nfields;        /*number of prefix fields       */
    char                *header;        /*table header line             */
    struct {
        char            *name ;         /*field name                    */
        int             silent;         /*this field is not printed     */
        int             elmtno ;        /*current array element number  */
        int             ndims ;         /*array dimensionality or zero  */
        int             offset[NDIMS] ; /*origin for printing           */
        int             dim[NDIMS] ;    /*dimension sizes               */
    } field[OUT_NFIELDS];
} out_t;

/*
 * function attributes
 */
#define HOLDFIRST       0x0001          /*don't eval first arg          */
#define HOLDREST        0x0002          /*don't eval remaining args     */
#define HOLD    (HOLDFIRST|HOLDREST)    /*don't eval any arguments      */
#define IMP_STRING      0x0004          /*next tok is an implied string */

/* List of strings */
typedef struct strlist_t {
    int         nused;
    char        *value[500];
} strlist_t;

/* Differencing options. */
typedef enum diff_rep_t {
    DIFF_REP_ALL        = 0,            /*report all differences        */
    DIFF_REP_BRIEF      = 1,            /*report all; don't print data  */
    DIFF_REP_SUMMARY    = 2             /*only report summary           */
} diff_rep_t;

typedef struct diffopt_t {
    diff_rep_t          report;         /*what to report                */
    int                 ignore_adds;    /*ignore things in B only       */
    int                 ignore_dels;    /*ignore things in A only       */
    int                 two_column;     /*use pdbdiff output style?     */
    double              c_abs, c_rel, c_eps; /*int8 tolerances          */
    double              s_abs, s_rel, s_eps; /*short tolerances         */
    double              i_abs, i_rel, i_eps; /*integer tolerances       */
    double              l_abs, l_rel, l_eps; /*long tolerances          */
    double              f_abs, f_rel, f_eps; /*float tolerances         */
    double              d_abs, d_rel, d_eps; /*double tolerances        */
    double              ll_abs, ll_rel, ll_eps; /*long long tolerances  */
    strlist_t           exclude;        /*objects to exclude            */
} diffopt_t;

#define DIFF_NOTAPP     "N/A"           /*not applicable                */
#define DIFF_SEPARATOR  " "             /*for two-column output         */

/*
 * SILO has a hard-to-use table of contents datatype, so we make our
 * own here.  Also, the DBObjectType constants cannot be used to index
 * into an array, so we define our own here.
 */
#define BROWSER_DB_CURVE           0
#define BROWSER_DB_MULTIMESH       1
#define BROWSER_DB_MULTIVAR        2
#define BROWSER_DB_MULTIMAT        3
#define BROWSER_DB_MULTIMATSPECIES 4
#define BROWSER_DB_QMESH           5
#define BROWSER_DB_QVAR            6
#define BROWSER_DB_UCDMESH         7
#define BROWSER_DB_UCDVAR          8
#define BROWSER_DB_PTMESH          9
#define BROWSER_DB_PTVAR           10
#define BROWSER_DB_MAT             11
#define BROWSER_DB_MATSPECIES      12
#define BROWSER_DB_VAR             13
#define BROWSER_DB_OBJ             14
#define BROWSER_DB_DIR             15
#define BROWSER_DB_ARRAY           16
#define BROWSER_DB_DEFVARS         17
#define BROWSER_DB_CSGMESH         18
#define BROWSER_DB_CSGVAR          19
#define BROWSER_DB_MULTIMESHADJ    20
#define BROWSER_DB_MRGTREE         21
#define BROWSER_DB_GROUPELMAP      22
#define BROWSER_DB_MRGVAR          23
#define BROWSER_NOBJTYPES          24   /*must be last                  */

typedef struct toc_t {
   char                 *name;          /*object name                   */
   int                  type;           /*a BROWSER_DB_* constant       */
} toc_t;

typedef struct DBdirectory {
   int                  nsyms;          /*number of symbols             */
   toc_t                *toc;           /*each entry                    */
   toc_t                **entry_ptr;    /*see `stc.c'                   */
} DBdirectory;

/*
 * The strtok(3) function isn't reentrant.  We have our own that is.
 * The third argument is this struct which holds all the state info.
 */
typedef struct strtok_t {
   char                 *stop;          /*end of previous token         */
   char                 save;           /*old character at `stop'       */
} strtok_t;

/* Table of contents for help */
typedef struct helptoc_t {
    char                *name;
    char                *desc;
} helptoc_t;

extern helptoc_t        HelpFuncToc[25];
extern int              NHelpFuncToc;
extern helptoc_t        HelpVarToc[50];
extern int              NHelpVarToc;
extern helptoc_t        HelpOpToc[25];
extern int              NHelpOpToc;

/*
 * Classes...
 */
extern class_t  C_ARY  ;                /*array data type               */
extern class_t  C_BIF  ;                /*built in function             */
extern class_t  C_CONS ;                /*LISP-like cons cells          */
extern class_t  C_FILE ;                /*SILO File                     */
extern class_t  C_NUM  ;                /*numbers                       */
extern class_t  C_PRIM ;                /*a primitive type              */
extern class_t  C_PTR  ;                /*a pointer type                */
extern class_t  C_RANGE ;               /*integer range object          */
extern class_t  C_SDO  ;                /*a silo data object            */
extern class_t  C_STC  ;                /*structure data type           */
extern class_t  C_STR  ;                /*strings                       */
extern class_t  C_SYM  ;                /*symbols                       */

/*** class initializers ***/
class_t ary_class   (void);
class_t bif_class   (void);
class_t cons_class  (void);
class_t file_class  (void);
class_t num_class   (void);
class_t prim_class  (void);
class_t ptr_class   (void);
class_t range_class (void);
class_t sdo_class   (void);
class_t stc_class   (void);
class_t str_class   (void);
class_t sym_class   (void);

/*** array.c ***/
extern int AryNProcessed;
obj_t ary_deref_nocopy (obj_t, int, obj_t*);
obj_t ary_typeof (obj_t);
void ary_footnotes_reset (void);
void ary_footnotes_print (void);

/*** bif.c ***/
int bif_lex_special (obj_t);

/*** browser.c ***/
extern int Verbosity;
extern char HistoryFile[];
extern char *ObjTypeName[BROWSER_NOBJTYPES];
void usage(void);
int different (double, double, double, double, double);
int differentll (long long, long long, double, double, double);
toc_t *browser_DBGetToc (DBfile*, int*, int(*)(toc_t*,toc_t*));
int sort_toc_by_name (toc_t*, toc_t*);
int sort_toc_by_type (toc_t*, toc_t*);

/*** cons.c ***/
obj_t cons_head (obj_t);
obj_t cons_tail (obj_t);

/*** file.c ***/
DBfile *file_file (obj_t);
int file_rdonly (obj_t);

/*** func.c ***/
extern diffopt_t DiffOpt;
obj_t V_array (int, obj_t[]);
obj_t V_assign (int, obj_t[]);
obj_t V_close (int, obj_t[]);
obj_t F_cons (obj_t, obj_t);
obj_t V_diff (int, obj_t[]);
obj_t V_dot (int, obj_t[]);
obj_t V_exit (int, obj_t[]);
void F_fbind (obj_t, obj_t);
obj_t V_file (int, obj_t[]);
obj_t F_flatten (obj_t);
obj_t F_head (obj_t);
obj_t V_help (int, obj_t[]);
obj_t V_include(int, obj_t[]);
int F_length (obj_t);
obj_t V_list (int, obj_t[]);
obj_t V_make_list (int, obj_t[]);
obj_t V_noprint (int, obj_t[]);
obj_t V_open (int, obj_t[]);
obj_t V_pipe (int, obj_t[]);
obj_t V_pointer (int, obj_t[]);
obj_t V_primitive (int, obj_t[]);
obj_t V_print (int, obj_t[]);
obj_t V_pwd (int, obj_t[]);
obj_t V_quote (int, obj_t[]);
obj_t V_redirect (int, obj_t[]);
obj_t F_reverse (obj_t);
obj_t V_setcwd (int, obj_t[]);
obj_t V_setf (int, obj_t[]);
obj_t V_struct (int, obj_t[]);
obj_t F_tail (obj_t);
obj_t V_typeof (int, obj_t[]);

/*** lex.c ***/
extern lex_t *LEX_STDIN;
lex_t *lex_open (const char*);
lex_t *lex_stream (FILE*);
lex_t *lex_string (const char*);
lex_t *lex_stack(void);
void lex_push(lex_t *f, lex_t *item);
lex_t *lex_close (lex_t*);
int lex_getc (lex_t*);
int lex_ungetc (lex_t*, int);
char *lex_gets (lex_t*, char*, int);
int lex_token (lex_t*, char**, int);
void lex_special (lex_t*, int);
int lex_consume (lex_t*);
void lex_set (lex_t*, int, char*);
char *lex_strtok (char*, const char*, strtok_t*);

/*** num.c ***/
int num_isint (obj_t);
int num_int (obj_t);
int num_isfp (obj_t);
double num_fp (obj_t);

/*** obj.c ***/
void obj_init (void);
obj_t obj_new (class_t, ...);
obj_t obj_dest (obj_t);
int obj_diff (obj_t, obj_t);
obj_t obj_copy (obj_t, int);
void obj_print (obj_t, out_t*);
obj_t obj_eval (obj_t);
obj_t obj_feval (obj_t);
obj_t obj_apply (obj_t, obj_t);
char *obj_name (obj_t);
int obj_usage (void);
void obj_walk1 (obj_t, void*, int, walk_t*);
int obj_walk2 (obj_t, void*, obj_t, void*, walk_t*);
int obj_walk3 (void*, obj_t, obj_t);
int obj_sizeof (obj_t);
obj_t obj_deref (obj_t, int, obj_t*);
obj_t obj_bind (obj_t, void*);
int obj_truth (obj_t);

/*** output.c ***/
extern out_t *OUT_STDOUT;
extern out_t *OUT_STDERR;
extern int OUT_NCOLS;
extern int OUT_NROWS;
extern int OUT_LTMAR;
extern int OUT_COL2;
pflags_t out_brokenpipe (out_t*);
void out_section(out_t*);
void out_error (const char*, obj_t);
void out_errorn (const char*, ...);
int out_error_disable (void);
int out_error_restore (void);
void out_indent (out_t*);
void out_column(out_t *f, int column, const char *separator);
void out_info (const char*, ...);
void out_init_size(void);
void out_init (void);
void out_line (out_t*, const char*);
int out_literal (out_t*, int);
void out_nl (out_t*);
void out_pop (out_t*);
void out_header(out_t *f, const char *header);
void out_prefix (out_t*);
void out_printf (out_t*, const char*, ...);
void out_progress (const char*);
void out_push (out_t*, const char*);
int *out_push_array (out_t*, const char*, int, const int*, const int*);
void out_puts (out_t*, const char*);
void out_putw (out_t*, const char*);
void out_reset (out_t*);
out_t *out_stream (FILE*);
void out_undent (out_t*);
int out_getindex (out_t*, int);

/*** parse.c ***/
void parse_init (void);
obj_t parse_stmt (lex_t*, int);

/*** prim.c ***/
extern prim_assoc_t PA_BR_OBJTYPE[];
extern prim_assoc_t PA_OBJTYPE[];
extern prim_assoc_t PA_DATATYPE[];
extern prim_assoc_t PA_ORDER[];
extern prim_assoc_t PA_ONOFF[];
extern prim_assoc_t PA_BOOLEAN[];
extern prim_assoc_t PA_COORDSYS[];
extern prim_assoc_t PA_COORDTYPE[];
extern prim_assoc_t PA_FACETYPE[];
extern prim_assoc_t PA_PLANAR[];
extern prim_assoc_t PA_CENTERING[];
extern prim_assoc_t PA_DEFVARTYPE[];
extern prim_assoc_t PA_BOUNDARYTYPE[];
extern prim_assoc_t PA_REGIONOP[];
extern prim_assoc_t PA_TOPODIM[];
extern prim_assoc_t PA_REPRBLOCK[];

obj_t prim_set_io_assoc (obj_t, prim_assoc_t*);
DBdatatype prim_silotype (obj_t);
void prim_octal(char *buf/*out*/, const void *_mem, size_t nbytes);

/*** range.c ***/
int range_range (obj_t, int*, int*);

/*** sdo.c ***/
obj_t sdo_assign (obj_t, obj_t);
obj_t sdo_cast (obj_t, obj_t);
obj_t sdo_typeof (obj_t);
obj_t sdo_file (obj_t);
void *sdo_mem (obj_t);

/*** stc.c ***/
obj_t stc_add (obj_t, obj_t, int, const char*);
void stc_sort (obj_t, int);
int stc_offset (obj_t, obj_t);
void stc_silo_types (void);

/*** str.c ***/
void str_doprnt (out_t*, char*, char*);

/*** switch.c ***/
switches_t *switch_new(void);
switch_t *switch_add(switches_t *sws,
                     const char *short_name, const char *long_name,
                     const char *arg_spec, switch_handler_t handler);
void switch_info(switch_t *sw, void *info);
void switch_doc(switch_t *sw, const char *doc_string);
switch_t *switch_find(switches_t *sws, const char *name);
void switch_usage(switches_t *sws, const char *arg0, const char *sname);
int switch_parse(switches_t *sws, int argc, char *argv[],
                 void(*error)(const char*, ...));
void switch_arg(switch_t *sw, int *type, size_t name_size, char *name,
                int *required, const char **dflt);

/*** sym.c ***/
void sym_init (void);
void sym_fbind (obj_t, obj_t);
obj_t sym_fboundp (obj_t);
void sym_vbind (obj_t, obj_t);
obj_t sym_vboundp (obj_t);
void sym_dbind(obj_t _self, obj_t value);
obj_t sym_dboundp(obj_t _self);
obj_t sym_self_set (obj_t);
int sym_truth (char*);
void sym_bi_set(const char *name, const char *value, const char *desc,
                const char *doc);
char *sym_bi_gets(const char *name);
int sym_bi_true(const char *name);
int sym_map(int(*func)(obj_t, void*), void *cdata);
void sym_doc(const char *symname, const char *docstr);

#endif /* !_BROWSER_H */

