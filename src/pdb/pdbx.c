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
 * PDBX.C - a set of higher level library routines on top of PDBLib
 *
 * Source Version: 9.0
 * Software Release #92-0043
 *
 */
#include "pdb.h"


/*-------------------------------------------------------------------------
 * Function:	lite_PD_get_attribute
 *
 * Purpose:	Get the value of the specified attribute for the specified
 *		variable.
 *
 * Return:	Success:	Ptr to the attribute value
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 11:38 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
lite_SC_byte *
lite_PD_get_attribute (PDBfile *file, char *vr, char *at) {
   lite_SC_byte 	*vl, **data;
   attribute 	*attr;
   attribute_value *avl;
   char 	fullname[MAXLINE];

   attr = PD_INQUIRE_ATTRIBUTE(at);
   if (attr == NULL) {
      sprintf(lite_PD_err, "ATTRIBUTE %s DOESN'T EXIST - PD_GET_ATTR", at);
      return(NULL);
   }

   strcpy(fullname, _lite_PD_fixname(file, vr));
   avl = PD_INQUIRE_ATTRIBUTE_VALUE(fullname);
   if (avl == NULL) {
      sprintf(lite_PD_err, "VARIABLE %s HAS NO ATTRIBUTES - PD_GET_ATTR",
	      fullname);
      return(NULL);
   }

   data = attr->data;
   if (data == NULL) {
      sprintf(lite_PD_err, "ATTRIBUTE DATA %s DOESN'T EXIST - PD_GET_ATTR",
	      at);
      return(NULL);
   }

   for (/*void*/; avl != NULL; avl = avl->next) {
      if (strcmp(at, avl->attr->name) == 0) break;
   }

   if (avl == NULL) {
      sprintf(lite_PD_err, "VARIABLE %s DOESN'T HAVE ATTRIBUTE %s - "
	      "PD_GET_ATTR", vr, at);
      return(NULL);
   }
   vl = data[avl->indx];

   return(vl);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_convert_attrtab
 *
 * Purpose:	Convert old format attribute table to new format.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT
 *		Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
void
_lite_PD_convert_attrtab (PDBfile *file) {

   int i, sz;
   defstr *dp;
   hashel *np, *prev;
   memdes *desc;

   struct ohashel {
      char *name;
      char *type;
      lite_SC_byte *def;
      struct ohashel *next;
   } *onp, *nxt, **otb;

   dp  = PD_inquire_type(file, "hashel");
   for (desc = dp->members; desc != 0; desc = desc->next) {
      if (strcmp(desc->member, "integer free") == 0) return;
   }

   sz  = file->attrtab->size;
   otb = (struct ohashel **) file->attrtab->table;
   for (i = 0; i < sz; i++) {
      prev = NULL;
      for (onp = otb[i]; onp!= NULL; onp = nxt) {
	 np = FMAKE(hashel, "_PD_CONVERT_ATTRTAB:np");
	 np->name = onp->name;
	 np->type = onp->type;
	 np->def  = onp->def;
#ifdef FIX_ME
	 np->free = TRUE;
#endif
	 np->next = NULL;
	 if (prev == NULL) otb[i]   = (struct ohashel *) np;
	 else prev->next = np;
	 prev = np;
	 nxt  = onp->next;
	 SFREE(onp);
      }
   }

   _lite_PD_rl_defstr(dp);
   lite_SC_hash_rem("hashel", file->host_chart);
   lite_SC_hash_rem("hashel", file->chart);

   dp = lite_PD_defstr(file, "hashel",
		       "char *name", 
		       "char *type", 
		       "char *def", 
		       "integer free", 
		       "hashel *next", 
		       lite_LAST);
   if (dp == NULL) {
      lite_PD_error("COULDN'T DEFINE HASHEL - _PD_CONVERT_ATTRTAB", PD_OPEN);
   }

   lite_PD_cast(file, "hashel", "def", "type");
}
#endif /* PDB_WRITE */
