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
 * PDPATH.C - grammar driven parser for variable specifications
 *
 * Source Version: 2.0
 * Software Release #92-0043
 *
 */
#include "config.h" /* For a possible redefinition of setjmp/longjmp */
#if !defined(_WIN32)
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#else
#include <silo_win32_compatibility.h>
#endif
#include "pdb.h"

/* The fundamental operations are:
 *        GOTO    - goto the place in memory or on disk implied by the
 *                  locator on the top of the stack
 *        INDEX   - compute the hyper-space shape implied by the
 *                  dimensions on the top of the stack this implies
 *                  an offset from the current location and a
 *                  number of items (max) from the offset
 *                  the current location is changed by offset from
 *                  the previous location
 *        MEMBER  - item on the top of the stack is a member name
 *                  and implies an offset from the current location
 *                  the current location is changed by offset from
 *                  the previous location
 *        DEREF   - assuming the current location is a pointer in
 *                  memory or an itag on disk dereference so that
 *                  the current location is at the pointee
 *        DIGRESS - begin a subroutine which will result with a
 *                - new integer value on the stack upon completion
 *        CAST    - specify an output type that overrides the
 *                - file type
 */

#define MAXPARSEDEPTH 150
#define LASTTOK        42
#define STATEFLAG   -1000

#define GOTO_C    1
#define MEMBER_C  2
#define INDEX_C   3
#define CAST_C    4
#define DEREF_C   5
#define RESULT_C  6

#define ERRCODE      256
#define OPEN_PAREN   257
#define CLOSE_PAREN  258
#define STAR         259
#define DOT          260
#define ARROW        261
#define IDENTIFIER   262
#define COMMA        263
#define COLON        264
#define INTEGER      265

#define input()                                                              \
   FRAME(lex_bf)[FRAME(index)++]

#define unput(c)                                                             \
   (FRAME(index) = (--FRAME(index) < 0) ? 0 : FRAME(index),                  \
    FRAME(lex_bf)[FRAME(index)] = c)

#define GOT_TOKEN(tok)                                                       \
    {if (FRAME(index) == start+1)                                            \
        return(tok);                                                         \
     else                                                                    \
        {unput(c);                                                           \
         return(_PD_next_token(start));};}

#define FRAME(x)   frames[frame_n].x
#define CURRENT(x) FRAME(stack)[FRAME(n)].x

typedef struct s_locator locator;
typedef struct s_parse_frame parse_frame;

struct s_locator {
   char intype[MAXLINE];
   int cmmnd;
   int indirect;
   SC_address ad;
   long number;
   dimdes *dims;
   symblock *blocks;
   long n_struct_ptr;
   long n_array_items;
   symindir indir_info;
};

struct s_parse_frame {
   locator *stack;                      /* locator stack */
   long n;                              /* current top of stack */
   long nx;                             /* allocated size of stack */
   long diskaddr;
   char path[MAXLINE];
   int flag;
   char *lex_bf;
   char *lval;
   char *val;
   char *v[MAXPARSEDEPTH];              /* parser value stack */
   char **pv;                           /* top of parser value stack */
   int current_token;                   /* current input token number */
   int error;                           /* error recovery flag */
   int n_error;                 /* number of errors */
   int state;                           /* current state */
   int tmp;                             /* extra var (lasts between blocks) */
   int s[MAXPARSEDEPTH];                /* parser state stack */
   int *ps;                             /* top of parser state stack */
   int index;
};

static parse_frame      *frames = NULL;
static int              frame_n;
static int              frame_nx;
static PDBfile          *file_s;
static int              colon ;
static char             text[MAXLINE];
static char             msg[MAXLINE];
static long             num_val;
static char             outtype[MAXLINE];

static long             _PD_deref_addr (int) ;
static void             _PD_disp_rules (int,char**) ;
static void             _PD_do_cast (char*) ;
static void             _PD_do_deref (void) ;
static long             _PD_do_digress (char*) ;
static void             _PD_do_goto (char*) ;
static void             _PD_do_index (char*) ;
static void             _PD_do_member (char*,int) ;
static char *           _PD_get_type_member (PDBfile*,char*,char*,memdes*,
                                                 defstr**) ;
static long             _PD_index_deref (int,dimdes**,long*) ;
static int              _PD_is_member (char*,memdes*,HASHTAB*,long*) ;
static int              _PD_lex (void) ;
static long             _PD_member_deref (int) ;
static int              _PD_next_token (int) ;
static long             _PD_num_indirects (char*,HASHTAB*) ;
static void             _PD_parse (void) ;
static long             _PD_reduce (void) ;
static void             _PD_restore_stack (void) ;
static void             _PD_rl_frames (void) ;
static void             _PD_save_stack (void) ;
static void             _PD_shift (char*,char*,dimdes*,symblock*,long,
                                       long,int,int) ;


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_effective_ep
 *
 * Purpose:     Look up the symbol table entry for the named quantity.
 *
 * Return:      Success:        An effective symbol table entry which
 *                              contains the type and dimensions of the
 *                              entire variable(!) and the disk address
 *                              and number of items referred to by the
 *                              hyper-index expression, if any.  If NAME
 *                              contains such a specification the returned
 *                              syment will be newly allocated.
 *
 *              Failure:        NULL
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  4, 1996  1:03 PM EST
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  8 17:26:38 PST 1998
 *    I eliminated some memory leaks.
 *
 *-------------------------------------------------------------------------
 */
syment *
_lite_PD_effective_ep (PDBfile *file, char *name, int flag, char *fullname) {

   int          alloc_frames;
   dimdes       *dims;
   char         *type;
   long         numb, addr;
   symindir     indr;
   symblock     *sp;
   syment       *ep;

   /*
    * To improve performance and to accomodate certain unusual variable names
    *  such as domain names, see if the variable name is literally in the file
    */
   ep = lite_PD_inquire_entry(file, name, flag, fullname);
   if (ep != NULL) return(lite_PD_copy_syment(ep));

   alloc_frames = FALSE;
   if (frames == NULL) {
      alloc_frames = TRUE;
      frame_n  = 0;
      frame_nx = 4;
      frames   = FMAKE_N(parse_frame, frame_nx, "_PD_EFFECTIVE_EP:frames");
      FRAME(stack) = NULL;
      FRAME(nx) = 0;
   }

   FRAME(lex_bf) = lite_SC_strsavef(name, "char*:_PD_EFFECTIVE_EP:lex_bf");
   FRAME(index) = 0;

   FRAME(n) = 0L;
   if (FRAME(stack) == NULL) {
      FRAME(nx) += 10;
      FRAME(stack) = FMAKE_N(locator, 10, "_PD_EFFECTIVE_EP:loc_stack");
   }

   switch (setjmp(_lite_PD_trace_err)) {
   case ABORT:
      if ((fullname != NULL) && flag) strcpy(fullname, name);
      if (alloc_frames) _PD_rl_frames();
      return(NULL);

   case ERR_FREE:
      if (alloc_frames) _PD_rl_frames();
      return(NULL);

   default:
      memset(lite_PD_err, 0, MAXLINE);
      break;
   }

   /*
    * Copy these arguments into global (file static) variables.
    */
   file_s      = file;
   FRAME(flag) = flag;

   _PD_parse();

   _PD_reduce();

   dims = CURRENT(dims);
   type = CURRENT(intype);
   numb = CURRENT(number);
   indr = CURRENT(indir_info);
   addr = CURRENT(ad).diskaddr;
   sp   = CURRENT(blocks);

   ep = _lite_PD_mk_syment(type, numb, addr, &indr, dims);

   if (sp != NULL) {
      SFREE(PD_entry_blocks(ep));
      PD_entry_blocks(ep) = sp;
      lite_SC_mark(sp, 1);
   }

   SFREE(dims);
   SFREE(sp);

   if (fullname != NULL) strcpy(fullname, FRAME(path));
   if (alloc_frames) _PD_rl_frames();

   return(ep);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_rl_frames
 *
 * Purpose:     Free the set parse frames.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  3:20 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_rl_frames (void) {

   SFREE(FRAME(stack));
   SFREE(FRAME(lex_bf));
   SFREE(frames);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_shift
 *
 * Purpose:     Perform a shift operation.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:21 AM EST
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  8 17:26:38 PST 1998
 *    I added calls to lite_SC_mark to bump memory reference counts as
 *    appropriate.
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
static void
_PD_shift (char *name, char *type, dimdes *dims, symblock *blocks,
           long numb, long addr, int indr, int cmmnd) {

   if (type[0] == '\0')
      lite_PD_error("NO TYPE SPECIFIED - _PD_SHIFT", PD_TRACE);

   if (frames == NULL) {
      frame_n  = 0;
      frame_nx = 2;
      frames   = FMAKE_N(parse_frame, frame_nx, "_PD_EFFECTIVE_EP:frames");
   }

   FRAME(n)++;
   if (FRAME(n) >= FRAME(nx)) {
      FRAME(nx) += 10;
      REMAKE_N(FRAME(stack), locator, FRAME(nx));
   }

   memset(FRAME(stack)+FRAME(n), 0, sizeof(locator));

   strcpy(CURRENT(intype), type);

   CURRENT(number)      = numb;
   CURRENT(ad.diskaddr) = addr;
   CURRENT(indirect)    = indr;
   CURRENT(dims)        = dims;
   CURRENT(blocks)      = blocks;
   CURRENT(cmmnd)       = cmmnd;

   lite_SC_mark(dims, 1);
   lite_SC_mark(blocks, 1);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_reduce
 *
 * Purpose:     Reduce the parse three.  This means looping over the
 *              locator stack through the latest GOTO command and
 *              determining a new locator whose intype, dimensions, number,
 *              and address can be used to create a valid effective symbol
 *              table entry or an actual one.  If there is an intermediate
 *              expression on the stack it will be read and the value (which
 *              can only be an index) is returned.
 *
 * Return:      Success:        See above
 *
 *              Failure:        
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  3:05 PM EST
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  8 17:26:38 PST 1998
 *    I added calls to lite_SC_mark to bump memory reference counts as
 *    appropriate.  I eliminated some memory leaks.
 *
 *-------------------------------------------------------------------------
 */
static long
_PD_reduce (void) {

   int i, nmn, nmx, cmnd;
   long addr, val, numb;
   char *type;
   dimdes *dims;
   symblock *sp;
   symindir iloc;

   val = 0L;
   nmx = FRAME(n);

   type = CURRENT(intype);
   numb = CURRENT(number);
   dims = CURRENT(dims);
   lite_SC_mark(dims, 1);

   /*
    * Find the most recent GOTO commmand.
    */
   for (i = nmx; i > 0; i--) {
      cmnd = FRAME(stack)[i].cmmnd;
      if (cmnd == GOTO_C) break;
   }

   nmn  = MAX(i, 1);
   addr = 0L;

   iloc.addr       = 0L;
   iloc.n_ind_type = 0L;
   iloc.arr_offs   = 0L;

   /*
    * Find the actual address of the specified object.
    */
   if (file_s->virtual_internal) {
      addr = FRAME(stack)[nmx].ad.diskaddr;
   } else {
      for (i = nmn; i <= nmx; i++) {
         cmnd = FRAME(stack)[i].cmmnd;
         if (cmnd == DEREF_C) {
            addr = _PD_deref_addr(i);
         } else if (cmnd == INDEX_C) {
            addr = _PD_index_deref(i, &dims, &numb);
            iloc = FRAME(stack)[i].indir_info;
         } else if (cmnd == MEMBER_C) {
            addr = _PD_member_deref(i);
         } else if (cmnd != CAST_C) {
            addr += FRAME(stack)[i].ad.diskaddr;
            FRAME(stack)[i].ad.diskaddr = addr;
         }
         SFREE(FRAME(stack)[i-1].dims);
         SFREE(FRAME(stack)[i-1].blocks);
      }
   }

   /*
    * This must be taken now because the address reduction may have
    * changed the original.
    */
   sp = CURRENT(blocks);

   FRAME(n) = nmn;

   /*
    * If we are not at the bottom of the locator stack we have
    * and intermediate expression which must by read in via _PD_rd_syment.
    */
   if (nmn != 1) {
      syment *ep;

      if (numb != 1L) {
         lite_PD_error("INTERMEDIATE MUST BE SCALAR INTEGER - _PD_REDUCE",
                       PD_TRACE);
      }

      ep = _lite_PD_mk_syment(CURRENT(intype), 1L, addr, NULL, NULL);
      _lite_PD_rd_syment(file_s, ep, "long", &val);
      _lite_PD_rl_syment(ep);

      FRAME(n)--;

   } else {

      /*
       * Otherwise we are at the end of the locator stack and the necessary
       * information to build an effective syment must be filled in the
       * bottom most locator
       */

      strcpy(CURRENT(intype), type);

      CURRENT(number)      = numb;
      CURRENT(ad.diskaddr) = addr;
      CURRENT(blocks)      = sp;
      CURRENT(dims)        = dims;
      CURRENT(indir_info)  = iloc;
      CURRENT(cmmnd)       = RESULT_C;
   }

   return(val);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_do_goto
 *
 * Purpose:     Carry out a goto command.  This should be starting out
 *              with something which is in the symbol table (it is an
 *              error if not).
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 10:50 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_do_goto (char *name) {

   char *type;
   int indr;
   long numb, addr;
   dimdes *dims;
   symblock *sp;
   syment *ep;
   defstr *dp;

   ep = lite_PD_inquire_entry(file_s, name, FRAME(flag), FRAME(path));
   if (ep == NULL) lite_PD_error("NON-EXISTENT ENTRY - _PD_DO_GOTO", PD_TRACE);

   /*
    * Shift the starting point information onto the locator stack.
    */
   numb = PD_entry_number(ep);
   addr = PD_entry_address(ep);
   type = PD_entry_type(ep);
   dims = PD_entry_dimensions(ep);
   sp   = PD_entry_blocks(ep);

   dp = _lite_PD_lookup_type(type, file_s->chart);
   if (dp == NULL) lite_PD_error("UNDEFINED TYPE - _PD_DO_GOTO", PD_TRACE);
   if (dp->size_bits && (addr > 0)) addr *= -SC_BITS_BYTE;

   /*
    * Indirect does NOT mean that the type is indirect but that the
    * entry in the symbol table refers to a dynamically allocated
    * quantity, hence indirect means no dimensions.
    */
   indr = (dims == NULL);

   _PD_shift(name, type, dims, sp, numb, addr, indr, GOTO_C);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_do_member
 *
 * Purpose:     Carry out a member command.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 10:55 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_do_member (char *name, int deref_flag) {

   char *type, t[MAXLINE];
   int indr;
   long addr, numb, nsitems;
   dimdes *dims;
   defstr *dp;
   memdes *desc, *nxt;
   HASHTAB *tab;

   if (file_s->virtual_internal) tab = file_s->host_chart;
   else tab = file_s->chart;

   /*
    * If we came here with the "->" syntax we will need to shift
    * a derefence onto the locator stack ahead of the member shift
    * also update the path while we're at it.
    */
   if (deref_flag) {
      _PD_do_deref();
      if (snprintf(t, sizeof(t), "%s->%s", FRAME(path), name)>=sizeof(t))
         t[sizeof(t)-1] = '\0';
   } else {
      if (snprintf(t, sizeof(t), "%s.%s", FRAME(path), name)>=sizeof(t))
         t[sizeof(t)-1] = '\0';
   }

   strcpy(FRAME(path), t);

   /*
    * NOTE: we had better be properly dereferenced at this point!!!!!!!
    * DO NOT IMAGINE THAT ANYTHING DIFFERENT CAN BE DONE!!!!!!
    */
   type = CURRENT(intype);
   if (_lite_PD_indirection(type))
      lite_PD_error("IMPROPERLY DEREFERENCED EXPRESSION - _PD_DO_MEMBER",
                    PD_TRACE);

   /*
    * Find the defstr whose members are to be searched.
    */
   dp = PD_inquire_table_type(tab, type);
   if (dp == NULL) lite_PD_error("UNKNOWN TYPE - _PD_DO_MEMBER", PD_TRACE);

   /*
    * Loop over the members accumulating offset to the new address
    * and the number of indirect members which will have to
    * be skipped over.
    */
   addr    = 0L;
   nsitems = 0L;
   for (desc = dp->members; desc != NULL; desc = nxt) {
      nxt = desc->next;
      if (_PD_is_member(name, desc, tab, &nsitems)) {
         type = _PD_get_type_member(file_s, FRAME(path), name, desc, &dp);

         addr = desc->member_offs;
         dims = desc->dimensions;
         numb = _lite_PD_comp_num(dims);
         indr = _lite_PD_indirection(type);

         if (file_s->virtual_internal) {
            SC_address ad;

            ad   = FRAME(stack)[FRAME(n)].ad;
            addr = ad.diskaddr + desc->member_offs;
         }

         /*
          * Shift the member onto the locator stack.
          */
         _PD_shift(name, type, dims, NULL,
                   numb, addr, indr, MEMBER_C);
         CURRENT(n_struct_ptr) = nsitems;

         return;
      }
   }

   lite_PD_error("UNKNOWN MEMBER - _PD_DO_MEMBER", PD_TRACE);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_do_deref
 *
 * Purpose:     Carry out a deref command.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 10:47 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_do_deref (void) {

   long addr;
   char t[MAXLINE];

   strcpy(t, CURRENT(intype));

   if (file_s->virtual_internal) {
      SC_address ad;

      ad         = FRAME(stack)[FRAME(n)].ad;
      ad.memaddr = *(char **) ad.memaddr;
      addr       = ad.diskaddr;

   } else {
      addr = 0L;
   }

   _PD_shift("", t, NULL, NULL, -1L, addr, 0, DEREF_C);

   /*
    * Since the shift added a new one this will dereference the current
    * locator.
    */
   lite_PD_dereference(CURRENT(intype));
}


/*-------------------------------------------------------------------------
 * Function:    _PD_do_index
 *
 * Purpose:     Carry out an index command.  This must always set the
 *              current location to point to the first element indexed.
 *              If more than one element is referenced then that information
 *              must be put into the locator for future action.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 10:52 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_do_index (char *expr) {

   int indr;
   long bpi, start, stop, step, numb, doff, addr;
   char t[MAXLINE], s[MAXLINE];
   char *type, *tok;
   dimdes *dims;
   symblock *sp;

   /*
    * Update the path.
    */
   if (snprintf(t, sizeof(t), "%s[%s]", FRAME(path), expr)>=sizeof(t))
      t[sizeof(t)-1] = '\0';
   strcpy(FRAME(path), t);

   dims = CURRENT(dims);
   type = CURRENT(intype);
   doff = PD_get_offset(file_s);

   if (dims != NULL) {
      strcpy(t, type);
      lite_PD_dereference(t);
      numb = _lite_PD_hyper_number(file_s, expr, 1L, dims, &start);
      indr = FALSE;
   } else if (_lite_PD_indirection(type)) {
      _PD_do_deref();

      /*
       * Find the offset which will be the first part of the
       * index expression find the number of items requested.
       */
      strcpy(t, expr);
      tok = lite_SC_firsttok(t, ",");

      strcpy(s, tok);
      tok = strtok(s, ":");
      if (tok == NULL) {
         lite_PD_error("BAD INDEX EXPRESSION - _PD_DO_INDEX", PD_TRACE);
      }

      start = lite_SC_stoi(tok) - doff;

      tok = strtok(NULL, ":");
      if (tok == NULL) stop = start;
      else stop = lite_SC_stoi(tok) - doff;

      step = lite_SC_stoi(strtok(NULL, ":"));
      if (step == 0L) step = 1L;

      numb = (stop - start)/step + 1;


      strcpy(t, CURRENT(intype));
      indr = TRUE;

   } else {
      lite_PD_error("CAN'T INDEX OBJECT - _PD_DO_INDEX", PD_TRACE);
   }

   bpi = _lite_PD_lookup_size(t, file_s->chart);

   if (file_s->virtual_internal) {
      SC_address ad;

      ad   = FRAME(stack)[FRAME(n)].ad;
      addr = ad.diskaddr;

   } else {
      addr = 0L;
   }

   addr += start*bpi;

   sp = CURRENT(blocks);

   _PD_shift(expr, t, dims, sp, numb, addr, indr, INDEX_C);

   CURRENT(n_array_items) = start;
}


/*-------------------------------------------------------------------------
 * Function:    _PD_do_cast
 *
 * Purpose:     Carry out a CAST command.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 10:47 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_do_cast (char *type) {

   int in;
   long n, da;
   char t[MAXLINE], s[MAXLINE];
   symblock *sp;
   dimdes *dm;

   /*
    * Update the path.
    */
   if (snprintf(t, sizeof(t), "(%s) %s", type, FRAME(path))>=sizeof(t))
      t[sizeof(t)-1] = '\0';
   strcpy(FRAME(path), t);

   da = CURRENT(ad.diskaddr);
   in = CURRENT(indirect);
   n  = CURRENT(number);
   sp = CURRENT(blocks);
   dm = CURRENT(dims);

   strcpy(s, CURRENT(intype));

   _PD_shift("", s, dm, sp, n, da, in, CAST_C);

   strcpy(outtype, type);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_do_digress
 *
 * Purpose:     Carry out a digress command.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 10:49 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
static long
_PD_do_digress (char *expr) {

   long val;
   char t[MAXLINE];

   /*
    * Save the path.
    * NOTE: this doesn't support more than one level of recursion!!
    */
   strcpy(t, FRAME(path));

   val = _PD_reduce();    

   /*
    * Restore the path.
    */
   strcpy(FRAME(path), t);

   return(val);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_is_member
 *
 * Purpose:     Determine whether or not the given member is the named
 *              member and return true iff it is.  Also return the updated
 *              number of struct indirections to track via the arg list.
 *
 * Return:      Success:        true or false
 *
 *              Failure:        never fails
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:18 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
static int
_PD_is_member (char *name, memdes *desc, HASHTAB *tab, long *pns) {


   if (strcmp(desc->name, name) == 0) {
      /*
       * If this is the member say so.
       */
      return(TRUE);
   } else {
      /*
       * Count up the number of indirects in the structure which will
       * be skipped.
       */
      if (_lite_PD_indirection(desc->type)) {
         *pns += _lite_PD_member_items(desc->member);
      }
      return(FALSE);
   }
}


/*-------------------------------------------------------------------------
 * Function:    _PD_get_type_member
 *
 * Purpose:     Get the true type of the member.  Handle any casts.
 *
 * Return:      Success:        The type.
 *
 *              Failure:        
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:15 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
static char *
_PD_get_type_member (PDBfile *file, char *path_name, char *name,
                     memdes *desc, defstr **pdp) {

   char *mtype;
   HASHTAB *tab;

   if (file->virtual_internal) tab = file->host_chart;
   else tab = file->chart;

   *pdp = PD_inquire_table_type(tab, desc->base_type);
   if (*pdp == NULL)
      lite_PD_error("UNDEFINED TYPE - _PD_GET_TYPE_MEMBER", PD_TRACE);

   if (desc->cast_offs < 0L) {
      mtype = desc->type;
   } else {
      if (file->virtual_internal) {
         SC_address ad;

         ad    = FRAME(stack)[FRAME(n)].ad;
         mtype = DEREF(ad.memaddr + desc->cast_offs);
         if (mtype == NULL) {
            if (DEREF(ad.memaddr + desc->member_offs) == NULL) {
               mtype = desc->type;
            } else {
               lite_PD_error("NULL CAST TO NON-NULL MEMBER - "
                             "_PD_GET_TYPE_MEMBER", PD_TRACE);
            }
         }

      } else {
         char s[MAXLINE], c;
         int i;

         /*
          * Build the path of the member which points to the real type.
          */
         strcpy(s, path_name);
         for (i = strlen(s) - 1; i >= 0; i--) {
            c = s[i];
            if ((c == '>') || (c == '.')) break;
         }
         s[i+1] = '\0';
         strcat(s, desc->cast_memb);

         _PD_save_stack();

         /*
          * Read the real type in.
          */
         lite_PD_read(file, s, &mtype);
         if (mtype == NULL) mtype = desc->type;

         _PD_restore_stack();
      }
   }

   return(mtype);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_save_stack
 *
 * Purpose:     Save the state of the current parse.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:31 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_save_stack (void) {

   frame_n++;
   if (frame_n >= frame_nx) {
      frame_nx += 2;
      REMAKE_N(frames, parse_frame, frame_nx);
   }

   memset(&frames[frame_n], 0, sizeof(parse_frame));
}


/*-------------------------------------------------------------------------
 * Function:    _PD_restore_stack
 *
 * Purpose:     Restore the state of the previous parse.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:31 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_restore_stack (void) {

   SFREE(FRAME(stack));
   SFREE(FRAME(lex_bf));
   frame_n--;
}


/*-------------------------------------------------------------------------
 * Function:    _PD_deref_addr
 *
 * Purpose:     Dereference a pointer and return the correct address
 *              of the pointee.  The entire parse tree is avaiable to
 *              provide all necessary context.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  4:11 PM EST
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  8 17:26:38 PST 1998
 *    I added calls to lite_SC_mark to bump memory reference counts as
 *    appropriate.
 *
 *-------------------------------------------------------------------------
 */
static long
_PD_deref_addr (int n) {

   long addr, numb, bpi;
   char *type;
   HASHTAB *tab;
   FILE *fp;
   dimdes *dims;
   symblock *sp;

   tab  = file_s->chart;
   type = FRAME(stack)[n-1].intype;
   bpi  = _lite_PD_lookup_size(type, tab);

   /*
    * Handle the case of in memory pointers.
    */
   if (file_s->virtual_internal) {
      addr = FRAME(stack)[n].ad.diskaddr;
      numb = FRAME(stack)[n].number;
   } else {
      /*
       * Handle the case of file pointers
       */
      PD_itag itag;

      addr = FRAME(stack)[n-1].ad.diskaddr;
      numb = FRAME(stack)[n-1].number;

      /*
       * Get past the level that contains the dereference
       * NOTE: PDB declines to write top level pointers which are
       *       useless numbers, it starts in with the pointees and
       *       hence the start of such objects are the itags of the
       *       pointees.
       */
      if (!_lite_PD_indirection(type)) addr += numb*bpi;

      fp = file_s->stream;
      if (io_seek(fp, addr, SEEK_SET)) {
         lite_PD_error("FSEEK FAILED TO FIND DATA - _PD_DEREF_ADDR",
                       PD_TRACE);
      }

      _lite_PD_rd_itag(file_s, &itag);

      addr = io_tell(fp);
      numb = itag.nitems;

      if (!_lite_PD_indirection(FRAME(stack)[n].intype)) {
         sp = FMAKE(symblock, "_PD_DEREF_ADDR:sp");
         sp->number   = numb;
         sp->diskaddr = addr;

         if ((n + 1) == FRAME(n)) {
            dims = _lite_PD_mk_dimensions(file_s->default_offset, numb);
         } else {
            dims = NULL;
         }

         FRAME(stack)[n].blocks = sp;
         FRAME(stack)[n].dims   = dims;

         if (n < FRAME(n)) {
            if (FRAME(stack)[n+1].cmmnd == INDEX_C) {
               FRAME(stack)[n+1].blocks = sp;
               FRAME(stack)[n+1].dims   = dims;
               lite_SC_mark(sp, 1);
               lite_SC_mark(dims, 1);
            }
         }
      }
   }

   FRAME(stack)[n].number      = numb;
   FRAME(stack)[n].ad.diskaddr = addr;

   return(addr);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_index_deref
 *
 * Purpose:     Handle indexing where a pointered type was just
 *              dereferenced.  This will mean skipping over itags and
 *              other pointees.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  4:20 PM EST
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  8 17:26:38 PST 1998
 *    I added calls to lite_SC_mark to bump memory reference counts as
 *    appropriate.  I eliminated some memory leaks.
 *
 *-------------------------------------------------------------------------
 */
static long
_PD_index_deref (int n, dimdes **pdims, long *pnumb) {

   long indx, addr, numb, naitems, bpi;
   char *type, *typc, *typp;
   symblock *nsp;
   symindir iloc;
   FILE *fp;
   HASHTAB *tab;

   nsp = NULL;

   iloc.addr       = 0L;
   iloc.n_ind_type = 0L;
   iloc.arr_offs   = 0L;

   if (file_s->virtual_internal) {
      /*
       * Handle in memory indexing.
       */
      addr = FRAME(stack)[n].ad.diskaddr;
   } else {
      /*
       * Handle file indexing.
       * Start at the address before the latest DEREF.
       */
      typp = FRAME(stack)[n-1].intype;
      type = FRAME(stack)[n].intype;
      typc = FRAME(stack)[n+1].intype;
      indx = FRAME(stack)[n].n_array_items;

      fp  = file_s->stream;
      tab = file_s->chart;

      iloc.n_ind_type = _PD_num_indirects(type, tab);
      iloc.arr_offs   = indx;

      /*
       * In order to know where to go you have to know whether the
       * next thing on the locator stack dereferences a pointer
       */
      if (((n < FRAME(n)) && _lite_PD_indirection(typc)) ||
          _lite_PD_indirection(typp)) {
         numb = FRAME(stack)[n-1].number;
         if ((indx < 0) || (numb < indx))
            lite_PD_error("INDEX OUT OF BOUNDS - _PD_INDEX_DEREF", PD_TRACE);

         /*
          * Handle GOTO, DEREF, INDEX.
          */
         if (FRAME(stack)[n-1].cmmnd == DEREF_C) {
            addr = FRAME(stack)[n-2].ad.diskaddr;
            if (io_seek(fp, addr, SEEK_SET))
               lite_PD_error("FSEEK FAILED TO FIND DATA - _PD_INDEX_DEREF",
                             PD_TRACE);

            /*
             * Skip over the thing that was DEREF'd to where its
             * pointees begin.
             */
            addr = _lite_PD_skip_over(file_s, 1L, TRUE);

            /*
             * Skip over all items before the indexed one.
             */
            numb    = _PD_num_indirects(type, tab);
            naitems = indx*MAX(1, numb);
            addr    = _lite_PD_skip_over(file_s, naitems, FALSE);

         } else {
            /*
             * Handle GOTO, INDEX.
             */
            addr = FRAME(stack)[n-1].ad.diskaddr;

            if (!_lite_PD_indirection(typp)) {
               bpi   = _lite_PD_lookup_size(typp, tab);
               addr += numb*bpi;
               if (io_seek(fp, addr, SEEK_SET))
                  lite_PD_error("FSEEK FAILED TO FIND DATA - _PD_INDEX_DEREF",
                                PD_TRACE);

               /*
                * Skip over all items before the indexed one.
                */
               numb    = _PD_num_indirects(typp, tab);
               naitems = indx*MAX(1, numb);
               addr    = _lite_PD_skip_over(file_s, naitems, FALSE);
            } else {
               /* NOTE: if we get here, then we have an array of pointers (the
                *       data for which is not written by PDB - the pointers are
                *       meaningless numbers) consequently we are staring at the
                *       ITAG of the first pointee
                */
               PD_itag itag;

               /*
                * Be sure that we are at the first ITAG.
                */
               if (io_seek(fp, addr, SEEK_SET))
                  lite_PD_error("FSEEK FAILED - _PD_INDEX_DEREF",
                                PD_TRACE);

               *pdims = NULL;

               /*
                * Skip over to the indexed element.
                */
               numb    = _PD_num_indirects(typp, tab);
               naitems = indx*MAX(1, numb);
               addr    = _lite_PD_skip_over(file_s, naitems, FALSE);

               _lite_PD_rd_itag(file_s, &itag);
               if (!itag.flag) {
                  if (io_seek(fp, addr, SEEK_SET))
                     lite_PD_error("FSEEK FAILED - _PD_INDEX_DEREF",
                                   PD_TRACE);
                  _lite_PD_rd_itag(file_s, &itag);
               }

               numb   = itag.nitems;
               *pnumb = numb;
               FRAME(stack)[n].number   = numb;

               /*
                * After doing one index the next thing has to be contiguous.
                */
               SFREE(FRAME(stack)[n+1].blocks);

               addr   = io_tell(fp);
            }
         }
      } else {
         /*
          * Handle direct types simply.
          * GOTCHA: it is a temporary measure to pass the old dimensions
          *         up the stack the correct thing to do is to distinguish
          *         between the dimensions of the source and the effective
          *         dimension of the target.  This will never be right until
          *         then.
          */
         symblock *sp;
         long nbl, nbb;

         if (*pdims == NULL) {
            *pdims = FRAME(stack)[n].dims;
            lite_SC_mark(FRAME(stack)[n].dims, 1);
         }

         SFREE(FRAME(stack)[n].dims);
         FRAME(stack)[n].dims = FRAME(stack)[n-1].dims;
         lite_SC_mark(FRAME(stack)[n-1].dims, 1);
         addr  = FRAME(stack)[n-1].ad.diskaddr;

         sp    = FRAME(stack)[n].blocks;
         numb  = FRAME(stack)[n].ad.diskaddr;
         bpi   = _lite_PD_lookup_size(type, tab);

         nbl       = FRAME(stack)[n-1].number;
         iloc.addr = addr + nbl*bpi;

         /*
          * Deal with multiblock entries.
          */
         nsp = NULL;

         /* NOTE: it is not the most general thing to assume that bitstreams
          *       (indicated by negative addresses) must be contiguous although
          *       all current examples are
          */
         if ((sp != NULL) && (addr >= 0)) {
            nbl = lite_SC_arrlen(sp)/sizeof(symblock);

            /*
             * Find out which block we got into.
             */
            while (TRUE) {
               nbb  = sp->number*bpi;
               addr = sp->diskaddr;
               if (numb < nbb) break;

               numb -= nbb;
               sp++;
               nbl--;
            }

            iloc.addr = addr + nbb;

            /*
             * Make a copy of the remaining blocks for the effective entry.
             */
            if (nbl > 0) {
               int i;

               nsp = FMAKE_N(symblock, nbl, "_PD_INDEX_DEREF:nsp");
               for (i = 0; i < nbl; i++) nsp[i] = *sp++;
            }

            /*
             * Adjust the first block to be consistent with the rest
             * of the locator.
             */
            nsp[0].number   -= numb/bpi;
            nsp[0].diskaddr  = addr + numb;
         }

         if (addr < 0) {
            defstr *dp;

            dp = PD_inquire_table_type(tab, type);
            addr -= (numb/bpi)*dp->size_bits;
         } else {
            *pnumb = FRAME(stack)[n].number;
            addr += numb;
         }
      }
   }

   SFREE(FRAME(stack)[n].blocks);
   FRAME(stack)[n].blocks      = nsp;
   FRAME(stack)[n].ad.diskaddr = addr;
   FRAME(stack)[n].indir_info  = iloc;

   return(addr);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_member_deref
 *
 * Purpose:     Find the member where a pointered type was just
 *              dereferenced.  This will mean skipping over itags and
 *              other pointees.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  4:38 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static long
_PD_member_deref (int n) {

   long addr, nsitems;
   char *type;

   /*
    * Handle in memory members.
    */
   if (file_s->virtual_internal) {
      addr = FRAME(stack)[n].ad.diskaddr;
   } else {
      /*
       * Handle file members.
       */
      int indir, cmmnd;
      long bpi, numb;

      cmmnd = FRAME(stack)[n-1].cmmnd;
      indir = _lite_PD_indirection(FRAME(stack)[n].intype);
      if ((cmmnd == GOTO_C) && indir) {
         addr = FRAME(stack)[n-1].ad.diskaddr;
         type = FRAME(stack)[n-1].intype;
         numb = FRAME(stack)[n-1].number;
         bpi  = _lite_PD_lookup_size(type, file_s->chart);

         addr += bpi*numb;

         if (io_seek(file_s->stream, addr, SEEK_SET))
            lite_PD_error("FSEEK FAILED TO FIND DATA - _PD_MEMBER_DEREF",
                          PD_TRACE);

      } else if ((cmmnd != INDEX_C) && indir) {
         addr = FRAME(stack)[n-2].ad.diskaddr;

         if (io_seek(file_s->stream, addr, SEEK_SET))
            lite_PD_error("FSEEK FAILED TO FIND DATA - _PD_MEMBER_DEREF",
                          PD_TRACE);

         /*
          * Skip over the thing that was DEREF'd to where its pointees begin.
          */
         addr = _lite_PD_skip_over(file_s, 1L, TRUE);
         
      } else {
         /*
          * Start at the address in the previous locator.
          */
         addr = FRAME(stack)[n-1].ad.diskaddr;
      }

      /*
       * Handle indirect types differently from direct ones.
       */
      type = FRAME(stack)[n].intype;
      if (_lite_PD_indirection(type)) {
         nsitems = FRAME(stack)[n].n_struct_ptr;

         if (io_seek(file_s->stream, addr, SEEK_SET))
            lite_PD_error("FSEEK FAILED TO FIND DATA - _PD_MEMBER_DEREF",
                          PD_TRACE);

         /*
          * Skip over all items before the specified member.
          */
         addr = _lite_PD_skip_over(file_s, nsitems, FALSE);
         
      } else {
         /*
          * Handle direct types simply.
          */
         addr += FRAME(stack)[n].ad.diskaddr;
      }
   }

   FRAME(stack)[n].ad.diskaddr = addr;

   return(addr);
}


/*-------------------------------------------------------------------------
 * Function:    _lite_PD_skip_over
 *
 * Purpose:     Given a number of units, skip over that many units
 *              including subunits referenced by the top level units.  If
 *              noind is true don't pick up the additional indirects.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  4:56 PM EST
 *
 * Modifications:
 *     Brad Whitlock, Wed Feb 23 19:01:08 PST 2000
 *     Added code to skip some logic when an itag cannot be read.
 *
 *-------------------------------------------------------------------------
 */
long
_lite_PD_skip_over (PDBfile *file, long skip, int noind) {

   long bytepitem, addr;
   int indir;
   FILE *fp;
   HASHTAB *tab;
   PD_itag itag;

   fp  = file->stream;
   tab = file->chart;

   while (skip-- > 0L)
   {
       if(TRUE == _lite_PD_rd_itag(file, &itag))
       {
           /*
            * Note whether this is an indirection.
            */
           indir = _lite_PD_indirection(itag.type);

           /*
            * If noind is TRUE don't pick up the indirects.
            */
          if (noind == FALSE)
          {
              /*
               * If it is an indirection we have more to skip over.
               */
              if (indir) skip += itag.nitems;

              /*
               * If it is a structure with indirections we have more to
               * skip over.
               */
             skip += itag.nitems*_PD_num_indirects(itag.type, tab);
          }

          /*
           * If it was not a NULL pointer find it.
           */
          if ((itag.addr != -1L) && (itag.nitems != 0L))
          {
              if (!itag.flag && (skip == -1))
              {
                  if (io_seek(fp, itag.addr, SEEK_SET))
                      lite_PD_error("CAN'T FIND REAL DATA - _PD_SKIP_OVER",
                                    PD_TRACE);
                  _lite_PD_rd_itag(file, &itag);
              }

              /*
               * Layered indirects have no "data" bytes written out to be
               * skipped over.
               */
              if (!indir)
              {
                  bytepitem = _lite_PD_lookup_size(itag.type, tab);
                  if (bytepitem == -1)
                      lite_PD_error("CAN'T FIND NUMBER OF BYTES - _PD_SKIP_OVER",
                                    PD_TRACE);
              }
              else
              {
                  bytepitem = 0;
              }

              /*
               * If its here, step over the data.
               */
              if (itag.flag && (skip > -1))
              {
                  addr = bytepitem*itag.nitems;
                  if (!indir)
                      if (io_seek(fp, addr, SEEK_CUR))
                          lite_PD_error("CAN'T SKIP TO ADDRESS - _PD_SKIP_OVER",
                                        PD_TRACE);
              }
          }
      } /* end if (_lite_PD_rd_itag(...) == TRUE). */
   } /* end while */

   addr = io_tell(fp);

   return(addr);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_num_indirects
 *
 * Purpose:     Count up the number of members of the given structure
 *              with indirect references.
 *
 * Return:      Success:        Number of indirect references.
 *
 *              Failure:        lite_PD_error()
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:06 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static long
_PD_num_indirects (char *type, HASHTAB *tab) {

   char *mtype;
   defstr *dp;

   mtype = _lite_PD_member_base_type(type);
   dp    = PD_inquire_table_type(tab, mtype);
   SFREE(mtype);

   if (dp == NULL) {
      lite_PD_error("CAN'T FIND TYPE - _PD_NUM_INDIRECTS", PD_TRACE);
   }

   return(dp->n_indirects);
}

/*--------------------------------------------------------------------------*/
/*                          LEXICAL SCANNER ROUTINES                        */
/*--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Function:    _PD_lex
 *
 * Purpose:     Lexical scanner called by the generated parser.  Text of
 *              identifiers is put in the global variable TEXT.  The
 *              numerical value of an integer token is put in the global
 *              variable NUM_VAL.  Legal token values are:
 *
 *              OPEN_PAREN       ( or [                                   
 *              CLOSE_PAREN      ) or ]                                   
 *              DOT              .                                        
 *              COMMA            ,                                        
 *              COLON            :                                        
 *              STAR             *                                        
 *              ARROW            ->                                       
 *              INTEGER          octal, decimal, or hexidecimal integer   
 *              IDENTIFIER       just about anything else (no white space)
 *              
 * Return:      Success:        The value of the lexical token.
 *
 *              Failure:        0 if at the end of the input string.
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  4:27 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
_PD_lex (void) {

   int c, d, start;

   start = FRAME(index);
   while (TRUE) {
      c = input();
      switch (c) {
      case '\0' :
         if (FRAME(index) == start+1) {
            unput(c);
            return(0);
         } else {
            unput(c);
            return(_PD_next_token(start));
         }

      case '(' :
      case '[' :
         GOT_TOKEN(OPEN_PAREN);

      case ')' :
      case ']' :
         GOT_TOKEN(CLOSE_PAREN);

      case '.' :
         GOT_TOKEN(DOT);

      case ',' :
         GOT_TOKEN(COMMA);

      case ':' :
         GOT_TOKEN(COLON);

      case '*' :
         GOT_TOKEN(STAR);

      case '-' :
         d = input();
         if (d == '>') {
            if (FRAME(index) == start+2) {
               return(ARROW);
            } else {
               unput(d);
               unput(c);
               return(_PD_next_token(start));
            }
         }

      default :
         break;
      }
   }
}


/*-------------------------------------------------------------------------
 * Function:    _PD_next_token
 *
 * Purpose:     Figure out whether the specified token is an identifier
 *              or an integer and take the apropriate action.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  6, 1996 11:04 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
_PD_next_token (int start) {

   int nc;
   char *end, s[MAXLINE], *tok;

   nc = FRAME(index) - start;
   strncpy(s, FRAME(lex_bf)+start, nc);
   s[nc] = '\0';

   /*
    * Eliminate whitespace from either end of the token.
    * NOTE: things like "a b" are illegal anyway.
    */
   tok = strtok(s, " \t\f\n\r");
   strcpy(text, tok);

   num_val = _lite_SC_strtol(text, &end, 0);
   tok     = text + strlen(text);
   if (tok == end) return(INTEGER);
   else return(IDENTIFIER);
}


/*-------------------------------------------------------------------------
 * Function:    _PD_parse
 *
 * Purpose:     Parse an expression which is in the lexical buffer of the
 *              current parse frame.
 *
 * Return:      Success:        TRUE
 *
 *              Failure:        FALSE
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  2:19 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_parse (void) {

   char **pvt;
   register char **lpv;         /* top of value stack */
   register int *lps;           /* top of state stack */
   register int lstate;         /* current state */
   register int  n;             /* internal state number info */
   register int len;

   static int exca[] = {-1, 1, 0, -1, -2, 0,} ;
   static int act[]  = { 3, 29,  5, 26, 24,  7,  7,  9, 19, 25,
                         3, 18,  5, 10, 17,  7, 11, 12, 14, 15,
                         20,  1, 16,  4,  6,  8, 13,  2,  0,  0,
                         0,  0,  0,  0,  0, 23, 21, 22, 28,  0,
                         27, 30} ;
   static int pact[] = { -247, -1000, -1000,  -255,  -244,  -247,
                         -1000, -1000,  -240, -1000,
                         -257,  -256,  -256, -1000,  -247, -1000,
                         -254, -1000,  -261, -1000,
                         -1000, -1000, -1000, -1000, -1000,  -257,
                         -257, -1000,  -263,  -257, -1000};
   static int pgo[] = {0, 20, 27, 25, 23, 24, 22, 14, 11};
   static int r1[]  = {0, 1, 1, 1, 3, 3, 2, 2, 4, 4,
                       4, 4, 6, 6, 7, 7, 7, 8, 8, 5};
   static int r2[]  = {0, 2, 9, 1, 3, 5,  2, 5, 3, 9,
                       7, 7, 2, 7, 2, 7, 11, 3, 3, 3};
   static int chk[] = {-1000,  -1,  -2, 257,  -4, 259,  -5, 262,  -3, 262,
                       257, 260, 261,  -1, 258, 259,  -6,  -7,  -8, 265,
                       -1,  -5,  -5,  -1, 258, 263, 264,  -7,  -8, 264, -8};
   static int def[] = { 3, -2,  1, 0, 6, 3, 8, 19,  0,  4,
                        3,  0,  0, 7, 3, 5, 0, 12, 14, 17,
                        18, 10, 11, 2, 9, 3, 3, 13, 15,  3, 16};

   static int negative_one = -1;

   /*
    * Initialize externals - _PD_parse may be called more than once.
    */
   FRAME(pv) = &FRAME(v)[negative_one];
   FRAME(ps) = &FRAME(s)[negative_one];

   FRAME(state)         = 0;
   FRAME(tmp)           = 0;
   FRAME(n_error)       = 0;
   FRAME(error)         = 0;
   FRAME(current_token) = -1;

   lpv    = FRAME(pv);
   lps    = FRAME(ps);
   lstate = FRAME(state);

   colon = FALSE;

   /*
    * loop as expressions are pushed onto the stack.
    */
   for (;;) {
      /*
       * Put a state and value onto the stacks.
       */
      if (++lps >= &FRAME(s)[MAXPARSEDEPTH])
         lite_PD_error("STACK OVERFLOW - _PD_PARSE", PD_TRACE);

      *lps   = lstate;
      *++lpv = FRAME(val);

      /*
       * We have a new state - find out what to do.
       */
      n = pact[lstate];
      if (n > STATEFLAG) {
         if ((FRAME(current_token) < 0) &&
             ((FRAME(current_token) = _PD_lex()) < 0))
            FRAME(current_token) = 0;

         /*
          * Valid shift.
          */
         n += FRAME(current_token);
         if ((n >= 0) && (n < LASTTOK)) {
            n = act[n];
            if (chk[n] == FRAME(current_token)) {
               FRAME(current_token) = -1;
               FRAME(val) = FRAME(lval);

               lstate = n;
               if (FRAME(error) > 0) FRAME(error)--;
               continue;
            }
         }
      }

      n = def[lstate];
      if (n == -2) {
         int *xi;

         if ((FRAME(current_token) < 0) &&
             ((FRAME(current_token) = _PD_lex()) < 0))
            FRAME(current_token) = 0;

         /*
          * Look through exception table.
          */
         xi = exca;

         while ((*xi != -1) || (xi[1] != lstate)) {
            xi += 2;
         }

         while ((*(xi += 2) >= 0) && (*xi != FRAME(current_token))) /*void*/ ;

         n = xi[1];
         if (n < 0) return;
      }

      /*
       * Check for syntax error.
       */
      if (n == 0) {
         if (FRAME(error) > 0)
            lite_PD_error("SYNTAX ERROR - _PD_PARSE", PD_TRACE);
      }

      /*
       * Reduction by production n.
       */
      FRAME(tmp) = n;           /* value to switch over */
      pvt = lpv;                        /* top of value stack */

      /*
       * Look in goto table for next state.
       * If r2[n] doesn't have the low order bit set
       * then there is no action to be done for this reduction
       * and no saving/unsaving of registers done.
       */
      len = r2[n];
      if (!(len & 01)) {
         len >>= 1;
         lpv -= len;
         FRAME(val) = lpv[1];

         n = r1[n];
         lps -= len;
         lstate = pgo[n] + *lps + 1;
         if ((lstate >= LASTTOK) ||
             (chk[lstate = act[lstate]] != -n)) {
            lstate = act[pgo[n]];
         }

         continue;
      }

      len >>= 1;
      lpv -= len;
      FRAME(val) = lpv[1];

      n   = r1[n];
      lps -= len;
      lstate = pgo[n] + *lps + 1;
         
      if ((lstate >= LASTTOK) ||
          (chk[lstate = act[lstate]] != -n)) {
         lstate = act[pgo[n]];
      }

      /*
       * Save until reenter driver code.
       */
      FRAME(state) = lstate;
      FRAME(ps)    = lps;
      FRAME(pv)    = lpv;

      _PD_disp_rules(FRAME(tmp), pvt);

      lpv    = FRAME(pv);
      lps    = FRAME(ps);
      lstate = FRAME(state);
   }
}


/*-------------------------------------------------------------------------
 * Function:    _PD_disp_rules
 *
 * Purpose:     Dispatch on the specified rule.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT PDB
 *              Mar  5, 1996  4:14 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
_PD_disp_rules (int rule, char **pvt) {

   switch (rule) {

      /* variable_expression :
       *      unary_expression
       *    | OPEN_PAREN type CLOSE_PAREN variable_expression
       */
   case 2:
      _PD_do_cast(pvt[-2]);
      break;

      /*    | */
   case 3:
      break;

      /* type :
       *      IDENTIFIER
       */
   case 4:
      FRAME(val) = text;
      break;

      /*    | type STAR */
   case 5:
      sprintf(msg, "%s *", pvt[-1]);
      FRAME(val) = msg;
      break;

      /* unary_expression :
       *      postfix_expression
       *    | STAR variable_expression
       */
   case 7:
      _PD_do_deref();
      break;

      /* postfix_expression :
       *      primary_expression
       */
   case 8:
      _PD_do_goto(pvt[-0]);
      break;

      /*    | postfix_expression OPEN_PAREN index_expression CLOSE_PAREN */
   case 9:
      _PD_do_index(pvt[-1]);
      SFREE(pvt[-1]);
      break;

      /*    | postfix_expression DOT primary_expression */
   case 10:
      _PD_do_member(pvt[-0], FALSE);
      break;

      /*    | postfix_expression ARROW primary_expression */
   case 11:
      _PD_do_member(pvt[-0], TRUE);
      break;

      /* index_expression :
       *         range
       *       | index_expression COMMA range
       */
   case 13:
      sprintf(msg, "%s,%s", pvt[-2], pvt[-0]);
      SFREE(pvt[-2]);
      SFREE(pvt[-0]);
      FRAME(val) = lite_SC_strsavef(msg, "char*:PARSE:COMMA");
      break;

      /* range : index
       *       | index COLON index
       */
   case 15:
      if (strcmp(pvt[-2], pvt[-0]) != 0) colon = TRUE;
      sprintf(msg, "%s:%s", pvt[-2], pvt[-0]);
      SFREE(pvt[-2]);
      SFREE(pvt[-0]);
      FRAME(val) = lite_SC_strsavef(msg, "char*:PARSE:COLON");
      break;

      /*       | index COLON index COLON index */
   case 16:
      if (strcmp(pvt[-4], pvt[-2]) != 0) colon = TRUE;
      sprintf(msg, "%s:%s:%s", pvt[-4], pvt[-2], pvt[-0]);
      SFREE(pvt[-4]);
      SFREE(pvt[-2]);
      SFREE(pvt[-0]);
      FRAME(val) = lite_SC_strsavef(msg, "char*:PARSE:COLON:COLON");
      break;

      /* index : INTEGER */
   case 17:
      sprintf(msg, "%ld", num_val);
      FRAME(val) = lite_SC_strsavef(msg, "char*:PARSE:INTEGER");
      break;

      /*       | variable_expression */
   case 18:
      sprintf(msg, "%ld", _PD_do_digress(pvt[-0]));
      FRAME(val) = lite_SC_strsavef(msg, "char*:PARSE:VARIABLE_EXPRESSION");
      break;

      /* primary_expression : IDENTIFIER */
   case 19:
      if (colon)
         lite_PD_error("HYPERINDEX ON NON-TERMINAL NODE - _PD_DISP_RULES",
                       PD_TRACE);
      FRAME(val) = text;
      break;
   }
}
