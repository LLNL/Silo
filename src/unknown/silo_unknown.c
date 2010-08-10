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
    DBErrFunc_t    oldErrfunc = 0;
    int            oldErrlvl;
    int            i, j, type, ntried = 0;
    int            driver_types_already_tried[MAX_FILE_OPTIONS_SETS+10+1];
    int            default_driver_priorities[MAX_FILE_OPTIONS_SETS+10+1] =
                       DEFAULT_DRIVER_PRIORITIES;
    int            priorities_are_set_to_default = 1;
    char          *me = "db_unk_Open";
    char           tried[1024], ascii[32];
    const int *opts_set_ids = db_get_used_file_options_sets_ids();
    static const char *hierarchy_names[] = {"NetCDF", "PDB Proper", "PDB",
                                            "Taurus", "", "", "Debug", "HDF5", "", ""};

    /* Save current error reporting and then turn it off */
    oldErrlvl = DBErrlvl();
    oldErrfunc = DBErrfunc();
    DBShowErrors(DB_SUSPEND, NULL);
    strcpy(tried, "attempted SILO drivers:");

    /* Initialize list of driver ids we've already tried */
    for (i = 0; i < sizeof(driver_types_already_tried)/sizeof(driver_types_already_tried[0]); i++)
        driver_types_already_tried[i] = -1;

    /* See if we're using default priorities or not */
    for (i = 0; i < sizeof(default_driver_priorities)/sizeof(default_driver_priorities[0]); i++)
    {
        if (SILO_Globals.unknownDriverPriorities[i] != default_driver_priorities[i])
        {
            priorities_are_set_to_default = 0;
            break;
        }
    }

    /* If we're not using default priorities, we need to try them first */
    if (!priorities_are_set_to_default)
    {
        for (type = 0; !opened && SILO_Globals.unknownDriverPriorities[type]!=-1; type++)
        {
            int driverId;
            int driverType = SILO_Globals.unknownDriverPriorities[type];
            int tried_already = 0;
            db_DriverTypeAndFileOptionsSetId(driverType, &driverId, 0);
            for (i = 0; driver_types_already_tried[i]!=-1; i++)
            {
                 if (driverType == driver_types_already_tried[i])
                 {
                     tried_already = 1;
                     break;
                 }
            }
            if (tried_already)
                continue;
            if (DBOpenCB[driverId] == NULL)
                continue;
            sprintf(ascii, " %s", hierarchy_names[driverId]);
            strcat(tried, ascii);
            PROTECT {
                opened = (DBOpenCB[driverId]) (name, mode, subtype_dummy);
            }
            CLEANUP {
                CANCEL_UNWIND;
            }
            END_PROTECT;
            driver_types_already_tried[ntried++] = driverType;
        }
    }
     
    /*
     * Try various registered options sets. Note, here we are going to try
     * ONLY file options sets registered by Silo client and NOT any of the
     * default file options sets.
     */
    if (DBOpenCB[DB_HDF5X]!=NULL)
    {
        for (i = 0; !opened && opts_set_ids[i]!=-1; i++)
        {
            int driverId;
            int driverType = DB_HDF5_OPTS(opts_set_ids[i]); 
            int tried_already = 0;
            db_DriverTypeAndFileOptionsSetId(driverType, &driverId, 0);
            for (j = 0; driver_types_already_tried[j]!=-1; j++)
            {
                 if (driverType == driver_types_already_tried[j])
                 {
                     tried_already = 1;
                     break;
                 }
            }
            if (tried_already)
                continue;
            /* skip the 'default' ones */
            if (opts_set_ids[i] < NUM_DEFAULT_FILE_OPTIONS_SETS)
                continue;
            sprintf(ascii, " DB_HDF5_OPTS(%d)", opts_set_ids[i]);
            strcat(tried, ascii);
            PROTECT {
                opened = (DBOpenCB[DB_HDF5X]) (name, mode, opts_set_ids[i]);
            }
            CLEANUP {
                CANCEL_UNWIND;
            }
            END_PROTECT;
            driver_types_already_tried[ntried++] = driverType;
        }
    }

    /*
     * Try each driver according to priority ordering specified in Silo
     * Globals being careful NOT re-try any that we already tried above.
     */
    if (priorities_are_set_to_default)
    {
        for (type = 0; !opened && SILO_Globals.unknownDriverPriorities[type]!=-1; type++)
        {
            int driverId;
            int driverType = SILO_Globals.unknownDriverPriorities[type];
            int tried_already = 0;
            db_DriverTypeAndFileOptionsSetId(driverType, &driverId, 0);
            for (j = 0; driver_types_already_tried[j]!=-1; j++)
            {
                 if (driverType == driver_types_already_tried[j])
                 {
                     tried_already = 1;
                     break;
                 }
            }
            if (tried_already)
                continue;
            if (DBOpenCB[driverId] == NULL)
                continue;
            sprintf(ascii, " %s", hierarchy_names[driverId]);
            strcat(tried, ascii);
            PROTECT {
                opened = (DBOpenCB[driverId]) (name, mode, subtype_dummy);
            }
            CLEANUP {
                CANCEL_UNWIND;
            }
            END_PROTECT;
            driver_types_already_tried[ntried++] = driverType;
        }
    }

    /*
     * Try default registered options sets now.
     */
    if (DBOpenCB[DB_HDF5X]!=NULL)
    {
        for (i = 0; !opened && opts_set_ids[i]!=-1; i++)
        {
            int driverId;
            int driverType = DB_HDF5_OPTS(opts_set_ids[i]); 
            int tried_already = 0;
            db_DriverTypeAndFileOptionsSetId(driverType, &driverId, 0);
            for (j = 0; driver_types_already_tried[j]!=-1; j++)
            {
                 if (driverType == driver_types_already_tried[j])
                 {
                     tried_already = 1;
                     break;
                 }
            }
            if (tried_already)
                continue;
            /* skip the 'default' ones */
            if (opts_set_ids[i] >= NUM_DEFAULT_FILE_OPTIONS_SETS)
                continue;
            sprintf(ascii, " DB_HDF5_OPTS(%d)", opts_set_ids[i]);
            strcat(tried, ascii);
            PROTECT {
                opened = (DBOpenCB[DB_HDF5X]) (name, mode, opts_set_ids[i]);
            }
            CLEANUP {
                CANCEL_UNWIND;
            }
            END_PROTECT;
            driver_types_already_tried[ntried++] = driverType;
        }
    }

    /* Return error reporting to behavior we had before entering this function */
    DBShowErrors(oldErrlvl, oldErrfunc);

    if (!opened)
    {
        if (DBGetDriverTypeFromPath(name) == DB_HDF5X)
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
