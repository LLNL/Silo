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
 * Created:             file.c
 *                      Dec  4 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             File I/O
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include <browser.h>
#ifdef HAVE_FNMATCH_H
#  include <fnmatch.h>
#endif

#include <config.h>     /*Silo configuration record*/
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define CHECK_SYMBOL(A)  if (!strncmp(str, #A, strlen(str))) return A

#define CHECK_SYMBOLN_INT(A)				\
if (!strncmp(tmp, #A"=", strlen(#A)+1))			\
{							\
    int n = sscanf(tmp, #A"=%d", &driver_ints[driver_nints]);\
    if (n == 1)						\
    {							\
        DBAddOption(opts, A, &driver_ints[driver_nints]);\
        driver_nints++;					\
        got_it = 1;					\
    }							\
}

#define CHECK_SYMBOLN_STR(A)				\
if (!strncmp(tmp, #A"=", strlen(#A)+1))			\
{							\
    driver_strs[driver_nstrs] = safe_strdup(&tmp[strlen(#A)]+1);\
    DBAddOption(opts, A, driver_strs[driver_nstrs]);	\
    driver_nstrs++;					\
    got_it = 1;						\
}

#define CHECK_SYMBOLN_SYM(A)				\
if (!strncmp(tmp, #A"=", strlen(#A)+1))			\
{							\
    driver_ints[driver_nints] = StringToOptval(&tmp[strlen(#A)]+1);\
    DBAddOption(opts, A, &driver_ints[driver_nints]);	\
    driver_nints++;					\
    got_it = 1;						\
}

static DBoptlist *driver_opts[] = {0,0,0,0,0,0,0,0,0,0,0};
static int driver_ints[100];
static int driver_nints = 0;
static char *driver_strs[] = {0,0,0,0,0,0,0,0,0,0};
static int driver_nstrs = 0;
static const int driver_nopts = sizeof(driver_opts)/sizeof(driver_opts[0]);

static void CleanupDriverStuff()
{
    int i;
    for (i = 0; i < driver_nopts; i++)
        if (driver_opts[i]) DBFreeOptlist(driver_opts[i]);
    for (i = 0; i < sizeof(driver_strs)/sizeof(driver_strs[0]); i++)
        if (driver_strs[i]) free(driver_strs[i]);
    memset(driver_opts, 0, sizeof(driver_opts));
    memset(driver_strs, 0, sizeof(driver_strs));
    memset(driver_ints, 0, sizeof(driver_ints));
    driver_nints = 0;
    driver_nstrs = 0;
}

static void MakeDriverOpts(DBoptlist **_opts, int *opts_id)
{
    DBoptlist *opts = DBMakeOptlist(30);
    int i;

    for (i = 0; i < driver_nopts; i++)
    {
        if (driver_opts[i] == 0)
        {
            driver_opts[i] = opts;
            break;
        }
    }

    *_opts = opts;
    *opts_id = DBRegisterFileOptionsSet(opts);
}

static int StringToOptval(const char *str)
{
    CHECK_SYMBOL(DB_PDB);
    CHECK_SYMBOL(DB_HDF5);
    CHECK_SYMBOL(DB_HDF5_SEC2);
    CHECK_SYMBOL(DB_HDF5_STDIO);
    CHECK_SYMBOL(DB_HDF5_CORE);
    CHECK_SYMBOL(DB_HDF5_SPLIT);
    CHECK_SYMBOL(DB_HDF5_MPIO);
    CHECK_SYMBOL(DB_HDF5_MPIP);
    CHECK_SYMBOL(DB_HDF5_LOG);
    CHECK_SYMBOL(DB_HDF5_DIRECT);
    CHECK_SYMBOL(DB_HDF5_FAMILY);
    CHECK_SYMBOL(DB_HDF5_SILO);
    
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_DEFAULT);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_SEC2);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_STDIO);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_CORE);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_LOG);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_SPLIT);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_DIRECT);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_FAMILY);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_MPIP);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_MPIO);
    CHECK_SYMBOL(DB_FILE_OPTS_H5_DEFAULT_SILO);

    CHECK_SYMBOL(DB_H5VFD_DEFAULT);
    CHECK_SYMBOL(DB_H5VFD_SEC2);
    CHECK_SYMBOL(DB_H5VFD_STDIO);
    CHECK_SYMBOL(DB_H5VFD_CORE);
    CHECK_SYMBOL(DB_H5VFD_LOG);
    CHECK_SYMBOL(DB_H5VFD_SPLIT);
    CHECK_SYMBOL(DB_H5VFD_DIRECT);
    CHECK_SYMBOL(DB_H5VFD_FAMILY);
    CHECK_SYMBOL(DB_H5VFD_MPIO);
    CHECK_SYMBOL(DB_H5VFD_MPIP);
    CHECK_SYMBOL(DB_H5VFD_SILO);
}

#define MYCLASS(X)      ((obj_file_t*)(X))

typedef struct obj_file_t {
   obj_pub_t    pub;
   int          rdonly;
   char         *name;
   DBfile       *f;
} obj_file_t;

class_t         C_FILE;
static obj_t    file_new (va_list);
static obj_t    file_dest (obj_t);
static void     file_print (obj_t, out_t*);
static char *   file_name (obj_t);
static obj_t    file_deref (obj_t, int, obj_t*);
static int      file_diff (obj_t, obj_t);


/*-------------------------------------------------------------------------
 * Function:    file_class
 *
 * Purpose:     Initializes the class.
 *
 * Return:      Success:        Ptr to the FILE class.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
class_t
file_class (void) {

   class_t      cls = calloc (1, sizeof(*cls));

   cls->name = safe_strdup ("FILE");
   cls->new = file_new;
   cls->dest = file_dest;
   cls->copy = NULL;
   cls->print = file_print;
   cls->objname = file_name;
   cls->deref = file_deref;
   cls->diff = file_diff;
   return cls;
}

/*-------------------------------------------------------------------------
 * Function:    file_reset_hdf5_vfd_options
 *
 * Purpose:     Sets hdf5 virtual file driver options for opening. 
 *
 * Programmer:  Mark C. Miller, Fri Feb 12 08:25:07 PST 2010
 *              The code to parse $h5vfdopts was cut-n-pasted from
 *              Robb Matzke's code in func.c for $exclude variable.
 *
 * Modifications:
 *   Mark C. Miller, Thu Mar 18 18:19:00 PDT 2010
 *   Recoded entirely to accomodate new interface for HDF5 options.
 *-------------------------------------------------------------------------
 */
#define MAX_FILE_OPTIONS_SETS 32
void file_reset_hdf5_vfd_options (void) {

    obj_t head=NIL, value=NIL, symbol=NIL, word=NIL;
    int opts_id;
    int added_options = 0;
    DBoptlist *opts;

    /* free all the old file options sets */
    DBUnregisterAllFileOptionsSets();
    CleanupDriverStuff();

    /* start a new driver options set */
    MakeDriverOpts(&opts, &opts_id);

    /* parse $h5vfdopts to get extensions */
    symbol = obj_new(C_SYM, "$h5vfdopts");
    head = sym_vboundp(symbol);
    symbol = obj_dest(symbol);
    if (head && C_CONS!=head->pub.cls) {
        head = obj_new(C_CONS, obj_copy(head, SHALLOW), NIL);
    }
    for (value=head; value; value=cons_tail(value)) {
        if (C_CONS!=value->pub.cls) {
            out_errorn("invalid value for $h5vfdopts");
            goto done;
        }
        word = cons_head(value);
        if (C_STR==word->pub.cls) {
            int got_it = 0;
            char *tmp = safe_strdup(obj_name(word));
            if (!strncmp(tmp,"_NEWSET_",8))
                MakeDriverOpts(&opts, &opts_id);
            CHECK_SYMBOLN_SYM(DBOPT_H5_VFD)
            CHECK_SYMBOLN_SYM(DBOPT_H5_RAW_FILE_OPTS)
            CHECK_SYMBOLN_STR(DBOPT_H5_RAW_EXTENSION)
            CHECK_SYMBOLN_SYM(DBOPT_H5_META_FILE_OPTS)
            CHECK_SYMBOLN_STR(DBOPT_H5_META_EXTENSION)
            CHECK_SYMBOLN_INT(DBOPT_H5_CORE_ALLOC_INC)
            CHECK_SYMBOLN_INT(DBOPT_H5_CORE_NO_BACK_STORE)
            CHECK_SYMBOLN_INT(DBOPT_H5_META_BLOCK_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_SMALL_RAW_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_ALIGN_MIN)
            CHECK_SYMBOLN_INT(DBOPT_H5_ALIGN_VAL)
            CHECK_SYMBOLN_INT(DBOPT_H5_DIRECT_MEM_ALIGN)
            CHECK_SYMBOLN_INT(DBOPT_H5_DIRECT_BLOCK_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_DIRECT_BUF_SIZE)
            CHECK_SYMBOLN_STR(DBOPT_H5_LOG_NAME)
            CHECK_SYMBOLN_INT(DBOPT_H5_LOG_BUF_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_SIEVE_BUF_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_CACHE_NELMTS)
            CHECK_SYMBOLN_INT(DBOPT_H5_CACHE_NBYTES)
            CHECK_SYMBOLN_INT(DBOPT_H5_FAM_SIZE)
            CHECK_SYMBOLN_SYM(DBOPT_H5_FAM_FILE_OPTS)
            CHECK_SYMBOLN_INT(DBOPT_H5_SILO_BLOCK_SIZE)
            CHECK_SYMBOLN_INT(DBOPT_H5_SILO_BLOCK_COUNT)
            free(tmp);
            if (!got_it)
            {
                out_errorn("invalid value for $h5vfdopts");
                goto done;
            }
            added_options = 1;
        } else {
            out_errorn("$h5vfdopts values should be strings");
            goto done;
        }
    }
    head = obj_dest(head);

    /* if the options set is empty, just get rid of it */
    if (!added_options)
    {
        DBUnregisterAllFileOptionsSets();
        CleanupDriverStuff();
    }

done:

    /* Free temp expressions */
    obj_dest(symbol);
    obj_dest(head);

}


/*-------------------------------------------------------------------------
 * Function:    file_new
 *
 * Purpose:     File constructor takes the name of the file to open for
 *              read-only.
 *
 * Return:      Success:        Ptr to a SILO file object.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 2 Apr 1997
 *      The second argument indicates if the file should be open for
 *      reading only.
 *
 *      Robb Matzke, 2 Apr 1997
 *      Opening a file with filters does not fail if the filter is not
 *      installed.
 *
 *      Robb Matzke, 29 Jul 1997
 *      The read-only flag is kept with the file.
 *
 *      Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *      I changed strdup to safe_strdup.
 *
 *      Robb Matzke, 2000-03-24
 *      If we ever see two different files being opened then turn off
 *      the `lowlevel' flag corresponding to the `-l' command-line
 *      switch. This will prevent someone from trying to diff a PDB and
 *      HDF5 file using the `-l' flag.
 *
 *      Mark C. Miller, Wed Sep  2 16:43:48 PDT 2009
 *      Made it turn off $lowlevel for certain HDF5 situations. The problem
 *      with HDF5 files is that the low-level data arrays for aggregate
 *      objects do NOT appear in the toc by default. They appear only if
 *      HDF5-friendly names are enabled. When they do NOT appear, doing
 *      a diff with $lowlevel set to anything other than zero means the
 *      low-level data arrays will NOT be diffed. I also added a warning
 *      about performance of diff for PDB files when lowlevel is turned
 *      off. In the PDB case, since low level arrays ALWAYS appear in the
 *      toc, doing a diff with them means those arrays are diffed twice;
 *      once as parts of aggregate objects and once as raw arrays.
 *
 *      Mark C. Miller, Fri Feb 12 08:26:07 PST 2010
 *      Added call to file_reset_split_vfd_extensions.
 *
 *      Mark C. Miller, Fri Mar 12 01:22:11 PST 2010
 *      Changed to call file_reset_hdf5_vfd_options
 *-------------------------------------------------------------------------
 */
static obj_t
file_new (va_list ap)
{
   obj_file_t   *self;
   char         *fname=NULL;
   DBfile       *f;
   int          rdonly;
   int          old_err_level;
   static int   seen_file_format = -1;

   fname = va_arg (ap, char*);
   rdonly = va_arg (ap, int);

   /* reset hdf5 vfd options (whether the are used or not) */
   file_reset_hdf5_vfd_options();

   /*
    * Open the file, and if that fails because the file doesn't have write
    * permission then try opening it for read-only
    */
#define DEFAULT_DB_TYPE DB_UNKNOWN 
   f = DBOpen (fname, DEFAULT_DB_TYPE, rdonly ? DB_READ : DB_APPEND);
   if (!f && E_FILENOWRITE==db_errno && !rdonly) {
      f = DBOpen (fname, DEFAULT_DB_TYPE, DB_READ);
      if (f) {
         out_info ("file opened, but without write permission");
      }
      rdonly = true;
   }

   /*
    * If the file could not be opened because a filter was missing then turn
    * on full error reporting and try again.  This causes the missing filter
    * to be a warning instead of an error.  See DBOpen() for documentation.
    */
   if (!f && E_NOTFILTER==db_errno) {
      old_err_level = DBErrlvl();
      DBShowErrors(DB_ALL, NULL);
      f = DBOpen (fname, DEFAULT_DB_TYPE, rdonly ? DB_READ : DB_APPEND);
      DBShowErrors(old_err_level, NULL);
   }

   if (!f) {
      out_errorn ("unable to open file: %s", fname);
   }

   if (!f) return NIL ; /*error already printed*/
   self = calloc (1, sizeof(obj_file_t));
   self->name = safe_strdup (fname);
   self->f = f;
   self->rdonly = rdonly;

   /* If we see any file which is not PDB then turn off the `$lowlevel'
    * flag for fear of the caller attempting a `diff' operation across
    * file formats. */
   if (seen_file_format<0) {
       seen_file_format = f->pub.type;
   } else if (seen_file_format!=f->pub.type && sym_bi_true("lowlevel")) {
       out_info("turning $lowlevel off due mixing file types");
       sym_bi_set("lowlevel", "0", NULL, NULL);
   } else if (DBGetDriverType(f) == DB_HDF5 && sym_bi_true("lowlevel") != 0 &&
              !DBGuessHasFriendlyHDF5Names(f)) {
      out_info("turning $lowlevel off because this is an HDF5 file without friendly names.");
      sym_bi_set("lowlevel", "0", NULL, NULL);
   } else if (DBGetDriverType(f) == DB_PDB && sym_bi_true("lowlevel") == 0) {
      out_info("having $lowlevel turned off for PDB files results in slower diff performance.");
   }

   return (obj_t)self;
}


/*-------------------------------------------------------------------------
 * Function:    file_dest
 *
 * Purpose:     Destroys a file object, closing the file when all
 *              references are gone.
 *
 * Return:      Success:        NIL
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
file_dest (obj_t _self) {

   obj_file_t   *self = MYCLASS(_self);

   if (0==self->pub.ref) {
      if (Verbosity>=2) out_info ("file_dest: closing file: %s", self->name);
      DBClose (self->f);
      memset (self, 0, sizeof(obj_file_t));
   }
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    file_print
 *
 * Purpose:     Prints a file object.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 3 Feb 1997
 *      Prints only the file name and current working directory since the
 *      `pwd' and `cd' commands use this.
 *
 *-------------------------------------------------------------------------
 */
static void
file_print (obj_t _self, out_t *f) {

   obj_file_t   *self = MYCLASS(_self);
   char         cwd[1024];

   if (DBGetDir (self->f, cwd)<0) strcpy (cwd, "???");
   out_printf (f, "%s%s:%s", self->name, self->rdonly?"[RDONLY]":"", cwd);
}


/*-------------------------------------------------------------------------
 * Function:    file_name
 *
 * Purpose:     Gets the name of a file.
 *
 * Return:      Success:        Ptr to the name of a file.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char *
file_name (obj_t _self) {

   return MYCLASS(_self)->name;
}


/*-------------------------------------------------------------------------
 * Function:    file_file
 *
 * Purpose:     Returns the DBfile associated with the object.
 *
 * Return:      Success:        Ptr to the DBfile
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
DBfile *
file_file (obj_t _self) {

   if (!_self || C_FILE!=_self->pub.cls) return NULL;
   return MYCLASS(_self)->f;
}

/*---------------------------------------------------------------------------
 * Function:    fix_objdups
 *
 * Purpose:     If the DBobject has multiple fields with the same name
 *              then the second and subsequent fields are renamed by
 *              appending a unique string.  These changes are destructive.
 *
 * Return:      Input argument.
 *
 * Programmer:  Robb Matzke
 *              Friday, May 19, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
static DBobject *
fix_objdups(DBobject *obj)
{
    char        **new_names, suffix[32];
    int         i, j, occur;
    
    if (!obj) return NULL;
    new_names = calloc(obj->ncomponents, sizeof(char*));

    /* Look for duplicates and generate new names */
    for (i=1; i<obj->ncomponents; i++) {
        for (j=0, occur=1; j<i; j++) {
            if (!strcmp(obj->comp_names[i], obj->comp_names[j])) occur++;
        }
        if (occur>1) {
            if (1==occur%10 && 11!=occur) {
                sprintf(suffix, " [%dst occurrence]", occur);
            } else if (2==occur%10 && 12!=occur) {
                sprintf(suffix, " [%dnd occurrence]", occur);
            } else if (3==occur%10 && 13!=occur) {
                sprintf(suffix, " [%drd occurrence]", occur);
            } else {
                sprintf(suffix, " [%dth occurrence]", occur);
            }
            new_names[i] = malloc(strlen(obj->comp_names[i])+strlen(suffix)+1);
            strcpy(new_names[i], obj->comp_names[i]);
            strcat(new_names[i], suffix);
        }
    }
    
    /* Replace old names with new names */
    for (i=0; i<obj->ncomponents; i++) {
        if (new_names[i]) {
            free(obj->comp_names[i]);
            obj->comp_names[i] = new_names[i];
        }
    }

    return obj;
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBGetObject
 *
 * Purpose:     Reads a SILO DBobject from the specified database and
 *              returns an object which can be printed nicely.  The
 *              new object is a structure whose fields are named according
 *              to the `comp_names' field of DBobject and whose values
 *              are based on the `pdb_names' field.  However, values that
 *              have type information like `<i>50' are converted to a
 *              numeric type.
 *
 *              The first few fields point to things like the DBobject,
 *              its name and type, and extra information about each field.
 *
 * Return:      Success:        Ptr to a new object
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 31 1997
 *
 * Modifications:
 *              Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *              I changed strdup to safe_strdup.
 *
 *              Robb Matzke, 2000-01-13
 *              Since we're building the datatype for these things on the fly
 *              we have to watch out for alignment constraints. We'll just
 *              assume that an object whose size is N must be aligned on a
 *              byte which is divisible by N.
 *
 *              Brad Whitlock, Thu Jan 20 18:01:32 PST 2000
 *              Added support for double precision components.
 *
 *              Robb Matzke, 2000-05-23
 *              If `$lowlevel' is `3' then only the true DBObject type
 *              information is returned. This can sometimes speed up the
 *              `diff' operator.  The values of $lowlevel are:
 *
 *                      0, 1, or unset: spontaneous members
 *                      2:              spontaneous and original members
 *                      3:              original members
 *
 *              The original members are the `comp_names' and `pdb_names'
 *              character string arrays which are stored in the file. The
 *              spontaneous members are created by taking the comp_names
 *              strings as names of members of a compound datatype and the
 *              corresponding pdb_names string as an encoded value which
 *              is translated into a string, integer, etc.
 *-------------------------------------------------------------------------
 */
static void *
browser_DBGetObject (DBfile *file, char *name, obj_t *type_ptr)
{
    DBobject    *obj=NULL;
    char        *b_obj=NULL, *s=NULL;
    int         i, need, offset, *flags=NULL, field_size;
    obj_t       type=NIL;
    int         lowlevel;

    obj = DBGetObject(file, name);
    if (!obj) return NULL;

    /*
     * Determine the value of the `$lowlevel' variable.  If it is large
     * enough, then the user wants really low level info, so we give him
     * the raw DBobject and our `flags' as fields of the structure.
     */
    lowlevel = sym_bi_true("lowlevel");

    /*
     * Count the space we need. The four void pointers are and the following
     * integer are properly aligned. We have to explicitly align all that
     * follows.
     */
    need = 4 * sizeof(void*) + sizeof (int);    /*header fields*/
    if (lowlevel<3) {
        obj = fix_objdups(obj);
        for (i=0; i<obj->ncomponents; i++) {
            if (!strncmp("'<i>", obj->pdb_names[i], 4)) {
                field_size = sizeof(int);
            } else if (!strncmp("'<f>", obj->pdb_names[i], 4)) {
                field_size = sizeof(double);
            } else if (!strncmp("'<d>", obj->pdb_names[i], 4)) {
                field_size = sizeof(double);
            } else if (!strncmp ("'<s>", obj->pdb_names[i], 4)) {
                field_size = sizeof(char*);
            } else {
                field_size = sizeof(char*);
            }

            while (need % field_size) need++;
            need += field_size;
        }
    }

    /*
     * Allocate the new object and fill in the header fields. The first five
     * fields (four pointers and an integer) are properly aligned when packed.
     */
    assert(sizeof(char*)==sizeof(void*));
    b_obj = malloc (need);
    *((DBobject**)b_obj) = obj;
    *((char**)(b_obj+sizeof(void*))) = obj->name;
    *((char**)(b_obj+2*sizeof(void*))) = obj->type;
    flags = calloc (obj->ncomponents, sizeof(int));
    *((int**)(b_obj+3*sizeof(void*))) = flags;
    *((int*)(b_obj+4*sizeof(void*))) = obj->ncomponents;
    offset = 4 * sizeof(void*) + sizeof(int);
    type = obj_new (C_STC, NULL, NULL);
    
    if (lowlevel>=2) {
        stc_add (type, obj_new (C_PTR,obj_new(C_STR,"DBobject")), 0, "raw");
    }
    if (lowlevel<3) {
        stc_add (type, obj_new (C_PRIM, "string"), 1*sizeof(char*), "name");
        stc_add (type, obj_new (C_PRIM, "string"), 2*sizeof(char*), "type");
    }
    
    /*
     * Fill in all the value fields.  If a value should be freed later
     * then the flags will be set to 0x00000001. Care is taken to make sure
     * that the fields we're defining on the fly are properly aligned.
     */
    if (lowlevel<3) {
        for (i=0; i<obj->ncomponents; i++) {

            if (!strncmp("'<i>", obj->pdb_names[i], 4)) {
                /*
                 * The value is an integer of some type.  Store the
                 * integer, not the string, so the user can say things like
                 * `typeof x.y' and get `int' instead of `string'.  Even at
                 * the low level, Eric would like this hidden.
                 */
                flags[i] = 0;
                while (offset % sizeof(int)) offset++;
                *((int*)(b_obj+offset)) = strtol(obj->pdb_names[i]+4, NULL, 0);
                stc_add(type, obj_new(C_PRIM, "int"), offset,
                        obj->comp_names[i]);
                offset += sizeof(int);

            } else if (!strncmp("'<f>", obj->pdb_names[i], 4) ||
                       !strncmp("'<d>", obj->pdb_names[i], 4)) {
                /*
                 * The value is a float or double.  We store it as double for
                 * the same reasons as <i> above.
                 */
                flags[i] = 0;
                while (offset % sizeof(double)) offset++;
                *((double*)(b_obj+offset)) = strtod(obj->pdb_names[i]+4, NULL);
                stc_add(type, obj_new(C_PRIM, "double"), offset,
                        obj->comp_names[i]);
                offset += sizeof(double);

            } else if (!strncmp ("'<s>", obj->pdb_names[i], 4)) {
                /*
                 * The value is a string, but the user should see only the
                 * part after the type.  Remember to get rid of the trailing
                 * single quote.
                 */
                flags[i] = 1;
                while (offset % sizeof(char*)) offset++;
                s = safe_strdup(obj->pdb_names[i]+4);
                *((char**)(b_obj+offset)) = s;
                if (*s) s[strlen(s)-1] = '\0';
                stc_add(type, obj_new(C_PRIM, "string"), offset,
                        obj->comp_names[i]);
                offset += sizeof(char*);

            } else {
                /*
                 * Either we don't recognize the type specifier or there is
                 * none.  We treat the entire value like a string.
                 */
                flags[i] = 1;
                while (offset % sizeof(char*)) offset++;
                *((char**)(b_obj+offset)) = safe_strdup(obj->pdb_names[i]);
                stc_add(type, obj_new(C_PRIM, "string"), offset,
                        obj->comp_names[i]);
                offset += sizeof(char*);
            }
        }
    }

    *type_ptr = type;
    return b_obj;
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBSaveObject
 *
 * Purpose:     Writes a DBobject over the top of some already existing
 *              DBobject.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *    Brad Whitlock, Thu Jan 20 17:59:59 PST 2000
 *    Added support for double precision components.
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
browser_DBSaveObject (obj_t _self, char *unused, void *mem, obj_t type) {

   obj_file_t   *self = MYCLASS(_self);
   char         *b_obj = (char*)mem;
   DBobject     *obj = *((DBobject**)b_obj);
   int          i, n, offset, nerrors=0, nchanges=0;
   char         *s, buf[64];
   double       d;
   obj_t        comp_name;

   if (self->rdonly) {
      out_errorn ("file `%s' is read-only", self->name);
      return -1;
   }

   for (i=0; i<obj->ncomponents; i++) {
      comp_name = obj_new (C_SYM, obj->comp_names[i]);
      offset = stc_offset (type, comp_name);
      comp_name = obj_dest (comp_name);

      if (offset<0) {
         out_errorn ("browser_DBPutObject: cannot find the structure byte "
                     "offset for field `%s'", obj->comp_names[i]);
         nerrors++;
         continue;
      }

      if (!strncmp ("'<i>", obj->pdb_names[i], 4)) {
         n = *((int*)(b_obj+offset));
         sprintf (buf, "'<i>%d'", n);
         if (strcmp (obj->pdb_names[i], buf)) {
            free (obj->pdb_names[i]);
            obj->pdb_names[i] = safe_strdup (buf);
            nchanges++;
         }

      } else if (!strncmp ("'<f>", obj->pdb_names[i], 4)) {
         d = *((double*)(b_obj+offset));
         sprintf (buf, "'<f>%g'", d);
         if (strcmp (obj->pdb_names[i], buf)) {
            free (obj->pdb_names[i]);
            obj->pdb_names[i] = safe_strdup (buf);
            nchanges++;
         }

      } else if (!strncmp ("'<d>", obj->pdb_names[i], 4)) {
         d = *((double*)(b_obj+offset));
         sprintf (buf, "'<d>%.30g'", d);
         if (strcmp (obj->pdb_names[i], buf)) {
            free (obj->pdb_names[i]);
            obj->pdb_names[i] = safe_strdup (buf);
            nchanges++;
         }

      } else if (!strncmp ("'<s>", obj->pdb_names[i], 4)) {
         s = *((char**)(b_obj+offset));
         if (strncmp(obj->pdb_names[i]+4, s, strlen(s))) {
            free (obj->pdb_names[i]);
            obj->pdb_names[i] = malloc (strlen(s)+5);
            strcpy (obj->pdb_names[i], "'<s>");
            strcpy (obj->pdb_names[i]+4, s);
            nchanges++;
         }

      } else {
         s = *((char**)(b_obj+offset));
         if (strcmp(obj->pdb_names[i], s)) {
            free (obj->pdb_names[i]);
            obj->pdb_names[i] = safe_strdup (s);
            nchanges++;
         }
      }
   }

   return DBChangeObject (file_file(_self), obj);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBGetMultimeshadj
 *
 * Purpose:     Deal with non-standard get-API for DBmultimeshadj objects 
 *
 * Return:      DBmultimeshadj 
 *
 * Programmer:  Mark C. Miller 
 *              August 24, 2005
 *
 *-------------------------------------------------------------------------
 */
static DBmultimeshadj*
browser_DBGetMultimeshadj(DBfile *file, char *name)
{
    return DBGetMultimeshadj(file, name, 0, NULL);
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeObject
 *
 * Purpose:     Frees a modified SILO DBobject which was created with the
 *              browser_DBGetObject function.  The underlying DBobject
 *              is freed as are any fields whose flag value has the least
 *              significant bit set.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 31 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
browser_DBFreeObject (void *mem, obj_t type) {

   char         *b_obj = (char*)mem;
   DBobject     *obj=NULL;
   int          i, n, *flags=NULL, offset;
   void         *comp_mem=NULL;
   obj_t        comp_name=NIL;

   if (!mem) return;
   obj = *((DBobject**)b_obj);
   flags = *((int**)(b_obj+3*sizeof(void*)));
   n = *((int*)(b_obj+4*sizeof(void*)));

   for (i=0; i<n; i++) {
      if (0 == (flags[i] & 0x01)) continue; /*no memory to free*/
      comp_name = obj_new (C_SYM, obj->comp_names[i]);
      offset = stc_offset (type, comp_name);
      if (offset<0) continue; /*field doesn't exist!*/
      comp_mem = *((void**)(b_obj+offset));
      if (comp_mem) free (comp_mem);
   }

   free (flags);
   DBFreeObject (obj);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBGetCompoundarray
 *
 * Purpose:     Reads a SILO DBcompoundarray from the specified database and
 *              returns an object which can be printed nicely.
 *
 * Return:      Success:        Ptr to a new object
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Feb  5 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void *
browser_DBGetCompoundarray (DBfile *file, char *name, obj_t *type_ptr) {

   DBcompoundarray      *ca;
   char                 **b_ca;
   char                 buf[64];
   int                  i, at, nbytes;
   obj_t                type=NIL, tmp=NIL;

   /*
    * Read the compound array and create a browser compound array.  The
    * first entry in the browser compound array points to the beginning
    * of the silo compound array so we can free it later.
    */
   ca = DBGetCompoundarray (file, name);
   if (!ca) return NULL;
   b_ca = calloc (ca->nelems+7, sizeof(char*));
   type = obj_new (C_STC, NULL, NULL);
   b_ca[0] = (char*)ca;

   /*
    * The browser compound array should point to the silo compound array
    * fields so that modifying the browser object actually modifies the
    * silo object.
    */
   b_ca[1] = ca->name;
   stc_add (type, obj_new(C_PRIM, "string"),
            1*sizeof(char*), "name");

   b_ca[2] = (char*)&(ca->id);
   stc_add (type, obj_new(C_PTR, obj_new(C_PRIM, "int")),
            2*sizeof(char*), "id");

   b_ca[3] = (char*)&(ca->nelems);
   stc_add (type, obj_new(C_PTR, obj_new(C_PRIM, "int")),
            3*sizeof(char*), "nelems");

   b_ca[4] = (char*)&(ca->nvalues);
   stc_add (type, obj_new(C_PTR, obj_new(C_PRIM, "int")),
            4*sizeof(char*), "nvalues");

   b_ca[5] = (char*)&(ca->datatype);
   tmp = obj_new (C_PRIM, "int");
   prim_set_io_assoc (tmp, PA_DATATYPE);
   stc_add (type, obj_new(C_PTR, tmp), 5*sizeof(char*), "datatype");
   tmp = NIL;

   tmp = obj_new (C_PRIM, "int");
   obj_bind (tmp, NULL);
   b_ca[6] = (char*)ca->elemlengths;
   sprintf (buf, "%d", ca->nelems);
   stc_add (type, obj_new(C_PTR, obj_new(C_ARY, buf, obj_copy(tmp,SHALLOW))),
            6*sizeof(char*), "elemlengths");
   tmp = NIL;

   /*
    * Create the array base type.
    */
   sprintf (buf, "%d", ca->datatype);
   tmp = obj_new (C_PRIM, buf);
   obj_bind (tmp, NULL);
   nbytes = obj_sizeof(tmp);
   assert (nbytes>0);

   /*
    * Create browser entries for each of the sub arrays.
    */
   for (i=at=0; i<ca->nelems; i++) {
      b_ca[7+i] = (char*)(ca->values) + at * nbytes;
      if (1==ca->elemlengths[i]) {
         stc_add (type, obj_new (C_PTR, obj_copy (tmp, SHALLOW)),
                  (i+7)*sizeof(char*), ca->elemnames[i]);
      } else {
         sprintf (buf, "%d", ca->elemlengths[i]);
         stc_add (type, obj_new (C_PTR,
                                 obj_new (C_ARY, buf, obj_copy(tmp,SHALLOW))),
                  (i+7)*sizeof(char*), ca->elemnames[i]);
      }
      at += ca->elemlengths[i];
   }

   /*
    * Sort the sub arrays into alphabetical order, but don't sort the
    * first five entries because they are part of the entire compound
    * array.
    */
   stc_sort (type, 6);


   /*
    * Free temp data and return
    */
   tmp = obj_dest (tmp);
   *type_ptr = type;
   return b_ca;
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeCompoundarray
 *
 * Purpose:     Frees a browser compound array and the silo compound array
 *              to which it points.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Feb  5 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeCompoundarray (void *mem, obj_t type) {

   char                 *b_ca = (char*)mem;
   DBcompoundarray      *ca;

   if (!b_ca) return;
   ca = *((DBcompoundarray**)b_ca);
   if (!ca) return;
   DBFreeCompoundarray (ca);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBGetDirectory
 *
 * Purpose:     Reads a directory.
 *
 * Return:      Success:        Ptr to a new DBDirectory type.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Jul 25 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void *
browser_DBGetDirectory (DBfile *file, char *name)
{
   char         cwd[1024];
   toc_t        *toc;
   int          nentries=0, i;
   DBdirectory  *dir;

   if (DBGetDir (file, cwd)<0) return NULL;
   if (DBSetDir (file, name)<0) return NULL;
   toc = browser_DBGetToc (file, &nentries, sort_toc_by_type);
   if (DBSetDir (file, cwd)<0) return NULL;

   dir = calloc (1, sizeof(DBdirectory));
   dir->nsyms = nentries;
   dir->toc = toc;
   if (nentries) dir->entry_ptr = calloc (nentries, sizeof (toc_t *));
   for (i=0; i<nentries; i++) {
      dir->entry_ptr[i] = dir->toc + i;
   }
   return dir;
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBGetSubarray
 *
 * Purpose:     Creates a browser object and associated type for one
 *              subarray from an array.
 *
 * Return:      Success:        Ptr to a browser object which also points
 *                              to the compound array from which it is
 *                              derived.  DO NOT FREE THE COMPOUND ARRAY
 *                              since that array is freed automatically
 *                              when this browser subarray object is
 *                              freed.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb  5 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void *
browser_DBGetSubarray (DBcompoundarray *ca, int elmtno, obj_t *type_ptr) {

   char         **b_ca;
   obj_t        type=NIL;
   int          i, at, nbytes;
   char         buf[64];

   assert (ca);
   assert (elmtno>=0 && elmtno<ca->nelems);

   b_ca = calloc (2, sizeof(char*));
   b_ca[1] = (char*)ca;

   /*
    * Create a type for the subarray.
    */
   sprintf (buf, "%d", ca->datatype);
   type = obj_new (C_PRIM, buf);
   obj_bind (type, NULL);
   nbytes = obj_sizeof(type);
   assert (nbytes>0);

   /*
    * Where does this subarray start?
    */
   for (i=at=0; i<elmtno; i++) at += ca->elemlengths[i];
   b_ca[0] = (char*)(ca->values) + at*nbytes;

   /*
    * And what type is it?
    */
   if (1==ca->elemlengths[elmtno]) {
      type = obj_new (C_PTR, type);
   } else {
      sprintf (buf, "%d", ca->elemlengths[elmtno]);
      type = obj_new (C_PTR, obj_new (C_ARY, buf, type));
   }

   *type_ptr = type;
   return b_ca;
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeSubarray
 *
 * Purpose:     Frees a compound array subarray and the associated compound
 *              array.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb  5 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeSubarray (void *mem, obj_t type) {

   char                 *b_ca = (char*)mem;
   DBcompoundarray      *ca;

   if (!b_ca) return;
   ca = *((DBcompoundarray**)(b_ca+sizeof(void*)));
   if (!ca) return;
   DBFreeCompoundarray (ca);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeDefvars
 *
 * Purpose:     Frees a DBdefvars
 *
 * Return:      void
 *
 * Programmer:  Mark C. Miller 
 *              August 9, 2005
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeDefvars (void *mem, obj_t type) {

   DBFreeDefvars ((DBdefvars*)mem);
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeCsgmesh
 *
 * Purpose:     Frees a DBcsgmesh
 *
 * Return:      void
 *
 * Programmer:  Mark C. Miller 
 *              August 9, 2005
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeCsgmesh (void *mem, obj_t type) {

   DBFreeCsgmesh ((DBcsgmesh*)mem);
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeCsgvar
 *
 * Purpose:     Frees a DBcsgvar
 *
 * Return:      void
 *
 * Programmer:  Mark C. Miller 
 *              August 9, 2005
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeCsgvar (void *mem, obj_t type) {

   DBFreeCsgvar ((DBcsgvar*)mem);
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeCSGZonelist
 *
 * Purpose:     Frees a DBcsgzonelist
 *
 * Return:      void
 *
 * Programmer:  Mark C. Miller 
 *              August 9, 2005
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeCSGZonelist (void *mem, obj_t type) {

   DBFreeCSGZonelist ((DBcsgzonelist*)mem);
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeCurve
 *
 * Purpose:     Frees a DBcurve
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeCurve (void *mem, obj_t type) {

   DBFreeCurve ((DBcurve*)mem);
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeMultimesh
 *
 * Purpose:     Frees a DBmultimesh.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeMultimesh (void *mem, obj_t type) {

   DBFreeMultimesh ((DBmultimesh*)mem);
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeMultimeshadj
 *
 * Purpose:     Frees a DBmultimeshadj.
 *
 * Return:      void
 *
 * Programmer:  Mark C. Miller 
 *              August 24, 2005 
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeMultimeshadj (void *mem, obj_t type) {

   DBFreeMultimeshadj ((DBmultimeshadj*)mem);
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeMultivar
 *
 * Purpose:     Frees a DBmultivar
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeMultivar (void *mem, obj_t type) {

   DBFreeMultivar ((DBmultivar*)mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeMultimat
 *
 * Purpose:     Frees a browser_DBmultimat
 *
 * Return:      void
 *
 * Programmer:  Mark C. Miller
 *              April 21, 2005 
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeMultimat (void *mem, obj_t type) {

   DBFreeMultimat((DBmultimat*) mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeMultimatspecies
 *
 * Purpose:     Frees a DBmultimatspecies
 *
 * Return:      void
 *
 * Programmer:  Jeremy S.Meredith
 *              Sept 17 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeMultimatspecies (void *mem, obj_t type) {

   DBFreeMultimatspecies((DBmultimatspecies*) mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeQuadmesh
 *
 * Purpose:     Frees a DBquadmesh.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeQuadmesh (void *mem, obj_t type) {

   DBFreeQuadmesh ((DBquadmesh*)mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeQuadvar
 *
 * Purpose:     Frees a DBquadvar.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeQuadvar (void *mem, obj_t type) {

   DBFreeQuadvar ((DBquadvar*)mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeUcdmesh
 *
 * Purpose:     Frees a DBucdmesh.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeUcdmesh (void *mem, obj_t type) {

   DBFreeUcdmesh ((DBucdmesh*)mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeUcdvar
 *
 * Purpose:     Frees a DBucdvar
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeUcdvar (void *mem, obj_t type) {

   DBFreeUcdvar ((DBucdvar*)mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreePointmesh
 *
 * Purpose:     Frees a DBpointmesh
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreePointmesh (void *mem, obj_t type) {

   DBFreePointmesh ((DBpointmesh*)mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeMeshvar
 *
 * Purpose:     Frees a DBmeshvar
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeMeshvar (void *mem, obj_t type) {

   DBFreeMeshvar ((DBmeshvar*)mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeMaterial
 *
 * Purpose:     Frees a DBmaterial
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeMaterial (void *mem, obj_t type) {

   DBFreeMaterial ((DBmaterial*)mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeMatspecies
 *
 * Purpose:     Frees a DBmatspecies
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeMatspecies (void *mem, obj_t type) {

   DBFreeMatspecies ((DBmatspecies*)mem);
}

/*ARGSUSED*/
static void 
browser_DBFreeMrgtree(void *mem, obj_t type) {
   DBFreeMrgtree ((DBmrgtree*)mem);
}

/*ARGSUSED*/
static void 
browser_DBFreeGroupelmap(void *mem, obj_t type) {
   DBFreeGroupelmap ((DBgroupelmap*)mem);
}

/*ARGSUSED*/
static void 
browser_DBFreeMrgvar(void *mem, obj_t type) {
   DBFreeMrgvar ((DBmrgvar*)mem);
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeVar
 *
 * Purpose:     Frees a silo variable.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Mar  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
browser_DBFreeVar (void *mem, obj_t type) {

   if (type && C_PRIM==type->pub.cls &&
       !strcmp(obj_name(type), "string")) {
      /*
       * Free the string too
       */
      char *s = *((char**)mem);
      if (s) free (s);
   }

   free (mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeDirectory
 *
 * Purpose:     Frees a directory struct.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Jul 25 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
browser_DBFreeDirectory (void *mem, obj_t type)
{
   DBdirectory  *dir = (DBdirectory *)mem;
   int          i;

   for (i=0; i<dir->nsyms; i++) {
      free (dir->toc[i].name);
   }
   free (dir->entry_ptr);
   free (dir->toc);
   free (dir);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeFacelist
 *
 * Purpose:     Frees a facelist struct
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              Friday, January 14, 2000
 *
 * Modifications:
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
static void
browser_DBFreeFacelist(void *mem, obj_t type)
{
    DBFreeFacelist((DBfacelist*)mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBFreeZonelist
 *
 * Purpose:     Frees a zonelist struct
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              Friday, January 14, 2000
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
static void
browser_DBFreeZonelist(void *mem, obj_t type)
{
    DBFreeZonelist((DBzonelist*)mem);
}

/*-------------------------------------------------------------------------
 * Function:    browser_DBFreePHZonelist
 *
 * Purpose:     Frees a phzonelist struct
 *
 * Return:      void
 *
 * Programmer:  Mark C. Miller 
 *              Wednesday, July 28, 2004
 *
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
static void
browser_DBFreePHZonelist(void *mem, obj_t type)
{
    DBFreePHZonelist((DBphzonelist*)mem);
}


/*-------------------------------------------------------------------------
 * Function:    browser_DBSaveVar
 *
 * Purpose:     Rewrites a changed variable to the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Apr  2 1997
 *
 * Modifications:
 *              Robb Matzke, 1999-07-15
 *              If the datatype is primitive 'string' then treat it as if it
 *              were a null terminated array of characters.
 *-------------------------------------------------------------------------
 */
static int
browser_DBSaveVar (obj_t _self, char *name, void *mem, obj_t type)
{
    obj_file_t     *self = MYCLASS(_self);
    int             dims[NDIMS], ndims;
    int             status, datatype;
    DBdatatype      silotype;
    DBfile         *dbfile = file_file(_self);

    if (self->rdonly)
    {
        out_errorn("file `%s' is read-only", self->name);
        return -1;
    }
    if (!type)
    {
        out_errorn("browser_DBSaveVar: cannot save %s (no type info)", name);
        return -1;
    }
    if (C_PRIM == type->pub.cls)
    {
        silotype = prim_silotype(type);
        if (C_PRIM == type->pub.cls)
        {
            silotype = prim_silotype(type);
            if (DB_NOTYPE == silotype && obj_name(type) &&
                !strcmp(obj_name(type), "string"))
            {
                /* A string */
                silotype = DB_CHAR;
                mem = *((char **)mem);
                dims[0] = strlen((char *)mem) + 1;
            } else if (DB_CHAR == silotype)
            {
                /* An array of characters (a.k.a., silo string) */
                mem = *((char **)mem);
                dims[0] = strlen((char *)mem) + 1;
            } else
            {
                dims[0] = 1;
            }
        }
        status = DBWrite(dbfile, name, mem, dims, 1, silotype);
        if (status < 0)
        {
            out_errorn("browser_DBSaveVar: cannot save %s", name);
            return -1;
        }
    } else
    {
        datatype = DBGetVarType(dbfile, name);
        ndims = DBGetVarDims(dbfile, name, NDIMS, dims);
        status = DBWrite(dbfile, name, mem, dims, ndims, datatype);
        if (status < 0)
        {
            out_errorn("browser_DBSaveVar: cannot save %s", name);
            return -1;
        }
    }

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    file_deref_DBdirectory
 *
 * Purpose:     Dereferences a directory.  If one says `foo.bar' where
 *              `foo' is a directory and `bar' is a member of the directory
 *              then the return value is the same as if one had said
 *              `foo/bar'.
 *
 * Return:      Success:        Ptr to an object in the directory.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.com
 *              Aug 25 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/*ARGSUSED*/
static obj_t
file_deref_DBdirectory (obj_t _self, int argc, obj_t argv[])
{
   DBdirectory  *dir;
   int          i;
   char         *name=NULL, buf[4096];
   obj_t        retval=NIL, tmp_argv[1];

   dir = sdo_mem (_self);
   name = obj_name (argv[0]);

   /*
    * Watch out for special names that might not appear in the
    * table of contents.
    */
   if (!strcmp(name, ".")) {
      return obj_copy (_self, SHALLOW);
   }

   /*
    * Now look for normal names.
    */
   for (i=0; i<dir->nsyms; i++) {
      if (!strcmp(dir->toc[i].name, name)) {
         sprintf (buf, "%s/%s", obj_name(_self), name);
         tmp_argv[0] = obj_new (C_STR, buf);
         retval = file_deref (sdo_file(_self), 1, tmp_argv);
         tmp_argv[0] = obj_dest (tmp_argv[0]);
         return retval;
      }
   }

   out_errorn ("not a directory member: %s", name);
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    file_deref
 *
 * Purpose:     Loads a silo object from a file.  SELF is the file and
 *              VARNAME is a symbol or variable name.
 *
 * Return:      Success:        PTR to a C_SDO silo data object.
 *
 *              Failure:        NIL
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 31 Jan 1997
 *      When reading a DBobject silo thingy, we do so with browser_DBGetObject
 *      which is a wrapper around the silo DBGetObject function that creates
 *      a data type that's more user friendly and will support assignments
 *      directly to the object.
 *
 *      Robb Matzke, 31 Jan 1997
 *      If `$lowlevel' is set and the object can't be loaded with a
 *      call to DBGetObject then we try to read the object as a miscellaneous
 *      silo variable.
 *
 *      Robb Matzke, 4 Feb 1997
 *      The prototype changed, but the functionality remains the same.
 *
 *      Robb Matzke, 21 Feb 1997
 *      A `savefunc' can be registered with the silo data object and that
 *      function is invoked if the object is dirty when it is destroyed.
 *
 *      Jeremy Meredith, Sept 21 1998
 *      Added support for multi-block material species.
 *
 *      Robb Matzke, 2000-05-23
 *      The documentation for DBGetToc() mentions that the returned
 *      pointer could be rendered invalid by calling any silo function.
 *      Therefore, we call DBGetToc() each time we call some silo function.
 *
 *      Robb Matzke, 2000-05-24
 *      Fixed bugs when dereferencing "/" in the file.
 *-------------------------------------------------------------------------
 */
static obj_t
file_deref (obj_t _self, int argc, obj_t argv[]) {

    obj_file_t  *self = MYCLASS(_self);
    char        *name=NULL, *orig=NULL, cwd[1024], fullname[1024];
    char        path[1024], *base=NULL, buf[1024];
    char        *typename=NULL;
    DBfile      *file=NULL;
    DBtoc       *toc=NULL;
    void        *r_mem=NULL;
    obj_t       type=NIL, retval=NIL, in=NIL, tmp=NIL;
    obj_t       varname=NIL;
    void        (*freefunc)(void*, obj_t)=NULL;
    void        *(*loadfunc)(DBfile*,char*)=NULL;
    int         (*savefunc)(obj_t,char*,void*,obj_t)=NULL;
    obj_t       (*dereffunc)(obj_t,int,obj_t[])=NULL;
    int         i, j, datatype, nelmts, ndims, dims[NDIMS];
    lex_t       *lex_in=NULL;
    DBobject    *obj=NULL;

    if (1!=argc) {
        out_errorn("file_deref: wrong number of arguments");
        return NIL;
    }
    varname = argv[0];

    assert(self && varname);
    assert(C_FILE==self->pub.cls);
    assert(C_SYM==varname->pub.cls || C_STR==varname->pub.cls);

    if (NULL==(name=orig=obj_name(varname))) {
        out_errorn ("file_deref: no variable name specified");
        return NIL;
    }
    file = self->f;

    /*
     * If the name doesn't start with `/' then create the full name.
     */
    if (DBGetDir(file, cwd)<0) return NIL;
    if ('/'!=name[0]) {
        if (!strcmp(cwd, "/")) {
            sprintf(fullname, "/%s", name);
        } else {
            sprintf(fullname, "%s/%s", cwd, name);
        }
        name = fullname;
    }


    /*
     * Split the name into a path and base name.
     */
    base = strrchr(name, '/');
    strncpy(path, name, base-name);
    path[base-name] = '\0';
    base++ ;    /*skip the `/'*/

    /*
     * If the name ended with a slash or it is "." or ".." then it must
     * be a directory.
     */
    if (!base || !base[0] || !strcmp(base, ".") || !strcmp(base, "..")) {
        if (NULL==(r_mem = browser_DBGetDirectory(file, name))) {
            return NIL;
        }
        savefunc = NULL;
        freefunc = browser_DBFreeDirectory;
        dereffunc = file_deref_DBdirectory;
        typename = "DBdirectory";
        goto done;
    }

    /*
     * Temporarily change to the specified directory so the table of
     * contents is loaded. According to DBGetToc() comments we should
     * request the table of contents each time we make a silo call.
     */
    if (DBSetDir(file, path[0]?path:"/")<0) return NIL;
    toc = DBGetToc(file);
    if (!toc) {
        out_errorn("file_deref: no table of contents for `%s' in `%s'",
                   path[0]?path:"/", self->name);
        goto error;
    }

    if (sym_bi_true("checksums"))
        DBSetEnableChecksums(1);
    else
        DBSetEnableChecksums(0);

    /*
     * If the user wants low-level information then get that instead of
     * the normal high-level stuff.
     */
    if (sym_bi_true("lowlevel")) {
        DBShowErrors(DB_SUSPEND, NULL);
        r_mem = browser_DBGetObject(file, base, &type);
        DBShowErrors(DB_RESUME, NULL);
        savefunc = browser_DBSaveObject;
        freefunc = browser_DBFreeObject;
        if (r_mem) goto done;
    }
    
    toc = DBGetToc(file); /*insure pointer is valid*/
    for (i=0; i<toc->ncurve; i++) {
        if (!strcmp(toc->curve_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetCurve;
            savefunc = NULL;
            freefunc = browser_DBFreeCurve;
            typename = "DBcurve";
            goto done;
        }
    }

    for (i=0; i<toc->ncsgmesh; i++) {
        if (!strcmp(toc->csgmesh_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetCsgmesh;
            savefunc = NULL;
            freefunc = browser_DBFreeCsgmesh;
            typename = "DBcsgmesh";
            goto done;
        }
    }

    for (i=0; i<toc->ncsgvar; i++) {
        if (!strcmp(toc->csgvar_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetCsgvar;
            savefunc = NULL;
            freefunc = browser_DBFreeCsgvar;
            typename = "DBcsgvar";
            goto done;
        }
    }

    for (i=0; i<toc->ndefvars; i++) {
        if (!strcmp(toc->defvars_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetDefvars;
            savefunc = NULL;
            freefunc = browser_DBFreeDefvars;
            typename = "DBdefvars";
            goto done;
        }
    }

    for (i=0; i<toc->nmultimesh; i++) {
        if (!strcmp(toc->multimesh_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetMultimesh;
            savefunc = NULL;
            freefunc = browser_DBFreeMultimesh;
            typename = "DBmultimesh";
            goto done;
        }
    }

    for (i=0; i<toc->nmultimeshadj; i++) {
        if (!strcmp(toc->multimeshadj_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))browser_DBGetMultimeshadj;
            savefunc = NULL;
            freefunc = browser_DBFreeMultimeshadj;
            typename = "DBmultimeshadj";
            goto done;
        }
    }

    for (i=0; i<toc->nmultivar; i++) {
        if (!strcmp(toc->multivar_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetMultivar;
            savefunc = NULL;
            freefunc = browser_DBFreeMultivar;
            typename = "DBmultivar";
            goto done;
        }
    }

    for (i=0; i<toc->nmultimat; i++) {
        if (!strcmp(toc->multimat_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetMultimat;
            savefunc = NULL;
            freefunc = browser_DBFreeMultimat;
            typename = "DBmultimat";
            goto done;
        }
    }

    for (i=0; i<toc->nmultimatspecies; i++) {
        if (!strcmp(toc->multimatspecies_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetMultimatspecies;
            savefunc = NULL;
            freefunc = browser_DBFreeMultimatspecies;
            typename = "DBmultimatspecies";
            goto done;
        }
    }

    for (i=0; i<toc->nqmesh; i++) {
        if (!strcmp(toc->qmesh_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetQuadmesh;
            savefunc = NULL;
            freefunc = browser_DBFreeQuadmesh;
            typename = "DBquadmesh";
            goto done;
        }
    }

    for (i=0; i<toc->nqvar; i++) {
        if (!strcmp(toc->qvar_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetQuadvar;
            savefunc = NULL;
            freefunc = browser_DBFreeQuadvar;
            typename = "DBquadvar";
            goto done;
        }
    }

    for (i=0; i<toc->nucdmesh; i++) {
        if (!strcmp(toc->ucdmesh_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetUcdmesh;
            savefunc = NULL;
            freefunc = browser_DBFreeUcdmesh;
            typename = "DBucdmesh";
            goto done;
        }
    }

    for (i=0; i<toc->nucdvar; i++) {
        if (!strcmp(toc->ucdvar_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetUcdvar;
            savefunc = NULL;
            freefunc = browser_DBFreeUcdvar;
            typename = "DBucdvar";
            goto done;
        }
    }

    for (i=0; i<toc->nptmesh; i++) {
        if (!strcmp(toc->ptmesh_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetPointmesh;
            savefunc = NULL;
            freefunc = browser_DBFreePointmesh;
            typename = "DBpointmesh";
            goto done;
        }
    }

    for (i=0; i<toc->nptvar; i++) {
        if (!strcmp(toc->ptvar_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetPointvar;
            savefunc = NULL;
            freefunc = browser_DBFreeMeshvar;
            typename = "DBmeshvar";
            goto done;
        }
    }

    for (i=0; i<toc->nmat; i++) {
        if (!strcmp(toc->mat_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetMaterial;
            savefunc = NULL;
            freefunc = browser_DBFreeMaterial;
            typename = "DBmaterial";
            goto done;
        }
    }

    for (i=0; i<toc->nmatspecies; i++) {
        if (!strcmp(toc->matspecies_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetMatspecies;
            savefunc = NULL;
            freefunc = browser_DBFreeMatspecies;
            typename = "DBmatspecies";
            goto done;
        }
    }

    for (i=0; i<toc->nmrgtree; i++) {
        if (!strcmp(toc->mrgtree_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetMrgtree;
            savefunc = NULL;
            freefunc = browser_DBFreeMrgtree;
            typename = "DBmrgtree";
            goto done;
        }
    }

    for (i=0; i<toc->ngroupelmap; i++) {
        if (!strcmp(toc->groupelmap_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetGroupelmap;
            savefunc = NULL;
            freefunc = browser_DBFreeGroupelmap;
            typename = "DBgroupelmap";
            goto done;
        }
    }

    for (i=0; i<toc->nmrgvar; i++) {
        if (!strcmp(toc->mrgvar_names[i], base)) {
            loadfunc = (void*(*)(DBfile*,char*))DBGetMrgvar;
            savefunc = NULL;
            freefunc = browser_DBFreeMrgvar;
            typename = "DBmrgvar";
            goto done;
        }
    }

    for (i=0; i<toc->narray; i++) {
        if (!strcmp(toc->array_names[i], base)) {
            if (Verbosity>=2) {
                out_info("file_deref: loading DBcompoundarray %s:%s",
                         self->name, name);
            }
            r_mem = browser_DBGetCompoundarray(file, base, &type);
            savefunc = NULL;
            freefunc = browser_DBFreeCompoundarray;
            if (r_mem) goto done;
        }
    }

    toc = DBGetToc(file); /*insure pointer is valid*/
    for (i=0; i<toc->ndir; i++) {
        if (!strcmp(toc->dir_names[i], base)) {
            if (Verbosity>=2) {
                out_info("file_deref: loading DBdirectory %s:%s",
                         self->name, name);
            }
            r_mem = browser_DBGetDirectory(file, base);
            toc = DBGetToc(file); /*insure pointer is valid*/
            savefunc = NULL;
            freefunc = browser_DBFreeDirectory;
            dereffunc = file_deref_DBdirectory;
            typename = "DBdirectory";
            if (r_mem) goto done;
        }
    }

    for (i=0; i<toc->nvar; i++) {
        if (!strcmp(toc->var_names[i], base)) {
            if (Verbosity>=2) {
                out_info("file_deref: loading variable %s:%s", self->name,
                         name);
            }
            datatype = DBGetVarType(file, base);
            nelmts   = DBGetVarLength(file, base);
            ndims    = DBGetVarDims(file, base, NDIMS, dims);
            r_mem    = DBGetVar(file, base);
            if (!r_mem) goto error;
            savefunc = browser_DBSaveVar;
            freefunc = browser_DBFreeVar;

            if (1==ndims && DB_CHAR==datatype) {
                /*
                 * Add a null terminator just to be sure.  Also make the data
                 * a pointer to a string instead of the string itself.
                 */
                char **s_ptr = malloc(sizeof(char*));
                *s_ptr = malloc(nelmts+1);
                memcpy(*s_ptr, r_mem, nelmts);
                (*s_ptr)[nelmts] = '\0';
                free(r_mem);
                r_mem = s_ptr;
                nelmts = 1;
                strcpy(buf, "primitive 'string'");

            } else if (1==nelmts) {
                /*
                 * Some other primitive type, not a string.
                 */
                sprintf(buf, "primitive %d", datatype);

            } else {
                /*
                 * An array of non-string primitive types.  We currently don't
                 * support an array of strings because silo overloads
                 * `DB_CHAR[]' to mean a string or an array of 8-bit integers.
                 */
                strcpy(buf, "array");
                for (j=0; j<ndims; j++) sprintf(buf+strlen(buf), " %d",
                                                dims[j]);
                sprintf(buf+strlen(buf), " (primitive %d)", datatype);
            }

            lex_in = lex_string(buf);
            typename = NULL;
            in = parse_stmt(lex_in, false);
            lex_in = lex_close(lex_in);
            type = obj_eval(in);
            in = obj_dest(in);
            goto done;
        }
        toc = DBGetToc(file); /*insure pointer is valid*/
    }

    /*
     * The object was not found.  If it's a component of some compound
     * array then print a warning message to that effect.
     */
    if (!strchr(orig, '/')) {
        for (i=0; i<toc->narray; i++) {
            DBcompoundarray *ca = DBGetCompoundarray(file,
                                                     toc->array_names[i]);
            assert(ca);
            for (j=0; ca && j<ca->nelems; j++) {
                if (!strcmp(ca->elemnames[j], orig)) {
                    if (r_mem) {
                        out_errorn("file_deref: `%s' is ambiguous", orig);
                        browser_DBFreeSubarray(r_mem, type);
                        r_mem = NULL;
                        type = obj_dest(type);
                        goto error;
                    } else {
                        r_mem = browser_DBGetSubarray(ca, j, &type);
                        assert(r_mem);
                        savefunc = NULL;
                        freefunc = browser_DBFreeSubarray;
                    }
                }
            }
            if (r_mem) goto done;
            toc = DBGetToc(file); /*insure pointer is valid*/
        }
    }

    /*
     * Some objects are special although silo doesn't have a dedicated list
     * for them in the table of contents.
     */
    if ((obj=fix_objdups(DBGetObject(file, base))) && obj->type) {
        if (!strcmp(obj->type, "facelist")) {
            loadfunc = (void*(*)(DBfile*, char*))DBGetFacelist;
            savefunc = NULL;
            freefunc = browser_DBFreeFacelist;
            typename = "DBfacelist";
            goto done;
        } else if (!strcmp(obj->type, "zonelist")) {
            loadfunc = (void*(*)(DBfile*, char*))DBGetZonelist;
            savefunc = NULL;
            freefunc = browser_DBFreeZonelist;
            typename = "DBzonelist";
            goto done;
        } else if (!strcmp(obj->type, "polyhedral-zonelist")) {
            loadfunc = (void*(*)(DBfile*, char*))DBGetPHZonelist;
            savefunc = NULL;
            freefunc = browser_DBFreePHZonelist;
            typename = "DBphzonelist";
            goto done;
        } else if (!strcmp(obj->type, "csgzonelist")) {
            loadfunc = (void*(*)(DBfile*, char*))DBGetCSGZonelist;
            savefunc = NULL;
            freefunc = browser_DBFreeCSGZonelist;
            typename = "DBcsgzonelist";
            goto done;
        } else if (!strcmp(obj->type, "edgelist")) {
            out_info("file_deref: edgelists are retrieved with DBGetObject() "
                     "because there is no DBGetEdgelist() function");
        }
    }

    /*
     * If all else fails then read the object as a DBObject (a low-level
     * PDB-like data structure).
     */
    DBShowErrors(DB_SUSPEND, NULL);
    r_mem = browser_DBGetObject(file, base, &type);
    typename = NULL;
    savefunc = browser_DBSaveObject;
    freefunc = browser_DBFreeObject;
    DBShowErrors(DB_RESUME, NULL);
    if (r_mem) goto done;

    out_errorn("file_deref: `%s' is not a database object in `%s'",
               name, self->name);

 error:
    if (obj) {
        DBFreeObject(obj);
        obj=NULL;
    }
    if (DBSetDir(file, cwd)<0) {
        out_errorn("file_deref: cannot restore cwd `%s' in `%s'",
                   cwd, self->name);
    }
    return NIL;

 done:
    assert(r_mem || loadfunc);

    if (obj) {
        DBFreeObject(obj);
        obj=NULL;
    }

    if (!r_mem) {
        if (Verbosity>=2) {
            out_info("file_deref: loading %s %s:%s",
                     typename?typename:"void type", self->name, name);
        }
        r_mem = (loadfunc)(file, name);
        if (!r_mem) goto error;
    }

    if (DBSetDir(file, cwd)<0) {
        out_errorn("file_deref: cannot restore cwd `%s' in `%s'",
                   cwd, self->name);
    }

    if (typename) {
        in = obj_new(C_SYM, typename);
        type = obj_copy(tmp=sym_vboundp(in), DEEP);
        tmp = obj_dest(tmp);
        in = obj_dest(in);
    }

    retval = obj_new(C_SDO, self, name, r_mem, type, r_mem, type,
                     freefunc, savefunc, dereffunc);
    type = obj_dest(type);

    /*
     * Bind the type quantities to actual values.
     */
    r_mem = sdo_mem(retval);
    type = sdo_typeof(retval);
    if (NIL==obj_bind(type, r_mem)) {
        out_error("file_deref: problems binding array dimensions", retval);
        retval = obj_dest(retval);
    }
    return retval;
}

/*---------------------------------------------------------------------------
 * Description: Determines if the SILO object named NAME matches one or
 *              more of the DiffOpt.exclude names from the $exclude
 *              variable.  NAME should be the full name of the object.
 *
 * Return:      Non-zero if NAME should be excluded; zero otherwise.
 *
 * Programmer:  Robb Matzke
 *              Thursday, June 29, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
static int
file_exclude(const char *cwd, const char *basename, int type)
{
    int                 i, exclude=false;
    char                fullname[1024];

    if (!DiffOpt.exclude.nused) return false;

    /* Construct full name */
    strcpy(fullname, cwd);
    strcat(fullname, basename);

    /* Look at each excluded name */
    for (i=0; i<DiffOpt.exclude.nused && !exclude; i++) {
        if (!strncmp(DiffOpt.exclude.value[i], "type:", 5)) {
            exclude = !strcmp(DiffOpt.exclude.value[i]+5, ObjTypeName[type]);
        } else {
#ifdef HAVE_FNMATCH
            if ('/'==DiffOpt.exclude.value[i][0]) {
                exclude = !fnmatch(DiffOpt.exclude.value[i], fullname, 0);
            } else {
                exclude = !fnmatch(DiffOpt.exclude.value[i], basename, 0);
            }
#else
            if ('/'==DiffOpt.exclude.value[i][0]) {
                exclude = !strcmp(DiffOpt.exclude.value[i], fullname);
            } else {
                exclude = !strcmp(DiffOpt.exclude.value[i], basename);
            }
#endif
        }
    }
    return exclude;
}

/*-------------------------------------------------------------------------
 * Function:    file_diff
 *
 * Purpose:     Compares file A and file B.
 *
 * Return:      Success:        0:      No differences
 *                              1:      Some differences
 *                              2:      Completely different.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb 18 1997
 *
 * Modifications:
 *              Robb Matkze, 2000-05-19
 *              It is possible for browser_DBGetToc() to return NULL if the
 *              specified directory is empty.
 *
 *              Robb Matzke, 2000-06-28
 *              Handles two-column output style.
 *-------------------------------------------------------------------------
 */
static int
file_diff (obj_t a, obj_t b)
{
    toc_t       *atoc=NULL, *btoc=NULL;
    DBfile      *afile, *bfile;
    int         i, j, an, bn, ndiff=0, status;
    obj_t       sym=NIL, aobj=NIL, bobj=NIL;
    out_t       *f = OUT_STDOUT;
    char        a_cwd[1024], b_cwd[1024];

    /* Get the table of contents for each file. */    
    afile = file_file(a);
    bfile = file_file(b);
    assert (afile && bfile);
    atoc = browser_DBGetToc(afile, &an, sort_toc_by_name);
    btoc = browser_DBGetToc(bfile, &bn, sort_toc_by_name);
    assert ((!an || atoc) && (!bn || btoc));

    /* Free any compression resources */
    DBFreeCompressionResources(afile, 0);
    DBFreeCompressionResources(bfile, 0);

    /* We may need to know our cwd below. */
    DBGetDir(afile, a_cwd);
    if (strcmp(a_cwd,"/")) strcat(a_cwd, "/");
    DBGetDir(bfile, b_cwd);
    if (strcmp(b_cwd,"/")) strcat(b_cwd, "/");

    for (i=j=ndiff=0; i<an || j<bn; i++,j++) {
        out_section(f);
        if (out_brokenpipe(f)) break;

        /* List the names of objects that appear only in A or
         * only in B. */
        while (i<an || j<bn) {
            while (i<an && (j>=bn || strcmp (atoc[i].name, btoc[j].name)<0)) {
                out_section(f);
                if (!file_exclude(a_cwd, atoc[i].name, atoc[i].type)) {
                    switch (DiffOpt.report) {
                    case DIFF_REP_ALL:
                    case DIFF_REP_BRIEF:
                        if (!DiffOpt.ignore_dels) {
                            ndiff++;
                            out_push(f, atoc[i].name);
                            out_puts(f, "appears only in file A");
                            out_nl(f);
                            out_pop(f);
                        }
                        break;
                    case DIFF_REP_SUMMARY:
                        return 1;
                    }
                } else if (Verbosity>=2) {
                    out_info("excluded %s%s", a_cwd, atoc[i].name);
                }
                i++;
            }

            while (j<bn && (i>=an || strcmp(btoc[j].name, atoc[i].name)<0)) {
                out_section(f);
                if (!file_exclude(b_cwd, btoc[j].name, btoc[j].type)) {
                    switch (DiffOpt.report) {
                    case DIFF_REP_ALL:
                        if (!DiffOpt.ignore_adds) {
                            ndiff++;
                            out_push(f, btoc[j].name);
                            if (DiffOpt.two_column) {
                                out_column(f, OUT_COL2, DIFF_SEPARATOR);
                            }
                            out_puts(f, "appears only in file B");
                            out_nl(f);
                            out_pop(f);
                        }
                        break;
                    case DIFF_REP_BRIEF:
                        if (!DiffOpt.ignore_adds) {
                            ndiff++;
                            out_push(f, btoc[j].name);
                            out_puts(f, "appears only in file B");
                            out_nl(f);
                            out_pop(f);
                        }
                        break;
                    case DIFF_REP_SUMMARY:
                        return 1;
                    }
                } else if (Verbosity>=2) {
                    out_info("excluded %s%s", b_cwd, btoc[i].name);
                }
                j++;
            }
            if (i<an && j<bn && !strcmp(atoc[i].name, btoc[j].name)) break;
        }

        if (i<an && BROWSER_DB_DIR==atoc[i].type &&
            j<bn && BROWSER_DB_DIR==btoc[j].type) {
            /* Diff two subdirectories. */
            out_section(f);
            assert (0==strcmp (atoc[i].name, btoc[j].name));
            if (!file_exclude(a_cwd, atoc[i].name, atoc[i].type) &&
                !file_exclude(b_cwd, btoc[j].name, btoc[j].type)) {
                if (DBSetDir (afile, atoc[i].name)<0) {
                    out_errorn ("file_diff: cannot cd to `%s:%s'",
                                obj_name(a), atoc[i].name);
                    ndiff++; /*assume different*/
                } else if (DBSetDir (bfile, btoc[j].name)<0) {
                    out_errorn ("file_diff: cannot cd to `%s:%s'",
                                obj_name(b), btoc[j].name);
                    status = DBSetDir (afile, "..");
                    assert (status>=0);
                    ndiff++; /*assume different*/
                } else {
                    out_push (f, atoc[i].name);
                    status = file_diff(a, b);
                    out_pop (f);
                    status = DBSetDir (afile, "..");
                    assert (status>=0);
                    status = DBSetDir (bfile, "..");
                    assert (status>=0);

                    if (status) {
                        ndiff++;
                        if (DIFF_REP_SUMMARY==DiffOpt.report) return 1;
                    }
                }
            } else if (Verbosity>=2) {
                out_info("excluded %s%s versus %s%s",
                         a_cwd, atoc[i].name, b_cwd, btoc[j].name);
            }
            
        } else if (i<an && j<bn) {
            /* Diff two objects. */            
            out_section(f);
            if (!file_exclude(a_cwd, atoc[i].name, atoc[i].type) &&
                !file_exclude(b_cwd, btoc[j].name, btoc[j].type)) {
                if (Verbosity>=1) {
                    char tmp[1024];
                    strcpy(tmp, "Differencing: ");
                    strcat(tmp, a_cwd);
                    strcat(tmp, atoc[i].name);
                    out_progress (tmp);
                }
                sym = obj_new (C_SYM, atoc[i].name);
                aobj = obj_deref (a, 1, &sym);
                bobj = obj_deref (b, 1, &sym);
                sym = obj_dest (sym);
                out_push (f, atoc[i].name);
                status = obj_diff (aobj, bobj);
                if (status) ndiff++;

                switch (DiffOpt.report) {
                case DIFF_REP_ALL:
                    if (2==status) {
                        out_line (f, "***************");
                        obj_print (aobj, f);
                        out_line (f, "---------------");
                        obj_print (bobj, f);
                        out_line (f, "***************");
                    }
                    break;
                case DIFF_REP_BRIEF:
                    if (2==status) {
                        out_puts(f, "different value(s)");
                        out_nl(f);
                    }
                    break;
                case DIFF_REP_SUMMARY:
                    if (status) {
                        out_pop(f);
                        aobj = obj_dest (aobj);
                        bobj = obj_dest (bobj);
                        out_progress(NULL);
                        return 1;
                    }
                    break;
                }

                out_pop(f);
                aobj = obj_dest (aobj);
                bobj = obj_dest (bobj);
            } else if (Verbosity>=2) {
                out_info("excluded %s%s versus %s%s",
                         a_cwd, atoc[i].name, b_cwd, btoc[j].name);
            }
        }
    }

    out_progress (NULL);
    return ndiff ? 1 : 0;
}


/*-------------------------------------------------------------------------
 * Function:    file_rdonly
 *
 * Purpose:     Determines if the file is read-only.
 *
 * Return:      Success:        0 if writable, 1 if read-only.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.com
 *              Jul 29 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
file_rdonly (obj_t _self)
{
   obj_file_t   *self = MYCLASS(_self);

   return self->rdonly;
}
