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
 * PDMEMB.C - member handling routines for PDBLib
 *
 * Source Version: 9.0
 * Software Release #92-0043
 *
 */
#include "pdb.h"
#if defined(_WIN32)
#include <silo_win32_compatibility.h>
#endif

/*--------------------------------------------------------------------------*/
/*                          ACCESSOR FUNCTIONS                              */
/*--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_member_type
 *
 * Purpose:	Strip off dimensions and member name by inserting a `\0'
 *		after the type specification.
 *
 * Return:	Success:	ptr to the string.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  4:44 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
_lite_PD_member_type (char *s) {

   char *t, *p, c, bf[MAXLINE];

   strcpy(bf, s);

   /*
    * Find a pointer to the last '*' in the string.
    */
   for (p = bf, t = bf; (c = *t) != '\0'; t++) {
      if (c == '*') p = t;
   }

   /*
    * If there was a '*' replace the next character with a '\0'.
    * Otherwise the type is not a pointer so return the first token.
    */
   if (p != bf) *(++p) = '\0';
   else strtok(bf, " \t\n\r");

   return(lite_SC_strsavef(bf, "char*:_PD_MEMBER_TYPE:bf"));
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_member_base_type
 *
 * Purpose:	Extract the base type (no indirections) of the
 *		given member and return a copy of it.
 *
 * Return:	Success:	a copy of it
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  4:31 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
_lite_PD_member_base_type (char *s) {

   char *token, bf[MAXLINE];

   strcpy(bf, s);
   token = lite_SC_firsttok(bf, " *");

   return(lite_SC_strsavef(token, "char*:_PD_MEMBER_BASE_TYPE:token"));
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_member_name
 *
 * Purpose:	Given the member description, extract the name and return
 *		a pointer to it.  New space is allocated for the name.
 *
 * Return:	Success:	A pointer to the name.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  4:43 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
_lite_PD_member_name (char *s) {

   char *pt, *token, bf[MAXLINE];

   strcpy(bf, s);
   lite_SC_firsttok(bf, " *");
   for (pt = bf; strchr(" \t*", *pt) != NULL; pt++);
   token = lite_SC_firsttok(pt, "([");

   return(lite_SC_strsavef(token, "char*:_PD_MEMBER_NAME:token"));
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_ex_dims
 *
 * Purpose:	Extract the dimension information from the given string.
 *
 * Return:	Success:	If FLAG is true, return dimensions
 *				interpreted as reference (eg, x(2)
 *				interpreted as x(defoff)).  If FLAG
 *				is false, return dimensions interpreted as
 *				definition (eg, x(2) interpreted as
 *				x(defoff:2).
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  4:17 PM EST
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:55:02 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *-------------------------------------------------------------------------
 */
dimdes *
_lite_PD_ex_dims (char *memb, int defoff, int flag) {

   char *token, *maxs, bf[MAXLINE];
   long mini, leng;
   dimdes *dims, *next, *prev;

   prev = NULL;
   dims = NULL;
   strcpy(bf, memb);
   token = lite_SC_firsttok(bf, "([\001\n");
   while ((token = lite_SC_firsttok(bf, ",)] ")) != NULL) {
      maxs = strchr(token, ':');
      if (maxs != NULL) {
	 *maxs++ = '\0';
	 mini = flag ? defoff : atol(token);
	 leng = atol(maxs) - atol(token) + 1L;
      } else {
	 mini = defoff;
	 leng = flag ? 1L : atol(token);
      }

      if (leng <= 0L) return(NULL);

      next = _lite_PD_mk_dimensions(mini, leng);

      if (dims == NULL) {
	 dims = next;
      } else {
	 prev->next = next;
      }

      prev = next;
   }

   return(dims);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_member_items
 *
 * Purpose:	Return the total number of items as specified by the
 *		dimensions in the given member.
 *
 * Return:	Success:	Number of items.
 *
 *		Failure:	-1
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  6, 1996 11:29 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
long
_lite_PD_member_items (char *s) {

   char *token, bf[MAXLINE];
   long acc;

   strcpy(bf, s);
   token = strtok(bf, "(\001\n");
   acc = 1L;
   while ((token = strtok(NULL, ",) ")) != NULL) {
      if ((acc *= atol(token)) <= 0) return(-1L);
   }

   return(acc);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_comp_num
 *
 * Purpose:	Compute the number of elements implied by the dimensions
 *		of a variable.
 *
 * Return:	Success:	Number of elements implied by dimensions
 *
 *		Failure:	1
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  4:02 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
long
_lite_PD_comp_num (dimdes *dims) {

   long acc;
   dimdes *lst;

   for (acc = 1L, lst = dims; lst != NULL; lst = lst->next) {
      acc *= (long) (lst->number);
   }

   return(acc);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_str_size
 *
 * Purpose:	Compute sizeof for the defstruct specified.
 *
 * Return:	Success:	sizeof
 *
 *		Failure:	-1 iff some member of the struct is
 *			 	undefined.
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  4:58 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
long
_lite_PD_str_size (memdes *str, HASHTAB *tab) {

   int align, al_max, lal;
   long i, sz, number;
   memdes *desc;

   sz     = 0L;
   al_max = 0;
   for (desc = str; desc != NULL; desc = desc->next) {
      number = desc->number;
      i = _lite_PD_lookup_size(desc->type, tab);
      if (i == -1L) return(-1L);

      align = _lite_PD_align((long) sz, desc->type, tab, &lal);
      if (align == -1) return(-1L);

      al_max = MAX(al_max, lal);

      desc->member_offs = sz + align;

      sz += align + i*number;
   }

   if (al_max != 0) {
      i  = (sz + al_max - 1)/al_max;
      sz = al_max*i;
   }

   return(sz);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_align
 *
 * Purpose:	Computes the number of bytes needed to put an object of
 *		TYPE on the proper byte boundary.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  6, 1996 10:45 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_align (long n, char *type, HASHTAB *tab, int *palign) {

   long offset, align, nword;
   defstr *dp;

   if (type == NULL) {
      *palign = 0;
      return(0);
   }

   if (_lite_PD_indirection(type)) dp = PD_inquire_table_type(tab, "*");
   else dp = PD_inquire_table_type(tab, type);

   if (dp == NULL) {
      *palign = -1;
      return(-1);
   } else {
      align = dp->alignment;
   }

   if (align != 0) {
      nword  = (n + align - 1)/align;
      offset = align*nword - n;
   } else {
      offset = 0;
   }

   *palign = align;

   return(offset);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_lookup_type
 *
 * Purpose:	Look up the type given in structure chart.
 *
 * Return:	Success:	the defstr
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  4:30 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
defstr *
_lite_PD_lookup_type (char *s, HASHTAB *tab) {

   char *token, bf[MAXLINE];

   /*
    * If it's a POINTER handle it now.
    */
   if (strchr(s, '*') != NULL) strcpy(bf, "*");
   else strcpy(bf, s);

   token = strtok(bf, " ");
   return(PD_inquire_table_type(tab, token));
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_lookup_size
 *
 * Purpose:	Look up the type given in structure chart and return the
 *		size.
 *
 * Return:	Success:	the size
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  2:02 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
long
_lite_PD_lookup_size (char *s, HASHTAB *tab) {

   char *token, bf[MAXLINE];
   defstr *dp;

   /*
    * If it's a POINTER handle it now.
    */
   if (strchr(s, '*') != NULL) strcpy(bf, "*");
   else strcpy(bf, s);

   token = strtok(bf, " ");
   dp    = PD_inquire_table_type(tab, token);
   if (dp != NULL) return(dp->size);
   else return(-1L);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_member_location
 *
 * Purpose:	
 *
 * Return:	Success:	The byte offset (0 based) of the given
 *				member from the beginning of the struct.
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  5, 1996  4:41 PM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
long
_lite_PD_member_location (char *s, HASHTAB *tab, defstr *dp, memdes **pdesc) {

   long addr;
   char *token, name[MAXLINE];
   memdes *desc, *nxt;

   strcpy(name, s);
   token = lite_SC_firsttok(name, ".\001");

   for (addr = 0L, desc = dp->members; desc != NULL; desc = nxt) {
      nxt = desc->next;
      if (strcmp(desc->name, token) == 0) {
	 addr  += desc->member_offs;
	 *pdesc = desc;
	 dp = PD_inquire_table_type(tab, desc->base_type);
	 if (dp != NULL) {
	    token = lite_SC_firsttok(name, ".\001");
	    if (token == NULL) return(addr);
	    else nxt = dp->members;
	 }
      }
   }

   return(-1L);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_inquire_entry
 *
 * Purpose:	Look up the symbol table entry for the named quantity.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 11:36 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
syment *
lite_PD_inquire_entry(PDBfile *file, char *name, int flag, char *fullname) {

   syment *ep;
   char s[MAXLINE];

   if (flag) strcpy(s, _lite_PD_fixname(file, name));
   else strcpy(s, name);

   if (fullname != NULL) strcpy(fullname, s);

   ep = (syment *) lite_SC_def_lookup(s, file->symtab);

   /*
    * If the file has directories and the entry is not "/",
    * treat entry with and without initial slash as equivalent, 
    */
   if ((ep == NULL) &&
       (PD_has_directories(file)) &&
       (strcmp(s, "/") != 0)) {
      if (strrchr(s, '/') == s) {
	 ep = (syment *) lite_SC_def_lookup(s + 1, file->symtab);
      } else if (strrchr(s, '/') == NULL) {
	 char t[MAXLINE];
	 if (snprintf(t, sizeof(t), "/%s", s)>=sizeof(t))
             t[sizeof(t)-1] = '\0';
	 ep = (syment *) lite_SC_def_lookup(t, file->symtab);
      }
   }

   return(ep);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_rd_itag
 *
 * Purpose:	Fill an itag from the file.
 *
 * Return:	Success:	TRUE
 *
 *		Failure:	FALSE
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  6, 1996 11:07 AM EST
 *
 * Modifications:
 *     Brad Whitlock, Wed Feb 23 18:57:13 PST 2000
 *     I made the function initialize the itag that is being returned
 *     when reading the itag fails.
 *
 *-------------------------------------------------------------------------
 */
int
_lite_PD_rd_itag (PDBfile *file, PD_itag *pi) {

   FILE *fp;
   char *token;
   static char bf[MAXLINE];

   fp = file->stream;

   _lite_PD_rfgets(bf, MAXLINE, fp);

   token = strtok(bf, "\001");
   if (token == NULL) goto failure;
   pi->nitems = atol(token);

   pi->type = strtok(NULL, "\001\n");
   if (pi->type == NULL) goto failure;

   token = strtok(NULL, "\001\n");
   if (token == NULL) {
      pi->addr = -1L;
      pi->flag = TRUE;
   } else {
      pi->addr  = atol(token);
      token = strtok(NULL, "\001\n");
      if (token == NULL) pi->flag = TRUE;
      else pi->flag = atoi(token);
   }

   return(TRUE);

failure:
   /*If we're here then reading the itag failed. */
   pi->nitems = 0;
   pi->type = NULL;
   pi->flag = TRUE;
   pi->addr = -1L;
   return(FALSE);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_adj_dimensions
 *
 * Purpose:	Adjust dimension expressions in name for append.  For now,
 *		indexes for all dimensions must include min and max
 *		(stride optional) and must match the post-append dimensions.
 * 		All but the most slowly varying index must match the
 *		pre-append dimensions.  Because the pre-append dimensions
 *		are already known the above conditions on the dimensions
 *		not changing may be relaxed in the future for user
 *		convenience or to support partial writes in appends.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT
 *		Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
int
_lite_PD_adj_dimensions (PDBfile *file, char *name, syment *ep) {

   char *token, *smax, *sinc;
   char head[MAXLINE], expr[MAXLINE], tail[MAXLINE], bf[MAXLINE];
   long imin, imax, istep, i;
   dimdes *dims;
    
   expr[0] = '\0';
   dims = ep->dimensions;
   strcpy(bf, name);
   strcpy(head, lite_SC_firsttok(bf, "([\001\n"));
   tail[0] = '\0';

   while ((token = lite_SC_firsttok(bf, ",)] ")) != NULL) {
      if (token[0] == '.') {
	 strcpy(tail, token);
	 break;
      }
      smax = strchr(token, ':');
      if (smax == NULL) {
	 lite_PD_error("MAXIMUM INDEX MISSING - _PD_ADJ_DIMENSIONS", PD_WRITE);
      }
      *smax++ = '\0';
      sinc = strchr(smax, ':');
      if (sinc != NULL) *sinc++ = '\0';

      imin = atol(token);
      imax = atol(smax);
      if (sinc != NULL) istep = atol(sinc);
      else istep = 1;

      if (imin == file->default_offset) {
	 if (((file->major_order == ROW_MAJOR_ORDER) &&
	      (dims == ep->dimensions)) ||
	     ((file->major_order == COLUMN_MAJOR_ORDER) &&
	      (dims->next == NULL))) {
	    i = dims->index_max + 1 - imin;
	    imin += i;
	    imax += i;
	 }
      }

      sprintf(expr, "%s%ld:%ld:%ld,", expr, imin, imax, istep);
      if (dims) dims = dims->next;
   }

   if (expr[0] != '\0') {
      if (strchr(head, '.') != NULL) {
	 lite_PD_error("APPEND TO TOP LEVEL OF STRUCT ONLY - "
		       "_PD_ADJ_DIMENSIONS", PD_WRITE);
      }
      expr[strlen(expr) - 1] = '\0';
      sprintf(name, "%s[%s]%s", head, expr, tail);
   }

   return(TRUE);
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_wr_itag
 *
 * Purpose:     write an itag to the file                               
 *		for a NULL pointer do:                                  
 *		    _lite_PD_wr_itag(file, 0L, type, -1L, 0)                 
 *		for a pointer to data elsewhere do:                     
 *		    _lite_PD_wr_itag(file, nitems, type, addr, 0)            
 *		for a pointer to data here do:                          
 *		    _lite_PD_wr_itag(file, nitems, type, addr, 1)            
 *		for a pointer to discontiguous data do:                 
 *		    _lite_PD_wr_itag(file, nitems, type, addr, 2)            
 * 		    then addr is interpreted as the address of the next 
 *		    block of data
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT
 *		Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
int
_lite_PD_wr_itag (PDBfile *file, long nitems, char *type, long addr,
		  int flag) {

   io_printf(file->stream, "%ld\001%s\001%ld\001%d\001\n",
	     nitems, type, addr, flag);

   return(TRUE);
}
#endif /* PDB_WRITE */
