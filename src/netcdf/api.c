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

#define SILO_NO_CALLBACKS
#include "silo_netcdf_private.h"

/*======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================

  Module Name                                                      api.c

  Purpose

        Perform all functions which deal with the application-program
        interface for the Standard Interfacing Library for scientific
        Output (SILO).

  Programmer

        Jeffery Long, NSSD/B

  Contents

        SILO Routine Summary (User-Level)
        --------------------------------


        dbid = silonetcdf_ncopen   (filename, mode)
        dbid = silonetcdf_nccreate (filename, mode)
               silonetcdf_ncclose  (dbid)
               SILO_ShowErrors (true_false)
               SILO_SetTarget  (data_std, data_align)

               silonetcdf_ncinqall (dbid, &ndim, &nvar, &nobj, &ndir, &ngatt, &recdim))
               silonetcdf_ncinquire (dbid, &ndim, &nvar, &ngatt, &recdim)

        id   = silonetcdf_ncdimdef (dbid, name, size)
        id   = silonetcdf_ncdimid  (dbid, name)
               silonetcdf_ncdiminq (dbid, dimid, &name, &size)

        id   = silonetcdf_ncdirdef (dbid, name)
        id   = silonetcdf_ncdirget (dbid)
               silonetcdf_ncdirset (dbid, dirid)
               silonetcdf_ncdirinq (dbid, dirid, &name, &parent)
               silonetcdf_ncdirlist(dbid, dirid, &nchild, &children)

        id   = silonetcdf_ncvardef (dbid, name, datatype, ndims, dimids)
        id   = silonetcdf_ncvarid  (dbid, name)
               silonetcdf_ncvarinq (dbid, varid, &name, &datatype, &ndims, &dims, &natts)
               silonetcdf_ncvarput1(dbid, varid, index[], &value)
               silonetcdf_ncvarget1(dbid, varid, index[], &value
               silonetcdf_ncvarput (dbid, varid, start[], count[], &values)
               silonetcdf_ncvarget (dbid, varid, start[], count[], &values)

               silonetcdf_ncattput (dbid, varid, name, datatype, len, &value)
               silonetcdf_ncattget (dbid, varid, name, &value)
               silonetcdf_ncattinq (dbid, varid, name, &datatype, &len)

        id   = silonetcdf_ncobjdef (dbid, name, type, ncomps)
        id   = silonetcdf_ncobjid  (dbid, name)
               silonetcdf_ncobjinq (dbid, objid, &name, &type, &ncomps)
               silonetcdf_ncobjput (dbid, objid, names, ids[], types[], pars[])
               silonetcdf_ncobjget (dbid, objid, names, ids[], types[], pars[])


  ======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

/*===========================================================
 * Global Data for this Module
 *===========================================================*/

char           err_string[256];
int            _dims[30];

/*----------------------------------------------------------------------
 *  Routine                                                     silonetcdf_ncopen
 *
 *  Purpose
 *
 *      Open an existing SILO file for access, returning a database ID
 *      that can subsequently be used to refer to the SILO file. In
 *      case of error, the function returns 'OOPS'.
 *
 *  Notes
 *
 *      This function will do the following:
 *              (1) Verify input parameters
 *              (2) Open the given file.
 *              (3) Read header information describing table sizes.
 *              (4) Create table data structures.
 *              (5) Read tables from file.
 *
 *  Modifications
 *      Robb Matzke, Tue Jan 10 17:20:26 EST 1995
 *      Device independence rewrite.  Explicit return values:
 *      success: netcdf ID number; failure: -1.  Removed checks for
 *      file name, file existence, file readability since this is done
 *      at higher levels.
 *
 *      Al Leibee, Thu Jun  9 13:15:37 PDT 1994
 *      PD_close a PDB file that is not a Silo file.
 *
 *---------------------------------------------------------------------
 */
/* ARGSUSED */
INTERNAL int
silonetcdf_ncopen (char *filename, int mode)
{
   PDBfile       *pdb_file;    /* PDB File descriptor. */
   int            dbid;        /* Database identifier. */

   /* Perform any necessary one-time initializations */
   silo_Init();

   /* Attempt to open file. */
   if ((pdb_file = lite_PD_open(filename, "r")) == NULL) {
      /* Not a SILO file. */
      /* silo_Error (lite_PD_err, SILO_ERROR); */
      silo_Error("File is not a SILO file.", SILO_ERROR);
      return (OOPS);
   }

   /* Make sure this is a SILO file. */
   if (!silo_Verify(pdb_file)) {
      silo_Error("File is not a SILO file", SILO_ERROR);
      (void)lite_PD_close(pdb_file);
      return (OOPS);
   }

   /*------------------------------------------------------------------
    * File has been opened successfully.  We must now associate an
    * integer identifier with this file (for caller), save the PDB
    * file pointer, and set up internal tables.
    * ---------------------------------------------------------------*/

   if ((dbid = silo_Attach(pdb_file)) == OOPS) {
      /* (Error message taken care of in silo_Attach) */
      (void)lite_PD_close(pdb_file);
      return (OOPS);
   }

   /* Allocate and read the internal SILO tables. */
   silo_MakeTables(dbid);

   if (silo_GetTables(dbid) == OOPS) {
      silo_Release(dbid);
      silo_Error("SILO file is corrupt. Make sure it was FTP'd in bin mode.",
		 SILO_ERROR);
      return (OOPS);
   }
   else
      return (dbid);
}

/*----------------------------------------------------------------------
 *  Routine                                                    silonetcdf_ncclose
 *
 *  Purpose
 *
 *      Close an open SILO file.
 *
 *  Notes
 *
 *      This function will do the following:
 *          (1) If file was modified, write current tables to file.
 *          (2) Close the file.
 *
 *  Modified
 *      Robb Matzke, Wed Jan 11 07:48:31 PST 1995
 *      This is a read-only driver, so we don't have to worry about
 *      updating the silo tables.
 *
 *      Sean Ahern, Mon Dec 18 17:33:55 PST 2000
 *      Fixed a typo: changed silonetcdf_silonetcdf_ncclose to
 *      silonetcdf_ncclose.
 *
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncclose(int dbid)
{
   ASSERT_DBID(dbid, OOPS);
   silo_Release(dbid);
   return (TRUE);
}

/*----------------------------------------------------------------------
 *  Routine                                                     silonetcdf_ncinqall
 *
 *  Purpose
 *
 *      Return information about all aspects of an open SILO file,
 *      given its database ID.  The number of dimension, variables,
 *      objects, and directories in the current directory is returned.
 *
 *  Return Value
 *
 *      In case of error, the function returns OOPS.
 *
 *---------------------------------------------------------------------
 */
INTERNAL int
silonetcdf_ncinqall(int dbid, int *ndim, int *nvar, int *nobj, int *ndir, int *ngatt,
	 int *recdim)
{
   int            dirid = silonetcdf_ncdirget(dbid);

   *ndim = silo_GetDimCount(dbid, dirid);
   *nvar = silo_GetVarCount(dbid, dirid);
   *nobj = silo_GetObjCount(dbid, dirid);
   *ndir = silo_GetDirCount(dbid, dirid);
   *ngatt = silo_GetAttCount(dbid, SILO_ROOT_DIR, 0);

   *recdim = -1;               /* Not supported currently */

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                   silonetcdf_ncdiminq
 *
 *  Purpose
 *
 *      Return the name and size of the dimension identified by the
 *      given ID.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncdiminq (int dbid, int dimid, char *name, int *size)
{
   DimEnt        *entry;

   *size = 0;

   if ((entry = silo_GetDimEnt(dbid, silonetcdf_ncdirget(dbid), dimid)) != NULL) {
      if (name != NULL)
	 strcpy(name, entry->name);
      *size = entry->size;
   }
   else {
      return (OOPS);
   }

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                   silonetcdf_ncdirget
 *
 *  Purpose
 *
 *      Return the directory ID of the current directory in the SILO.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncdirget (int dbid)
{

   ASSERT_DBID(dbid, OOPS);

   return (silo_table[dbid].curr_dir);
}

/*----------------------------------------------------------------------
 *  Routine                                                   silonetcdf_ncdirset
 *
 *  Purpose
 *
 *      Select the current directory.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncdirset (int dbid, int dirid)
{

   ASSERT_DBID(dbid, OOPS);

   switch (dirid) {
   case SILO_ROOT_DIR:
      /* Set directory to root directory. */

      silo_table[dbid].curr_dir = SILO_ROOT_DIR;
      break;

   default:

      /* Set directory to specified directory (if it IS a directory) */
      ASSERT_DIR(dbid, dirid, OOPS);

      silo_table[dbid].curr_dir = dirid;
      break;
   }

   return (OKAY);

}

/*----------------------------------------------------------------------
 *  Routine                                                  silonetcdf_ncdirlist
 *
 *  Purpose
 *
 *      Return a list of all subdirectories beneath given directory.
 *---------------------------------------------------------------------
 */
INTERNAL int
silonetcdf_ncdirlist (int dbid, int dirid, int *ndirs, int *dirs)
{
   int            i, n = 0;

   if (dirs == NULL)
      return (OOPS);

   for (i = 0; i < dirTable[dbid]->num_used; i++) {

      if (dirTable[dbid]->ent[i]->parent == dirid) {

	 dirs[n++] = dirTable[dbid]->ent[i]->absid;
      }
   }
   *ndirs = n;

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                    silonetcdf_ncvarid
 *
 *  Purpose
 *
 *      Return the variable ID of the given variable name.
 *
 *  Notes
 *
 *      The difficulty with this routine is that there can be multiple
 *      occurences of 'name' throughout the file. Which one's ID should
 *      be returned?  Currently, we only look in CURRENT directory.
 *
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncvarid (int dbid, char *name)
{
   int            id;

   id = silo_GetVarId(dbid, silonetcdf_ncdirget(dbid), name);

   return (id);
}

/*----------------------------------------------------------------------
 *  Routine                                                   silonetcdf_ncvarinq
 *
 *  Purpose
 *
 *      Return the name, datatype, dimensions, and number of attributes
 *      associated with the given variable ID.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncvarinq (int dbid, int varid, char *name, int *datatype, int *ndims,
	  int dimids[], int *natts)
{
   int            i, dirid;
   VarEnt        *ent;

   *datatype = *ndims = *natts = 0;

   dirid = silonetcdf_ncdirget(dbid);

   if ((ent = silo_GetVarEnt(dbid, dirid, varid)) == NULL)
      return (OOPS);

   *datatype = ent->type;
   *ndims = ent->ndims;
   *natts = silo_GetAttCount(dbid, dirid, varid);

   if (name != NULL)
      strcpy(name, ent->name);

   if (dimids != NULL && ent->dimids != NULL) {
      for (i = 0; i < ent->ndims; i++)
	 dimids[i] = ent->dimids[i];
   }

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                  silonetcdf_ncvarget1
 *
 *  Purpose
 *
 *      Read a single element of the given variable from the SILO file.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncvarget1 (int dbid, int varid, int index[], void *value)
{
   VarEnt        *ent;
   int            iret, i, dirid;
   long           ind[9];

   ASSERT_DBID(dbid, OOPS);
   ASSERT_VAR(dbid, varid, OOPS);

   dirid = silonetcdf_ncdirget(dbid);

   ent = silo_GetVarEnt(dbid, dirid, varid);
   if (ent == NULL)
      return (OOPS);

   if (ent->iname == NULL)
      return (OOPS);

   /* Read array from file with offset */
   for (i = 0; i < ent->ndims; i++) {
      ind[3 * i] = (long)index[i];
      ind[3 * i + 1] = (long)index[i];
      ind[3 * i + 2] = 1;
   }

   iret = lite_PD_read_alt(silo_table[dbid].pdbfile, ent->iname, value, ind);

   return (iret == TRUE ? OKAY : OOPS);
}

/*----------------------------------------------------------------------
 *  Routine                                                   silonetcdf_ncvarget
 *
 *  Purpose
 *
 *      Read a hypercube of values from the SILO file.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncvarget (int dbid, int varid, int start[], int count[], void *values)
{
   VarEnt        *ent;
   char          *tmp;
   int            i, size, iwhole, dirid;

   ASSERT_DBID(dbid, OOPS);
   ASSERT_VAR(dbid, varid, OOPS);

   dirid = silonetcdf_ncdirget(dbid);

   ent = silo_GetVarEnt(dbid, dirid, varid);
   if (ent == NULL) {
      sprintf(err_string,
	      "VarGet: Variable not found: # %d", varid);
      silo_Error(err_string, SILO_DEBUG);
      return (OOPS);
   }

   /* Verify variable has been written */
   if (ent->iname == NULL) {

      silo_Error("VarGet: Variable hasn't been written; cannot read.",
		 SILO_DEBUG);
      return (OOPS);
   }

   /* Verify validity of counts. */
   for (i = 0; i < ent->ndims; i++) {
      if (count[i] < 1) {
	 silo_Error("VarGet: Count <= 0", SILO_ERROR);
	 return (OOPS);
      }
   }

   /* Verify validity of indeces. */
   for (i = 0; i < ent->ndims; i++) {
      silonetcdf_ncdiminq(dbid, ent->dimids[i], NULL, &size);
      if (start[i] < 0 || start[i] > size ||
	  start[i] + count[i] > size) {
	 sprintf(err_string,
		 "VarGet: Invalid hypercube index on var # %d", varid);
	 silo_Error(err_string, SILO_ERROR);
	 return (OOPS);
      }
   }

   iwhole = 1;
   for (i = 0; i < ent->ndims; i++) {
      _dims[i] = silo_GetDimSize(dbid, ent->dimids[i]);

      if (_dims[i] != count[i])
	 iwhole = 0;
   }

   /*
    * If reading whole thing, read it directly into user-provided
    * space.  If reading hypercube subset, must read entire variable
    * and extract subset into user's space.
    */
   if (iwhole) {

      silo_Read(dbid, ent->iname, values);

   }
   else {

      /* Read entire array */
      tmp = (char *)ALLOC_N(char, ent->nels * ent->lenel);

      silo_Read(dbid, ent->iname, tmp);
      silo_GetHypercube(values, tmp, _dims, ent->ndims,
			start, count, silo_GetMachDataSize(ent->type));
      FREE(tmp);
   }

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                   silonetcdf_ncattget
 *
 *  Purpose
 *
 *      Get an attribute value associated with the given var/dir/obj
 *      from the SILO file, given the attribute name and var/dir/obj ID.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncattget (int dbid, int varid, char *name, void *value)
{
   AttEnt        *ent;
   int            dirid;

   ASSERT_DBID(dbid, OOPS);
   ASSERT_NAME(name, OOPS);

   dirid = silonetcdf_ncdirget(dbid);

   /* Return with error if attribute doesn't exist for this variable. */
   ent = silo_GetAttEnt(dbid, dirid, varid, name);
   if (ent == NULL)
      return (OOPS);

   silo_Read(dbid, ent->iname, value);

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                   silonetcdf_ncattinq
 *
 *  Purpose
 *
 *      Return information about an attribute, given its name.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncattinq (int dbid, int varid, char *name, int *datatype, int *len)
{
   AttEnt        *ent;
   int            dirid;

   *datatype = *len = 0;

   dirid = silonetcdf_ncdirget(dbid);

   /* Search all attributes of the variable for 'name' */
   ent = silo_GetAttEnt(dbid, dirid, varid, name);
   if (ent == NULL)
      return (OOPS);

   *datatype = ent->type;
   *len = ent->nels;

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                    silonetcdf_ncobjid
 *
 *  Purpose
 *
 *      Return the object ID of the object with the given name.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncobjid (int dbid, char *name)
{
   int            id;

   ASSERT_DBID(dbid, OOPS);
   ASSERT_NAME(name, OOPS);

   id = silo_GetObjId(dbid, silonetcdf_ncdirget(dbid), name);

   return (id);
}

/*----------------------------------------------------------------------
 *  Routine                                                   silonetcdf_ncobjinq
 *
 *  Purpose
 *
 *      Return information about the given object.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncobjinq (int dbid, int objid, char *name, int *type, int *ncomps)
{
   ObjEnt        *ent;

   *type = 0;
   *ncomps = 0;

   if ((ent = silo_GetObjEnt(dbid, silonetcdf_ncdirget(dbid), objid)) == NULL)
      return (OOPS);

   *type = ent->type;
   *ncomps = ent->ncomps;
   if (name != NULL)
      strcpy(name, ent->name);

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                   silonetcdf_ncobjget
 *
 *  Purpose
 *
 *      Read an object description from the SILO file.
 *---------------------------------------------------------------------*/
INTERNAL int
silonetcdf_ncobjget (int dbid, int objid, char *comp_names, int comp_ids[],
	  int comp_types[], int comp_pars[])
{
   int            i;
   ObjEnt        *ent;

   ASSERT_DBID(dbid, OOPS);
   ASSERT_OBJ(dbid, objid, OOPS);
   ASSERT_PTR(comp_names, OOPS);
   ASSERT_PTR(comp_ids, OOPS);
   ASSERT_PTR(comp_types, OOPS);
   ASSERT_PTR(comp_pars, OOPS);

   if ((ent = silo_GetObjEnt(dbid, silonetcdf_ncdirget(dbid), objid)) == NULL)
      return (OOPS);

   for (i = 0; i < ent->ncomps; i++) {

      comp_ids[i] = ent->compids[i];
      comp_types[i] = ent->comptypes[i];
      comp_pars[i] = ent->comppars[i];
   }

   strcpy(comp_names, ent->compnames);

   return (OKAY);
}
