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
 * Created:             str.c
 *                      Dec  4 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             The string class
 *
 * Modifications:       
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include <browser.h>
#define MYCLASS(X)      ((obj_str_t*)(X))

typedef struct obj_str_t {
   obj_pub_t    pub;
   char         *s;
} obj_str_t;

class_t         C_STR;
static obj_t    str_new (va_list);
static obj_t    str_copy (obj_t, int);
static obj_t    str_dest (obj_t);
static void     str_print (obj_t, out_t*);
static char *   str_name (obj_t);
static int      str_diff (obj_t, obj_t);


/*-------------------------------------------------------------------------
 * Function:    str_class
 *
 * Purpose:     Initializes the STR class.
 *
 * Return:      Success:        Ptr to the str class.
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
class_t
str_class (void) {

   class_t      cls = calloc (1, sizeof(*cls));

   cls->name = strdup ("STR");
   cls->new = str_new;
   cls->copy = str_copy;
   cls->dest = str_dest;
   cls->print = str_print;
   cls->objname = str_name;
   cls->diff = str_diff;
   return cls;
}


/*-------------------------------------------------------------------------
 * Function:    str_new
 *
 * Purpose:     Creates a new STR object having the specified string value.
 *
 * Return:      Success:        Ptr to the new str object.
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
str_new (va_list ap) {

   char         *s;
   obj_str_t    *self = calloc (1, sizeof(obj_str_t));

   assert (self);
   s = va_arg (ap, char*);
   self->s = strdup (s);
   return (obj_t)self;
}


/*-------------------------------------------------------------------------
 * Function:    str_copy
 *
 * Purpose:     Copies a string.
 *
 * Return:      Success:        Ptr to a copy of SELF.
 *
 *              Failure:        abort()
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Jan 23 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
str_copy (obj_t _self, int flag) {

   obj_str_t    *self = MYCLASS(_self);
   obj_str_t    *retval = NULL;

   if (SHALLOW==flag) {
      retval = self;

   } else {
      retval = calloc (1, sizeof(obj_str_t));
      retval->s = strdup (self->s);
   }
   return (obj_t)retval;
}


/*-------------------------------------------------------------------------
 * Function:    str_dest
 *
 * Purpose:     Destroys a string object when the reference count becomes
 *              zero.
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
str_dest (obj_t _self) {

   obj_str_t    *self = MYCLASS(_self);

   if (0==self->pub.ref) {
      free (self->s);
      self->s = NULL;
   }
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    str_doprnt
 *
 * Purpose:     Prints a string with non-printable characters.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 31 1997
 *
 * Modifications:
 *              Robb Matzke, 2000-10-19
 *              Understands formats of `b16', `b8', and `b2'.
 *
 *              Mark C. Miller, 03Jun08
 *              Made it properly handle strings longer than size of buf.
 *              Made it properly use truncate variable to control behavior.
 *
 *    Mark C. Miller, Wed Jan 14 10:13:50 PST 2009
 *    Eliminated extraneous blank line being output from browser as well
 *    as line breaking occuring on arrays of strings by removing out_nl
 *    call and using out_puts for string output and out_printf otherwise.
 *-------------------------------------------------------------------------
 */
void
str_doprnt (out_t *f, char *fmt, char *s)
{
    char        buf[1024], c;
    int         at, i, j, n, maxn;
    unsigned    mask;

    if (!s) {
        out_puts(f, "(null)");
        return;
    }

    i = 0;
    n = strlen(s);
    maxn = sym_bi_true("truncate");
    if (maxn <= 1) maxn = n;
    while (i < n && i < maxn)
    {
       buf[0] = '\0';
       for (at=0; i < n && i < maxn && s[i]; i++) {
           if (at+2>=sizeof(buf)) {
               buf[at] = '\0';
               break;
           }

           switch ((c=s[i])) {
           case '\\':
               buf[at++] = c;
               buf[at++] = c;
               break;

           case '\b':
               /* `b' is a valid hexadecimal digit */
               if (!strcmp(fmt, "b16")) {
                   sprintf(buf+at, "\\%02x", (unsigned char)c);
                   at += strlen(buf+at);
               } else {
                   buf[at++] = '\\';
                   buf[at++] = 'b';
               }
               break;

           case '\n':
               buf[at++] = '\\';
               buf[at++] = 'n';
               break;

           case '\r':
               buf[at++] = '\\';
               buf[at++] = 'r';
               break;

           case '\t':
               buf[at++] = '\\';
               buf[at++] = 't';
               break;

           case '"':
               buf[at++] = '\\';
               buf[at++] = '"';
               break;
            
           default:
               if (c>=' ' && c<='~') {
                   buf[at++] = c;
               } else if (!strcmp(fmt, "b16")) {
                   sprintf(buf+at, "\\%02x", (unsigned char)c);
                   at += strlen(buf+at);
               } else if (!strcmp(fmt, "b2")) {
                   buf[at++] = '\\';
                   for (j=0, mask=0x80; j<8; j++, mask>>=1) {
                       buf[at++] = (unsigned)c & mask ? '1':'0';
                   }
               } else {
                   /*default is octal*/
                   sprintf(buf+at, "\\%03o", (unsigned char)c);
                   at += strlen(buf+at);
               }
               break;
           }
       }

       buf[at] = '\0';
       if (!strcmp(fmt, "b16") || !strcmp(fmt, "b8") || !strcmp(fmt, "b2"))
           out_puts(f, buf);
       else
           out_printf(f, fmt, buf);
    }
}
   

/*-------------------------------------------------------------------------
 * Function:    str_print
 *
 * Purpose:     Prints a string to the specified file.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *              Robb Matzke, 2000-06-02
 *              Uses the $fmt_string format.
 *
 *              Robb Matzke, 2000-10-23
 *              Looks at $obase
 *-------------------------------------------------------------------------
 */
static void
str_print(obj_t _self, out_t *f)
{
    obj_str_t   *self = MYCLASS(_self);
    char         *fmt;
    int         obase = sym_bi_true("obase");

    if (16==obase) {
        fmt = safe_strdup("b16");
    } else if (8==obase) {
        fmt = safe_strdup("b8");
    } else if (2==obase) {
        fmt = safe_strdup("b2");
    } else {
        fmt =  sym_bi_gets("fmt_string");
    }
   
    if (fmt && *fmt) {
        str_doprnt (f, fmt, self->s);
        free(fmt);
    } else {
        out_putw(f, self->s);
    }
}


/*-------------------------------------------------------------------------
 * Function:    str_name
 *
 * Purpose:     Returns a pointer to the string value.
 *
 * Return:      Success:        Ptr to string.
 *
 *              Failure:        Never fails.
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char *
str_name (obj_t _self) {

   return MYCLASS(_self)->s;
}


/*-------------------------------------------------------------------------
 * Function:    str_diff
 *
 * Purpose:     Compares two strings and reports differences.
 *
 * Return:      Success:        0:      same
 *                              2:      different
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb 18 1997
 *
 * Modifications:
 *              Robb Matzke, 2000-06-27
 *              Supports 2-column diff output style.
 *-------------------------------------------------------------------------
 */
static int
str_diff (obj_t _a, obj_t _b)
{
    obj_str_t   *a = MYCLASS(_a);
    obj_str_t   *b = MYCLASS(_b);
    out_t       *f = OUT_STDOUT;
    int         status = strcmp(a->s, b->s) ? 2 : 0;

    switch (DiffOpt.report) {
    case DIFF_REP_ALL:
        if (DiffOpt.two_column) {
            obj_print(_a, f);
            out_column(f, OUT_COL2, DIFF_SEPARATOR);
            obj_print(_b, f);
            out_nl(f);
            status = 1;
        }
        break;
    case DIFF_REP_BRIEF:
    case DIFF_REP_SUMMARY:
        break;
    }
    return status;
}
