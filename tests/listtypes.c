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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <silo.h>
#include <config.h>

#define ALLOC_N(T, N) (T *)malloc((N) * sizeof(T))
#define FREE(P) if(P) {free(P); (P) = NULL;}

/* Prototypes */
void PrintFileComponentTypes(char *);
void PrintObjectComponentsType(DBfile *, char *, char *);

/*********************************************************************
 *
 * Purpose: Converts an int DB type to its string name.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 20 14:12:23 PST 2000
 *
 * Input Arguments:
 *    type : This is an integer representing a DB type.
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Sep 21 15:17:08 PDT 2009
 *   Adding support for long long type.
 *
 *   Mark C. Miller, Mon Dec  7 09:50:19 PST 2009
 *   Conditionally compile long long support only when its
 *   different from long.
 *
 *   Mark C. Miller, Mon Jan 11 16:02:16 PST 2010
 *   Made long long support UNconditionally compiled.
 ********************************************************************/

char *
IntToTypename(int type)
{
    char *retval;

    switch(type)
    {
    case DB_INT:
        retval = "DB_INT";
        break;
    case DB_SHORT:
        retval = "DB_SHORT";
        break;
    case DB_LONG:
        retval = "DB_LONG";
        break;
    case DB_LONG_LONG:
        retval = "DB_LONG_LONG";
        break;
    case DB_FLOAT:
        retval = "DB_FLOAT";
        break;
    case DB_DOUBLE:
        retval = "DB_DOUBLE";
        break;
    case DB_CHAR:
        retval = "DB_CHAR";
        break;
    case DB_VARIABLE:
        retval = "DB_VARIABLE";
        break;
    default:
        retval = "DB_NOTYPE";
    }

    return retval;
}

/*********************************************************************
 *
 * Purpose: Main function for listtypes.c. This function iterates
 *          over the command line arguments and supplies them to
 *          a function that prints the component types for a file.
 *          This program tests the DBGetComponentType function.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 20 13:05:37 PST 2000
 *
 * Input Arguments:
 *     argc : The number of command line arguments.
 *     argv : An array containing the command line arguments.
 *
 * Modifications:
 *     Thomas R. Treadway, Thu Jul  5 16:33:38 PDT 2007
 *     Chaneged main's return type to int, to stop gcc-4.x whining.
 *
 ********************************************************************/

int
main(int argc, char *argv[])
{
    int i;

    if(argc < 2)
    {
        printf("Usage: listtypes filename [filename ...]\n");
        exit(0);
    }

    DBShowErrors (DB_NONE, NULL);

    /* Print the types for components in the specified files. */
    for(i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "show-all-errors"))
            DBShowErrors (DB_ALL_AND_DRVR, NULL);
        else
            PrintFileComponentTypes(argv[i]);
    }
    
    return 0;
}

/*********************************************************************
 *
 * Purpose: Macroize code that processes entries in the TOC.
 *
 * The last 'S' argument to the macro is used to handle plural or
 * singular form of specification of toc data members.
 *
 * Programmer: Mark C. Miller
 * Date:       June 19, 2008 
 *
 * Modifications:
 *
 *   Mark C. Miller, Wed Sep 23 11:55:48 PDT 2009
 *   Added misc. variables.
 ********************************************************************/
#define PRINT_OBJS(theFile, theToc, theClass, Indent)                \
    nobjs += theToc->n ## theClass;                                  \
    for (i = 0; i < theToc->n ## theClass; i++)                      \
        PrintObjectComponentsType(theFile, theToc->theClass ## _names[i], Indent);

int ProcessCurrentDirectory(DBfile *dbfile, DBtoc *dbtoc, int depth)
{
    int i, j, nobjs;
    char indent[1024];
    int ndirs = dbtoc->ndir;

    /* compute an indent */
    for (i = 0; i < depth * 3; i++)
        indent[i] = ' ';
    indent[i] = '\0';

    /* descend into subdirs, first */
    if(ndirs > 0)
    {
        /* Make a list of all the directory names since the list
         * in the TOC will change as we change directories.
         */
        char currentdir[1024];
        char **dir_names = ALLOC_N(char *, ndirs);
        DBGetDir(dbfile, currentdir);
        for(i = 0; i < ndirs; i++)
        {
            dir_names[i] = ALLOC_N(char, 1+strlen(dbtoc->dir_names[i])+
                                   strlen(currentdir));
            sprintf(dir_names[i], "%s%s", currentdir, dbtoc->dir_names[i]);
        }

        /* Search each directory for objects. */
        for(j = 0; j < ndirs; j++)
        {
            /* Change directories and get the TOC. */
            DBtoc *current_dbtoc;
            DBSetDir(dbfile, dir_names[j]);
            current_dbtoc = DBGetToc(dbfile);
            printf("%sDirectory: %s\n", indent, dir_names[j]);
            if (ProcessCurrentDirectory(dbfile, current_dbtoc, depth+1) <= 0)
                printf("%s<directory contains no objects>\n\n", indent);
        }

        /* Free the directory list. */
        for(i = 0; i < ndirs; i++)
            FREE(dir_names[i]);
        FREE(dir_names);

        DBSetDir(dbfile, currentdir);
        dbtoc = DBGetToc(dbfile);
    }

    /* Print the objects in the top directory. */
    nobjs = ndirs; 
    PRINT_OBJS(dbfile, dbtoc, obj, indent);
    PRINT_OBJS(dbfile, dbtoc, defvars, indent);
    PRINT_OBJS(dbfile, dbtoc, array, indent);
    PRINT_OBJS(dbfile, dbtoc, var, indent);
    PRINT_OBJS(dbfile, dbtoc, curve, indent);
    PRINT_OBJS(dbfile, dbtoc, ptmesh, indent);
    PRINT_OBJS(dbfile, dbtoc, ptvar, indent);
    PRINT_OBJS(dbfile, dbtoc, qmesh, indent);
    PRINT_OBJS(dbfile, dbtoc, qvar, indent);
    PRINT_OBJS(dbfile, dbtoc, ucdmesh, indent);
    PRINT_OBJS(dbfile, dbtoc, ucdvar, indent);
    PRINT_OBJS(dbfile, dbtoc, csgmesh, indent);
    PRINT_OBJS(dbfile, dbtoc, csgvar, indent);
    PRINT_OBJS(dbfile, dbtoc, mat, indent);
    PRINT_OBJS(dbfile, dbtoc, matspecies, indent);
    PRINT_OBJS(dbfile, dbtoc, multimesh, indent);
    PRINT_OBJS(dbfile, dbtoc, multimeshadj, indent);
    PRINT_OBJS(dbfile, dbtoc, multivar, indent);
    PRINT_OBJS(dbfile, dbtoc, multimat, indent);
    PRINT_OBJS(dbfile, dbtoc, multimatspecies, indent);
    PRINT_OBJS(dbfile, dbtoc, mrgtree, indent);
    PRINT_OBJS(dbfile, dbtoc, mrgvar, indent);
    PRINT_OBJS(dbfile, dbtoc, groupelmap, indent);
    return nobjs;
}

/*********************************************************************
 *
 * Purpose: Reads the specified file and prints out the types of all
 *          of its components.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 20 12:21:36 PDT 2000
 *
 * Input Arguments:
 *     filename : The path and name the file we want to print out.
 *
 * Modifications:
 *
 *   Mark C. Miller, Wed Sep 23 11:56:19 PDT 2009
 *   Made it use DB_UNKNOWN driver to open.
 ********************************************************************/

void
PrintFileComponentTypes(char *filename)
{
    DBfile *dbfile = NULL;
    DBtoc  *dbtoc = NULL;

    /* Open the data file. Return if it cannot be read. */
    if((dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ)) == NULL)
    {
        printf("File: %s\n    <could not be opened>\n\n", filename);
        return;
    }

    /* Read the file's table of contents. */
    if((dbtoc = DBGetToc(dbfile)) == NULL)
    {
        printf("File: %s\n    <could not read TOC>\n\n", filename);
        DBClose(dbfile);
        return;
    }

    printf("File: %s\n", filename);

    if (ProcessCurrentDirectory(dbfile, dbtoc, 0) <= 0)
        printf("<file contains no objects>\n\n");

    /* Close the file. */
    DBClose(dbfile);
}

/*********************************************************************
 *
 * Purpose: Prints all of the components in the specified object.
 *          The component's name, type, and value are printed. The
 *          value is printed using the type information gathered
 *          from DBGetComponentType.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 20 13:09:48 PST 2000
 *
 * Input Arguments:
 *     dbfile  : The database file.
 *     objname : The name of the object whose components we're
 *               going to print.
 *
 * Modifications:
 *
 *  Mark C. Miller, Wed Sep 23 11:56:40 PDT 2009
 *  Added support for misc. variable printing.
 ********************************************************************/

void
PrintObjectComponentsType(DBfile *dbfile, char *objname, char *indent)
{
    int  i, comptype = DB_NOTYPE;
    DBobject *obj = NULL;

    /* Get the component names for the object. */
    if((obj = DBGetObject(dbfile, objname)) == NULL)
    {
        int len = DBGetVarLength(dbfile, objname);
        comptype = DBGetVarType(dbfile, objname);
        printf("%sObject: \"%s\" is a simple array\n", indent, objname);
        printf("    Length: %d  Type: %-11s\n", len, IntToTypename(comptype));
        return;
    }

    printf("%sObject: \"%s\"\n", indent, objname);
    if(obj->ncomponents > 0)
    {
        void *comp = NULL;

        /* For each component, read its type and print it. */
        for(i = 0; i < obj->ncomponents; i++)
        {
            comptype = DBGetComponentType(dbfile, objname, obj->comp_names[i]);

            comp = NULL;
            if(comptype != DB_VARIABLE)
                comp = DBGetComponent(dbfile, objname, obj->comp_names[i]);

            printf("    %sComponent: %-15s  Type: %-11s",
                   indent, obj->comp_names[i], IntToTypename(comptype));

            if(comp != NULL)
            {
                /* Use the type information returned by DBGetComponentType 
                 * to correctly process the component data.
                 */
                switch(comptype)
                {
                case DB_INT:
                    printf(" Value: %d\n", *((int *)comp));
                    break;
                case DB_CHAR:
                    printf(" Value: %s\n", (char *)comp);
                    break;
                case DB_FLOAT:
                    printf(" Value: %g\n", *((float *)comp));
                    break;
                case DB_DOUBLE:
                    printf(" Value: %.30g\n", *((double *)comp));
                    break;
                default:
                    printf(" Value: ???\n");
                }

                /* Free the component memory. */
                free(comp);
            }
            else
                printf("\n");
        }
        printf("\n");
    }
    else
        printf("    %s<no components>\n\n", indent);

    /* Free the object.*/
    DBFreeObject(obj);
}
