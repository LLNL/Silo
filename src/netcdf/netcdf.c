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

  Module Name                                                      netcdf.c

  Purpose

        Perform all functions which deal with the underlying database
        library, including reading, writing, opening, and closing files.

  Programmer

        Jeffery Long, NSSD/B

  Contents

        NETCDF Routine Summary (Low-Level)


        dbid = netcdf_Attach (pdbfile)
               netcdf_Release  (dbid)
        ndx  = netcdf_GetIndex (dbid)
               netcdf_Error (errmsg, errcode)
        int  = netcdf_GetNextID(dbid)
        t_f  = netcdf_Modified (dbid)
                   netcdf_SetModified (dbid, t_f)
               netcdf_Setup (dbid)
               netcdf_Read  (dbid, varname, ptr)
               netcdf_PartialRead (dbid, varname, ptr, offset, len)
               netcdf_Write (dbid, varname, datatype, ptr, dims, ndims)
               netcdf_Write1 (dbid, varname, datatype, ptr)
               netcdf_GetTables (dbid)
               netcdf_PutTables (dbid)
    size = netcdf_GetDataSize (dbid, datatype)
        size = netcdf_GetMachDataSize (datatype)
        ndx  = netcdf_GetIndex1 (index, dims, ndims)
               netcdf_PutHypercube (sink, source, dims, ndims, start, count, lenel)
               netcdf_GetHypercube (sink, source, dims, ndims, start, count, lenel)
        size = netcdf_GetDimSize (dbid, dimid)
        dimid= netcdf_GetDimID (dbid, len)
        nbyte= netcdf_GetVarSize (dbid, varid, &nels, &nbytes_el)
        name = netcdf_MakeVarName (dbid, varid)
        name = netcdf_MakeAttName (dbid, dirid, varid, attname)
        true = netcdf_Verify (file)

        char *netcdf_GetDatatypeString (type)
        int   netcdf_GetDatatypeID (typename)
        char *netcdf_GetEntitytypeString (type)

        int   n_GetVarDatatype (dbid, varname, datatype)

  ======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

/*===========================================================
 * Global Data for this Module
 *===========================================================*/

SILOTable      silo_table[MAX_SILO];
static int     num_active_dbs = 0;
static int     header[HEADER_SIZE];

char           silo_err[256];

void silo_Error (char*, int) ;

/*----------------------------------------------------------------------
 *  Routine                                                    silo_Init
 *
 *  Purpose
 *
 *      Initialize internal tables. This should be called ONCE only.
 *
 *--------------------------------------------------------------------*/
void
silo_Init (void)
{
   int            i;
   static int     initialized = FALSE;

   if (initialized)
      return;

   initialized = TRUE;

   for (i = 0; i < MAX_SILO; i++) {

      /* Clear out this entry in table. */
      silo_table[i].dbid = -1;
      silo_table[i].pdbfile = (PDBfile *) 0;
      silo_table[i].curr_dir = 0;
   }
}

/*----------------------------------------------------------------------
 *  Routine                                                   silo_Attach
 *
 *  Purpose
 *
 *      Add the given PDB to the database list kept locally.
 *
 *  Syntax & Parameters
 *
 *      int silo_Attach (PDBFile *pdb_file);
 *
 *      silo_Attach {Out}   {dbid if successful, else 0}
 *      pdb_file    {In}    {PDB file identifier}
 *
 *  Notes
 *
 *      This is NOT a user-level function.
 *
 *--------------------------------------------------------------------*/
int
silo_Attach (PDBfile *pdb_file)
{
   int            next;

   /* Get index of next available entry in internal table. */
   if ((next = silo_GetIndex(-1)) < 0) {
      silo_Error("Too many SILO's are open; tables are full.", SILO_ERROR);
      return (OOPS);
   }

   /* Update internal SILO table. */
   silo_table[next].dbid = next;
   silo_table[next].modified = FALSE;
   silo_table[next].pdbfile = pdb_file;
   silo_table[next].curr_dir = 0;

   num_active_dbs++;

   return (silo_table[next].dbid);
}

/*----------------------------------------------------------------------
 *  Routine                                                  silo_Release
 *
 *  Purpose
 *
 *      Release the given database tables; close the associated PDB.
 *
 *  Syntax & Parameters
 *
 *      int silo_Release (dbid db_id)
 *
 *      silo_Release   {Out}   {TRUE if successful, else FALSE}
 *      dbid          {In}    {DB identifier}
 *
 *  Notes
 *
 *      This is NOT a user-level function.
 *
 *--------------------------------------------------------------------*/
INTERNAL int
silo_Release (int dbid)
{
   int            index;

   /* Find index of dbid in internal table. */
   index = silo_GetIndex(dbid);

   /* Close the associated PDB file. */
   (void)lite_PD_close(silo_table[index].pdbfile);

   /* Clear out this entry in table. */
   silo_ClearTables(dbid);

   silo_table[index].dbid = -1;
   silo_table[index].pdbfile = (PDBfile *) 0;
   silo_table[index].curr_dir = 0;

   num_active_dbs--;

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                 silo_GetIndex
 *
 *  Purpose
 *
 *      Return the index of the given DB in the internal DB table.
 *
 *  Notes
 *
 *      This is NOT a user-level function.
 *
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetIndex (int dbid)
{
   int            i, index = -1;

   /*
    * Search through the internal table for the given dbid.
    * Return the index of the entry, or else OOPS.
    */
   for (i = 0; i < MAX_SILO; i++) {
      if (silo_table[i].dbid == dbid) {
	 index = i;
	 break;
      }
   }

   if (index < 0)
      silo_Error("Cannot find requested SILO.", SILO_ERROR);

   return (index);
}

/*----------------------------------------------------------------------
 *  Routine                                                    silo_Error
 *
 *  Purpose
 *
 *      Assign the SILO error message.
 *
 *  Notes
 *
 *      This is NOT a user-level function.
 *
 * Modified
 *
 *      Robb Matzke, Tue Jan 10 17:12:15 EST 1995
 *      Error handling is done by SILO-DVI, so this function is
 *      now a no-op.  Perhaps this function should call db_perror
 *      directly just in case the caller didn't check return status.
 *
 *--------------------------------------------------------------------*/
/* ARGSUSED */
void
silo_Error (char *errmsg, int errcode)
{

   strcpy(silo_err, errmsg);
}

/*----------------------------------------------------------------------
 *  Routine                                                    silo_Read
 *
 *  Purpose
 *
 *      Read the requested variable from the specified DB.
 *
 *--------------------------------------------------------------------*/
INTERNAL int
silo_Read (int dbid, char *varname, void *ptr)
{

   ASSERT_NAME(varname, 0);
   ASSERT_PTR(ptr, 0);
   ASSERT_DBID(dbid, 0);

   if (lite_PD_read(silo_table[dbid].pdbfile, varname, ptr) == 0) {
      silo_Error("Cannot read requested variable.", SILO_DEBUG);
      return (OOPS);
   }

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                silo_GetTables
 *
 *  Purpose
 *
 *      Read the internal SILO tables from the specified DB.
 *
 *  Parameters
 *
 *      dbid     =|   Database identifier to write to.
 *
 *  Notes
 *
 *
 *
 *--------------------------------------------------------------------*/
int
silo_GetTables (int dbid)
{
   DirEnt       **dirents;
   DimEnt       **diments;
   AttEnt       **attents;
   VarEnt       **varents;
   ObjEnt       **objents;

   dirents = NULL;
   diments = NULL;
   attents = NULL;
   varents = NULL;
   objents = NULL;

   /* Read header, which contains number of entities per type */
   if (silo_Read(dbid, HEADER_NAME, header) == OOPS) {
      silo_Error("SILO header missing from file", SILO_ERROR);
      return (OOPS);
   }

   /*
    *  Read various entity tables.
    */
   silo_Read(dbid, DIRENT_NAME, &dirents);
   if (dirents == NULL && header[3] > 0)
      return (OOPS);
   dirTable[dbid]->ent = dirents;
   dirTable[dbid]->num_used = header[3];
   dirTable[dbid]->num_alloced = header[3];

   silo_Read(dbid, DIMENT_NAME, &diments);
   if (diments == NULL && header[4] > 0)
      return (OOPS);
   dimTable[dbid]->ent = diments;
   dimTable[dbid]->num_used = header[4];
   dimTable[dbid]->num_alloced = header[4];

   silo_Read(dbid, ATTENT_NAME, &attents);
   if (attents == NULL && header[5] > 0)
      return (OOPS);
   attTable[dbid]->ent = attents;
   attTable[dbid]->num_used = header[5];
   attTable[dbid]->num_alloced = header[5];

   silo_Read(dbid, VARENT_NAME, &varents);
   if (varents == NULL && header[6] > 0)
      return (OOPS);
   varTable[dbid]->ent = varents;
   varTable[dbid]->num_used = header[6];
   varTable[dbid]->num_alloced = header[6];

   silo_Read(dbid, OBJENT_NAME, &objents);
   if (objents == NULL && header[7] > 0)
      return (OOPS);
   objTable[dbid]->ent = objents;
   objTable[dbid]->num_used = header[7];
   objTable[dbid]->num_alloced = header[7];

   return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                              silo_GetDataSize
 *
 *  Purpose
 *
 *      Return the byte length of the given data type within the
 *      database (i.e., on the DESTINATION machine).
 *
 *  Notes
 *
 *  Modified
 *
 *      Robb Matzke, Wed Jan 11 07:12:13 PST 1995
 *      Changed SILO_... data types to DB_... data types.
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetDataSize (int dbid, int datatype)
{
   int            size;
   char          *type;
   defstr        *dp;

   switch (datatype) {
   case DB_INT:
      type = "integer";
      break;
   case DB_SHORT:
      type = "short";
      break;
   case DB_LONG:
      type = "long";
      break;
   case DB_FLOAT:
      type = "float";
      break;
   case DB_DOUBLE:
      type = "double";
      break;
   case DB_CHAR:
      type = "char";
      break;
   default:
      type = NULL;
      break;
   }

   size = 0;

   if (type != NULL) {
      dp = (defstr *)lite_SC_def_lookup(type, silo_table[dbid].pdbfile->chart);
      if (dp != NULL)
	 size = dp->size;
   }

   return (size);
}

/*----------------------------------------------------------------------
 *  Routine                                          silo_GetMachDataSize
 *
 *  Purpose
 *
 *      Return the byte length of the given data type ON THE CURRENT
 *      MACHINE.
 *
 *  Notes
 *
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetMachDataSize (int datatype)
{
   int            size;

   switch (datatype) {
   case DB_INT:
      size = sizeof(int);

      break;
   case DB_SHORT:
      size = sizeof(short);

      break;
   case DB_LONG:
      size = sizeof(long);

      break;
   case DB_FLOAT:
      size = sizeof(float);

      break;
   case DB_DOUBLE:
      size = sizeof(double);

      break;
   case DB_CHAR:
      size = sizeof(char);

      break;
   default:
      size = 0;
      break;
   }

   return (size);
}

/*----------------------------------------------------------------------
 *  Routine                                                silo_GetIndex1
 *
 *  Purpose
 *
 *      Return the 1D index equivalent of the given index array.
 *
 *  Notes
 *
 *--------------------------------------------------------------------*/
INTERNAL int
silo_GetIndex1 (int index[], int dims[], int ndims)
{
   int            i, m, ndx = 0;

   for (m = 1, i = 0; i < ndims - 1; i++)
      m *= dims[i];

   for (ndx = 0, i = ndims - 1; i >= 0; i--) {
      ndx += index[i] * m;
      if (i > 0)
	 m /= dims[i - 1];
   }

   return (ndx);
}

/*----------------------------------------------------------------------
 *  Routine                                             silo_GetHypercube
 *
 *  Purpose
 *
 *      Get a hypercube of data from another (larger)  hypercube.
 *
 *  Notes
 *
 *      'sink' and 'source' are multi-dimensional arrays of simple
 *      type (i.e., float, int, double).  This routine copies data
 *      from some specified location within an ndims-dimensional
 *      array (source) into another ndims-dimensional array (sink).
 *
 *      This is a recursive function.
 *
 *  Modifications
 *      Sean Ahern, Wed Oct  4 14:22:12 PDT 1995
 *      Changed sink and source to be void pointers.
 *
 *--------------------------------------------------------------------*/
INTERNAL void
silo_GetHypercube(void *sink, void *source, int *dims, int ndims, int *start,
		  int *count, int  lenel)
{
   int            i, isrc, isink;  /* Byte indeces into sink and source */
   int            sink_start[10],  /* Starting index into sink h-cube */
                  source_start[10];  /* Starting index into source h-cube */
   char          *local_sink, *local_source; /* Index by bytes */

   local_sink = (char*)sink;
   local_source = source;

   /* If sink is 1D, just do a memory copy from source */
   if (ndims == 1) {
      isrc = start[0] * lenel;
      isink = 0;

      memcpy(&local_sink[isink], &local_source[isrc], count[0] * lenel); /*OK*/
   }
   else {
      /* Build index arrays for sink and source */
      for (i = 0; i < ndims; i++) {
	 sink_start[i] = 0;
	 source_start[i] = 0;
      }
      source_start[ndims - 1] = start[ndims - 1];

      for (i = 0; i < count[ndims - 1]; i++) {

	 /* Find starting location of given indeces */
	 isink = lenel * silo_GetIndex1(sink_start, count, ndims);
	 isrc = lenel * silo_GetIndex1(source_start, dims, ndims);

	 silo_GetHypercube(&local_sink[isink], &local_source[isrc],
			   dims, ndims - 1, start, count, lenel);

	 sink_start[ndims - 1]++;
	 source_start[ndims - 1]++;
      }
   }
}

/*----------------------------------------------------------------------
 *  Routine                                               silo_GetDimSize
 *
 *  Function
 *
 *      Return the size of the given dimension ID.  This is a
 *      convenience function, so ncdiminq needn't be called.
 *
 *  Modifications:
 *
 *    Hank Childs, Thu Oct 12 10:26:10 PDT 2000
 *    Changed ncdiminq to silonetcdf_ncdiminq to take care of unresolved
 *    symbol.
 *
 *---------------------------------------------------------------------*/
INTERNAL int
silo_GetDimSize (int dbid, int dimid)
{
   int            size;

   silonetcdf_ncdiminq(dbid, dimid, NULL, &size);

   return (size);
}

/*----------------------------------------------------------------------
 *  Routine                                               silo_GetVarSize
 *
 *  Function
 *
 *      Return the size of the given variable; the number of elements
 *      and number of bytes per element are returned via arguments.
 *      The total byte-length is the function return value.
 *
 *      This has been provided as a convenience function.
 *
 *---------------------------------------------------------------------*/
INTERNAL int
silo_GetVarSize (int dbid, int varid, int *nels, int *nbytes_el)
{
   VarEnt        *ent;

   *nels = *nbytes_el = 0;

   if ((ent = silo_GetVarEnt(dbid, silonetcdf_ncdirget(dbid), varid)) == NULL)
      return (OOPS);

   *nels = ent->nels;
   *nbytes_el = ent->lenel;

   return (*nels * *nbytes_el);
}

/*----------------------------------------------------------------------
 *  Routine                                                  silo_Verify
 *
 *  Function
 *
 *      Return TRUE if this is a SILO file.
 *
 *  Modifications:
 *
 *      Jim Reus, 23 Apr 97
 *      Change to prototype form.
 *
 *---------------------------------------------------------------------*/
int
silo_Verify (PDBfile *file)
{
   char           whatami[32];

   /* Read the 'whatami' variable and verify that this is a SILO file */
   if (lite_PD_read(file, WHATAMI_NAME, whatami) == 0)
      return (FALSE);

   if (STR_BEGINSWITH(whatami, "silo-pdb-2"))
      return (TRUE);
   else {
      if (STR_BEGINSWITH(whatami, "silo-pdb"))
	 silo_Error("File is out of date (old version of SILO)",
		    SILO_ERROR);

      return (FALSE);
   }
}
