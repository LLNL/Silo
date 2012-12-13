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
 * PDRDWR.C - new read/write routines for PDBLib
 *
 * Source Version: 9.0
 * Software Release #92-0043
 *
 */
#include "config.h" /* For a possible redefinition of setjmp/longjmp */
#if !defined(_WIN32)
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif
#include "pdb.h"

#define DONE         1
#define LEAF         2
#define LEAF_ITEM    3
#define LEAF_RET     4
#define LEAF_INDIR   5
#define INDIRECT     6
#define INDIR_ITEM   7
#define INDIR_RET    8
#define BLOCK        9
#define BLOCK_ITEM  10
#define BLOCK_RET   11
#define SKIP_TO     12
#define SKIP_RET    13

#define SAVE_S(s, t)                                                         \
    {str_stack[str_ptr++] = s;                                               \
     s = lite_SC_strsavef(t, "char*:SAVE_S:t");}
#define RESTORE_S(s)                                                         \
    {SFREE(s);                                                               \
     s = str_stack[--str_ptr];}

#define SAVE_I(val)     lval_stack[lval_ptr++].diskaddr = (long) val;
#define RESTORE_I(val)  val = lval_stack[--lval_ptr].diskaddr;

#define SAVE_P(val)           lval_stack[lval_ptr++].memaddr = (char *) val;
#define RESTORE_P(type, val)  val = (type *) lval_stack[--lval_ptr].memaddr;

#define SET_CONT(ret)                                                        \
   {call_stack[call_ptr++] = ret;                                            \
    dst = _lite_PD_indirection(litype) ? INDIRECT : LEAF;                         \
    continue;}

#define SET_CONT_RD(ret, branch)                                             \
   {call_stack[call_ptr++] = ret;                                            \
    dst = branch;                                                            \
    continue;}

#define GO_CONT                                                              \
   {dst = call_stack[--call_ptr];                                            \
    continue;}

#define GO(lbl)                                                              \
    {dst = lbl;                                                              \
     continue;}


static long             call_ptr = 0L ;
static long             lval_ptr = 0L ;
static long             str_ptr = 0L ;
static long             call_stack[1000] ;
static SC_address       lval_stack[1000] ;
static char             *str_stack[1000] ;

static dimind *         _PD_compute_hyper_strides (PDBfile*,char*,dimdes*,
                                                       int*) ;
static void             _PD_effective_addr (long*,long*,long,symblock*) ;
static int              _PD_rd_hyper_index (PDBfile*,syment*,char*,dimind*,
                                                char*,char*,long,symblock*,int,
                                                int) ;
static int              _PD_rd_ind_tags (PDBfile*,char**,PD_itag*) ;
static void             _PD_rd_leaf_members (PDBfile*,char*,long,char*,
                                                 char*,int) ;
static int              _PD_read_hyper_space (PDBfile*,syment*,char*,char*,
                                                  char*,symblock*,int,int,long,
                                                  long,long) ;
static char *           _PD_wr_hyper_index (PDBfile*,char*,dimind*,char*,
                                            char*,long,symblock*,int,int);
static void             _PD_wr_leaf_members (PDBfile*,char*,char*,long,lite_SC_byte*);
static int              _PD_wr_ind_itags (PDBfile*,long,char*);
static char *           _PD_write_hyper_space (PDBfile*,char*,char*,char*,
                                               symblock*,int,int,long,
                                               long,long);


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_indexed_read_as
 *
 * Purpose:     Read part of an entry from the PDB file into the location
 *              pointed to by VR.  IND contains one triplet of long ints
 *              per variable dimension specifying start, stop, and step
 *              for the index.
 *
 * Return:      Success:        Number of items successfully read.
 *
 *              Failure:        0
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  4, 1996  4:52 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_indexed_read_as (PDBfile *file, char *fullpath, char *type, lite_SC_byte *vr,
                          int nd, long *ind, syment *ep) {

   int          i, err;
   long         start, stop, step;
   char         expr[MAXLINE], index[MAXLINE], hname[MAXLINE];

   switch (setjmp(_lite_PD_read_err)) {
   case ABORT:
      return(FALSE);
   case ERR_FREE:
      return(TRUE);
   default:
      memset(lite_PD_err, 0, MAXLINE);
      break;
   }

   strcpy(index, "(");
   for (i = 0; i < nd; i++) {
      start = ind[0];
      stop  = ind[1];
      step  = ind[2];
      ind  += 3;
      if (start == stop) sprintf(expr, "%ld,", start);
      else if (step <= 1L) sprintf(expr, "%ld:%ld,", start, stop);
      else sprintf(expr, "%ld:%ld:%ld,", start, stop, step);
      strcat(index, expr);
   }

   if (strlen(index) > 1) {
      index[strlen(index)-1] = ')';
      sprintf(hname, "%s%s", fullpath, index);
   } else {
      strcpy(hname, fullpath);
   }

   _lite_PD_rl_syment_d(ep);
   ep = _lite_PD_effective_ep(file, hname, TRUE, fullpath);
   if (ep == NULL)
      lite_PD_error("CAN'T FIND ENTRY - _PD_INDEXED_READ_AS", PD_READ);

   PD_entry_number(ep) = lite_PD_hyper_number(file, hname, ep);
   if (type == NULL) type = PD_entry_type(ep);

   err = _lite_PD_hyper_read (file, hname, type, ep, vr);
   _lite_PD_rl_syment_d(ep);

   return(err);
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_hyper_read
 *
 * Purpose:     Parse an index expression of the form
 *
 *              <expr>  := <spec> | <expr>, <spec>
 *              <spec>  := <start> |
 *                         <start>:<stop> |
 *                         <start>:<stop>:<step>
 *              <start> := starting integer index value
 *              <stop>  := ending integer index value
 *              <step>  := integer index step value
 *
 * Return:      Success:        Number of items successfully read.
 *
 *              Failure:        0
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  4, 1996  4:45 PM EST
 *
 * Modifications:
 *
 *  Mark C. Miller, Mon Jan 11 16:04:56 PST 2010
 *  Fixed src/dst overlap in strcpy flagged by valgrind. Note, in this
 *  context, src is guarenteed to be an 'end' part of dst so a simple
 *  manual shift of bytes works.
 *-------------------------------------------------------------------------
 */
int
_lite_PD_hyper_read (PDBfile *file, char *name, char *outtype,
                     syment *ep, lite_SC_byte *vr) {

   long         hbyt, fbyt;
   int          nd, c, nrd, i, slen;
   char         s[MAXLINE], *expr;
   dimdes       *dims;
   dimind       *pi;
   syment       *dep, *tep;

   /*
    * To accomodate certain bad users do one quick test
    * see if the variable name is literally in the file
    * this lets things such as foo(2,10) be variable names
    */
   dep = lite_PD_inquire_entry(file, name, FALSE, NULL);
   if (dep != NULL) return(_lite_PD_rd_syment(file, ep, outtype, vr));

   dims = PD_entry_dimensions(ep);
   strcpy(s, name);
   slen = strlen(s);
   c = slen > 0 ? s[slen-1] : 0;
   if (((c != ')') && (c != ']')) || (dims == NULL)) {
      return(_lite_PD_rd_syment(file, ep, outtype, vr));
   }

   if (_lite_PD_indirection(outtype)) {
      lite_PD_error("CAN'T HYPER INDEX INDIRECT TYPE - _PD_HYPER_READ",
                    PD_READ);
   }

   expr = lite_SC_lasttok(s, "[]()");
   for (i = 0; expr[i] != '\0'; i++)
      s[i] = expr[i];
   s[i] = '\0';

   pi = _PD_compute_hyper_strides(file, s, dims, &nd);
   if (pi == NULL)
      lite_PD_error("CAN'T FIND HYPER INDICES - _PD_HYPER_READ", PD_READ);

   fbyt = _lite_PD_lookup_size(PD_entry_type(ep), file->chart);
   if (fbyt == -1) {
      lite_PD_error("CAN'T FIND NUMBER OF FILE BYTES - _PD_HYPER_READ",
                    PD_READ);
   }

   hbyt = _lite_PD_lookup_size(outtype, file->host_chart);
   if (hbyt == -1) {
      lite_PD_error("CAN'T FIND NUMBER OF HOST BYTES - _PD_HYPER_READ",
                    PD_READ);
   }

   /*
    * Make a dummy for the hyper read to use as scratch space.
    */
   tep = _lite_PD_mk_syment(NULL, 0L, 0L, &(ep->indirects), NULL);

   nrd = _PD_rd_hyper_index(file, tep, vr, pi,
                            PD_entry_type(ep), outtype,
                            PD_entry_address(ep),
                            PD_entry_blocks(ep), hbyt, fbyt);

   _lite_PD_rl_syment(tep);

   SFREE(pi);

   return(nrd);
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_rd_syment
 *
 * Purpose:     From the information in the symbol table entry EP read
 *              the entry from the PDBfile FILE into the location
 *              pointed by by VR.  At this point the things have been
 *              completely dereferenced.  This new version is written in
 *              a continuation passing style so that PDB has control over
 *              the stack andisn't blowing out the execution stack for
 *              long linked lists.
 *
 * Return:      Success:        Number of items successfully read.
 *
 *              Failure:        0
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  2:51 PM EST
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  7 09:54:39 PST 1998
 *    Remove the caching of pointer references.
 *
 *    Sean Ahern, Wed Jul 14 14:03:01 PDT 2004
 *    Don't flush if the file has been opened as "read only".
 *
 *    Kathleen Bonnell, Thu Jan  8 17:40:36 PST 2009
 *    Fixed UMR for litype by adding initialization.
 *-------------------------------------------------------------------------
 */
long
_lite_PD_rd_syment (PDBfile *file, syment *ep, char *outtype, lite_SC_byte *vr) {

   FILE *fp;
   int dst, vif, size, boffs, readonly;
   long i, n, nitems, bytepitem, addr, eaddr, nrd;
   long flag;
   char bf[MAXLINE], *pv, *litype=NULL, *lotype, *svr, **lvr;
   symblock *sp;
   symindir iloc;
   defstr *dp;
   memdes *desc, *mem_lst;
   PD_itag pi;

   fp  = file->stream;
   vif = file->virtual_internal;
   readonly = file->mode == PD_OPEN;

#ifdef PDB_WRITE
   if (!vif && !readonly && io_flush(fp))
      lite_PD_error("FFLUSH FAILED BEFORE READ - _PD_RD_SYMENT", PD_READ);
#endif

   call_ptr = 0L;
   lval_ptr = 0L;
   str_ptr  = 0L;
   lotype   = NULL;

   SAVE_S(lotype, outtype);

   fp   = file->stream;
   iloc = ep->indirects;

   file->flushed = FALSE;

   call_stack[call_ptr++] = DONE;
   dst = BLOCK;

   /*
    * Some AIX compilers will erroneously take the default case if
    * this is terminated with a semicolon
    */
   while (TRUE) {
      switch (dst) {
         /*
          * Count on this being right and _lite_PD_effective_ep will handle all
          * issues about partial reads across discontiguous blocks by correctly
          * making an effective syment for which this logic works!!!!!!
          */
      case BLOCK :
         bytepitem = _lite_PD_lookup_size(outtype, file->host_chart);
         if (bytepitem == -1) {
            lite_PD_error("CAN'T FIND NUMBER OF BYTES - _PD_RD_SYMENT",
                          PD_READ);
         }

         sp = PD_entry_blocks(ep);
         n  = PD_n_blocks(ep);
         if (n == 1)
            sp[0].number = PD_entry_number(ep);

         pv  = (char *) vr;
         nrd = 0L;
         i   = 0L;

      case BLOCK_ITEM :
         if (i >= n)
            {GO_CONT;};

         addr   = sp[i].diskaddr;
         nitems = sp[i].number;

         /*
          * If negative we are staring at a bit address.
          */
         if (addr < 0) {
            eaddr = (-addr) >> 3;
            boffs = -addr - (eaddr << 3);
         } else {
            eaddr = addr;
            boffs = 0;
         }

         if (!vif && io_seek(fp, eaddr, SEEK_SET)) {
            lite_PD_error("FSEEK FAILED TO FIND ADDRESS - _PD_RD_SYMENT",
                          PD_READ);
         }

         SAVE_I(i);
         SAVE_I(n);
         SAVE_S(litype, PD_entry_type(ep));
         SAVE_P(pv);
         SET_CONT(BLOCK_RET);

      case BLOCK_RET:
         RESTORE_P(char, pv);
         RESTORE_S(litype);
         RESTORE_I(n);
         RESTORE_I(i);

         pv += nitems*bytepitem;
         i++;

         GO(BLOCK_ITEM);

      case LEAF:
         if (vif) {
            SC_address ad;
            ad.diskaddr = addr;
            memcpy(pv, ad.memaddr, nitems*bytepitem);
         } else {
            _PD_rd_leaf_members(file, pv, nitems, litype, lotype, boffs);
         }
         nrd += nitems;

         /*
          * The host type must be used to get the correct member offsets
          * for the in memory copy - the file ones might be wrong!!
          */
         dp = PD_inquire_host_type(file, lotype);
         if (dp == NULL) lite_PD_error("BAD TYPE - _PD_RD_SYMENT", PD_READ);

         mem_lst = dp->members;
         if (!dp->n_indirects || (mem_lst == NULL)) GO_CONT;

         if (lite_pdb_rd_hook != NULL) {
            mem_lst = (*lite_pdb_rd_hook)(dp->members);
         }

         /*
          * For an array of structs read the indirects for each array
          * element.
          */
         size = dp->size;
         svr  = pv;
         i    = 0L;

      case LEAF_ITEM :
         if (i >= nitems) GO_CONT;
         desc = mem_lst;

      case LEAF_INDIR :
         if (desc == NULL) {
            i++;
            svr += size;
            GO(LEAF_ITEM);
         }

         if (!_lite_PD_indirection(desc->type)) {
            desc = desc->next;
            GO(LEAF_INDIR);
         }

         SAVE_I(i);
         SAVE_I(size);
         SAVE_P(mem_lst);
         SAVE_P(desc);
         SAVE_P(svr);
         SAVE_P(pv);
         pv  = svr + desc->member_offs;
         lvr = (char **) pv;
         SET_CONT_RD(LEAF_RET, SKIP_TO);

      case LEAF_RET :
         RESTORE_P(char, pv);
         RESTORE_P(char, svr);
         RESTORE_P(memdes, desc);
         RESTORE_P(memdes, mem_lst);
         RESTORE_I(size);
         RESTORE_I(i);

         desc = desc->next;
         GO(LEAF_INDIR);

      case INDIRECT :
         SAVE_P(pv);
         lvr = (char **) pv;
         i   = 0L;

      case INDIR_ITEM :
         if (i >= nitems) {
            RESTORE_P(char, pv);
            nrd += nitems;
            GO_CONT;
         }

         SAVE_I(i);
         SAVE_I(nrd);
         SAVE_P(lvr);
         lvr = &lvr[i];
         SET_CONT_RD(INDIR_RET, SKIP_TO);

      case INDIR_RET :
         RESTORE_P(char *, lvr);
         RESTORE_I(nrd);
         RESTORE_I(i);
         i++;

         GO(INDIR_ITEM);

      case SKIP_TO :
         if (iloc.addr > 0L) {
            long naitems;

            io_seek(fp, iloc.addr, SEEK_SET);
            iloc.addr = -1L;

            naitems = iloc.n_ind_type*iloc.arr_offs;
            addr    = _lite_PD_skip_over(file, naitems, TRUE);
         }

         if (vif) {
            SC_address ad;

            ad.diskaddr = addr;
            DEREF(pv) = DEREF(ad.memaddr);
            GO_CONT;
         } else if (_PD_rd_ind_tags(file, lvr, &pi) == -1) {
            GO_CONT;
         }

         /*
          * Now read the data.
          */
         SAVE_I(nrd);
         SAVE_I(nitems);

         nitems = pi.nitems;
         addr   = pi.addr;
         flag   = pi.flag;

         SAVE_S(litype, pi.type);
         SAVE_S(lotype, litype);
         SAVE_I(addr);
         SAVE_I(flag);
         SAVE_P(pv);
         pv = lvr[0];
         SET_CONT(SKIP_RET);

      case SKIP_RET :
         RESTORE_P(char, pv);
         RESTORE_I(flag);
         RESTORE_S(lotype);
         RESTORE_S(litype);
         RESTORE_I(addr);
         RESTORE_I(nitems);
         RESTORE_I(nrd);

         /*
          * Restore the file pointer to its original location if necessary.
          */
         if (flag != 1L) {
            if (io_seek(fp, addr, SEEK_SET)) {
               lite_PD_error("FAILED TO FIND OLD ADDRESS - _PD_RD_SYMENT",
                             PD_READ);
            }
         }

         addr = io_tell(fp);
         if (addr == -1L) {
            lite_PD_error("CAN'T FIND RETURN ADDRESS - _PD_RD_SYMENT",
                          PD_READ);
         }

         GO_CONT;

      case DONE :
         RESTORE_S(lotype);
         return(nrd);

      default  :
         sprintf(bf, "UNDECIDABLE CASE - _PD_RD_SYMENT");
         lite_PD_error(bf, PD_READ);
      }
   }
}


/*-------------------------------------------------------------------------
 * Function:    _PD_rd_hyper_index
 *
 * Purpose:     Do the real work of parsing an index expression into
 *              components and recursively determine the disk locations
 *              to read into the buffer OUT.  OUT is to be filled in order
 *              from smallest index to largest.  The offset is specified
 *              by the starting address which is ADDR.  FBYT is the number
 *              of bytes in the file for each item to be read.  HBYT is the
 *              number of bytes in memory for each item to be read.
 *
 * Return:      Success:        The number of items successfully read.
 *
 *              Failure:        0
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  2:33 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
_PD_rd_hyper_index (PDBfile *file, syment *ep, char *out, dimind *pi,
                    char *intype, char *outtype, long addr, symblock *sp,
                    int hbyt, int fbyt) {

   long offset, stride, start, stop, step;
   int nrd, nir;

   /*
    * For each index specification compute the range and recurse.
    * */
   stride = fbyt*pi->stride;
   start  = stride*pi->start;
   stop   = stride*pi->stop;
   step   = stride*pi->step;

   if (addr < 0) {
      defstr* dpf;
      dpf  = _lite_PD_lookup_type(intype, file->chart);
      stop = addr - dpf->size_bits*((stop - start)/fbyt);
      step = -dpf->size_bits*(step/fbyt);
   } else {
      stop = addr + (stop - start);
   }

   /*
    * At the bottom of the recursion do the actual reads.
    * */
   nrd = 0;
   if (stride <= (long) fbyt) {
      nrd += _PD_read_hyper_space(file, ep, out, intype, outtype,
                                  sp, hbyt, fbyt, addr, stop, step);

   } else if (addr < 0) {
      for (offset = -addr; offset <= -stop; offset -= step) {
         nir = _PD_rd_hyper_index(file, ep, out, pi + 1, intype, outtype,
                                  -offset, sp, hbyt, fbyt);
         nrd += nir;
         out += nir*hbyt;
      }

   } else {
      for (offset = addr; offset <= stop; offset += step) {
         nir = _PD_rd_hyper_index(file, ep, out, pi + 1, intype, outtype,
                                  offset, sp, hbyt, fbyt);
         nrd += nir;
         out += nir*hbyt;
      }
   }

   return(nrd);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_read_hyper_space
 *
 * Purpose:     Read a part of a hyper-surface from a data array. ADDR is
 *              the starting address.  STOP is the upper bound on the
 *              address. STEP is the increment of the address for each
 *              entry.  FBYT is the number bytes in the file for each item
 *              to be read.  HBYT is the number of bytes in memory for each
 *              item to be read.  EP is a scratch syment for temporary use.
 *
 * Return:      Success:        The number of items successfully read.
 *
 *              Failure:        0
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  4:50 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
_PD_read_hyper_space (PDBfile *file, syment *ep, char *out, char *intype,
                      char *outtype, symblock *sp, int hbyt, int fbyt,
                      long addr, long stop, long step) {

   long eaddr, nb, nitems;
   int nrd;

   PD_entry_type(ep)       = intype;
   PD_entry_dimensions(ep) = NULL;

   nrd = 0;
   if (addr >= 0) {
      /*
       * Items logically contiguous.
       */
      if (step == fbyt) {
         long niw;

         /*
          * Read across blocks.
          */
         nitems = (stop - addr)/step + 1L;
         while (nitems > 0) {
            eaddr = addr;

            _PD_effective_addr(&eaddr, &nb, fbyt, sp);

            /*
             * NOTE: this subverts _PD_effective_addr in part, but
             * because _lite_PD_effective_ep cannot be modified to build an
             * effective syment for indirectly referenced data which
             * cannot be distinguished from an explicitly dimensioned
             * array, this is the best that can be done
             */
            if ((eaddr == 0) || (nb == 0)) {
               eaddr = addr;
               nb    = nitems;
            }

            niw = MIN(nitems, nb);

            PD_entry_address(ep) = eaddr;
            PD_entry_number(ep)  = niw;
            nrd += _lite_PD_rd_syment(file, ep, outtype, out);

            nitems -= niw;
            addr   += fbyt*niw;
            out    += hbyt*niw;
         }
      } else {
         /*
          * Items not logically contiguous.
          */
         PD_entry_number(ep) = 1L;
         for (; addr <= stop; addr += step, out += hbyt) {
            eaddr = addr;
            _PD_effective_addr(&eaddr, &nb, fbyt, sp);
            PD_entry_address(ep) = eaddr;
            nrd += _lite_PD_rd_syment(file, ep, outtype, out);
         }
      }
   } else {
      /*
       * We have a bitstream.
       */
      defstr* dpf;

      dpf = _lite_PD_lookup_type(intype, file->chart);
      /*
       * Items logically contiguous.
       */
      if (step == -dpf->size_bits) {
         nitems = (stop - addr)/step + 1L;

         /*
          * NOTE: multi-block bitstreams are not supported.
          */
         PD_entry_number(ep)  = nitems;
         PD_entry_address(ep) = addr;
         nrd += _lite_PD_rd_syment(file, ep, outtype, out);

         out += hbyt*nitems;
      } else {
         /*
          * Items not logically contiguous.
          */
         PD_entry_number(ep) = 1L;
         for (; addr >= stop; addr += step, out += hbyt) {
            PD_entry_address(ep) = addr;
            nrd += _lite_PD_rd_syment(file, ep, outtype, out);
         }
      }
   }

   PD_entry_type(ep) = NULL;

   return(nrd);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_rd_leaf_members
 *
 * Purpose:     Read the leaves only for NITEMS of INTYPE from the
 *              PDBfile FILE into the location pointed to by VR as type
 *              OUTTYPE.  At this level it is guaranteed that the type
 *              will not be a pointer.
 *
 * Return:      Success:        Number of items successfully read.
 *
 *              Failure:        0
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:08 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_rd_leaf_members (PDBfile *file, char *vr, long nitems, char *intype,
                     char *outtype, int boffs) {

   FILE *fp;
   long bytepitemin, in_offs, out_offs, nir, nia;
   char *buf, *vbuf, *svr;
   defstr *dpf;

   fp = file->stream;

   dpf         = _lite_PD_lookup_type(intype, file->chart);
   bytepitemin = dpf->size;
   if (bytepitemin == -1)
      lite_PD_error("CAN'T FIND NUMBER OF BYTES - _PD_RD_LEAF_MEMBERS",
                    PD_READ);

   if ((dpf->convert > 0) || (strcmp(intype, outtype) != 0)) {
      if (dpf->size_bits) {
         nia = (((nitems*dpf->size_bits + boffs + SC_BITS_BYTE - 1)
                 /SC_BITS_BYTE) + bytepitemin - 1)/bytepitemin;
      } else {
         nia = nitems;
      }
      buf = (char *) lite_SC_alloc(nia, bytepitemin,
                                   "_PD_RD_LEAF_MEMBERS:buffer");
      if (buf == NULL)
         lite_PD_error("CAN'T ALLOCATE MEMORY - _PD_RD_LEAF_MEMBERS", PD_READ);

      nir = io_read(buf, (size_t) bytepitemin, (size_t) nia, fp);
      if (nir == nia) {
         vbuf     = buf;
         svr      = vr;
         in_offs  = 0L;
         out_offs = 0L;
         lite_PD_convert(&svr, &vbuf, intype, outtype, nitems,
                         file->std, file->host_std, file->host_std,
                         &in_offs, &out_offs,
                         file->chart, file->host_chart, boffs, PD_READ);
         SFREE(buf);
      } else {
         SFREE(buf);
         lite_PD_error("FILE READ FAILED - _PD_RD_LEAF_MEMBERS", PD_READ);
      }
   } else {
      nir = io_read(vr, (size_t) bytepitemin, (size_t) nitems, fp);
      if (nir != nitems)
         lite_PD_error("DATA READ FAILED - _PD_RD_LEAF_MEMBERS", PD_READ);
   }
}


/*-------------------------------------------------------------------------
 * Function:    _PD_rd_ind_tags
 *
 * Purpose:     Read itags and setup the read of an indirection.
 *
 * Return:      Success:        TRUE iff there is indirect data to be read.
 *
 *              Failure:        FALSE
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  4:46 PM EST
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  7 09:54:39 PST 1998
 *    Remove the caching of pointer references.
 *
 *-------------------------------------------------------------------------
 */
static int
_PD_rd_ind_tags (PDBfile *file, char **vr, PD_itag *pi) {

   long bytepitem, nitems, addr, oaddr;
   int flag;
   char *type, bf2[MAXLINE];
   FILE *fp;

   fp = file->stream;

   if (!_lite_PD_rd_itag(file, pi)) {
      lite_PD_error("BAD ITAG - _PD_RD_IND_TAGS", PD_READ);
   }

   nitems = pi->nitems;
   type   = pi->type;
   addr   = pi->addr;
   flag   = pi->flag;

   /*
    * If it was a NULL pointer stop here.
    */
   if ((addr == -1L) || (nitems == 0L)) {
      *vr = NULL;
      return(-1);
   }

   /*
    * Get the size and allocate storage for it.
    */
   bytepitem = _lite_PD_lookup_size(type, file->host_chart);
   if (bytepitem == -1)
      lite_PD_error("CAN'T FIND NUMBER OF BYTES - _PD_RD_IND_TAGS",
                    PD_READ);

   DEREF(vr) = (char *) lite_SC_alloc(nitems, bytepitem,
                                      "_PD_RD_IND_TAGS:vr");

   /*
    * If flag != 1 it was written somewhere else
    * GOTCHA: watch for new case of flag == 2 which means a discontiguous block
    *         Deal with this, if and when it arises.
    */
   if (flag != 1) {
      oaddr = io_tell(fp);
      if (oaddr == -1L)
         lite_PD_error("CAN'T FIND CURRENT ADDRESS - _PD_RD_IND_TAGS",
                       PD_READ);

      /*
       * Jump to the place where the original is described.
       */
      if (io_seek(fp, addr, SEEK_SET))
         lite_PD_error("FAILED TO FIND ADDRESS - _PD_RD_IND_TAGS",
                       PD_READ);

      /*
       * Read the descriptor so that the file pointer is left on the actual
       * data.
       * NOTE: read into separate buffer so not to clobber "type" which
       *       is strtok'd into bf1.
       */
      _lite_PD_rfgets(bf2, MAXLINE, fp);

      pi->addr = oaddr;
   }

   return(0);
}

/*--------------------------------------------------------------------------*/
/*                          AUXILLIARY ROUTINES                             */
/*--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Function:    _PD_effective_addr
 *
 * Purpose:     Given a disk address computed on the assumption of one
 *              contiguous block and list of symblocks, compute and
 *              return the actual disk address.  Also return the number
 *              of items remaining in the block after the effective
 *              address.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 10:57 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_effective_addr (long *paddr, long *pnitems, long bpi, symblock *sp) {

   int i;
   long nb, nt, ad, addr;

   addr = *paddr;
   i    = 0;
   ad   = sp[i].diskaddr;
   nt   = addr - ad;
   while (TRUE) {
      nb  = sp[i].number*bpi;
      nt -= nb;

      if ((nb <= 0L) || (nt < 0L)) break;

      i++;
      ad   = sp[i].diskaddr;
      addr = ad + nt;
   }

   *paddr   = addr;
   *pnitems = (ad + nb - addr)/bpi;
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_indirection
 *
 * Purpose:     Is TYPE an indirect type.
 *
 * Return:      Success:        TRUE iff TYPE has a `*' as the last
 *                              non-blank character.
 *
 *              Failure:        Never fails.
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  2:01 PM EST
 *
 * Modifications:
 *     Brad Whitlock, Wed Feb 23 19:11:09 PST 2000
 *     Made the function return FALSE if s == NULL. This prevents a crash.
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_indirection (char *s)
{
    if(s != NULL)
    {
        char *t;

        for (t = s + strlen(s); t > s; t--)
        {
            if (*t == '*') return(TRUE);
        }
    }

    return(FALSE);
}


/*-------------------------------------------------------------------------
 * Function:    lite_PD_dereference
 *
 * Purpose:     Starting at the end of the string work backwards to the
 *              first non-blank character and if it is a `*' insert
 *              `\0' in its place.
 *
 * Return:      Success:        Ptr to the beginning of the string.
 *
 *              Failure:        never fails
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:44 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
lite_PD_dereference (char *s) {

   char *t;

   for (t = s + strlen(s); t > s; t--) {
      if (*t == '*') break;
   }

   /*
    * Check for whitespace to remove - eg. "char *" -> "char"
    */
   for (t-- ; t > s; t--) {
      if (strchr(" \t", *t) == NULL) {
         *(++t) = '\0';
         break;
      }
   }

   return(s);
}


/*-------------------------------------------------------------------------
 * Function:    lite_PD_hyper_number
 *
 * Purpose:     Return the number of elements implied by a hyper
 *              index expression.
 *
 * Return:      Success:        number of elements
 *
 *              Failure:
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:45 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
long
lite_PD_hyper_number (PDBfile *file, char *name, syment *ep) {

   char s[MAXLINE];
   int c;

   /*
    * If name is of the form a[...] strip off the name part
    * by design _lite_PD_hyper_number can't handle anything but the index part.
    */
   strcpy(s, name);
   c = s[0];
   if (strchr("0123456789-.", c) == NULL) lite_SC_firsttok(s, "()[]");

   return(_lite_PD_hyper_number(file, s,
                                PD_entry_number(ep),
                                PD_entry_dimensions(ep), NULL));
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_hyper_number
 *
 * Purpose:     <expr>  := <spec> | <expr>, <spec>
 *              <spec>  := <start> |
 *                         <start>:<stop> |
 *                         <start>:<stop>:<step>
 *              <start> := starting integer index value
 *              <stop>  := ending integer index value
 *              <step>  := integer index step value
 *
 * Return:      Success:        The number of elements implied by a
 *                              hyper index expression.
 *
 *              Failure:
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:17 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
long
_lite_PD_hyper_number (PDBfile *file, char *indxpr, long numb,
                       dimdes *dims, long *poff) {

   int nd;
   long i, maxs, sum, offs;
   char s[MAXLINE];
   dimind *pi;

   strcpy(s, indxpr);
   pi = _PD_compute_hyper_strides(file, s, dims, &nd);

   offs = 0L;
   sum  = 1L;
   for (i = 0; i < nd; i++) {
      maxs  = (pi[i].stop - pi[i].start + pi[i].step)/pi[i].step;
      offs += pi[i].start*pi[i].stride;
      sum  *= maxs;
   }

   SFREE(pi);

   if (poff != NULL) *poff = offs;

   return(sum);
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_init_dimind
 *
 * Purpose:     Fill a dimind struct given the stride and an ASCII
 *              index epxression.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  4:26 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
_lite_PD_init_dimind (dimind *pi, long offset, long stride, char *expr) {

   char s[MAXLINE], *token;
   long start, stop, step;

   if (expr != NULL) strcpy(s, expr);
   else s[0] = '\0';

   token = strtok(s, " \t:");
   if (token == NULL) start = 0L;
   else start = atol(token);

   token = strtok(NULL, " \t:");
   if (token == NULL) stop = start;
   else stop = atol(token);

   token = strtok(NULL, " \t:");
   if (token == NULL) step = 1L;
   else step = atol(token);

   pi->stride = stride;
   pi->start  = start - offset;
   pi->stop   = stop - offset;
   pi->step   = step;
}


/*-------------------------------------------------------------------------
 * Function:    _PD_compute_hyper_strides
 *
 * Purpose:     Initialize and return an array of dimension indices
 *              representing the hyper strides from the given hyper
 *              index expression.
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  1:37 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static dimind *
_PD_compute_hyper_strides (PDBfile *file, char *ind, dimdes *dims, int *pnd) {

   int i, nd;
   long maxs;
   dimdes *pd;
   dimind *pi;

   if (dims == NULL) {
      pi = FMAKE(dimind, "_PD_COMPUTE_HYPER_STRIDES:pi");

      _lite_PD_init_dimind(pi, file->default_offset, 0L,
                           lite_SC_firsttok(ind, ",()[]\n\r"));

      *pnd = 1;
      return(pi);
   }

   /*
    * Count the number of dimensions and allocate some temporaries.
    */
   for (nd = 0, pd = dims; pd != NULL; pd = pd->next, nd++) /*void*/ ;
   pi = FMAKE_N(dimind, nd, "_PD_COMPUTE_HYPER_STRIDES:pi");

   /*
    * Pre-compute the strides, offsets, and so on for the hyper-space walk.
    */
   if (file->major_order == COLUMN_MAJOR_ORDER) {
      maxs = 1L;
      for (i = nd - 1, pd = dims; i >= 0; i--) {
         _lite_PD_init_dimind(&pi[i], pd->index_min, maxs,
                              lite_SC_firsttok(ind, ",()[]\n\r"));
         if (pd != NULL) {
            maxs *= pd->number;
            pd    = pd->next;
         }
      }

   } else if (file->major_order == ROW_MAJOR_ORDER) {
      for (maxs = 1L, pd = dims->next; pd != NULL; pd = pd->next) {
         maxs *= pd->number;
      }

      for (i = 0, pd = dims; i < nd; i++) {
         _lite_PD_init_dimind(&pi[i], pd->index_min, maxs,
                              lite_SC_firsttok(ind, ",()[]\n\r"));
         if (pd->next != NULL) {
            pd    = pd->next;
            maxs /= pd->number;
         }
      }
   }

   *pnd = nd;

   return(pi);
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_hyper_write
 *
 * Purpose:     Parse an index expression of the form
 *
 *                 <expr>  := <spec> | <expr>, <spec>
 *                 <spec>  := <start> |
 *                            <start>:<stop> |
 *                            <start>:<stop>:<step>
 *                 <start> := starting integer index value
 *                 <stop>  := ending integer index value
 *                 <step>  := integer index step value
 *
 *              and write the specified elements to the PDBfile
 *              from the array provided
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Robb Matzke
 *              robb@callisto.matzke.cioe.com
 *              Apr 17, 1996
 *
 * Modifications:
 *
 *  Mark C. Miller, Wed Jan 21 18:03:08 PST 2009
 *  Silenced valgrind errors by fixing overlapping strcpy.
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
int
_lite_PD_hyper_write (PDBfile *file, char *name, syment *ep, lite_SC_byte *vr,
                      char *intype) {

   long hbyt, fbyt;
   int nd, c, slen;
   char s[MAXLINE], *expr;
   dimdes *dims;
   dimind *pi;

   memset(s, 0, sizeof(s));
   dims = PD_entry_dimensions(ep);
   strcpy(s, name);
   slen = strlen(s);
   c = slen > 0 ? s[slen-1] : 0;
   if (((c != ')') && (c != ']')) || (dims == NULL)) {
      return(_lite_PD_wr_syment(file, vr, PD_entry_number(ep),
                                intype, PD_entry_type(ep)));
   }

   if (_lite_PD_indirection(PD_entry_type(ep))) {
      lite_PD_error("CAN'T HYPER INDEX INDIRECT TYPE - _PD_HYPER_WRITE",
                    PD_WRITE);
   }

   expr = lite_SC_lasttok(s, "[]()");
   if (s + strlen(expr) + 1 >= expr)
   {
       int i;
       for (i=0; expr[i] != 0; i++)
           s[i] = expr[i];
       s[i] = 0;
   }
   else
   {
       strcpy(s, expr);
   }

   pi = _PD_compute_hyper_strides(file, s, dims, &nd);
   if (pi == NULL) {
      lite_PD_error("CAN'T FIND HYPER INDICES - _PD_HYPER_WRITE", PD_WRITE);
   }

   fbyt = _lite_PD_lookup_size(PD_entry_type(ep), file->chart);
   if (fbyt == -1) {
      lite_PD_error("CAN'T FIND NUMBER OF FILE BYTES - _PD_HYPER_WRITE",
                    PD_WRITE);
   }

   hbyt = _lite_PD_lookup_size(intype, file->host_chart);
   if (hbyt == -1) {
      lite_PD_error("CAN'T FIND NUMBER OF HOST BYTES - _PD_HYPER_WRITE",
                    PD_WRITE);
   }

   _PD_wr_hyper_index(file, vr, pi, intype,
                      PD_entry_type(ep), PD_entry_address(ep),
                      PD_entry_blocks(ep), hbyt, fbyt);

   SFREE(pi);

   return(TRUE);
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_wr_syment
 *
 * Purpose:     Write the NUMBER of elements of type TYPE from memory pointed
 *              to by VAR. This new version is written in a continuation
 *              passing style so that PDB has control over the stack and isn't
 *              blowing out the execution stack for long linked lists.
 *
 * Return:      Success:        Number of items successfully written.
 *
 *              Failure:
 *
 * Programmer:  Robb Matzke
 *              robb@callisto.matzke.cioe.com
 *              Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
long
_lite_PD_wr_syment (PDBfile *file, char *vr, long nitems,
                    char *intype, char *outtype) {
   int dst, size;
   long i;
   defstr *dp;
   memdes *desc, *mem_lst;
   char bf[MAXLINE], *litype, *lotype, *svr, *ttype;
   FILE *fp;

   size = 0;
   call_ptr = 0L;
   lval_ptr = 0L;
   str_ptr  = 0L;
   litype   = NULL;
   lotype   = NULL;
   svr      = NULL;

   SAVE_S(litype, intype);
   SAVE_S(lotype, outtype);

   fp = file->stream;

   file->flushed = FALSE;

   call_stack[call_ptr++] = DONE;
   dst = _lite_PD_indirection(litype) ? INDIRECT : LEAF;

   /*
    * some AIX compilers will erroneously take the default case if
    * this is terminated with a semicolon.
    */
   while (1) {
      switch (dst) {
      case LEAF:
         _PD_wr_leaf_members(file, litype, lotype, nitems, vr);
         dp = PD_inquire_host_type(file, litype);
         if (dp == NULL) {
            lite_PD_error("BAD TYPE - _PD_WR_SYMENT", PD_WRITE);
         }

         mem_lst = dp->members;
         if (!dp->n_indirects || (mem_lst == NULL)) GO_CONT;

         if (lite_pdb_wr_hook != NULL) {
            mem_lst = (*lite_pdb_wr_hook)(file, vr, dp);
         }

         /*
          * If type is a struct with pointered members write them out now
          * for an array of structs write the indirects for each array element
          */
         size = dp->size;
         svr  = vr;
         i    = 0L;

      case LEAF_ITEM:
         if (i >= nitems) GO_CONT;
         desc = mem_lst;

      case LEAF_INDIR :
         if (desc == NULL) {
            i++;
            if (svr == NULL)
                lite_PD_error("SVR UMR - _PD_WR_SYMENT", PD_WRITE) ;
            svr += size;
            GO(LEAF_ITEM);
         }

         PD_CAST_TYPE(ttype, desc, svr+desc->member_offs, svr,
                      lite_PD_error, "BAD CAST - _PD_WR_SYMENT", PD_WRITE);

         SAVE_S(litype, ttype);

         if (!_lite_PD_indirection(litype)) {
            RESTORE_S(litype);
            desc = desc->next;
            GO(LEAF_INDIR);
         }

         SAVE_I(nitems);
         nitems = desc->number;

         SAVE_I(i);
         SAVE_I(size);
         SAVE_P(mem_lst);
         SAVE_P(desc);
         SAVE_P(svr);
         SAVE_P(vr);
         vr = svr + desc->member_offs;
         SET_CONT(LEAF_RET);

      case LEAF_RET :
         RESTORE_P(char, vr);
         RESTORE_P(char, svr);
         RESTORE_P(memdes, desc);
         RESTORE_P(memdes, mem_lst);
         RESTORE_I(size);
         RESTORE_I(i);
         RESTORE_I(nitems);
         RESTORE_S(litype);

         desc = desc->next;
         GO(LEAF_INDIR);

      case INDIRECT :
         if (vr == NULL) {
            io_printf(fp, "%ld\001%s\001%ld\001%d\001\n", 0L, litype,
                      -1L, TRUE);
            GO_CONT;
         }

         /*
          * Dereference a local copy of the type.
          */
         SAVE_S(litype, litype);
         lite_PD_dereference(litype);

         /*
          * Write the data.
          */
         i = 0L;

      case INDIR_ITEM :
         if (i >= nitems) {
            RESTORE_S(litype);
            GO_CONT;
         }

         SAVE_P(vr);
         vr = DEREF(vr);
         if (vr == NULL) {
            _lite_PD_wr_itag(file, 0L, litype, -1L, FALSE);
            RESTORE_P(char, vr);
            i++;
            vr += sizeof(char *);
            GO(INDIR_ITEM);
         }

         SAVE_I(nitems);
         nitems = _lite_PD_number_refd(vr, litype, file->host_chart);
         if (nitems == -1L) {
            sprintf(bf, "CAN'T GET POINTER LENGTH ON %s - _PD_WR_SYMENT",
                    litype);
            lite_PD_error(bf, PD_WRITE);
         }

         if (nitems == -2L) {
            sprintf(bf, "UNKNOWN TYPE %s - _PD_WR_SYMENT", litype);
            lite_PD_error(bf, PD_WRITE);
         }

         if (!_PD_wr_ind_itags(file, nitems, litype)) {
            RESTORE_I(nitems);
            RESTORE_P(char, vr);
            i++;
            vr += sizeof(char *);
            GO(INDIR_ITEM);
         }

         SAVE_I(i);
         SAVE_S(lotype, litype);
         SET_CONT(INDIR_RET);

      case INDIR_RET :
         RESTORE_S(lotype);
         RESTORE_I(i);
         RESTORE_I(nitems);
         RESTORE_P(char, vr);

         i++;
         vr += sizeof(char *);

         GO(INDIR_ITEM);

      case DONE :
         RESTORE_S(lotype);
         RESTORE_S(litype);

         /*
          * update the end of data mark
          */
         _lite_PD_eod(file);

         return(nitems);

      default:
         lite_PD_error("_UNDECIDABLE CASE - _PD_WR_SYMENT", PD_WRITE) ;
      }
   }
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:    _PD_wr_hyper_index
 *
 * Purpose:     Do the real work of parsing an index expression into
 *              compontents and recursively determine the disk locations
 *              to read into the buffer OUT.  OUT is to be filled in order
 *              from smallest index to largest.  The offset is specified by
 *              the starting address which is ADDR.  FBYT is the number
 *              of bytes in the file for each item to be read. HBYT is the
 *              number of bytes in memory for each item to be read.
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Adapted from PACT
 *              Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
static char *
_PD_wr_hyper_index (PDBfile *file, char *out, dimind *pi,
                    char *intype, char *outtype, long addr,
                    symblock *sp, int hbyt, int fbyt) {

   long offset, stride, start, stop, step;

   /*
    * For each index specification compute the range and recurse.
    */
   stride = fbyt*pi->stride;
   start  = stride*pi->start;
   stop   = stride*pi->stop;
   step   = stride*pi->step;

   stop  = addr + stop - start;
   start = addr;

   /*
    * At the bottom of the recursion do the actual operations.
    */
   if (stride <= (long) fbyt) {
      out = _PD_write_hyper_space(file, out, intype, outtype, sp, hbyt, fbyt,
                                  start, stop, step);
   } else {
      for (offset = start; offset <= stop; offset += step) {
         out = _PD_wr_hyper_index(file, out, pi + 1, intype, outtype,
                                  offset, sp, hbyt, fbyt);
      }
   }

   return(out);
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:    _PD_wr_leaf_members
 *
 * Purpose:     Write the direct leaf data
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Adapted from PACT
 *              Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
static void
_PD_wr_leaf_members (PDBfile *file, char *intype, char *outtype,
                     long nitems, lite_SC_byte *vr) {

   long bytepitem, in_offs, out_offs;
   int ret;
   size_t nb;
   char *svr, *vbuf, *buf;
   FILE *fp;
   defstr *dpf;

   fp = file->stream;

   dpf       = _lite_PD_lookup_type(outtype, file->chart);
   bytepitem = dpf->size;
   if (bytepitem == -1) {
      lite_PD_error("CAN'T GET NUMBER OF BYTES - _PD_WR_LEAF_MEMBERS",
                    PD_WRITE);
   }

   /*
    * Dispatch all other writes.
    */
   if ((dpf->convert > 0) || (strcmp(intype, outtype) != 0)) {
      buf=(char*)lite_SC_alloc(nitems,bytepitem,"_PD_WR_LEAF_MEMBERS:buffer");
      if (buf == NULL) {
         lite_PD_error("CAN'T ALLOCATE MEMORY - _PD_WR_LEAF_MEMBERS",
                       PD_WRITE);
      }
      vbuf     = buf;
      svr      = vr;
      in_offs  = 0L;
      out_offs = 0L;
      lite_PD_convert(&vbuf, &svr, intype, outtype, nitems,
                      file->host_std, file->std, file->host_std,
                      &in_offs, &out_offs,
                      file->host_chart, file->chart, 0, PD_WRITE);
      nb  = io_write(buf, (size_t) bytepitem, (size_t) nitems, fp);
      ret = (nb == nitems) ? TRUE : FALSE;
      SFREE(buf);
   } else {
      nb  = io_write(vr, (size_t) bytepitem, (size_t) nitems, fp);
      ret = (nb == nitems) ? TRUE : FALSE;
   }

   if (!ret) {
      lite_PD_error("BYTE WRITE FAILED - _PD_WR_LEAF_MEMBERS", PD_WRITE);
   }
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_number_refd
 *
 * Purpose:     Compute the number of items pointed to by VR
 *
 * Return:      Success:        Number of items.
 *
 *              Failure:        -1: Score did not allocate the block.
 *                              -2: type is unknown.
 *
 * Programmer:  Robb Matzke
 *              robb@callisto.matzke.cioe.com
 *              Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
long
_lite_PD_number_refd (lite_SC_byte *vr, char *type, HASHTAB *tab) {

   long bytepitem, nitems;

   if (vr == NULL) return(0L);

   if ((nitems = lite_SC_arrlen(vr)) <= 0) return(-1L);

   if ((bytepitem = _lite_PD_lookup_size(type, tab)) == -1) return(-2L);

   nitems /= bytepitem;

   return(nitems);
}
#endif /* PDB_WRITE */



/*-------------------------------------------------------------------------
 * Function:    _PD_wr_ind_itags
 *
 * Purpose:     Handle the memory of pointers and write the itags
 *              correctly.
 *
 * Return:      Success:        TRUE
 *
 *              Failure:        FALSE
 *
 * Programmer:  Robb Matzke
 *              robb@callisto.matzke.cioe.com
 *              Apr 17, 1996
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  7 09:54:39 PST 1998
 *    Remove the caching of pointer references.
 *
 *    Eric Brugger, Thu Sep 23 10:36:58 PDT 1999
 *    Removed unused argument vr.
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
static int
_PD_wr_ind_itags (PDBfile *file, long nitems, char *type) {

   FILE *fp;
   long addr;

   fp = file->stream;

   /*
    * Save the address of the header because
    * we don't know the data address yet
    */
   addr = io_tell(fp);
   if (addr == -1L) {
      lite_PD_error("FAILED TO FIND ADDRESS - _PD_WR_IND_ITAGS", PD_WRITE);
   }

   /*
    * Write some info for the read
    * TRUE if this is the first time for the pointer
    */
   _lite_PD_wr_itag(file, nitems, type, addr, TRUE);

   return(TRUE);
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:    _PD_write_hyper_space
 *
 * Purpose:     Write a part of a hyper-surface to disk
 *              - ADDR is the starting address
 *              - STOP is the upper bound on the address
 *              - STEP is the increment of the address for each entry
 *              - FBYT is the number of bytes in the file for each
 *              - item to be written
 *              - HBYT is the number of bytes in memory for each
 *              - item to be written
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Robb Matzke
 *              robb@callisto.matzke.cioe.com
 *              Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
static char *
_PD_write_hyper_space(PDBfile *file, char *in, char *intype, char *outtype,
                      symblock *sp, int hbyt, int fbyt, long addr,
                      long stop, long step) {

   long nb, eaddr;

   if (step == fbyt) {
      /*
       * Items logically contiguous.
       */
      long nitems, niw;

      nitems = (stop - addr)/step + 1L;

      /*
       * Get writes across blocks correct.
       */
      while (nitems > 0) {
         /*
          * Adjust the address for the correct block.
          */
         eaddr = addr;
         _PD_effective_addr(&eaddr, &nb, fbyt, sp);
         if (io_seek(file->stream, eaddr, SEEK_SET)) {
            lite_PD_error("FSEEK FAILED TO FIND ADDRESS - "
                          "_PD_WRITE_HYPER_SPACE", PD_WRITE);
         }

         /*
          * NOTE: this subverts _PD_effective_addr in part, but because
          * _lite_PD_effective_ep cannot be modified to build an effective
          * syment for indirectly referenced data which cannot be
          * distinguished from an explicitly dimensioned array, this is the
          * best that can be done.
          */
         if ((eaddr == 0) || (nb == 0)) {
            eaddr = addr;
            nb    = nitems;
         }

         niw = MIN(nb, nitems);

         _lite_PD_wr_syment(file, in, niw, intype, outtype);

         nitems -= niw;
         addr   += fbyt*niw;
         in     += hbyt*niw;
      }
   } else {
      /*
       * Items logically discontinuous.
       */
      for (/*void*/; addr <= stop; addr += step, in += hbyt) {
         eaddr = addr;
         _PD_effective_addr(&eaddr, &nb, fbyt, sp);
         if (io_seek(file->stream, eaddr, SEEK_SET)) {
            lite_PD_error("FSEEK FAILED - _PD_WRITE_HYPER_SPACE", PD_WRITE);
         }
         _lite_PD_wr_syment(file, in, 1L, intype, outtype);
      }
   }

   return(in);
}
#endif /* PDB_WRITE */
