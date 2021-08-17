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
 * PDBMM.C - memory management for the PDB library system
 *
 * Source Version: 9.0
 * Software Release #92-0043
 *
 */
#include <limits.h>
#include "pdb.h"


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_mk_pdb
 *
 * Purpose:	Construct and return a pointer to a PDBFile
 *
 * Return:	Success:	Ptr to a new PDB file
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996 11:45 AM EST
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  7 09:50:45 PST 1998
 *    Remove the caching of pointer references.
 *
 *    Eric Brugger, Tue Dec  8 15:16:07 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *    Mark C. Miller, Fri Apr 13 22:35:57 PDT 2012
 *    Added options arg and S,M,L,XL hash table size options. Added
 *    ignore_apersand_ia_ptr_syms option.
 *-------------------------------------------------------------------------
 */
PDBfile *
_lite_PD_mk_pdb (char *name, const char *options) {

   PDBfile *file;
   int symtsz = HSZMEDIUM;

   file = FMAKE(PDBfile, "_PD_MK_PDB:file");
   if (file == NULL) return(NULL);

   file->stream     = NULL;
   file->name       = lite_SC_strsavef(name, "char*:_PD_MK_PDB:name");
   file->type       = NULL;

   if (strchr(options, 's')) symtsz = HSZSMALL;
   else if (strchr(options, 'm')) symtsz = HSZMEDIUM;
   else if (strchr(options, 'l')) symtsz = HSZLARGE;
   else if (strchr(options, 'x')) symtsz = HSZXLARGE;
   else symtsz = HSZMEDIUM;

   file->symtab     = lite_SC_make_hash_table(symtsz, NODOC);
   file->chart      = lite_SC_make_hash_table(1, NODOC);
   file->host_chart = lite_SC_make_hash_table(1, NODOC);
   file->attrtab    = NULL;
   file->mode       = 0;            /* read only, write only, read-write ? */

   file->maximum_size     = LONG_MAX;                       /* family info */
   file->previous_file    = NULL;

   file->flushed          = FALSE;                       /* born unflushed */
   file->virtual_internal = FALSE;                 /* disk file by default */
   file->current_prefix   = NULL;       /* read/write variable name prefix */
   file->system_version   = 0;

   file->default_offset = 0;           /* default offset for array indexes */
   file->major_order    = ROW_MAJOR_ORDER;

   file->std   = NULL;
   file->align = NULL;
   file->host_std   = _lite_PD_copy_standard(lite_INT_STANDARD);
   file->host_align = _lite_PD_copy_alignment(lite_INT_ALIGNMENT);

   file->symtaddr = 0L;
   file->chrtaddr = 0L;
   file->headaddr = 0L;

   file->ignore_apersand_ptr_ia_syms = 0;
   if (strchr(options, 'i')) file->ignore_apersand_ptr_ia_syms = 1;

   return(file);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_rl_pdb
 *
 * Purpose:	Release the storage associated with the PDBfile
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996 12:03 PM EST
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  7 10:51:58 PST 1998
 *    Removed call to lite_PD_reset_ptr_list since it was removed.  I
 *    added a call to free current_prefix to close a memory leak.
 *
 *-------------------------------------------------------------------------
 */
void
_lite_PD_rl_pdb (PDBfile *file) {

   SFREE(file->date);

   _lite_PD_rl_standard(file->std);
   _lite_PD_rl_standard(file->host_std);
   _lite_PD_rl_alignment(file->align);
   _lite_PD_rl_alignment(file->host_align);

   if (file->attrtab != NULL) _lite_PD_clr_table(file->attrtab, NULL);

   _lite_PD_clr_table(file->host_chart,(FreeFuncType)_lite_PD_rl_defstr);
   _lite_PD_clr_table(file->chart,(FreeFuncType)_lite_PD_rl_defstr);
   _lite_PD_clr_table(file->symtab,(FreeFuncType)_lite_PD_rl_syment_d);

   if (file->previous_file != NULL) SFREE(file->previous_file);

   if (file->current_prefix != NULL) SFREE(file->current_prefix);

   if (file->type != NULL) SFREE(file->type);

   if (lite_LAST != NULL) SFREE(lite_LAST);

   if (lite_PD_DEFSTR_S != NULL) SFREE(lite_PD_DEFSTR_S);
   lite_PD_DEFSTR_S = NULL;

   if (lite_PD_SYMENT_S != NULL) SFREE(lite_PD_SYMENT_S);
   lite_PD_SYMENT_S = NULL;

   if (lite_io_close_hook == (PFfclose) _lite_PD_pio_close)
      lite_io_close_hook = (PFfclose) fclose;

   if (lite_io_seek_hook == (PFfseek) _lite_PD_pio_seek)
      lite_io_seek_hook = (PFfseek) fseek;

   if (lite_io_printf_hook == (PFfprintf) _lite_PD_pio_printf)
      lite_io_printf_hook = (PFfprintf) fprintf;

   SFREE(file->name);
   SFREE(file);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_clr_table
 *
 * Purpose:	Release the storage associated with a homogeneous hash
 *		table.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  1:36 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
_lite_PD_clr_table (HASHTAB *tab, FreeFuncType rel) {

   int i, n;
   hashel **tb, *hp, *nxt;

   n  = tab->size;
   tb = tab->table;
   for (i = 0; i < n; i++) {
      for (hp = tb[i]; hp != NULL; hp = nxt) {
	 nxt = hp->next;
	 SFREE(hp->name);
	 if (rel != NULL) (*rel)(hp->def);
	 SFREE(hp);
      }
      tb[i] = NULL;
   }

   lite_SC_rl_hash_table(tab);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_mk_standard
 *
 * Purpose:	Allocate, initialize, and return a pointer to a
 *		data standard.
 *
 * Return:	Success:	Ptr to the new data standard struct.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  2:15 PM EST
 *
 * Modifications:
 *
 *   Mark C. Miller, Fri Nov 13 15:33:42 PST 2009
 *   Added support for long long datatype.
 *
 *   Mark C. Miller, Tue Nov 17 22:23:42 PST 2009
 *   Changed support for long long to match more closely what PDB
 *   proper does.
 *-------------------------------------------------------------------------
 */
data_standard *
_lite_PD_mk_standard (void) {

   data_standard *std;

   std = FMAKE(data_standard, "_PD_MK_STANDARD:std");

   std->ptr_bytes     = 0;
   std->short_bytes   = 0;
   std->short_order   = 0;
   std->int_bytes     = 0;
   std->int_order     = 0;
   std->long_bytes    = 0;
   std->long_order    = 0;
   std->longlong_bytes = 0;
   std->longlong_order = 0;
   std->float_bytes   = 0;
   std->float_format  = NULL;
   std->float_order   = NULL;
   std->double_bytes  = 0;
   std->double_format = NULL;
   std->double_order  = NULL;

   return(std);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_copy_standard
 *
 * Purpose:	Copy the given data standard.
 *
 * Return:	Success:	Ptr to the new data standard struct.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 12:59 PM EST
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:16:07 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *   Mark C. Miller, Fri Nov 13 15:33:42 PST 2009
 *   Added support for long long datatype.
 *
 *   Mark C. Miller, Tue Nov 17 22:23:42 PST 2009
 *   Changed support for long long to match more closely what PDB
 *   proper does.
 *-------------------------------------------------------------------------
 */
data_standard *
_lite_PD_copy_standard (data_standard *src) {

   data_standard *std;
   int 		j, n;
   int 		*ostd, *osrc;
   long 	*fstd, *fsrc;

   std = FMAKE(data_standard, "_PD_COPY_STANDARD:std");

   std->ptr_bytes    = src->ptr_bytes;
   std->short_bytes  = src->short_bytes;
   std->short_order  = src->short_order;
   std->int_bytes    = src->int_bytes;
   std->int_order    = src->int_order;
   std->long_bytes   = src->long_bytes;
   std->long_order   = src->long_order;
   std->longlong_bytes = src->longlong_bytes;
   std->longlong_order = src->longlong_order;
   std->float_bytes  = src->float_bytes;
   std->double_bytes = src->double_bytes;
   
   n    = lite_FORMAT_FIELDS;
   std->float_format  = FMAKE_N(long, n, "_PD_COPY_STANDARD:float_format");
   fstd = std->float_format;
   fsrc = src->float_format;
   for (j = 0; j < n; j++, *(fstd++) = *(fsrc++)) /*void*/ ;

   n    = std->float_bytes;
   std->float_order   = FMAKE_N(int,  n, "_PD_COPY_STANDARD:float_order");
   ostd = std->float_order;
   osrc = src->float_order;
   for (j = 0; j < n; j++, *(ostd++) = *(osrc++)) /*void*/ ;

   n    = lite_FORMAT_FIELDS;
   std->double_format = FMAKE_N(long, n, "_PD_COPY_STANDARD:double_format");
   fstd = std->double_format;
   fsrc = src->double_format;
   for (j = 0; j < n; j++, *(fstd++) = *(fsrc++)) /*void*/ ;

   n    = std->double_bytes;
   std->double_order  = FMAKE_N(int,  n, "_PD_COPY_STANDARD:double_order");
   ostd = std->double_order;
   osrc = src->double_order;
   for (j = 0; j < n; j++, *(ostd++) = *(osrc++)) /*void*/ ;

   return(std);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_rl_standard
 *
 * Purpose:	Release a data standard structure.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996  1:01 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void 
_lite_PD_rl_standard (data_standard *std) {

   if (lite_SC_arrlen(std) > 0) {
      SFREE(std->float_format);
      SFREE(std->float_order);
      SFREE(std->double_format);
      SFREE(std->double_order);
      SFREE(std);
   }
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_mk_alignment
 *
 * Purpose:	Allocate, initialize and return a pointer to a
 *		data_alignment.
 *
 * Return:	Success:	Ptr to the new data_alignment struct.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  2:04 PM EST
 *
 * Modifications:
 *
 *   Mark C. Miller, Fri Nov 13 15:33:42 PST 2009
 *   Added support for long long datatype.
 *
 *   Mark C. Miller, Tue Nov 17 22:23:42 PST 2009
 *   Changed support for long long to match more closely what PDB
 *   proper does.
 *-------------------------------------------------------------------------
 */
data_alignment *
_lite_PD_mk_alignment (char *vals) {

   data_alignment *align;

   align = FMAKE(data_alignment, "_PD_MK_ALIGNMENT:align");

   align->char_alignment   = vals[0];
   align->ptr_alignment    = vals[1];
   align->short_alignment  = vals[2];
   align->int_alignment    = vals[3];
   align->long_alignment   = vals[4];
   align->longlong_alignment = vals[4]; /* default same as long */
   align->float_alignment  = vals[5];
   align->double_alignment = vals[6];

   if (strlen(vals) > 7) align->struct_alignment = vals[7];
   else align->struct_alignment = 0;


   return(align);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_copy_alignment
 *
 * Purpose:	Copies a data_alignment structure.
 *
 * Return:	Success:	Ptr to new data alignment.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 12:56 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
data_alignment *
_lite_PD_copy_alignment (data_alignment *src) {

   data_alignment *align;

   align = FMAKE(data_alignment, "_PD_COPY_ALIGNMENT:align");
   *align = *src;
   return(align);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_rl_alignment
 *
 * Purpose:	Release a data alignment structure.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 12:57 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
_lite_PD_rl_alignment (data_alignment *align) {

   if (lite_SC_arrlen(align) > 0) {
      SFREE(align);
   }
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_copy_dims
 *
 * Purpose:	Make and return a copy of the given dimension list.
 *
 * Return:	Success:	copy of dimension list.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  8, 1996
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:16:07 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *-------------------------------------------------------------------------
 */
dimdes *
lite_PD_copy_dims (dimdes *odims) {

   dimdes *od, *ndims, *prev, *next;

   prev  = NULL;
   ndims = NULL;
    
   for (od = odims; od != NULL; od = od->next) {
      next  = FMAKE(dimdes, "PD_COPY_DIMS:next");
      *next = *od;
      next->next = NULL;

      if (ndims == NULL) {
	 ndims = next;
      } else {
	 prev->next = next;
      }

      prev = next;
   }

   return(ndims);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_copy_syment
 *
 * Purpose:	Make and return a copy of the given syment.
 *
 * Return:	Success:	a new syment
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  6, 1996 11:39 AM EST
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:16:07 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *-------------------------------------------------------------------------
 */
syment *
lite_PD_copy_syment (syment *osym) {

   int i, n;
   char *ntype;
   syment *nsym;
   symblock *nsp, *osp;
   dimdes *ndims;

   if (osym == NULL) return(NULL);

   nsym = FMAKE(syment, "PD_COPY_SYMENT:nsym");

   n   = PD_n_blocks(osym);
   osp = PD_entry_blocks(osym);
   nsp = FMAKE_N(symblock, n, "PD_COPY_SYMENT:blocks");
   for (i = 0; i < n; i++) nsp[i] = osp[i];

   ntype = lite_SC_strsavef(PD_entry_type(osym),
			    "char*:PD_COPY_SYMENT:type");
   ndims = lite_PD_copy_dims(PD_entry_dimensions(osym));

   PD_entry_blocks(nsym)     = nsp;
   PD_entry_type(nsym)       = ntype;
   PD_entry_dimensions(nsym) = ndims;
   PD_entry_number(nsym)     = PD_entry_number(osym);
   PD_entry_indirects(nsym)  = PD_entry_indirects(osym);

   return(nsym);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_mk_syment
 *
 * Purpose:	Make and return a pointer to an entry for the symbol table.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  2:16 PM EST
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:16:07 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *-------------------------------------------------------------------------
 */
syment *
_lite_PD_mk_syment (char *type, long numb, long addr,
		    symindir *indr, dimdes *dims) {

   syment *ep;
   symblock *sp;
   char *t;

   ep = FMAKE(syment, "_PD_MK_SYMENT:ep");
   sp = FMAKE(symblock, "_PD_MK_SYMENT:sp");

   PD_entry_blocks(ep) = sp;

   sp->number   = numb;
   sp->diskaddr = addr;

   if (type == NULL) {
      t = NULL;
   } else {
      t = lite_SC_strsavef(type, "char*:_PD_MK_SYMENT:type");
   }
   
   PD_entry_type(ep)       = t;
   PD_entry_number(ep)     = numb;
   PD_entry_dimensions(ep) = dims;

   if (indr == NULL) {
      symindir iloc;
      iloc.addr       = 0L;
      iloc.n_ind_type = 0L;
      iloc.arr_offs = 0L;
      PD_entry_indirects(ep) = iloc;
   } else {
      PD_entry_indirects(ep)  = *indr;
   }

   return(ep);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_rl_syment
 *
 * Purpose:	Reclaim the space of the given syment.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996 12:05 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
_lite_PD_rl_syment (syment *ep) {

   SFREE(PD_entry_type(ep));
   SFREE(PD_entry_blocks(ep));
   SFREE(ep);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_rl_syment_d
 *
 * Purpose:	Reclaim the space of the given syment including its
 *		dimensions.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996 12:06 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
_lite_PD_rl_syment_d (syment *ep) {

   if (ep == NULL) return;
    
   _lite_PD_rl_dimensions(PD_entry_dimensions(ep));
   _lite_PD_rl_syment(ep);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_mk_defstr
 *
 * Purpose:	Make a defstr entry for the structure chart.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  4:45 PM EST
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:16:07 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *-------------------------------------------------------------------------
 */
defstr *
_lite_PD_mk_defstr (char *type, memdes *lst, long sz, int align, int flg,
		    int conv, int *ordr, long *formt) {

   defstr *dp;
   memdes *desc;
   int n;

   dp = FMAKE(defstr, "_PD_MK_DEFSTR:dp");

   dp->type       = lite_SC_strsavef(type, "char*:_PD_MK_DEFSTR:type");
   dp->alignment  = align;
   dp->convert    = conv;
   dp->onescmp    = 0;
   dp->unsgned    = 0;
   dp->order_flag = flg;
   dp->order      = ordr;
   dp->format     = formt;
   dp->members    = lst;

   if (sz >= 0) {
      dp->size_bits = 0L;
      dp->size      = sz;
   } else {
      dp->size_bits = -sz;
      dp->size      = (-sz + 7) >> 3L;
      dp->unsgned   = TRUE;
   }

   /*
    * Find the number of indirects.
    */
   for (n = 0, desc = lst; desc != NULL; desc = desc->next) {
      if (_lite_PD_indirection(desc->type)) n++;
   }
   dp->n_indirects = n;

   return(dp);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_rl_defstr
 *
 * Purpose:	Free up the storage associated with a defstr.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  3:18 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
_lite_PD_rl_defstr (defstr *dp) {

   memdes *desc, *next;
   int *ord;
   long *frm;

   for (desc = dp->members; desc != NULL; desc = next) {
      next = desc->next;
      _lite_PD_rl_descriptor(desc);
   }

   ord = dp->order;
   if ((ord != NULL) && (lite_SC_arrlen(ord) > -1)) SFREE(ord);

   frm = dp->format;
   if ((frm != NULL) && (lite_SC_arrlen(frm) > -1)) SFREE(dp->format);

   SFREE(dp->type);
   SFREE(dp);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_copy_members
 *
 * Purpose:	Copy a linked list of members.
 *
 * Return:	Success:	ptr to the new list
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  6, 1996 11:38 AM EST
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:16:07 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *-------------------------------------------------------------------------
 */
memdes *
lite_PD_copy_members (memdes *desc) {

   memdes *newm, *nnxt, *thism, *prevm;
   char *ms, *ts, *bs, *ns, *cs;
   dimdes *nd;

   newm  = NULL;
   prevm = NULL;
   for (thism = desc; thism != NULL; thism = thism->next) {
      nnxt = FMAKE(memdes, "PD_COPY_MEMBERS:nnxt");

      ms = lite_SC_strsavef(thism->member,
			    "char*:PD_COPY_MEMBERS:member");
      ts = lite_SC_strsavef(thism->type,
			    "char*:PD_COPY_MEMBERS:type");
      bs = lite_SC_strsavef(thism->base_type,
			    "char*:PD_COPY_MEMBERS:base_type");
      ns = lite_SC_strsavef(thism->name,
			    "char*:PD_COPY_MEMBERS:name");
      nd = lite_PD_copy_dims(thism->dimensions);

      nnxt->member      = ms;
      nnxt->type        = ts;
      nnxt->base_type   = bs;
      nnxt->name        = ns;
      nnxt->dimensions  = nd;
      nnxt->next        = NULL;

      nnxt->member_offs = thism->member_offs;
      nnxt->cast_offs   = thism->cast_offs;
      nnxt->number      = thism->number;

      if (thism->cast_memb != NULL) {
	 cs = lite_SC_strsavef(thism->cast_memb,
			       "char*:PD_COPY_MEMBERS:cast_memb");
	 nnxt->cast_memb  = cs;
      } else {
	 nnxt->cast_memb = NULL;
      }

      if (newm == NULL) {
	 newm = nnxt;
      } else {
	 prevm->next = nnxt;
      }

      prevm = nnxt;
   }

   return(newm);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_mk_descriptor
 *
 * Purpose:	Build a member descriptor out of the given string.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  2:06 PM EST
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:16:07 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *-------------------------------------------------------------------------
 */
memdes *
_lite_PD_mk_descriptor (char *member, int defoff) {

   memdes *desc;
   char *ms, *ts, *bs, *ns, *p;
   dimdes *nd;

   desc = FMAKE(memdes, "_PD_MK_DESCRIPTOR:desc");

   /*
    * Get rid of any leading white space.
    */
   for (p = member; strchr(" \t\n\r\f", *p) != NULL; p++) /*void*/ ;

   ms = lite_SC_strsavef(p, "char*:_PD_MK_DESCRIPTOR:member");
   ts = _lite_PD_member_type(p);
   bs = _lite_PD_member_base_type(p);
   ns = _lite_PD_member_name(p);
   nd = _lite_PD_ex_dims(p, defoff, FALSE);

   desc->member      = ms;
   desc->type        = ts;
   desc->base_type   = bs;
   desc->name        = ns;
   desc->dimensions  = nd;

   desc->number      = _lite_PD_comp_num(desc->dimensions);
   desc->member_offs = -1L;
   desc->cast_offs   = -1L;
   desc->cast_memb   = NULL;
   desc->next        = NULL;

   return(desc);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_rl_descriptor
 *
 * Purpose:	Release a member descriptor
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  4:55 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
_lite_PD_rl_descriptor (memdes *desc) {

   SFREE(desc->member);
   SFREE(desc->name);
   SFREE(desc->type);
   SFREE(desc->base_type);
   SFREE(desc->cast_memb);

   _lite_PD_rl_dimensions(desc->dimensions);

   SFREE(desc);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_mk_dimensions
 *
 * Purpose:	Build a dimension descriptor out of the given members
 *
 *              struct s_dimdes             
 *		   {long index_min;         
 *		    long index_max;         
 *		    long number;            
 * 		    struct s_dimdes *next;};
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  2:08 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
dimdes *
_lite_PD_mk_dimensions (long mini, long leng) {

   dimdes *dims;

   dims            = FMAKE(dimdes, "_PD_MK_DIMENSIONS:dims");
   dims->index_min = mini;
   dims->index_max = mini + leng - 1L;
   dims->number    = leng;
   dims->next      = NULL;
   
   return(dims);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_rl_dimensions
 *
 * Purpose:	Release a dimension descriptor.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  3:19 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
_lite_PD_rl_dimensions (dimdes *dims) {

   dimdes *pp, *nxt;
   int nc;

   for (pp = dims; pp != NULL; pp = nxt) {
      nxt = pp->next;
      nc  = lite_SC_ref_count(pp);
      SFREE(pp);
      if (nc > 1) break;
   }
}
