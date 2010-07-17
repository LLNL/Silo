/*

                           Copyright (c) 1991 - 2009
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/

/*
 * Driver for opening an as-yet-undetermined-type database.
 */

#include "silo_unknown_private.h"

/*-------------------------------------------------------------------------
 * Function:    db_unk_Open
 *
 * Purpose:     Attempt to open a database whose type we do not know.
 *              We do this by invoking each callback defined in the
 *              DBOpenCB vector watching out not to invoke this function
 *              recursively.
 *
 *              If a device driver is unable to open a file, it is required
 *              to call db_perror() and return a NULL pointer.  This function
 *              prevents other drivers from issuing error messages by
 *              setting the error reporting level to DB_NONE.  When a driver
 *              can't open a file, it will call db_perror which will return
 *              to this function as things are unwinding back toward the
 *              DBOpen() that started all this.  When this function gets
 *              control from an error, it will cancel the error unwinding
 *              and try the next driver.
 *
 *              If all drivers fail, this function raises an E_NOTIMP
 *              error.
 *
 * Return:      Success:        ptr to new file whose callbacks and type
 *                              are initialized for the appropriate driver.
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Mon Dec 12 14:09:59 EST 1994
 *
 * Modifications:
 *     Sean Ahern, Mon Jan  8 17:55:54 PST 1996
 *     Added the mode parameter and logic.
 *
 *     Sean Ahern, Tue Jan  9 18:04:17 PST 1996
 *     Added the ability to specify a driver hierarchy.
 *
 *     Sean Ahern, Mon Oct 12 17:45:08 PDT 1998
 *     Removed AIO, since we no longer support it.
 *
 *     Mark Miller, Thu Mar 25 17:54:02 PST 1999
 *     Added DMF driver
 *
 *     Mark C. Miller, Tue Aug  1 10:35:32 PDT 2006
 *     Added subtype arg. Eliminated exudos. Moved HDF5 to second place.
 *
 *     Mark C. Miller, Mon Nov 19 10:45:05 PST 2007
 *     Added HDF5 driver warning.
 *
 *     Mark C. Miller, Fri Feb 12 08:16:37 PST 2010
 *     Replaced use of access() system call with db_silo_stat.
 *     Added loop over split vfds trying various defined extension pairs.
 *
 *     Mark C. Miller, Thu Mar 18 18:16:22 PDT 2010
 *     Increased size of tried/ascii to accomodate HDF5 options sets.
 *
 *     Mark C. Miller, Sat May 15 16:11:13 PDT 2010
 *     Add slot for PDB Proper. Put it ahead of PDB (lite) in priority.
 *
 *     Mark C. Miller, Fri May 21 08:24:31 PDT 2010
 *     Moved logic stating a file and checking for write permissions up
 *     to interface layer, silo.c
 *-------------------------------------------------------------------------*/
INTERNAL DBfile *
db_unk_Open(char *name, int mode, int subtype_dummy)
{
    DBfile        *opened = NULL;
    int            type;
    char          *me = "db_unk_Open";
    char           tried[1024], ascii[32];

    /* Hierarchy defined as:
     *      DB_PDBP, DB_PDB, DB_HDF5, DB_NETCDF, DB_TAURUS, DB_DEBUG
     * The reason we specify them as numbers instead of DB_WHATEVER is
     * that the driver might not be defined in silo.h.
     */
    static int     hierarchy[] = { 1, 2, 7, 0, 3, 6 };
    static char *  hierarchy_names[] = { "PDBP", "PDB", "HDF5", "NetCDF", "Taurus",
                                         "debug" };
    static int     nhiers = sizeof(hierarchy) / sizeof(hierarchy[0]);

    /*
     * Try each driver...
     */
    DBShowErrors(DB_SUSPEND, NULL);
    strcpy(tried, "attempted SILO drivers:");
    for (type = 0; !opened && type < nhiers; type++)
    {
        if (DBOpenCB[hierarchy[type]] == NULL)
            continue;
        sprintf(ascii, " %s", hierarchy_names[type]);
        strcat(tried, ascii);
        PROTECT {
            opened = (DBOpenCB[hierarchy[type]]) (name, mode, subtype_dummy);
        }
        CLEANUP {
            CANCEL_UNWIND;
        }
        END_PROTECT;
    }

    /*
     * try various HDF5 options sets 
     */
    if (!opened && DBOpenCB[7]!=NULL)
    {
        int i;
        const int *opts_set_ids = db_get_used_file_options_sets_ids();

        for (i = 0; !opened && opts_set_ids[i]!=-1; i++)
        {
            sprintf(ascii, " DB_HDF5_OPTS(%d)", opts_set_ids[i]);
            strcat(tried, ascii);
            PROTECT {
                opened = (DBOpenCB[7]) (name, mode, opts_set_ids[i]);
            }
            CLEANUP {
                CANCEL_UNWIND;
            }
            END_PROTECT;
        }
    }
    DBShowErrors(DB_RESUME, NULL);

    if (opened == NULL)
    {
        if (DBGetDriverTypeFromPath(name) == 7)
        {
            db_perror(tried, E_NOHDF5, me);
        }
        else
        {
            db_perror(tried, E_NOTIMP, me);
        }
    }
    return (opened);
}
