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
/*
 * PDCONV.C - routines to do conversions between data formats
 *
 * Source Version: 9.0
 * Software Release #92-0043
 *
 */
#include <limits.h>
#include "pdb.h"
#include "pdform.h"

#define ONES_COMP_NEG(n, nb, incr)                                           \
    {if (nb == 8*sizeof(long))                                               \
        n = ~n + incr;                                                       \
     else                                                                    \
        {long msk;                                                           \
         msk = (1L << nb) - 1L;                                              \
         n   = (~n + incr) & msk;};}

/*
 * Forward declarations...
 */
static void             _PD_btrvout (char*,long,long);
static int              _PD_get_bit (char*,int,int,int*);
static void             _PD_insert_field (long,int,char*,int,int,int);
static void             _PD_ncopy (char**,char**,long,long);
static void             _PD_ones_complement (char*,long,int);
static void             _PD_reorder (char*,long,int,int*);
static void             _PD_set_bit (char*,int);
static void             _PD_sign_extend (char*,long,int,int,int*);

/*--------------------------------------------------------------------------*/
/*                             DATA_STANDARDS                               */
/*--------------------------------------------------------------------------*/

/*
 * Floating Point Format Descriptor
 *
 * -   format[0] = # of bits per number
 * -   format[1] = # of bits in exponent
 * -   format[2] = # of bits in mantissa
 * -   format[3] = start bit of sign
 * -   format[4] = start bit of exponent
 * -   format[5] = start bit of mantissa
 * -   format[6] = high order mantissa bit
 * -   format[7] = bias of exponent
 *
 */

int     lite_FORMAT_FIELDS = 8 ;

int     lite_def_float_order[]    = {1, 2, 3, 4};
int     lite_def_double_order[]   = {1, 2, 3, 4, 5, 6, 7, 8};
int     lite_ieee_float_order[]   = {1, 2, 3, 4};
int     lite_ieeea_double_order[] = {1, 2, 3, 4, 5, 6, 7, 8};
int     lite_ieeeb_double_order[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
int     lite_intel_float_order[]  = {4, 3, 2, 1};
int     lite_intel_double_order[] = {8, 7, 6, 5, 4, 3, 2, 1};
int     lite_vax_float_order[]    = {2, 1, 4, 3};
int     lite_vax_double_order[]   = {2, 1, 4, 3, 6, 5, 8, 7};
int     lite_cray_float_order[]   = {1, 2, 3, 4, 5, 6, 7, 8};

long    lite_def_float[]    = {32L,  8L, 23L, 0L, 1L,  9L, 0L,   0x7FL};
long    lite_def_double[]   = {64L, 11L, 52L, 0L, 1L, 12L, 0L,  0x3FFL};
long    lite_ieee_float[]   = {32L,  8L, 23L, 0L, 1L,  9L, 0L,   0x7FL};
long    lite_ieeea_double[] = {64L, 11L, 52L, 0L, 1L, 12L, 0L,  0x3FFL};
long    lite_ieeeb_double[] = {96L, 15L, 64L, 0L, 1L, 32L, 1L, 0x3FFEL};
long    lite_intel_float[]  = {32L,  8L, 23L, 0L, 1L,  9L, 0L,   0x7FL};
long    lite_intel_double[] = {64L, 11L, 52L, 0L, 1L, 12L, 0L,  0x3FFL};
long    lite_cray_float[]   = {64L, 15L, 48L, 0L, 1L, 16L, 1L, 0x4000L};

#ifdef GFLOAT
long    lite_vax_float[]    = {32L,  8L, 23L, 0L, 1L,  9L, 0L,   0x81L};
long    lite_vax_double[]   = {64L, 11L, 52L, 0L, 1L, 12L, 0L,  0x401L};
#else
long    lite_vax_float[]    = {32L,  8L, 23L, 0L, 1L,  9L, 0L,   0x81L};
long    lite_vax_double[]   = {64L,  8L, 55L, 0L, 1L,  9L, 0L,   0x81L};
#endif


/*
 * Data standard for the different architectures
 *
 *   struct s_data_standard
 *      {int ptr_bytes;
 *       int short_bytes;
 *       int short_order;
 *       int int_bytes;
 *       int int_order;
 *       int long_bytes;
 *       int long_order;
 *       int float_bytes;
 *       long *float_format;
 *       int *float_order;
 *       int double_bytes;
 *       long *double_format;
 *       int *double_order;};
 *
 *   typedef struct s_data_standard data_standard;
 *
 *   Mark C. Miller, Fri Nov 13 15:33:42 PST 2009
 *   Added support for long long datatype.
 */

data_standard
 lite_DEF_STD    = {4,                                    /* size of pointer */
                    2, NORMAL_ORDER,              /* size and order of short */
                    4, NORMAL_ORDER,                /* size and order of int */
                    4, NORMAL_ORDER,               /* size and order of long */
                    4, NORMAL_ORDER,          /* size and order of long long */
                    4, lite_def_float,                   /* float definition */
                    lite_def_float_order,
                    8, lite_def_double,                 /* double definition */
                    lite_def_double_order},
 lite_IEEEA_STD  = {4,                                    /* size of pointer */
                    2, NORMAL_ORDER,              /* size and order of short */
                    4, NORMAL_ORDER,                /* size and order of int */
                    4, NORMAL_ORDER,               /* size and order of long */
                    8, NORMAL_ORDER,          /* size and order of long long */
                    4, lite_ieee_float,                  /* float definition */
                    lite_ieee_float_order,
                    8, lite_ieeea_double,               /* double definition */
                    lite_ieeea_double_order},
 lite_IEEEB_STD  = {4,                                    /* size of pointer */
                    2, NORMAL_ORDER,              /* size and order of short */
                    2, NORMAL_ORDER,                /* size and order of int */
                    4, NORMAL_ORDER,               /* size and order of long */
                    4, NORMAL_ORDER,          /* size and order of long long */
                    4, lite_ieee_float,                  /* float definition */
                    lite_ieee_float_order,
                    12, lite_ieeeb_double,              /* double definition */
                    lite_ieeeb_double_order},
 lite_INTELA_STD = {4,                                    /* size of pointer */
                    2, REVERSE_ORDER,             /* size and order of short */
                    2, REVERSE_ORDER,               /* size and order of int */
                    4, REVERSE_ORDER,              /* size and order of long */
                    4, REVERSE_ORDER,         /* size and order of long long */
                    4, lite_intel_float,                 /* float definition */
                    lite_intel_float_order,
                    8, lite_intel_double,               /* double definition */
                    lite_intel_double_order},
 lite_VAX_STD    = {4,                                    /* size of pointer */
                    2, REVERSE_ORDER,             /* size and order of short */
                    4, REVERSE_ORDER,               /* size and order of int */
                    4, REVERSE_ORDER,              /* size and order of long */
                    4, REVERSE_ORDER,         /* size and order of long long */
                    4, lite_vax_float,                   /* float definition */
                    lite_vax_float_order,
                    8, lite_vax_double,                 /* double definition */
                    lite_vax_double_order},
 lite_CRAY_STD   = {8,                                    /* size of pointer */
                    8, NORMAL_ORDER,              /* size and order of short */
                    8, NORMAL_ORDER,                /* size and order of int */
                    8, NORMAL_ORDER,               /* size and order of long */
                    8, NORMAL_ORDER,          /* size and order of long long */
                    8, lite_cray_float,                  /* float definition */
                    lite_cray_float_order,
                    8, lite_cray_float,                 /* double definition */
                    lite_cray_float_order};


/*--------------------------------------------------------------------------*/
/*                            DATA_ALIGNMENTS                               */
/*--------------------------------------------------------------------------*/

/*
 * Word Alignment for the different architectures
 *
 *
 *   struct s_data_alignment
 *      {int char_alignment;
 *       int ptr_alignment;
 *       int short_alignment;
 *       int int_alignment;
 *       int long_alignment;
 *       int float_alignment;
 *       int double_alignment;
 *       int struct_alignment;};
 *
 *   typedef struct s_data_alignment data_alignment;
 *
 *   Mark C. Miller, Fri Nov 13 15:33:42 PST 2009
 *   Added support for long long datatype.
 */

/*
                                     long long
                                         |          
                                         |
                                         V
                          0  1  2  3  4  5  6  7  8
*/
data_alignment
 lite_RS6000_ALIGNMENT = {1, 4, 2, 4, 4, 8, 4, 4, 0},
 lite_SPARC_ALIGNMENT  = {1, 4, 2, 4, 4, 4, 4, 8, 0},
 lite_MIPS_ALIGNMENT   = {1, 4, 2, 4, 4, 8, 4, 8, 0},
 lite_M68000_ALIGNMENT = {1, 2, 2, 2, 2, 2, 2, 2, 0},
 lite_INTELA_ALIGNMENT = {1, 2, 2, 2, 2, 2, 2, 2, 0},
 lite_UNICOS_ALIGNMENT = {4, 8, 8, 8, 8, 8, 8, 8, 8},
 lite_DEF_ALIGNMENT    = {1, 4, 4, 4, 4, 4, 4, 4, 0};


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Function:    lite_PD_convert
 *
 * Purpose:     Convert from one machine format to another NITEMS of type
 *              TYPE from IN and put them in OUT.  ISTD and OSTD are the
 *              data format standards of IN and OUT respectively.
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:34 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_PD_convert (char **out, char **in, char *intype, char *outtype,
                 long nitems, data_standard *istd, data_standard *ostd,
                 data_standard *hstd, long *pin_offs, long *pout_offs,
                 HASHTAB *in_chart, HASHTAB *out_chart, int boffs, int error) {

   int ret = FALSE, tmp;
   long i, mitems, in_incr, out_incr;
   char *mtype;
   defstr *idp, *odp, *midp, *modp;
   memdes *desc;

   if (_lite_PD_indirection(outtype)) {
      in_incr     = _lite_PD_align(*pin_offs, intype, in_chart, &tmp);
      out_incr    = _lite_PD_align(*pout_offs, outtype, out_chart, &tmp);
      *pin_offs  += in_incr;
      *pout_offs += out_incr;
      *in        += istd->ptr_bytes + in_incr;
      *out       += ostd->ptr_bytes + out_incr;
      return(TRUE);
   }

   idp = PD_inquire_table_type(in_chart, intype);
   if (idp == NULL)
      lite_PD_error("BAD TYPE IN_CHART - PD_CONVERT", error);

   odp = PD_inquire_table_type(out_chart, outtype);
   if (odp == NULL)
      lite_PD_error("BAD TYPE OUT_CHART - PD_CONVERT", error);

   /*
    * If members is non-NULL then it is a derived type.
    */
   if (odp->members != NULL) {
      for (i = 0L; i < nitems; i++) {
         in_incr     = _lite_PD_align(*pin_offs, intype, in_chart, &tmp);
         out_incr    = _lite_PD_align(*pout_offs, outtype, out_chart, &tmp);
         *pin_offs  += in_incr;
         *pout_offs += out_incr;
         *in        += in_incr;
         *out       += out_incr;

         for (desc = odp->members; desc != NULL; desc = desc->next) {
            mitems   = desc->number;
            mtype    = desc->type;
            in_incr  = _lite_PD_align(*pin_offs, mtype, in_chart, &tmp);
            out_incr = _lite_PD_align(*pout_offs, mtype, out_chart, &tmp);

            /*
             * Increment the offsets to the alignments.
             */
            *pin_offs  += in_incr;
            *pout_offs += out_incr;
            *in        += in_incr;
            *out       += out_incr;

            if (_lite_PD_indirection(mtype)) {
               /*
                * Don't even think about converting pointers - they will be
                * recomputed. However, don't throw away information about
                * whether or not pointer is null.
                */
               if (!_lite_PD_null_pointer(*in, istd->ptr_bytes)) **out = 1;
               *in        += istd->ptr_bytes;
               *out       += ostd->ptr_bytes;
               *pin_offs  += istd->ptr_bytes;
               *pout_offs += ostd->ptr_bytes;
               ret         = TRUE;

            } else if (_lite_PD_prim_typep(desc->base_type, in_chart, error)) {
               /*
                * Check for direct primitives.
                */
               modp = PD_inquire_table_type(out_chart, mtype);
               if (modp == NULL)
                  lite_PD_error("BAD OUT TYPE IN STRUCT - PD_CONVERT", error);

               midp = PD_inquire_table_type(in_chart, mtype);
               if (midp == NULL)
                  lite_PD_error("BAD IN TYPE IN STRUCT - PD_CONVERT", error);

               ret = _lite_PD_convert((char **) out, in, mitems, boffs,
                                      midp, modp, hstd, pin_offs, pout_offs);

            } else {
               /*
                * Recurse for direct derived types.
                */
               ret = lite_PD_convert(out, in, mtype, mtype, mitems,
                                     istd, ostd, hstd, pin_offs, pout_offs,
                                     in_chart, out_chart, boffs, error);
            }

            if (!ret)
               lite_PD_error("STRUCT CONVERSION FAILED - PD_CONVERT",
                             error);
         }
      }

   } else {
      /*
       * if members is NULL then it is a primitive type.
       */
      ret = _lite_PD_convert((char **) out, in, nitems, boffs, idp, odp,
                             hstd, pin_offs, pout_offs);
      if (!ret)
         lite_PD_error("PRIMITIVE CONVERSION FAILED - PD_CONVERT", error);
   }

   return(ret);
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_null_pointer
 *
 * Purpose:     Does input contain only null bytes?
 *
 * Return:      Success:        TRUE
 *
 *              Failure:        FALSE
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:02 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_null_pointer (char *in, int bytes) {

   int i;

   for (i = 0; i < bytes; i++) {
      if (*in++) return(FALSE);
   }

   return(TRUE);
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_prim_typep
 *
 * Purpose:     Is the given struct member a primitive data type?
 *
 * Return:      Success:        TRUE
 *
 *              Failure:        FALSE
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:03 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_prim_typep (char *memb, HASHTAB *chrt, int error) {

   defstr *pd;

   pd = PD_inquire_table_type(chrt, memb);
   if (pd == NULL)
      lite_PD_error("BAD TYPE FROM STRUCTURE CHART - _PD_PRIM_TYPEP", error);

   return((pd->members == NULL));
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_convert
 *
 * Purpose:     Convert primitive types from one machine format to another.
 *              Guaranteed that there will be no indirects here. Convert
 *              NITEMS of type TYPE from IN and put them in OUT.  ISTD
 *              defines the data format of the data from IN.  PIN_OFFS and
 *              POUT_OFFS are pointers to external offset counters and
 *              they are incremented to account for data alignment.
 *
 * Return:      Success:        TRUE
 *
 *              Failure:        FALSE
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:58 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_convert (char **out, char **in, long nitems, int boffs,
                  defstr *idp, defstr *odp, data_standard *hstd,
                  long *pin_offs, long *pout_offs) {

   int l_order, l_bytes, in_bits, onescmp, unsgned, ret;
   long in_bytes, out_bytes;
   int in_flag, out_flag, *in_order, *out_order;
   long *in_format, *out_format;
   char *in_type, *out_type;

   in_type    = idp->type;
   out_type   = odp->type;
   in_bits    = idp->size_bits;
   in_bytes   = idp->size;
   out_bytes  = odp->size;
   in_order   = idp->order;
   out_order  = odp->order;
   in_flag    = idp->order_flag;
   out_flag   = odp->order_flag;
   in_format  = idp->format;
   out_format = odp->format;
   onescmp    = idp->onescmp;
   unsgned    = idp->unsgned;

   if ((strchr(in_type, '*') != NULL) || (strchr(out_type, '*') != NULL))
      return(FALSE);

   l_order = hstd->long_order;
   l_bytes = hstd->long_bytes;

   ret = TRUE;
   if ((in_format != NULL) && (out_format != NULL)) {
      /*
       * Handle floating point conversions.
       */
      _lite_PD_fconvert(out, in, nitems, boffs, in_format, in_order,
                        out_format, out_order, l_order, l_bytes, onescmp);

      *pin_offs  += in_bytes*nitems;
      *pout_offs += out_bytes*nitems;
   } else if (in_bits > 0) {
      /*
       * Handle integral bitstreams.
       */
      int ityp;

      if (out_bytes == sizeof(char)) ityp = SC_CHAR_I;
      else if (out_bytes == sizeof(short)) ityp = SC_SHORT_I;
      else if (out_bytes == sizeof(int)) ityp = SC_INTEGER_I;
      else if (out_bytes == sizeof(long)) ityp = SC_LONG_I;
      else return(FALSE);

      /*
       * Unpack the bitstream into a bytestream.
       */
      ret = _lite_PD_unp_bits(*out, *in, ityp, in_bits,
                              0, nitems, nitems, boffs);

      if (strcmp(in_type, lite_SC_CHAR_S) == 0) {
         /*
          * Convert characters.
          */
         _lite_PD_convert_ascii(*out, *in, nitems, in_bits, 0);

      } else {
         /*
          * Convert integers.
          */
         if (!unsgned)
            _PD_sign_extend(*out, nitems, out_bytes,
                            in_bits, out_order);

         *in  += in_bytes*nitems;
         *out += out_bytes*nitems;
         *pin_offs  += in_bytes*nitems;
         *pout_offs += out_bytes*nitems;
      }

   } else if ((in_flag != -1) && (out_flag != -1)) {
      /*
       * Handle integer conversions.
       */
      _lite_PD_iconvert(out, in, nitems,
                        (int) in_bytes, in_flag,
                        (int) out_bytes, out_flag, onescmp);

      *pin_offs  += in_bytes*nitems;
      *pout_offs += out_bytes*nitems;

   } else {
      /*
       * Handle character or unconverted types.
       */
      _PD_ncopy(out, in, nitems, in_bytes);

      *pin_offs  += in_bytes*nitems;
      *pout_offs += out_bytes*nitems;
   }

   return(ret);
}

/*--------------------------------------------------------------------------*/
/*                           ASCII CONVERSION ROUTINES                      */
/*--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_convert_ascii
 *
 * Purpose:     Translate characters fromBCPI bit ASCII to 8-bit ASCII.
 *              This is only here to allow PDB to read old machine data
 *              which may not be 8-bit ASCII.
 *
 * Inputs:      IN:     Input buffer.
 *              NITEMS: Number of fields to translate.
 *              BCPI:   Bits per character on input.
 *              OFFS:   Zero-origin bit offset from IN to start of data.
 *
 * Output:      OUT:    Output buffer.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:05 PM EST
 *
 * Modifications:
 *      Sean Ahern, Wed Apr 12 10:44:26 PDT 2000
 *      Put in an ARGSUSED comment to prevent compiler warnings.  I don't want
 *      to remove the parameters, since I don't know how this is called.
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
void
_lite_PD_convert_ascii (char *out, char *in, long nitems, int bpci, int offs) {

   long i;

   /*
    * Stretch input characters into 8 bits right-justified
    * with left bits 0.
    */

   /* translate chars by adding a blank (0x20) character
    * to give upper-case letters then or-ing the sum with a blank
    * to give lower-case letters
    */
   for (i = 0; i < nitems; i++) out[i] = (out[i] + ' ') | ' ';
}

/*--------------------------------------------------------------------------*/
/*                      INTEGER CONVERSION ROUTINES                         */
/*--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_iconvert
 *
 * Purpose:     Convert integers of nbi bytes to integers of nbo bytes.
 *              The number of bytes for each integer are give.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:14 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
_lite_PD_iconvert (char **out, char **in, long nitems, long nbi, int ordi,
                   long nbo, int ordo, int onescmp) {

   long i;
   int j;
   char *lout, *lin, *po, *pi;

   lin = *in;
   lout = *out;

   /*
    * Convert nitems integers.
    * test sign bit to properly convert negative integers
    */
   if (nbi < nbo) {
      if (ordi == REVERSE_ORDER) {
         for (j = nbi; j < nbo; j++) {
            po = lout + j - nbi;
            pi = lin + nbi - 1;
            for (i = 0L; i < nitems; i++) {
               *po = (*pi & 0x80) ? 0xff : 0;
               po += nbo;
               pi += nbi;
            }
         }
         for (j = nbi; j > 0; j--) {
            po = lout + nbo - j;
            pi = lin + j - 1;
            for (i = 0L; i < nitems; i++) {
               *po = *pi;
               po += nbo;
               pi += nbi;
            }
         }
      } else {
         for (j = nbi; j < nbo; j++) {
            po = lout + j - nbi;
            pi = lin;
            for (i = 0L; i < nitems; i++) {
               *po = (*pi & 0x80) ? 0xff : 0;
               po += nbo;
               pi += nbi;
            }
         }
         for (j = 0; j < nbi; j++) {
            po = lout + j + nbo - nbi;
            pi = lin + j;
            for (i = 0L; i < nitems; i++) {
               *po = *pi;
               po += nbo;
               pi += nbi;
            }
         }
      }

   } else if (nbi >= nbo) {
      if (ordi == REVERSE_ORDER) {
         for (j = nbo; j > 0; j--) {
            po = lout + nbo - j;
            pi = lin + j - 1;
            for (i = 0L; i < nitems; i++) {
               *po = *pi;
               po += nbo;
               pi += nbi;
            }
         }
      } else {
         for (j = nbi - nbo; j < nbi; j++) {
            po = lout + j - nbi + nbo;
            pi = lin + j;
            for (i = 0L; i < nitems; i++) {
               *po = *pi;
               po += nbo;
               pi += nbi;
            }
         }
      }
   }

   /*
    * If the input used ones complement arithmetic
    * convert to twos complement.
    */
   if (onescmp) _PD_ones_complement(*out, nitems, nbo);

   if (ordo == REVERSE_ORDER) _PD_btrvout(*out, nbo, nitems);

   *in  += nitems*nbi;
   *out += nitems*nbo;
}

/*--------------------------------------------------------------------------*/
/*                       FLOAT CONVERSION ROUTINES                          */
/*--------------------------------------------------------------------------*/

/* Parametrized Data Conversion Method
 *
 * Floating point formats are characterized by a set of parameters which
 * describe the fundamental elements of a floating point number. These are
 *
 *  Sign     - always assumed to be a single bit
 *           - requires bit offset
 *  Exponent - assumed to be a biased integer smaller than 32 bits
 *           - (this allows the conversion to use a long on all known
 *           - platforms - an exponent greater than 32 bits long would
 *           - allow much larger numbers than should be needed for
 *           - scientific computations)
 *           - requires a bit offset, a bit length, and a bias
 * Mantissa  - assumed to be a bitstream of arbitrary length
 *           - requires a bit offset and a bit length
 * HMB       - in all floating point representations the mantissa is
 *           - normalized so that the most significant bit is one.
 *           - in some formats the one is explicitly included in the
 *           - representation and in others it is only implicit
 *           - this gives some formats an extra bit of precision.
 *           - requires a flag which is TRUE if the HMB is explicit
 *
 * Two other factors involved are: the byte order which could be
 * mixed with the bit layout of the numbers but isn't in actual practice
 * on current machines; and whether one's complement or two's complement
 * arithmetic is used. Modern machines all use two's complement arithmetic
 * and the model used here and now is that data from one's complement
 * machines is to be read only.  This restriction is relatively easy
 * to relax, but there is no evidence that it should be.
 *
 * An issue which is not a problem in the current implementation is that
 * old machines with byte sizes other than 8 bits can be accomodated
 * because the conversions treat the input and output as bitstreams
 * instead of bytestreams.
 *
 * The conversion process is summarized as follows:
 *   1) Extract the sign bit and exponent field from the input number
 *   2) Subtract the bias of the source format and add the bias
 *      of the target format
 *   3) Check for overflow in the exponent
 *   4) Insert the new exponent and the sign bit in the target stream
 *   5) Copy the mantissa bits from the source to the target
 *      compensating for differences in the HMB between the two
 *      formats
 *   6) Take care of any known anomalies - e.g. CRAY format is
 *      inconsistent in that the HMB is explicitly on for all numbers
 *      with the exception of 0.0
 *   7) Reorder the bytes of the target stream appropriately
 *
 * The floating point formats for a variety of platforms are supplied by
 * PDBLib and are defined at the top of this file
 *
 */


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_fconvert
 *
 * Purpose:     General floating point conversion routine.  Convert from
 *              floating point format specified by infor to format
 *              specified by outfor.
 *
 *              floating point format specification:
 *
 *                format[0] = # of bits per number
 *                format[1] = # of bits in exponent
 *                format[2] = # of bits in mantissa
 *                format[3] = start bit of sign
 *                format[4] = start bit of exponent
 *                format[5] = start bit of mantissa
 *                format[6] = high order mantissa bit (CRAY needs this)
 *                format[7] = bias of exponent
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:08 PM EST
 *
 * Modifications:
 *      Sean Ahern, Fri Mar  2 09:39:53 PST 2001
 *      Added a logic path suggested by Dan Schikore for when the formats
 *      are the same.
 *
 *      Sean Ahern, Fri Mar  2 09:40:15 PST 2001
 *      Reformatted some of the code.
 *
 *-------------------------------------------------------------------------*/
void
_lite_PD_fconvert (char **out, char **in, long nitems, int boffs, long *infor,
                   int *inord, long *outfor, int *outord, int l_order,
                   int l_bytes, int onescmp) {

   long i, expn, expn_max, hexpn, mant, DeltaBias, hmbo, hmbi;
   int nbits, inbytes, outbytes, sign;
   int indxin, indxout, inrem, outrem, dindx;
   int bi_sign, bo_sign, bi_exp, bo_exp, bi_mant, bo_mant;
   int nbi_exp, nbo_exp, nbi, nbo;
   char *lout, *lin;
   unsigned char *rout;
   static int BitsMax = 8*sizeof(long);

   nbi     = infor[0];
   nbo     = outfor[0];
   nbi_exp = infor[1];
   nbo_exp = outfor[1];

   bi_sign = infor[3] + boffs;
   bo_sign = outfor[3];
   bi_exp  = infor[4] + boffs;
   bo_exp  = outfor[4];
   bi_mant = infor[5] + boffs;
   bo_mant = outfor[5];

   hmbo    = (outfor[6] & 1L);
   hmbi    = (infor[6] & 1L);

   inbytes   = (nbi + 7) >> 3;
   outbytes  = (nbo + 7) >> 3;
   DeltaBias = outfor[7] + hmbo - infor[7] - hmbi;
   hexpn     = 1L << (outfor[1] - 1L);
   expn_max  = (1L << outfor[1]) - 1L;

    if ( (inord[0] != outord[0]) ||
         (infor[0] != outfor[0]) || (infor[1] != outfor[1]) ||
         (infor[2] != outfor[2]) || (infor[3] != outfor[3]) ||
         (infor[4] != outfor[4]) || (infor[5] != outfor[5]) ||
         (infor[6] != outfor[6]) || (infor[7] != outfor[7]) )
   {
       /*
        * Zero out the output buffer.
        */
       memset(*out, 0, nitems*outbytes);

       lout = *out;
       lin  = *in;
       for (i = 0L; i < nitems; i++) {
          /*
           * Move the exponent over.
           */
          expn = _lite_PD_extract_field(lin, bi_exp, nbi_exp, inbytes, inord);
          sign = _PD_get_bit(lin, bi_sign, inbytes, inord);

          /*
           * If we have a negative number and ones complement arithmetic on the
           * input side (won't have it on the output side with modern data)
           * take the complement of the exponent and mantissa.
           */
          if (onescmp) {
             if (sign) {
                ONES_COMP_NEG(expn, nbi_exp, 1L);
             } else {
                expn += (expn < hexpn);
             }
          }

          if (expn != 0) expn += DeltaBias;

          if ((0 <= expn) && (expn < expn_max)) {
             _PD_insert_field(expn, nbo_exp, lout, bo_exp,
                              l_order, l_bytes);

             if (sign) _PD_set_bit(lout, bo_sign);

             indxin  = bi_mant;
             inrem   = infor[2];
             indxout = bo_mant;
             outrem  = outfor[2];

             dindx = hmbo - hmbi;
             if (dindx > 0) {
                /*
                 * If input high mantissa bit (HMB) is assumed 1 and not written
                 * (e.g. IEEE) but output HMB is assumed 0 (e.g. CRAY) write the
                 * input starting at the output HMB+1 and set the HMB.
                 */
                _PD_set_bit(lout, indxout);
                indxout += dindx;
                outrem  -= dindx;
             } else if (dindx < 0) {
                /*
                 * If input HMB is assumed 0 (e.g. CRAY) but output HMB is assumed
                 * 1 and not written (e.g. IEEE) take the input from HMB+1 and
                 * write it to output HMB.
                 */
                indxin -= dindx;
                inrem  += dindx;
             }

             /*
              * Move the mantissa over in sizeof(long) packets.
              */
             while ((inrem > 0) && (outrem > 0)) {
                nbits = MIN(BitsMax, inrem);
                nbits = MIN(nbits, outrem);
                mant  = _lite_PD_extract_field(lin, indxin, nbits,
                                               inbytes, inord);

                /*
                 * Do complement for negative ones complement data.
                 */
                if (onescmp && sign) ONES_COMP_NEG(mant, nbits, 0L);

                _PD_insert_field(mant, nbits, lout, indxout,
                                 l_order, l_bytes);

                indxin  += nbits;
                indxout += nbits;
                inrem   -= nbits;
                outrem  -= nbits;
             }
          } else if (expn_max <= expn) {
             /*
              * In case of overflow use 1.0e+(expn_max)
              */
             _PD_insert_field(expn_max, nbo_exp, lout, bo_exp,
                              l_order, l_bytes);

             if (_PD_get_bit(lin, bi_sign, inbytes, inord))
                _PD_set_bit(lout, bo_sign);
          }

          bi_sign += nbi;
          bi_exp  += nbi;
          bi_mant += nbi;
          bo_sign += nbo;
          bo_exp  += nbo;
          bo_mant += nbo;
       }

       /*
        * Handle CRAY inconsistency which has zero as the only floating point
        * number with a 0 in the HMB. Also problem for IEEE 96 bit float.
        * Fixed by Dave Munro
        */
       if (hmbo) {
          int j, mask = (1 << (7 - bo_mant % 8));

          indxout = outfor[5]/8;
          rout    = (unsigned char *) *out;
          for (i = 0L; i < nitems; i++, rout += outbytes) {
             for (j = 0; j < outbytes; j++) {
                if ((j == indxout) ? (rout[j] != mask) : rout[j]) break;
             }
             if (j == outbytes) rout[indxout] = 0;
          }
       }
    }
    else
       memcpy(*out, *in, nitems*outbytes);

   /*
    * Put the output bytes into the specified order.
    */
    for (i=0; i<outbytes; i++)
       if (inord[i] != outord[i])
          break;
    if (i != outbytes)
       _PD_reorder(*out, nitems, outbytes, outord);

   *in  += nitems*inbytes;
   *out += nitems*outbytes;
}

/*--------------------------------------------------------------------------*/
/*                             HELPER ROUTINES                              */
/*--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Function:    _PD_ncopy
 *
 * Purpose:     Copy the NITEMS of size BYTEPITEM from IN to OUT
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:22 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_ncopy (char **out, char **in, long nitems, long bytepitem) {

   long nbytes;

   nbytes = nitems*bytepitem;
   memcpy(*out, *in, nbytes);

   *in  += nbytes;
   *out += nbytes;
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_unp_bits
 *
 * Purpose:     Unpack an array that contains a bitstream.  Arguments are
 *
 *              ITYP    the target type of the data when unpacked
 *              NBITS   the number of bits per item
 *              PADSZ   the number of bits of pad preceding the fields
 *              FPP     the number of fields per pad
 *              NITEMS  the number of items expected
 *              OFFS    the bit offset of the first pad
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:26 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_unp_bits (char *out, char *in, int ityp, int nbits, int padsz,
                   int fpp, long nitems, long offs) {

   long i, bita, fld, np, *pl;
   char *pc;
   int *pi;
   short *ps;

   switch (ityp) {
   case SC_CHAR_I :
      pc = (char *) out;
      break;
   case SC_SHORT_I :
      ps = (short *) out;
      break;
   case SC_INTEGER_I :
      pi = (int *) out;
      break;
   case SC_LONG_I :
      pl = (long *) out;
      break;};

   for (i = 0L; i < nitems; i++) {
      np   = 1 + i/fpp;
      bita = np*padsz + i*nbits + offs;
      fld  = _lite_PD_extract_field(in, bita, nbits, INT_MAX, NULL);

      switch (ityp) {
      case SC_CHAR_I :
         pc[i] = (char) fld;
         break;
      case SC_SHORT_I :
         ps[i] = (short) fld;
         break;
      case SC_INTEGER_I :
         pi[i] = (int) fld;
         break;
      case SC_LONG_I :
         pl[i] = (long) fld;
         break;
      }
   }

   return(TRUE);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_sign_extend
 *
 * Purpose:     Sign extend integers which may come from odd bit
 *              sized sources.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:23 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_sign_extend (char *out, long nitems, int nbo, int nbti, int *ord) {

   int i, j, sba, mask, sign, indx;
   unsigned char *lout;

   sba = 8*nbo - nbti;

   /*
    * If sba is less than zero we have truncated the integers and really
    * don't know about the sign.
    */
   if (sba < 0) return;

   lout = (unsigned char *) out;
   for (i = 0L; i < nitems; i++) {
      sign = _PD_get_bit((char *) lout, sba, nbo, ord);
      if (sign) {
         for (j = 0; (sba > 8); j++) {
            indx = (ord != NULL) ? ord[j] : j;
            lout[indx] = 0xFF;
         }

         mask = ((1 << sba) - 1) << nbti;
         indx = (ord != NULL) ? ord[j] : j;
         lout[indx] |= mask;
      }
      lout += nbo;
   }
}


/*-------------------------------------------------------------------------
 * Function:    _PD_ones_complement
 *
 * Purpose:     Convert ones complement integers to twos complement.
 *              Note: modern machines use two's complement arithmetic
 *                    and therefore this is a one way conversion.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:40 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_ones_complement (char *out, long nitems, int nbo) {

   int i, j;
   unsigned int carry;
   signed char *lout;

   lout = (signed char *) out;
   for (i = 0L; i < nitems; i++) {
      if (*lout < 0) {
         carry = 1;
         for (j = nbo-1; (j >= 0) && (carry > 0); j--) {
            carry  += lout[j];
            lout[j] = carry & 0xFF;
            carry   = (carry > 0xFF);
         }
      }

      lout += nbo;
   }
}


/*-------------------------------------------------------------------------
 * Function:    _PD_btrvout
 *
 * Purpose:     Byte reverse nitems words each word is nb bytes long
 *              where nb is even.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:31 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_btrvout (char *out, long nb, long nitems) {

   long i, jl, jh, nbo2;
   char tmp;
   char *p1, *p2;

   nbo2 = nb >> 1;
   for (jl = 0; jl < nbo2; jl++) {
      jh = nb - jl - 1;
      p1 = out + jh;
      p2 = out + jl;
      for (i = 0L; i < nitems; i++) {
         tmp = *p1;
         *p1 = *p2;
         *p2 = tmp;
         p1 += nb;
         p2 += nb;
      }
   }
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_extract_field
 *
 * Purpose:     Make a copy of the bit field specified by the starting
 *              bit OFFS and the number of bits NBI from the byte array
 *              pointed to by IN.  All indexing is zero based.  The copy
 *              is to be put in a long and returned.  This imposes a 32
 *              bit limit (minimum) so repeated calls must be made for
 *              longer fields.
 *
 * Return:
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:32 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
long
_lite_PD_extract_field (char *in, int offs, int nbi, int nby, int *ord) {

   long n, bit_field;
   int offy, tgt, ind;
   unsigned char mask, bpb;

   bit_field = 0L;

   /*
    * Move past the apropriate number of bytes so that the start bit is
    * in the first byte.  OFFY is the offset of the byte containing the
    * bit OFFS.
    */
   n    = offs >> 3;
   offy = n % nby;
   n   -= offy;
   offs = offs % 8;

   /*
    * Advance the pointer past the unneeded items.
    */
   in += n;

   bpb  = 8 - offs;
   if (ord == NULL) {
      ind = offy++;
   } else {
      if (offy >= nby) {
         offy -= nby;
         in += nby;
      }
      ind = (ord[offy++] - 1);
   }

   tgt  = in[ind];
   mask = (1 << bpb) - 1;
   bit_field = ((bit_field << bpb) | (tgt & mask));
   nbi -= bpb;
   if (nbi < 0) {
      bit_field = bit_field >> (-nbi);
   } else {
      for (; nbi > 0; nbi -= bpb) {
         if (ord == NULL) {
            ind = offy++;
         } else {
            if (offy >= nby) {
               offy -= nby;
               in += nby;
            }
            ind = (ord[offy++] - 1);
         }

         tgt  = in[ind];
         bpb  = MIN(nbi, 8);
         mask = (1 << bpb) - 1;
         bit_field = ((bit_field << bpb) |
                      ((tgt >> (8 - bpb)) & mask));
      }
   }
   return(bit_field);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_insert_field
 *
 * Purpose:     Copy the least significatn NB bits from the given long
 *              into the byte array pointed to by OUT.  All indexint
 *              is zero based. OFFS is the offst from the beginning of
 *              OUT in bits.  This assumes that the output bit array is
 *              initialized to all zeros after offs.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:35 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_insert_field (long in_long, int nb, char *out, int offs,
                  int l_order, int l_bytes) {

   int mi, n, dm;
   long longmask;
   static int MaxBits = 8*sizeof(long);
   unsigned char fb;
   char *in;

   in = (char *) &in_long;

   /*
    * If the output start bit is not in the first byte move past the apropriate
    * number of bytes so that the start bit is in the first byte.
    */
   if (offs > 7) {
      out  += (offs >> 3);
      offs %= 8;
   }

   /*
    * If mi is less than offs, copy the first dm bits over, reset offs to 0,
    * advance mi by dm, and handle the rest as if mi >= offs.
    */
   mi = MaxBits - nb;
   if (mi < offs) {
      dm = MaxBits - (8 - offs);
      if (nb == MaxBits) {
         longmask = ~((1L << dm) - 1L);
      } else {
         longmask = ((1L << nb) - 1L) ^ ((1L << dm) - 1L);
      }

      fb = ((in_long & longmask) >> dm) & ((1L << (nb - dm)) - 1L);
      *(out++) |= fb;

      mi  += 8 - offs;
      offs = 0;
   }

   /*
    * Assuming mi >= offs, left shift the input so that it is bit aligned
    * with the output.
    */
   dm = mi - offs;
   longmask = ~((1L << dm) - 1L);
   in_long  = (in_long << dm) & longmask;

   /*
    * Reorder the bytes apropriately.
    */
   if (l_order == REVERSE_ORDER) _PD_btrvout(in, l_bytes, 1L);

   /*
    * Copy the remaining aligned bytes over.
    */
   for (n = (offs+nb+7)/8; n > 0; n--, *(out++) |= *(in++)) /*void*/ ;
}


/*-------------------------------------------------------------------------
 * Function:    _PD_set_bit
 *
 * Purpose:     Set the bit specified as o offset from the given pointer.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:37 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_set_bit (char *base, int offs) {

   int nbytes, mask;

   nbytes  = offs >> 3;
   base   += nbytes;
   offs   -= 8*nbytes;

   mask    = (1 << (7 - offs));
   *base  |= mask;
}


/*-------------------------------------------------------------------------
 * Function:    _PD_get_bit
 *
 * Purpose:     Return the bit specified as on offst from the given ptr.
 *
 * Return:      Success:        0 or 1
 *
 *              Failure:
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:38 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
_PD_get_bit (char *base, int offs, int nby, int *ord) {

   int nbytes, mask, n;

   n      = offs >> 3;
   nbytes = n % nby;
   n     -= nbytes;
   offs   = offs % 8;

   if (ord == NULL) base += (n + nbytes);
   else base += (n + (ord[nbytes] - 1));

   mask = (1 << (7 - offs));

   return((*base & mask) != 0);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_reorder
 *
 * Purpose:     Given a pointer to an array ARR with NITEMS of NBYTES each
 *              put them in the order defined by ORD.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 12:39 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_reorder (char *arr, long nitems, int nbytes, int *ord) {

   char local[MAXLINE];
   int j;

   for (; nitems > 0; nitems--) {
      arr--;
      for (j = 0; j < nbytes; local[j] = arr[ord[j]], j++) /*void*/ ;
      arr++;
      for (j = 0; j < nbytes; *(arr++) = local[j++]) /*void*/ ;
   }
}
