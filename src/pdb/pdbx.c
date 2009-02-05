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
byte *
lite_PD_get_attribute (PDBfile *file, char *vr, char *at) {
   byte 	*vl, **data;
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
      byte *def;
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
