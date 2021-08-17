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

  Module Name                                                   object.c

  Purpose

        This module contains functions for easily reading SILO objects.

  Programmer

        Jeffery Long, NSSD/B

  Description

        The principal function here is SO_GetObject. It operates on a
        SO_Object data structure, which is simply a set of names,
        pointers, and data types which describe the components of a
        SILO object. SO_GetObject interprets the object description and
        reads the object components into the pointers provided.

        There are two macros defined in silo.h for simplifying the use
        of SO_GetObject. They are PREP_OBJ and DEF_OBJ.  PREP_OBJ takes
        a single argument, the address of a SO_Object to manipulate.
        DEF_OBJECT takes four arguments: the component name, the address
        of where to store the component, the component data type, and
        a sentinel indicating whether or not the component is a scalar.

  Contents

        SO_GetObject    (silo_id, obj_id, object)
        SO_GetComponent (silo_id, ent_id, ent_type, ent_parent)


  ======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

/*
 *  Module-wide global variables.
 */
static int     _so_force_single = FALSE;

/*----------------------------------------------------------------------
 *  Routine                                                 SO_GetObject
 *
 *  Purpose
 *
 *      Read the contents of an object, one entity at a time, and store
 *      each entity into the given structure.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *  Modifications
 *      Al Leibee, Thu Oct 21 15:34:21 PDT 1993
 *      Add datatype arg to SO_ReadComponent to coerce variable from
 *      datatype in file to datatype in memory.
 *
 *--------------------------------------------------------------------*/
INTERNAL int
SO_GetObject(int sid, int objid, SO_Object *tobj)
{
    int            i, j;
    int            type, ncomps, error;
    int            compids[50], comptypes[50], comppars[50];
    char          *s, *name, compnames[512];
    char           delim[2];
    int          **ipp;
    long         **lpp;
    float        **fpp;
    double       **dpp;
    char         **cpp;

    /* Determine number of components in object */
    silonetcdf_ncobjinq(sid, objid, NULL, &type, &ncomps);

    /* Read object */
    error = silonetcdf_ncobjget(sid, objid, compnames, compids, comptypes, comppars);
    if (error)
        return (-1);

    delim[0] = compnames[0];
    delim[1] = '\0';

    /* Parse object names and build structure */
    s = &compnames[1];
    name = (char *)strtok(s, delim);

    for (i = 0; i < ncomps; i++) {

        for (j = 0; j < tobj->num; j++) {

            if (tobj->ptr[j] != NULL &&
                STR_EQUAL(name, tobj->name[j])) {

                switch (tobj->alloced[j]) {

                    case TRUE: /* Read directly into space provided */

                        /*-----------------------------------------------------
                         *  Special case for scalar integer: if component
                         *  is of type literal, use component ID as a literal.
                         *----------------------------------------------------*/

                        if (tobj->type[j] == DB_INT &&
                            comptypes[i] == SILO_TYPE_LIT) {

                            *((int *)tobj->ptr[j]) = compids[i];

                        }
                        else {

                            SO_ReadComponent(sid, compids[i], comptypes[i],
                                             comppars[i], tobj->type[j],
                                             tobj->ptr[j]);

                        }
                        break;

                    case FALSE:  /* Space not allocated yet, do it. */

                        switch (tobj->type[j]) {
                            case DB_INT:
                                ipp = (int **)tobj->ptr[j];
                                *ipp = (int *)SO_GetComponent(sid, compids[i],
                                                            comptypes[i],
                                                            comppars[i]);
                                break;

                            case DB_LONG:
                                lpp = (long **)tobj->ptr[j];
                                *lpp = (long *)SO_GetComponent(sid, compids[i],
                                                            comptypes[i],
                                                            comppars[i]);
                                break;

                            case DB_FLOAT:
                                fpp = (float **)tobj->ptr[j];
                                *fpp = (float *)SO_GetComponent(sid, compids[i],
                                                            comptypes[i],
                                                            comppars[i]);
                                break;

                            case DB_DOUBLE:
                                dpp = (double **)tobj->ptr[j];
                                *dpp = (double *)SO_GetComponent(sid, compids[i],
                                                            comptypes[i],
                                                            comppars[i]);
                                break;

                            case DB_CHAR:
                            case DB_NOTYPE:
                                cpp = (char **)tobj->ptr[j];
                                *cpp = (char *)SO_GetComponent(sid, compids[i],
                                                            comptypes[i],
                                                            comppars[i]);
                                break;

                            default:
                                break;
                        }
                        break;

                    default:
                        break;
                }

                /*
                 *  If force-single is on, must change datatype
                 *  field to 'float'.
                 */
                if (_so_force_single && STR_EQUAL("datatype", name))
                    *((int *)tobj->ptr[j]) = DB_FLOAT;

            }
        }                       /* for-j */

        /*
         * Note - the preferred way of doing the following would
         * be with a call to strtok like: "strtok(NULL, ";")".
         * However, PDBlib also uses strtok, hence messing up
         * my use of it; thus I must provide the string argument
         * for each call.
         */
        name = (char *)strtok(name + strlen(name) + 1, delim);
        if (name == NULL)
            break;

    }                           /* for-i */

    return(0);
}

/*----------------------------------------------------------------------
 *  Routine                                              SO_GetComponent
 *
 *  Purpose
 *
 *      Alloc space for and read an entity of arbitrary type from
 *      a SILO file.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *  Modifications
 *     Al Leibee, Tue Sep  7 11:32:16 PDT 1993
 *     Replace SCORE mem allocation by system mem allocation
 *     so that memory returned to application is system
 *     allocated and can be freed by system memman.
 *
 *--------------------------------------------------------------------*/
INTERNAL void *
SO_GetComponent(int sid, int entid, int enttype, int entpar)
{
    int            datatype, ndims, natts, nels;
    int            i, original_dir;
    int            dims[20], start[20], count[20], index[5];
    char          *var;
    double        *local_d;
    float         *local_f;

    /*
     *  Save current dir; restore when done.
     */
    original_dir = silonetcdf_ncdirget(sid);

    /*
     *  Set directory to parent of requested entity.
     */
    if (OOPS == silonetcdf_ncdirset(sid, entpar))
        return (NULL);

    switch (enttype) {

        case SILO_TYPE_DIM:

            var = ALLOC_N(char, sizeof(int));

            if (OOPS == (silonetcdf_ncdiminq(sid, entid, NULL, (int *)var))) {
                FREE(var);
                return (NULL);
            }
            break;

        case SILO_TYPE_VAR:

            if (OOPS == (silonetcdf_ncvarinq(sid, entid, NULL,
                                             &datatype, &ndims, dims, &natts)))
                return (NULL);

            start[0] = 0;
            index[0] = 0;
            count[0] = 1;
            nels = 0;

            if (ndims > 0) {
                for (nels = 1, i = 0; i < ndims; i++) {
                    start[i] = 0;
                    count[i] = silo_GetDimSize(sid, dims[i]);
                    nels *= count[i];
                }
            }

            var = ALLOC_N(char, nels * silo_GetMachDataSize(datatype));

            if (nels == 1) {
                if (OOPS == (silonetcdf_ncvarget1(sid, entid, index, var))) {
                    FREE(var);
                    return (NULL);
                }
            }
            else {
                if (OOPS == (silonetcdf_ncvarget(sid, entid, start, count, var))) {
                    FREE(var);
                    return (NULL);
                }
            }

            /*
             *  Map double precision values to float if force-single flag is TRUE.
             */
            if (datatype == DB_DOUBLE && _so_force_single) {
                local_d = (double *)var;
                local_f = ALLOC_N(float, nels);

                for (i = 0; i < nels; i++)
                    local_f[i] = (float)local_d[i];

                FREE(var);
                var = (char *)local_f;
            }
            break;

        default:
            var = ALLOC_N(char, sizeof(int));
            memcpy(var, &entid, sizeof(int));

            break;
    }

    silonetcdf_ncdirset(sid, original_dir);

    return ((void *)var);
}

/*----------------------------------------------------------------------
 *  Routine                                             SO_ReadComponent
 *
 *  Purpose
 *
 *      Read an entity of arbitrary type from a SILO file into the
 *      provided space.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *  Modifications
 *     Al Leibee, Thu Oct 21 15:34:21 PDT 1993
 *     Added 'vartype' argument -- data type of the space into which
 *                                 the variable is to be read.
 *
 *     Sean Ahern, Wed Oct  4 13:49:54 PDT 1995
 *     Fixed some pointer type problems.
 *
 *--------------------------------------------------------------------*/
INTERNAL int
SO_ReadComponent(int sid, int entid, int enttype, int entpar, int vartype,
                 void *var)
{
    int            datatype, ndims, natts;
    int            i, nels, original_dir;
    int            dims[20], start[20], count[20], index[5];
    char          *local_var;
    double        *local_d;
    float         *local_f;

    /*
     *  Save current dir; restore when done.
     */
    original_dir = silonetcdf_ncdirget(sid);

    /*
     *  Set directory to parent of requested entity.
     */
    if (OOPS == silonetcdf_ncdirset(sid, entpar))
        return 0 ;

    switch (enttype) {

        case SILO_TYPE_DIM:

            if (OOPS == (silonetcdf_ncdiminq(sid, entid, NULL, (int *)var))) {
                return (OOPS);
            }
            break;

        case SILO_TYPE_VAR:

            if (OOPS == (silonetcdf_ncvarinq(sid, entid, NULL,
                                             &datatype, &ndims, dims, &natts)))
                return (OOPS);

            start[0] = 0;
            index[0] = 0;
            count[0] = 1;
            nels = 0;

            if (ndims > 0) {
                for (nels = 1, i = 0; i < ndims; i++) {
                    start[i] = 0;
                    count[i] = silo_GetDimSize(sid, dims[i]);
                    nels *= count[i];
                }
            }

            /*
             *  Allocate a local double array if requested variable's
             *  file datatype is DOUBLE and force-single flag is TRUE, or
             *  the variable's file datatype is DOUBLE and the variable's
             *  memory space is FLOAT.
             */

            if ((datatype == DB_DOUBLE && _so_force_single) ||
                (datatype == DB_DOUBLE && vartype == DB_FLOAT))
                local_var = (char *)ALLOC_N(double, nels);

            else
                local_var = (char*)var;

            if (nels == 1) {
                if (OOPS == (silonetcdf_ncvarget1(sid, entid, index, local_var))) {
                    return (OOPS);
                }
            }
            else {
                if (OOPS == (silonetcdf_ncvarget(sid, entid, start, count, local_var))) {
                    return (OOPS);
                }
            }

            /*
             *  Map double precision values to float if force-single flag
             *  is TRUE, or
             *  if if requested variable's file datatype is DOUBLE and
             *  the variable's memory space is FLOAT.
             */
            if ((datatype == DB_DOUBLE && _so_force_single) ||
                (datatype == DB_DOUBLE && vartype == DB_FLOAT)) {
                local_d = (double *)local_var;
                local_f = (float *)var;

                for (i = 0; i < nels; i++)
                    local_f[i] = (float)local_d[i];

                FREE(local_var);
            }

            break;

        default:
            memcpy(var, &entid, sizeof(int));

            break;
    }

    silonetcdf_ncdirset(sid, original_dir);

    return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                               SO_ForceSingle
 *
 *  Purpose
 *
 *      Set an internal sentinel stating whether or not real data should
 *      be forced to single precision before being returned.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *  Modified
 *      Robb Matzke, Wed Jan 11 07:25:59 PST 1995
 *      Device independence rewrite.
 *--------------------------------------------------------------------*/
INTERNAL int
SO_ForceSingle(int on_off)
{
    _so_force_single = on_off ? TRUE : FALSE;
    return 0;
}
