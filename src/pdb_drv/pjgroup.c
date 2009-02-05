#define NEED_SCORE_MM
#define NO_CALLBACKS
#include "silo_pdb_private.h"

/*======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================

  Module Name                                                  pjgroup.c

  Purpose

        This module provides a capability for reading and writing
        group descriptions in a PDB file.

  Programmer

        Jeffery Long, NSSD/B


  Description


  Contents

        User-Callable Functions (C)

                int      PJ_put_group (file, group, overwrite)
                int      PJ_get_group (file, name, PJgroup **group)
                PJgroup *PJ_make_group (name, type, comp_names, pdb_names, num)
                int      PJ_rel_group (group)

        User-Callable Functions (Fortran)


        Internal-Use-Only Functions




  ======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

/*
 **********************
 * Modification History
 **********************
 * $Id: pjgroup.c,v 1.4 1995/08/30 00:21:38 ahern Exp $
 * $Header: /SRC/pact/repository/src/silo/pdb/pjgroup.c,v 1.4 1995/08/30 00:21:38 ahern Exp $
 * $Log: pjgroup.c,v $
 * Revision 1.4  1995/08/30  00:21:38  ahern
 *  < ahern | D95_08_29_17_20_15 > Reformatted whitespace and tabs. You'll find that most changes are in comments.
 *
 * Revision 1.3  1995/08/28  21:53:40  ahern
 *  < ahern | D95_08_28_14_49_09 > Ran indent on silo.
 *
 * Revision 1.2  1995/04/21  16:16:34  brugger
 *  < brugger | D95_04_21_09_10_55 > I merged in Robbs silo filters enhancements into silo.
 *
 * Revision 1.1  1995/02/01  00:54:03  brugger
 *  < brugger | D95_01_31_16_47_08 > I incorporated robbs new device independent silo.
 *
 * Revision 1.2  1994/10/17  16:17:29  brugger
 *  < brugger | D94_10_17_09_09_09 > I restructured silo
 *
 * Revision 1.1.1.1  1994/04/13  22:55:17  sabrown
 * Reconfigure SILO
 *
 * Revision 1.2  1993/09/27  16:30:37  leibee
 *  < leibee | Mon Sep 27 09:25:46 PDT 1993 > Commit of v2_3 of SILO
 *
 *
 * Al Leibee, Wed Jul 21 09:01:32 PDT 1993
 * Ensure that PJ_make_group input strings are
 * SCORE-allocated.
 *
 * Revision 1.2  1993/07/13  16:57:51  leibee
 * Updated slide, pdbext, silo, swat modules: made allocate/free usage consistant re SCORE and system memory managers; reduced dependence on SCORE memory manager; removed check for SCORE-allocated array within FREE; added configman files.
 *
 *
 * Al Leibee, Wed Jul  7 08:00:00 PDT 1993
 * Made allocation/free usage consistant.
 */

/*=============================
 * Global Data for this Module
 *=============================*/

/*--------------------------------------------------------------------
 *  Routine                                              PJ_get_group
 *
 *  Purpose
 *
 *      Read a group description from the given PDB file.
 *
 *  Notes
 *
 *  Modified
 *
 * 	Robb Matzke, 5 Feb 1997
 *	This function returns 0 instead of dumping core if NAME is
 *	not a group.
 *
 *    Eric Brugger, Mon Dec  7 11:03:09 PST 1998
 *    Removed call to lite_PD_reset_ptr_list since it was removed.
 *
 *--------------------------------------------------------------------
 */
INTERNAL int
PJ_get_group(
   PDBfile       *file,        /* PDB file pointer */
   char          *name,        /* Name of group desc to read */
   PJgroup      **group)       /* Variable to write into */
{
   syment	*ep;
   
   /*
    * Make sure the thing we're looking up is really a group.
    */
   ep = lite_PD_inquire_entry (file, name, TRUE, NULL);
   if (!ep || strcmp(PD_entry_type(ep), "Group *")) return 0;


   return ((int)PJ_read(file, name, group));

}

/*--------------------------------------------------------------------
 *  Routine                                              PJ_make_group
 *
 *  Purpose
 *
 *      Return a group description variable constructed from given
 *      elements.
 *
 *  Notes
 *
 *  Modifications
 *
 *      Al Leibee, Wed Jul 21 09:01:32 PDT 1993
 *      Ensure that inputted strings are SCORE-allocated.
 *
 *      Al Leibee, Wed Jul  7 08:00:00 PDT 1993
 *      Character strings known to have been allocated by SCORE.
 *
 *      Jeremy Meredith, Fri Nov 19 10:04:54 PST 1999
 *      Changed SC_strdup to safe_strdup.
 *
 *      Jeremy Meredith, Tue Nov 30 09:12:15 PST 1999
 *      I changed safe_strdup back to SC_strdup.  Not allocating the
 *      memory with SCORE's memory management routines caused severe
 *      problems because the memory block was not tagged correctly.
 *
 *--------------------------------------------------------------------
 */
INTERNAL PJgroup *
PJ_make_group (
   char          *name,        /* Name of this group */
   char          *type,        /* Type of this group */
   char         **comp_names,  /* Array of component names (num) */
   char         **pdb_names,   /* Array of internal (PDB) names (num) */
   int            num)         /* Number of components provided */
{
   PJgroup       *group;
   char          *sc_type, *sc_name;
   char         **sc_comp_nms;
   char         **sc_int_nms;
   int            i;

   if (num <= 0 || name == NULL || type == NULL ||
       comp_names == NULL || pdb_names == NULL)
      return (NULL);

   group = SCALLOC(PJgroup);

   /*----------------------------------------
    *  Ensure strings allocated by SCORE
    *  by making duplicates.
    *----------------------------------------*/
   sc_comp_nms = SCALLOC_N(char *, num);
   sc_int_nms = SCALLOC_N(char *, num);

   for (i = 0; i < num; i++) {
      sc_comp_nms[i] = SC_strdup(comp_names[i]);
      sc_int_nms[i] = SC_strdup(pdb_names[i]);
   }
   sc_type = SC_strdup(type);
   sc_name = SC_strdup(name);

   /*----------------------------------------
    *  Store new pointers in group variable.
    *----------------------------------------*/
   group->name = sc_name;
   group->type = sc_type;
   group->ncomponents = num;
   group->comp_names = sc_comp_nms;
   group->pdb_names = sc_int_nms;

   return (group);
}

/*--------------------------------------------------------------------
 *  Routine                                              PJ_rel_group
 *
 *  Purpose
 *
 *      Release the space associated with the given group.
 *
 *  Notes
 *
 *  Modifications
 *
 *      Al Leibee, Wed Jul  7 08:00:00 PDT 1993
 *      FREE to SCFREE to be consistant with allocation.
 *
 *      Sean Ahern, Mon Nov 23 17:02:49 PST 1998
 *      Made the return value meaningful.
 *--------------------------------------------------------------------*/
INTERNAL int
PJ_rel_group (PJgroup *group) {

   int            i;

   if (group == NULL || group->ncomponents <= 0)
      return (FALSE);

   for (i = 0; i < group->ncomponents; i++) {
      SCFREE(group->comp_names[i]);
      SCFREE(group->pdb_names[i]);
   }

   SCFREE(group->name);
   SCFREE(group->type);
   SCFREE(group->comp_names);
   SCFREE(group->pdb_names);
   SCFREE(group);

   return (TRUE);
}

/*--------------------------------------------------------------------
 *  Routine                                              PJ_print_group
 *
 *  Purpose
 *
 *      Print the given group.
 *
 *  Notes
 *
 *--------------------------------------------------------------------
 */
INTERNAL int
PJ_print_group (PJgroup *group, FILE *fp) {

   int            i;

   if (group == NULL || group->ncomponents <= 0)
      return (FALSE);

   if (fp == NULL)
      fp = stdout;

   fprintf(fp, "Group: %s is of type %s and has %d components.\n",
	   group->name, group->type, group->ncomponents);

   for (i = 0; i < group->ncomponents; i++) {
      fprintf(fp, "Component [%d] = %s  ==> %s\n", i,
	      group->comp_names[i], group->pdb_names[i]);
   }
   return 0;
}

/*--------------------------------------------------------------------
 *  Routine                                             PJ_put_group
 *
 *  Purpose
 *
 *      Write a group description into the given PDB file.
 *
 *  Return:	Success:	non-zero
 *
 * 		Failute:	zero
 *
 *  Notes
 *
 *
 *  Modifications
 *
 *     Al Leibee, Wed Jul  7 08:00:00 PDT 1993
 *     FREE to SCFREE to be consistant with allocation.
 *
 *     Robb Matzke, 7 Mar 1997
 *     Added the OVERWRITE argument.  If non-zero, then we can overwrite
 *     any existing group with the same name.
 *
 *    Eric Brugger, Mon Dec  7 11:03:09 PST 1998
 *    Removed call to lite_PD_reset_ptr_list since it was removed.
 *
 *--------------------------------------------------------------------
 */
#ifdef PDB_WRITE
INTERNAL int
PJ_put_group (
   PDBfile       *file,        /* PDB file pointer */
   PJgroup       *group,       /* Group variable to write */
   int		 overwrite)
{

   char         **varlist;
   char           name[MAXNAME];

   if (file == NULL || group == NULL)
      return (FALSE);

   /*----------------------------------------
    *  Define the group struct, if it hasn't
    *  been already.
    *----------------------------------------*/

   if (PD_inquire_type(file, "Group") == NULL) {

      if ((lite_PD_defstr(file, "Group",
			  "char    *name",
			  "char    *type",
			  "char    **comp_names",
			  "char    **pdb_names",
			  "integer ncomponents",
			  lite_LAST)) == NULL)
	 printf("PJ_put_group -- Error defining Group structure.\n");
   }

   /*----------------------------------------
    *  Build an absolute pathname.
    *---------------------------------------*/
   PJ_get_fullpath(file, lite_PD_pwd(file), group->name, name);

   /*----------------------------------------
    *  Make sure this group description hasn't
    *  already been written.
    *----------------------------------------*/
   if (!overwrite) {
      varlist = lite_SC_hash_dump(file->symtab, name);

      if (varlist != NULL && varlist[0] != NULL)
	 return (FALSE);

      /*FREE(varlist); */
      SCFREE(varlist);
   }

   /*----------------------------------------
    *  Write the group description variable.
    *----------------------------------------*/
   return ((int)PJ_write(file, name, "Group *", &group));
}
#endif /* PDB_WRITE */

