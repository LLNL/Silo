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
 * Robb Matzke, Fri Dec 9 13:05:38 EST 1994
 * Changed for device independence.  This is a support file and is not
 * strictly part of SILO or the SILO-Taurus device driver.  However,
 * we are including the `silo_taurus_private.h' header file to get the
 * memory management macros, `taurus.h' header file, and function
 * prototype for `db_taur_extface'.
 */

#define SILO_NO_CALLBACKS
#include "config.h"
#include "silo_taurus_private.h"

#if HAVE_FCNTL_H
#include <fcntl.h>              /*open */
#endif
#if HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>              /*open */
#endif
#include <math.h>               /*sqrt, acos */
#if HAVE_SYS_STAT_H
#include <sys/stat.h>           /*stat */
#endif
#include <ctype.h>		/*isspace */

#define MAXBUF 100000

#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif

/*
 * ftpi = (4/3)*pi, ttpi = (2/3)*pi
 */
#define ftpi 4.188790205
#define ttpi 2.094395102

/*
 * The list of taurus variables.
 */
var_list_s     taur_var_list[] =
{
    {"disp_x", "mesh1", 3, NODAL_VAR,
     VAL_COORDX, VAR_DISPX},
    {"disp_y", "mesh1", 3, NODAL_VAR,
     VAL_COORDY, VAR_DISPY},
    {"disp_z", "mesh1", 3, NODAL_VAR,
     VAL_COORDZ, VAR_DISPZ},
    {"disp_mag", "mesh1", 3, NODAL_VAR,
     VAL_COORDX, VAR_DISP_MAG},
    {"vel_x", "mesh1", 3, NODAL_VAR,
     VAL_VELX, VAR_NORMAL},
    {"vel_y", "mesh1", 3, NODAL_VAR,
     VAL_VELY, VAR_NORMAL},
    {"vel_z", "mesh1", 3, NODAL_VAR,
     VAL_VELZ, VAR_NORMAL},
    {"vel_mag", "mesh1", 3, NODAL_VAR,
     VAL_VELX, VAR_VEL_MAG},
    {"acc_x", "mesh1", 3, NODAL_VAR,
     VAL_ACCX, VAR_NORMAL},
    {"acc_y", "mesh1", 3, NODAL_VAR,
     VAL_ACCY, VAR_NORMAL},
    {"acc_z", "mesh1", 3, NODAL_VAR,
     VAL_ACCZ, VAR_NORMAL},
    {"acc_mag", "mesh1", 3, NODAL_VAR,
     VAL_ACCX, VAR_ACC_MAG},
    {"temp_x", "mesh1", 3, NODAL_VAR,
     VAL_TEMPX, VAR_NORMAL},
    {"temp_y", "mesh1", 3, NODAL_VAR,
     VAL_TEMPY, VAR_NORMAL},
    {"temp_z", "mesh1", 3, NODAL_VAR,
     VAL_TEMPZ, VAR_NORMAL},
    {"m_xx_bending", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_RES1, VAR_NORMAL},
    {"m_yy_bending", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_RES2, VAR_NORMAL},
    {"m_xy_bending", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_RES3, VAR_NORMAL},
    {"q_xx_shear", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_RES4, VAR_NORMAL},
    {"q_yy_shear", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_RES5, VAR_NORMAL},
    {"n_xx_normal", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_RES6, VAR_NORMAL},
    {"n_yy_normal", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_RES7, VAR_NORMAL},
    {"n_xy_normal", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_RES8, VAR_NORMAL},
    {"thickness", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_THICKNESS, VAR_NORMAL},
    {"int_energy", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_INT_ENG, VAR_NORMAL},
    {"surf_stress_1", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGX, VAR_SURF_STRESS_1},
    {"surf_stress_2", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGX, VAR_SURF_STRESS_2},
    {"surf_stress_3", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGX, VAR_SURF_STRESS_3},
    {"surf_stress_4", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGX, VAR_SURF_STRESS_4},
    {"surf_stress_5", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGX, VAR_SURF_STRESS_5},
    {"surf_stress_6", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGX, VAR_SURF_STRESS_6},
    {"eff_upp_stress", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGX, VAR_UP_STRESS},
    {"eff_low_stress", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGX, VAR_LOW_STRESS},
    {"eff_max_stress", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGX, VAR_MAX_STRESS},
    {"upp_surf_eps", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_OUT_EPS_EFF, VAR_NORMAL},
    {"low_surf_eps", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_IN_EPS_EFF, VAR_NORMAL},
    {"low_xx_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_IN_SIGX, VAR_NORMAL},
    {"low_yy_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_IN_SIGY, VAR_NORMAL},
    {"low_zz_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_IN_SIGZ, VAR_NORMAL},
    {"low_xy_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_IN_SIGXY, VAR_NORMAL},
    {"low_yz_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_IN_SIGYZ, VAR_NORMAL},
    {"low_zx_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_IN_SIGZX, VAR_NORMAL},
    {"upp_xx_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_OUT_SIGX, VAR_NORMAL},
    {"upp_yy_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_OUT_SIGY, VAR_NORMAL},
    {"upp_zz_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_OUT_SIGZ, VAR_NORMAL},
    {"upp_xy_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_OUT_SIGXY, VAR_NORMAL},
    {"upp_yz_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_OUT_SIGYZ, VAR_NORMAL},
    {"upp_zx_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_OUT_SIGZX, VAR_NORMAL},
    {"mid_xx_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGX, VAR_NORMAL},
    {"mid_yy_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGY, VAR_NORMAL},
    {"mid_zz_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGZ, VAR_NORMAL},
    {"mid_xy_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGXY, VAR_NORMAL},
    {"mid_yz_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGYZ, VAR_NORMAL},
    {"mid_zx_strain", "shell_mesh", 4, ZONAL_VAR,
     VAL_SHELL_MID_SIGZX, VAR_NORMAL},
    {"stress_xx", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGX, VAR_SIGX},
    {"stress_yy", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGY, VAR_SIGY},
    {"stress_zz", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGZ, VAR_SIGZ},
    {"stress_xy", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGXY, VAR_SIGXY},
    {"stress_yz", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGYZ, VAR_SIGYZ},
    {"stress_zx", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGZX, VAR_SIGZX},
    {"stress_eps", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_EPS_EFF, VAR_EPS},
    {"pressure", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGX, VAR_PRESSURE},
    {"stress_eff", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGX, VAR_SIG_EFF},
    {"princ_dev_stress_1", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGX, VAR_DEV_STRESS_1},
    {"princ_dev_stress_2", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGX, VAR_DEV_STRESS_2},
    {"princ_dev_stress_3", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGX, VAR_DEV_STRESS_3},
    {"max_shear_stress", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGX, VAR_MAX_SHEAR_STR},
    {"princ_stress_1", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGX, VAR_PRINC_STRESS_1},
    {"princ_stress_2", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGX, VAR_PRINC_STRESS_2},
    {"princ_stress_3", "hs_mesh", 5, ZONAL_VAR,
     VAL_HEX_SIGX, VAR_PRINC_STRESS_3},
    {"temperature", "mesh1", 8, NODAL_VAR,
     VAL_TEMP, VAR_NORMAL},
    {"flux_x", "mesh1", 8, NODAL_VAR,
     VAL_FLUXX, VAR_NORMAL},
    {"flux_y", "mesh1", 8, NODAL_VAR,
     VAL_FLUXY, VAR_NORMAL},
    {"flux_z", "mesh1", 8, NODAL_VAR,
     VAL_FLUXZ, VAR_NORMAL},
    {"vel_x", "mesh1", 9, NODAL_VAR,
     VAL_VELX, VAR_NORMAL},
    {"vel_y", "mesh1", 9, NODAL_VAR,
     VAL_VELY, VAR_NORMAL},
    {"vel_z", "mesh1", 9, NODAL_VAR,
     VAL_VELZ, VAR_NORMAL},
    {"vel_mag", "mesh1", 9, NODAL_VAR,
     VAL_VELX, VAR_VEL_MAG},
    {"vort_x", "mesh1", 9, NODAL_VAR,
     VAL_VORTX, VAR_NORMAL},
    {"vort_y", "mesh1", 9, NODAL_VAR,
     VAL_VORTY, VAR_NORMAL},
    {"vort_z", "mesh1", 9, NODAL_VAR,
     VAL_VORTZ, VAR_NORMAL},
    {"vort_mag", "mesh1", 9, NODAL_VAR,
     VAL_VORTX, VAR_VORT_MAG},
    {"pressure", "hs_mesh", 9, ZONAL_VAR,
     VAL_PRESSURE, VAR_PRESSURE},
    {"dummy", "dummy", 10, NODAL_VAR,
     0, VAR_NORMAL}
};

/*-------------------------------------------------------------------------
 * Function:    fam_name
 *
 * Purpose:
 *
 * Return:      Success:        void
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to proto type form.
 *
 *-------------------------------------------------------------------------
 */
static void
fam_name (char *basename, int filenumber, char *filename)
{

    if (filenumber == 0)
        strcpy(filename, basename);
    else if (filenumber < 100)
        sprintf(filename, "%s%02d", basename, filenumber);
    else
        sprintf(filename, "%s%03d", basename, filenumber);
}

/*-------------------------------------------------------------------------
 * Function:    fix_title
 *
 * Purpose:     Fixes the screwy title.  For some reason, the title
 *              contains extra padding at character positions
 *              n*8+6 and n*8+7.  The title may also contain leading
 *              and trailing whitespace.  This routine will remove
 *              all of this.  This routine replaces a buggy version
 *              from the old Taurus stuff (see modification section
 *              below).
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke, Wed Jan 25 15:46:03 PST 1995
 *
 * Modifications:
 *
 *    Robb Matzke, Wed Jan 25 15:20:03 PST 1995
 *    Removed final `for' loop since the title should have
 *    only 41 characters counting the null terminator.
 *    Fixed final while loop for cases where the title contains
 *    no printable characters or all whitespce.  After removing
 *    the junk part of the title and shortening the title, we
 *    zero-fill everything to the right.  This is because the
 *    title size is initialized before we know what the title
 *    is and the browser output routines will print all bytes
 *    of the title, including the stuff after the null terminator.
 *
 *    Eric Brugger, Fri Apr 28 10:09:40 PDT 1995
 *    I modified the routine to correct screwy titles and leave normal
 *    ones alone.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
PRIVATE void
fix_title (char *title)
{
    int            i, j;
    int            fixit;

    /*
     * Determine if the title needs fixing.
     */
    fixit = TRUE;
    for (i = 0; i < 40; i += 8) {
        if (title[i + 6] != ' ' || title[i + 7] != ' ')
            fixit = FALSE;
    }

    /*
     * Fix it if necessary.
     */
    if (fixit == TRUE) {
        j = 0;
        for (i = 0; i < 6; i++)
            title[j++] = title[i];
        for (i = 8; i < 14; i++)
            title[j++] = title[i];
        for (i = 16; i < 22; i++)
            title[j++] = title[i];
        for (i = 24; i < 30; i++)
            title[j++] = title[i];
        for (i = 32; i < 38; i++)
            title[j++] = title[i];
        for (i = 0; i < 10; i++)
            title[j++] = ' ';
    }

    /*
     * Remove trailing blanks.
     */
    j = 39;
    while (j > 0 && isspace(title[j]))
        j--;
    title[j + 1] = '\0';
}

/*-------------------------------------------------------------------------
 * Function:    init_file_info
 *
 * Purpose:
 *
 * Return:      Success:        void
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static void
init_file_info (TAURUSfile *taurus)
{
    int            i;
    int            nfiles;
    struct stat    statbuf;

    /*
     * Determine the number of files.
     */
    nfiles = 0;
    fam_name(taurus->basename, nfiles, taurus->filename);
    while (stat(taurus->filename, &statbuf) != -1) {
        nfiles++;
        fam_name(taurus->basename, nfiles, taurus->filename);
    }
    taurus->nfiles = nfiles;

    /*
     * Determine the size of each file in the family.
     */
    taurus->filesize = ALLOC_N(int, nfiles);

    for (i = 0; i < nfiles; i++) {
        fam_name(taurus->basename, i, taurus->filename);
        stat(taurus->filename, &statbuf);
        taurus->filesize[i] = statbuf.st_size;
    }
}

/*-------------------------------------------------------------------------
 * Function:    taurus_read
 *
 * Purpose:
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *
 *    Eric Brugger, Fri Apr 28 10:41:17 PDT 1995
 *    I modified the routine to handle the case where the address is
 *    not in the specified file.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static int
taurus_read (TAURUSfile *taurus, int ifile, int iadd, int length, char *buffer)
{
    int            n;
    int            ibuf;
    int            idisk;

    /*
     * Skip to the correct file if the address is not in the
     * specified file.
     */
    while (iadd > taurus->filesize[ifile]) {
        iadd -= taurus->filesize[ifile];
        ifile++;
    }

    /*
     * Read the file.
     */
    ibuf = 0;
    idisk = iadd;
    while (length > 0) {
        /*
         * If the desired file is not open, close the current file
         * and open the desired file.
         */
        if (taurus->ifile != ifile) {
            if (taurus->fd != -1)
                close(taurus->fd);
            fam_name(taurus->basename, ifile, taurus->filename);
            if ((taurus->fd = open(taurus->filename, O_RDONLY)) < 0) {
                return (-1);
            }
            taurus->ifile = ifile;
        }

        /*
         * Read the maximum amount from the current file.
         */
        n = MIN(taurus->filesize[ifile] - idisk, length);
        lseek(taurus->fd, idisk, SEEK_SET);

        if (read(taurus->fd, &buffer[ibuf], n) != n) {
            return (-2);
        }

        ibuf += n;
        length -= n;
        ifile++;
        idisk = 0;
    }

    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    init_state_info
 *
 * Purpose:
 *
 * Return:      Success:        void
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Tue Mar 28 15:03:37 PST 1995
 *    I modified the routines to read a topaz3d data file.
 *
 *    Eric Brugger, Wed Apr 26 13:52:00 PDT 1995
 *    I modified the routine to set the directory to none.
 *
 *    Eric Brugger, Fri Apr 28 10:12:02 PDT 1995
 *    I modified the routine to handle states that overlapped several
 *    files.
 *
 *    Eric Brugger, Thu Jul 27 12:49:40 PDT 1995
 *    I modified the routine to handle files generated by hydra.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static void
init_state_info (TAURUSfile *taurus)
{
    int            i;
    int            geomsize;
    int            statesize;
    int            totsize;
    int            maxstates;
    int            nstates;
    int            loc;
    int            ifile;
    int            nfiles;
    int            nv1dact, nv2dact, nv3dact;

    nfiles = taurus->nfiles;

    nv1dact = taurus->nv1d;
    nv2dact = taurus->nv2d;
    nv3dact = taurus->nv3d;
    if (taurus->activ >= 1000 && taurus->activ <= 1005) {
        if (taurus->nel2 > 0)
            nv1dact++;
        if (taurus->nel4 > 0)
            nv2dact++;
        if (taurus->nel8 > 0)
            nv3dact++;
    }

    /*
     * Determine the file and disk address for the start of each
     * state in the database.
     */
    geomsize = (taurus->ndim * taurus->numnp +
                9 * taurus->nel8 +
                5 * taurus->nel4 +
                6 * taurus->nel2) * sizeof(int);

    switch (taurus->icode) {
            /*
             * topaz3d.
             */
        case 1:
            /*
             * This is an extension to the taurus data base.  If it
             * is four then fluxes are present.
             */
            if (taurus->it == 4)
                statesize = (4 * taurus->numnp + 1) * sizeof(int);

            else
                statesize = (1 * taurus->numnp + 1) * sizeof(int);

            break;
            /*
             * dyna3d or nike3d.
             */
        case 2:
        case 6:
        case 200:
            statesize = (taurus->it * taurus->numnp +
                         taurus->ndim * taurus->numnp *
                         (taurus->iu + taurus->iv + taurus->ia) +
                         taurus->nel8 * nv3dact +
                         taurus->nel4 * nv2dact +
                         taurus->nel2 * nv1dact +
                         taurus->nglbv + 1) * sizeof(int);

            break;
    }

    totsize = 0;
    for (i = 0; i < nfiles; i++)
        totsize += taurus->filesize[i];
    maxstates = (totsize / statesize) + 1;
    taurus->state_file = ALLOC_N(int, maxstates);
    taurus->state_loc = ALLOC_N(int, maxstates);
    taurus->state_time = ALLOC_N(float, maxstates);

    loc = 64 * sizeof(int) + geomsize;

    ifile = 0;
    while (loc >= taurus->filesize[ifile]) {
        loc -= taurus->filesize[ifile];
        ifile++;
    }

    for (nstates = 0;; nstates++) {
        if (nfiles == 1) {
            if ((loc + statesize) > taurus->filesize[ifile])
                break;
        }
        else if (statesize <= taurus->filesize[1]) {
            if ((loc + statesize) > taurus->filesize[ifile]) {
                ifile++;
                loc = 0;
                if (ifile >= nfiles)
                    break;
            }
        }
        else {
            while (loc > 0) {
                loc -= taurus->filesize[ifile];
                ifile++;
            }
            loc = 0;
            if (ifile >= nfiles)
                break;
        }

        taurus->state_file[nstates] = ifile;
        taurus->state_loc[nstates] = loc;

        loc += statesize;
    }

    taurus->nstates = nstates;
    taurus->state = -1;
    taurus->idir = -1;

    /*
     * Read in the time for each state.
     */
    for (i = 0; i < nstates; i++)
        taurus_read(taurus, taurus->state_file[i], taurus->state_loc[i],
                    sizeof(float), (char*)&taurus->state_time[i]);
}

/*-------------------------------------------------------------------------
 * Function:    init_var_info
 *
 * Purpose:
 *
 * Return:      Success:        void
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Tue Mar 28 15:03:37 PST 1995
 *    I modified the routines to read a topaz3d data file.
 *
 *    Eric Brugger, Thu Apr 27 08:47:01 PDT 1995
 *    I modified the code to work properly with state directories.
 *
 *    Eric Brugger, Thu Jul 27 12:49:40 PDT 1995
 *    I modified the routine to handle files generated by hydra.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static void
init_var_info (TAURUSfile *taurus)
{
    int            i;
    int            loc;

    /*
     * Set up the default values.
     */
    for (i = 0; i < MAX_VAL; i++)
        taurus->var_start[i] = -1;

    loc = (1 + taurus->nglbv) * sizeof(float);

    /*
     * Topaz3d data.
     */
    if (taurus->icode == 1) {
        taurus->var_start[VAL_TEMP] = loc;
        taurus->var_ncomps[VAL_TEMP] = 1;
        taurus->var_len[VAL_TEMP] = taurus->numnp;
        taurus->var_offset[VAL_TEMP] = 0;
        loc += taurus->numnp * sizeof(float);

        /*
         * This is an extension to the taurus data base.  If it
         * is four then fluxes are present.
         */
        if (taurus->it == 4) {
            for (i = VAL_FLUXX; i <= VAL_FLUXZ; i++) {
                taurus->var_start[i] = loc;
                taurus->var_ncomps[i] = 3;
                taurus->var_len[i] = taurus->numnp;
            }
            taurus->var_offset[VAL_FLUXX] = 0;
            taurus->var_offset[VAL_FLUXY] = 1;
            taurus->var_offset[VAL_FLUXZ] = 2;

            loc += 3 * taurus->numnp * sizeof(float);
        }

        return;
    }

    /*
     * Hydra data.
     */
    if (taurus->icode == 200) {
        if (taurus->iv != 0) {
            for (i = VAL_VELX; i <= VAL_VELZ; i++) {
                taurus->var_start[i] = loc;
                taurus->var_ncomps[i] = 3;
                taurus->var_len[i] = taurus->numnp;
            }
            taurus->var_offset[VAL_VELX] = 0;
            taurus->var_offset[VAL_VELY] = 1;
            taurus->var_offset[VAL_VELZ] = 2;
            loc += taurus->ndim * taurus->numnp * sizeof(float);
        }

        if (taurus->ia != 0) {
            for (i = VAL_VORTX; i <= VAL_VORTZ; i++) {
                taurus->var_start[i] = loc;
                taurus->var_ncomps[i] = 3;
                taurus->var_len[i] = taurus->numnp;
            }
            taurus->var_offset[VAL_VORTX] = 0;
            taurus->var_offset[VAL_VORTY] = 1;
            taurus->var_offset[VAL_VORTZ] = 2;
            loc += taurus->ndim * taurus->numnp * sizeof(float);
        }

        if (taurus->nel8 > 0 && taurus->nv3d == 7) {
            taurus->var_start[VAL_PRESSURE] = loc;
            taurus->var_ncomps[VAL_PRESSURE] = 7;
            taurus->var_len[VAL_PRESSURE] = taurus->nel8;
            taurus->var_offset[VAL_PRESSURE] = 0;
            loc += taurus->nv3d * taurus->nel8 * sizeof(float);
        }

        return;
    }

    /*
     * Initial nodal coordinates (for displacement calculations).
     */
    if (taurus->iu != 0) {
        for (i = VAL_COORDX; i <= VAL_COORDZ; i++) {
            taurus->var_start[i] = 64 * sizeof(int);

            taurus->var_ncomps[i] = 3;
            taurus->var_len[i] = taurus->numnp;
        }
        taurus->var_offset[VAL_COORDX] = 0;
        taurus->var_offset[VAL_COORDY] = 1;
        taurus->var_offset[VAL_COORDZ] = 2;
        loc += taurus->ndim * taurus->numnp * sizeof(float);
    }

    /*
     * Nodal velocities.
     */
    if (taurus->iv != 0) {
        for (i = VAL_VELX; i <= VAL_VELZ; i++) {
            taurus->var_start[i] = loc;
            taurus->var_ncomps[i] = 3;
            taurus->var_len[i] = taurus->numnp;
        }
        taurus->var_offset[VAL_VELX] = 0;
        taurus->var_offset[VAL_VELY] = 1;
        taurus->var_offset[VAL_VELZ] = 2;
        loc += taurus->ndim * taurus->numnp * sizeof(float);
    }

    /*
     * Nodal accelerations.
     */
    if (taurus->ia != 0) {
        for (i = VAL_ACCX; i <= VAL_ACCZ; i++) {
            taurus->var_start[i] = loc;
            taurus->var_ncomps[i] = 3;
            taurus->var_len[i] = taurus->numnp;
        }
        taurus->var_offset[VAL_ACCX] = 0;
        taurus->var_offset[VAL_ACCY] = 1;
        taurus->var_offset[VAL_ACCZ] = 2;
        loc += taurus->ndim * taurus->numnp * sizeof(float);
    }

    /*
     * Nodal temperatures.
     */
    if (taurus->it != 0) {
        for (i = VAL_TEMPX; i <= VAL_TEMPZ; i++) {
            taurus->var_start[i] = loc;
            taurus->var_ncomps[i] = 3;
            taurus->var_len[i] = taurus->numnp;
        }
        taurus->var_offset[VAL_TEMPX] = 0;
        taurus->var_offset[VAL_TEMPY] = 1;
        taurus->var_offset[VAL_TEMPZ] = 2;
        loc += taurus->ndim * taurus->numnp * sizeof(float);
    }

    /*
     * Brick data.
     */
    if (taurus->nel8 > 0 && taurus->nv3d == 7) {
        for (i = VAL_HEX_SIGX; i <= VAL_HEX_EPS_EFF; i++) {
            taurus->var_start[i] = loc;
            taurus->var_ncomps[i] = 7;
            taurus->var_len[i] = taurus->nel8;
        }
        taurus->var_offset[VAL_HEX_SIGX] = 0;
        taurus->var_offset[VAL_HEX_SIGY] = 1;
        taurus->var_offset[VAL_HEX_SIGZ] = 2;
        taurus->var_offset[VAL_HEX_SIGXY] = 3;
        taurus->var_offset[VAL_HEX_SIGYZ] = 4;
        taurus->var_offset[VAL_HEX_SIGZX] = 5;
        taurus->var_offset[VAL_HEX_EPS_EFF] = 6;
        loc += taurus->nv3d * taurus->nel8 * sizeof(float);
    }

    /*
     * Beam data.
     */
    if (taurus->nel2 > 0 && taurus->nv1d == 6) {
        loc += taurus->nv1d * taurus->nel2 * sizeof(float);
    }

    /*
     * Shell data.
     */
    if (taurus->nel4 > 0 && taurus->nv2d >= 33) {
        for (i = VAL_SHELL_MID_SIGX; i <= VAL_SHELL_INT_ENG; i++) {
            taurus->var_start[i] = loc;
            taurus->var_ncomps[i] = taurus->nv2d;
            taurus->var_len[i] = taurus->nel4;
        }
        taurus->var_offset[VAL_SHELL_MID_SIGX] = 0;
        taurus->var_offset[VAL_SHELL_MID_SIGY] = 1;
        taurus->var_offset[VAL_SHELL_MID_SIGZ] = 2;
        taurus->var_offset[VAL_SHELL_MID_SIGXY] = 3;
        taurus->var_offset[VAL_SHELL_MID_SIGYZ] = 4;
        taurus->var_offset[VAL_SHELL_MID_SIGZX] = 5;
        taurus->var_offset[VAL_SHELL_MID_EPS_EFF] = 6;  /* not used */
        taurus->var_offset[VAL_SHELL_IN_SIGX] = 7;
        taurus->var_offset[VAL_SHELL_IN_SIGY] = 8;
        taurus->var_offset[VAL_SHELL_IN_SIGZ] = 9;
        taurus->var_offset[VAL_SHELL_IN_SIGXY] = 10;
        taurus->var_offset[VAL_SHELL_IN_SIGYZ] = 11;
        taurus->var_offset[VAL_SHELL_IN_SIGZX] = 12;
        taurus->var_offset[VAL_SHELL_IN_EPS_EFF] = 13;
        taurus->var_offset[VAL_SHELL_OUT_SIGX] = 14;
        taurus->var_offset[VAL_SHELL_OUT_SIGY] = 15;
        taurus->var_offset[VAL_SHELL_OUT_SIGZ] = 16;
        taurus->var_offset[VAL_SHELL_OUT_SIGXY] = 17;
        taurus->var_offset[VAL_SHELL_OUT_SIGYZ] = 18;
        taurus->var_offset[VAL_SHELL_OUT_SIGZX] = 19;
        taurus->var_offset[VAL_SHELL_OUT_EPS_EFF] = 20;
        taurus->var_offset[VAL_SHELL_RES1] = 21;
        taurus->var_offset[VAL_SHELL_RES2] = 22;
        taurus->var_offset[VAL_SHELL_RES3] = 23;
        taurus->var_offset[VAL_SHELL_RES4] = 24;
        taurus->var_offset[VAL_SHELL_RES5] = 25;
        taurus->var_offset[VAL_SHELL_RES6] = 26;
        taurus->var_offset[VAL_SHELL_RES7] = 27;
        taurus->var_offset[VAL_SHELL_RES8] = 28;
        taurus->var_offset[VAL_SHELL_THICKNESS] = 29;
        taurus->var_offset[VAL_SHELL_ELDEP1] = 30;  /* not used */
        taurus->var_offset[VAL_SHELL_ELDEP2] = 31;  /* not used */
        taurus->var_offset[VAL_SHELL_INT_ENG] = 32;
        /*
         * The variables in the following if block are not used.
         */
        if (taurus->nv2d == 45 || taurus->nv2d == 46) {
            for (i = VAL_SHELL_EPSX_IN; i <= VAL_SHELL_EPSZX_OUT; i++) {
                taurus->var_start[i] = loc;
                taurus->var_ncomps[i] = taurus->nv2d;
                taurus->var_len[i] = taurus->nel4;
            }
            taurus->var_offset[VAL_SHELL_EPSX_IN] = 33;
            taurus->var_offset[VAL_SHELL_EPSY_IN] = 34;
            taurus->var_offset[VAL_SHELL_EPSZ_IN] = 35;
            taurus->var_offset[VAL_SHELL_EPSXY_IN] = 36;
            taurus->var_offset[VAL_SHELL_EPSYZ_IN] = 37;
            taurus->var_offset[VAL_SHELL_EPSZX_IN] = 38;
            taurus->var_offset[VAL_SHELL_EPSX_OUT] = 39;
            taurus->var_offset[VAL_SHELL_EPSY_OUT] = 40;
            taurus->var_offset[VAL_SHELL_EPSZ_OUT] = 41;
            taurus->var_offset[VAL_SHELL_EPSXY_OUT] = 42;
            taurus->var_offset[VAL_SHELL_EPSYZ_OUT] = 43;
            taurus->var_offset[VAL_SHELL_EPSZX_OUT] = 44;
        }
        loc += taurus->nv2d * taurus->nel4 * sizeof(float);
    }
}

/*-------------------------------------------------------------------------
 * Function:    init_mat_info
 *
 * Purpose:
 *
 * Return:      Success:        void
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Mon Aug 28 13:37:09 PDT 1995
 *    I modified the routine to find the actual materials referenced
 *    rather than the maximum material number and assume that they were
 *    all used.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static void
init_mat_info (TAURUSfile *taurus)
{
    int            i;
    int            iadd, ibuf, imat;
    int            len;
    int            lbuf;
    int           *buf, *buf2;
    int            maxmat;
    int            nmat;
    int           *matnos;

    maxmat = 0;

    /*
     * Set up for reading the nodelists and material data.
     */
    iadd = 64 * sizeof(int) + taurus->numnp * taurus->ndim * sizeof(float);

    ibuf = 0;
    lbuf = (9 * taurus->nel8) + (5 * taurus->nel4) + (6 * taurus->nel2);
    buf = ALLOC_N(int, lbuf);

    /*
     * Read the hexahedron elements.
     */
    if (taurus->nel8 > 0) {
        len = 9 * taurus->nel8 * sizeof(int);

        taurus_read(taurus, 0, iadd, len, (char*)buf);

        for (i = 0; i < taurus->nel8; i++)
            if (buf[i * 9 + 8] > maxmat)
                maxmat = buf[i * 9 + 8];

        iadd += len;
        ibuf += 9 * taurus->nel8;
    }

    /*
     * Read the beam elements.
     */
    if (taurus->nel2 > 0) {
        len = 6 * taurus->nel2 * sizeof(int);

        taurus_read(taurus, 0, iadd, len, (char*)(&buf[ibuf]));

        for (i = 0; i < taurus->nel2; i++)
            if (buf[ibuf + i * 6 + 5] > maxmat)
                maxmat = buf[ibuf + i * 6 + 5];

        iadd += len;
        ibuf += 6 * taurus->nel2;
    }

    /*
     * Read the shell elements.
     */
    if (taurus->nel4 > 0) {
        len = 5 * taurus->nel4 * sizeof(int);

        taurus_read(taurus, 0, iadd, len, (char*)(&buf[ibuf]));

        for (i = 0; i < taurus->nel4; i++)
            if (buf[ibuf + i * 5 + 4] > maxmat)
                maxmat = buf[ibuf + i * 5 + 4];
    }

    /*
     * Find all the materials referenced.
     */
    buf2 = ALLOC_N(int, maxmat);

    for (i = 0; i < maxmat; i++)
        buf2[i] = 0;

    ibuf = 0;
    if (taurus->nel8 > 0) {
        for (i = 0; i < taurus->nel8; i++)
            buf2[buf[i * 9 + 8] - 1] = 1;
        ibuf += taurus->nel8 * 9;
    }
    if (taurus->nel2 > 0) {
        for (i = 0; i < taurus->nel2; i++)
            buf2[buf[ibuf + i * 6 + 5] - 1] = 1;
        ibuf += taurus->nel2 * 6;
    }
    if (taurus->nel4 > 0) {
        for (i = 0; i < taurus->nel4; i++)
            buf2[buf[ibuf + i * 5 + 4] - 1] = 1;
    }

    /*
     * Find nmat, and set the matnos array.
     */
    nmat = 0;
    for (i = 0; i < maxmat; i++)
        nmat += buf2[i];
    matnos = ALLOC_N(int, nmat);

    imat = 0;
    for (i = 0; i < maxmat; i++)
        if (buf2[i] == 1)
            matnos[imat++] = i + 1;

    FREE(buf);
    FREE(buf2);

    taurus->nmat = nmat;
    taurus->matnos = matnos;
}

/*-------------------------------------------------------------------------
 * Function:    taurus_calc
 *
 * Purpose:
 *
 * Return:      Success:        void
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Thu Apr 27 08:47:01 PDT 1995
 *    I modified the code to work properly with state directories.
 *
 *    Eric Brugger, Fri Jul 28 08:41:03 PDT 1995
 *    I added a calculation for the vorticity magnitude.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static void
taurus_calc (TAURUSfile *taurus, float *buf, int lbuf, int ncomps, int offset,
             int var_id, float *var, int ivar)
{
    int            ibuf;
    double         t1, t2, t3, t4, t5;
    double         pr, aa, bb, cc, dd, angp;
    float         *buf2, *buf3, *buf4;

    switch (var_id) {
        case VAR_NORMAL:
        case VAR_SIGX:
        case VAR_SIGY:
        case VAR_SIGZ:
        case VAR_SIGXY:
        case VAR_SIGYZ:
        case VAR_SIGZX:
        case VAR_EPS:
            for (ibuf = offset; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = buf[ibuf];
                ivar++;
            }
            break;
        case VAR_PRESSURE:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = -(buf[ibuf] + buf[ibuf + 1] + buf[ibuf + 2]) / 3.0;
                ivar++;
            }
            break;
        case VAR_SIG_EFF:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                pr = -(buf[ibuf] + buf[ibuf + 1] + buf[ibuf + 2]) / 3.0;
                buf[ibuf] += pr;
                buf[ibuf + 1] += pr;
                buf[ibuf + 2] += pr;
                aa = buf[ibuf + 3] * buf[ibuf + 3] + buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 5] * buf[ibuf + 5] - buf[ibuf] * buf[ibuf + 1] -
                    buf[ibuf + 1] * buf[ibuf + 2] - buf[ibuf] * buf[ibuf + 2];
                var[ivar] = sqrt(3.0 * fabs(aa));
                ivar++;
            }
            break;
        case VAR_DEV_STRESS_1:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                pr = -(buf[ibuf] + buf[ibuf + 1] + buf[ibuf + 2]) / 3.0;
                buf[ibuf] += pr;
                buf[ibuf + 1] += pr;
                buf[ibuf + 2] += pr;
                aa = buf[ibuf + 3] * buf[ibuf + 3] + buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 5] * buf[ibuf + 5] - buf[ibuf] * buf[ibuf + 1] -
                    buf[ibuf + 1] * buf[ibuf + 2] - buf[ibuf] * buf[ibuf + 2];
                bb = buf[ibuf] * buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 1] * buf[ibuf + 5] * buf[ibuf + 5] +
                    buf[ibuf + 2] * buf[ibuf + 3] * buf[ibuf + 3] -
                    buf[ibuf] * buf[ibuf + 1] * buf[ibuf + 2] - 2.0 *
                    buf[ibuf + 3] * buf[ibuf + 4] * buf[ibuf + 5];
                buf[ibuf] = aa;
                buf[ibuf + 1] = bb;
            }
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                if (buf[ibuf] < 1.0e-25)
                    var[ivar] = 0.;
                else {
                    aa = buf[ibuf];
                    bb = buf[ibuf + 1];
                    cc = -sqrt(27.0 / aa) * bb * 0.5 / aa;
                    cc = MAX(MIN(cc, 1.0), -1.0);
                    angp = acos(cc) / 3.0;
                    dd = 2.0 * sqrt(aa / 3.0);
                    var[ivar] = dd * cos(angp);
                }
                ivar++;
            }
            break;
        case VAR_DEV_STRESS_2:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                pr = -(buf[ibuf] + buf[ibuf + 1] + buf[ibuf + 2]) / 3.0;
                buf[ibuf] += pr;
                buf[ibuf + 1] += pr;
                buf[ibuf + 2] += pr;
                aa = buf[ibuf + 3] * buf[ibuf + 3] + buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 5] * buf[ibuf + 5] - buf[ibuf] * buf[ibuf + 1] -
                    buf[ibuf + 1] * buf[ibuf + 2] - buf[ibuf] * buf[ibuf + 2];
                bb = buf[ibuf] * buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 1] * buf[ibuf + 5] * buf[ibuf + 5] +
                    buf[ibuf + 2] * buf[ibuf + 3] * buf[ibuf + 3] -
                    buf[ibuf] * buf[ibuf + 1] * buf[ibuf + 2] - 2.0 *
                    buf[ibuf + 3] * buf[ibuf + 4] * buf[ibuf + 5];
                buf[ibuf] = aa;
                buf[ibuf + 1] = bb;
            }
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                if (buf[ibuf] < 1.0e-25)
                    var[ivar] = 0.;
                else {
                    aa = buf[ibuf];
                    bb = buf[ibuf + 1];
                    cc = -sqrt(27.0 / aa) * bb * 0.5 / aa;
                    cc = MAX(MIN(cc, 1.0), -1.0);
                    angp = acos(cc) / 3.0;
                    dd = 2.0 * sqrt(aa / 3.0);
                    var[ivar] = dd * cos(angp + ftpi);
                }
                ivar++;
            }
            break;
        case VAR_DEV_STRESS_3:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                pr = -(buf[ibuf] + buf[ibuf + 1] + buf[ibuf + 2]) / 3.0;
                buf[ibuf] += pr;
                buf[ibuf + 1] += pr;
                buf[ibuf + 2] += pr;
                aa = buf[ibuf + 3] * buf[ibuf + 3] + buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 5] * buf[ibuf + 5] - buf[ibuf] * buf[ibuf + 1] -
                    buf[ibuf + 1] * buf[ibuf + 2] - buf[ibuf] * buf[ibuf + 2];
                bb = buf[ibuf] * buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 1] * buf[ibuf + 5] * buf[ibuf + 5] +
                    buf[ibuf + 2] * buf[ibuf + 3] * buf[ibuf + 3] -
                    buf[ibuf] * buf[ibuf + 1] * buf[ibuf + 2] - 2.0 *
                    buf[ibuf + 3] * buf[ibuf + 4] * buf[ibuf + 5];
                buf[ibuf] = aa;
                buf[ibuf + 1] = bb;
            }
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                if (buf[ibuf] < 1.0e-25)
                    var[ivar] = 0.;
                else {
                    aa = buf[ibuf];
                    bb = buf[ibuf + 1];
                    cc = -sqrt(27.0 / aa) * bb * 0.5 / aa;
                    cc = MAX(MIN(cc, 1.0), -1.0);
                    angp = acos(cc) / 3.0;
                    dd = 2.0 * sqrt(aa / 3.0);
                    var[ivar] = dd * cos(angp + ttpi);
                }
                ivar++;
            }
            break;
        case VAR_MAX_SHEAR_STR:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                pr = -(buf[ibuf] + buf[ibuf + 1] + buf[ibuf + 2]) / 3.0;
                buf[ibuf] += pr;
                buf[ibuf + 1] += pr;
                buf[ibuf + 2] += pr;
                aa = buf[ibuf + 3] * buf[ibuf + 3] + buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 5] * buf[ibuf + 5] - buf[ibuf] * buf[ibuf + 1] -
                    buf[ibuf + 1] * buf[ibuf + 2] - buf[ibuf] * buf[ibuf + 2];
                bb = buf[ibuf] * buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 1] * buf[ibuf + 5] * buf[ibuf + 5] +
                    buf[ibuf + 2] * buf[ibuf + 3] * buf[ibuf + 3] -
                    buf[ibuf] * buf[ibuf + 1] * buf[ibuf + 2] - 2.0 *
                    buf[ibuf + 3] * buf[ibuf + 4] * buf[ibuf + 5];
                buf[ibuf] = aa;
                buf[ibuf + 1] = bb;
            }
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                if (buf[ibuf] < 1.0e-25)
                    var[ivar] = 0.;
                else {
                    aa = buf[ibuf];
                    bb = buf[ibuf + 1];
                    cc = -sqrt(27.0 / aa) * bb * 0.5 / aa;
                    cc = MAX(MIN(cc, 1.0), -1.0);
                    angp = acos(cc) / 3.0;
                    dd = sqrt(aa / 3.0);
                    var[ivar] = dd * (cos(angp) - cos(angp + ttpi));
                }
                ivar++;
            }
            break;
        case VAR_PRINC_STRESS_1:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                pr = -(buf[ibuf] + buf[ibuf + 1] + buf[ibuf + 2]) / 3.0;
                buf[ibuf] += pr;
                buf[ibuf + 1] += pr;
                buf[ibuf + 2] += pr;
                aa = buf[ibuf + 3] * buf[ibuf + 3] + buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 5] * buf[ibuf + 5] - buf[ibuf] * buf[ibuf + 1] -
                    buf[ibuf + 1] * buf[ibuf + 2] - buf[ibuf] * buf[ibuf + 2];
                bb = buf[ibuf] * buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 1] * buf[ibuf + 5] * buf[ibuf + 5] +
                    buf[ibuf + 2] * buf[ibuf + 3] * buf[ibuf + 3] -
                    buf[ibuf] * buf[ibuf + 1] * buf[ibuf + 2] - 2.0 *
                    buf[ibuf + 3] * buf[ibuf + 4] * buf[ibuf + 5];
                buf[ibuf] = aa;
                buf[ibuf + 1] = bb;
                buf[ibuf + 2] = pr;
            }
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                if (buf[ibuf] < 1.0e-25)
                    var[ivar] = -buf[ibuf + 2];
                else {
                    aa = buf[ibuf];
                    bb = buf[ibuf + 1];
                    cc = -sqrt(27.0 / aa) * bb * 0.5 / aa;
                    cc = MAX(MIN(cc, 1.0), -1.0);
                    angp = acos(cc) / 3.0;
                    dd = 2.0 * sqrt(aa / 3.0);
                    var[ivar] = dd * cos(angp) - buf[ibuf + 2];
                }
                ivar++;
            }
            break;
        case VAR_PRINC_STRESS_2:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                pr = -(buf[ibuf] + buf[ibuf + 1] + buf[ibuf + 2]) / 3.0;
                buf[ibuf] += pr;
                buf[ibuf + 1] += pr;
                buf[ibuf + 2] += pr;
                aa = buf[ibuf + 3] * buf[ibuf + 3] + buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 5] * buf[ibuf + 5] - buf[ibuf] * buf[ibuf + 1] -
                    buf[ibuf + 1] * buf[ibuf + 2] - buf[ibuf] * buf[ibuf + 2];
                bb = buf[ibuf] * buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 1] * buf[ibuf + 5] * buf[ibuf + 5] +
                    buf[ibuf + 2] * buf[ibuf + 3] * buf[ibuf + 3] -
                    buf[ibuf] * buf[ibuf + 1] * buf[ibuf + 2] - 2.0 *
                    buf[ibuf + 3] * buf[ibuf + 4] * buf[ibuf + 5];
                buf[ibuf] = aa;
                buf[ibuf + 1] = bb;
                buf[ibuf + 2] = pr;
            }
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                if (buf[ibuf] < 1.0e-25)
                    var[ivar] = -buf[ibuf + 2];
                else {
                    aa = buf[ibuf];
                    bb = buf[ibuf + 1];
                    cc = -sqrt(27.0 / aa) * bb * 0.5 / aa;
                    cc = MAX(MIN(cc, 1.0), -1.0);
                    angp = acos(cc) / 3.0;
                    dd = 2.0 * sqrt(aa / 3.0);
                    var[ivar] = dd * cos(angp + ftpi) - buf[ibuf + 2];
                }
                ivar++;
            }
            break;
        case VAR_PRINC_STRESS_3:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                pr = -(buf[ibuf] + buf[ibuf + 1] + buf[ibuf + 2]) / 3.0;
                buf[ibuf] += pr;
                buf[ibuf + 1] += pr;
                buf[ibuf + 2] += pr;
                aa = buf[ibuf + 3] * buf[ibuf + 3] + buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 5] * buf[ibuf + 5] - buf[ibuf] * buf[ibuf + 1] -
                    buf[ibuf + 1] * buf[ibuf + 2] - buf[ibuf] * buf[ibuf + 2];
                bb = buf[ibuf] * buf[ibuf + 4] * buf[ibuf + 4] +
                    buf[ibuf + 1] * buf[ibuf + 5] * buf[ibuf + 5] +
                    buf[ibuf + 2] * buf[ibuf + 3] * buf[ibuf + 3] -
                    buf[ibuf] * buf[ibuf + 1] * buf[ibuf + 2] - 2.0 *
                    buf[ibuf + 3] * buf[ibuf + 4] * buf[ibuf + 5];
                buf[ibuf] = aa;
                buf[ibuf + 1] = bb;
                buf[ibuf + 2] = pr;
            }
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                if (buf[ibuf] < 1.0e-25)
                    var[ivar] = -buf[ibuf + 2];
                else {
                    aa = buf[ibuf];
                    bb = buf[ibuf + 1];
                    cc = -sqrt(27.0 / aa) * bb * 0.5 / aa;
                    cc = MAX(MIN(cc, 1.0), -1.0);
                    angp = acos(cc) / 3.0;
                    dd = 2.0 * sqrt(aa / 3.0);
                    var[ivar] = dd * cos(angp + ttpi) - buf[ibuf + 2];
                }
                ivar++;
            }
            break;
        case VAR_DISPX:
            buf2 = taurus->coords[0];
            for (ibuf = offset; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = buf2[ivar] - buf[ibuf];
                ivar++;
            }
            break;
        case VAR_DISPY:
            buf2 = taurus->coords[1];
            for (ibuf = offset; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = buf2[ivar] - buf[ibuf];
                ivar++;
            }
            break;
        case VAR_DISPZ:
            buf2 = taurus->coords[2];
            for (ibuf = offset; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = buf2[ivar] - buf[ibuf];
                ivar++;
            }
            break;
        case VAR_DISP_MAG:
            buf2 = taurus->coords[0];
            buf3 = taurus->coords[1];
            buf4 = taurus->coords[2];
            for (ibuf = offset; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = sqrt((buf2[ivar] - buf[ibuf]) *
                                 (buf2[ivar] - buf[ibuf]) +
                                 (buf3[ivar] - buf[ibuf + 1]) *
                                 (buf3[ivar] - buf[ibuf + 1]) +
                                 (buf4[ivar] - buf[ibuf + 2]) *
                                 (buf4[ivar] - buf[ibuf + 2]));
                ivar++;
            }
            break;
        case VAR_VEL_MAG:
        case VAR_ACC_MAG:
        case VAR_VORT_MAG:
            for (ibuf = offset; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = sqrt(buf[ibuf] * buf[ibuf] +
                                 buf[ibuf + 1] * buf[ibuf + 1] +
                                 buf[ibuf + 2] * buf[ibuf + 2]);
                ivar++;
            }
            break;
        case VAR_SURF_STRESS_1:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = (buf[ibuf + 26] / buf[ibuf + 29]) + 6.0 *
                    (buf[ibuf + 21] / (buf[ibuf + 29] * buf[ibuf + 29]));
                ivar++;
            }
            break;
        case VAR_SURF_STRESS_2:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = (buf[ibuf + 26] / buf[ibuf + 29]) - 6.0 *
                    (buf[ibuf + 21] / (buf[ibuf + 29] * buf[ibuf + 29]));
                ivar++;
            }
            break;
        case VAR_SURF_STRESS_3:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = (buf[ibuf + 27] / buf[ibuf + 29]) + 6.0 *
                    (buf[ibuf + 22] / (buf[ibuf + 29] * buf[ibuf + 29]));
                ivar++;
            }
            break;
        case VAR_SURF_STRESS_4:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = (buf[ibuf + 27] / buf[ibuf + 29]) - 6.0 *
                    (buf[ibuf + 22] / (buf[ibuf + 29] * buf[ibuf + 29]));
                ivar++;
            }
            break;
        case VAR_SURF_STRESS_5:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = (buf[ibuf + 28] / buf[ibuf + 29]) + 6.0 *
                    (buf[ibuf + 23] / (buf[ibuf + 29] * buf[ibuf + 29]));
                ivar++;
            }
            break;
        case VAR_SURF_STRESS_6:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                var[ivar] = (buf[ibuf + 28] / buf[ibuf + 29]) - 6.0 *
                    (buf[ibuf + 23] / (buf[ibuf + 29] * buf[ibuf + 29]));
                ivar++;
            }
            break;
        case VAR_UP_STRESS:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                t1 = (buf[ibuf + 26] / buf[ibuf + 29]) + 6.0 *
                    (buf[ibuf + 21] / (buf[ibuf + 29] * buf[ibuf + 29]));
                t2 = (buf[ibuf + 27] / buf[ibuf + 29]) + 6.0 *
                    (buf[ibuf + 22] / (buf[ibuf + 29] * buf[ibuf + 29]));
                t3 = (buf[ibuf + 28] / buf[ibuf + 29]) + 6.0 *
                    (buf[ibuf + 23] / (buf[ibuf + 29] * buf[ibuf + 29]));
                var[ivar] = sqrt(t1 * t1 - t1 * t2 + t2 * t2 + 3.0 * t3 * t3);
                ivar++;
            }
            break;
        case VAR_LOW_STRESS:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                t1 = (buf[ibuf + 26] / buf[ibuf + 29]) - 6.0 *
                    (buf[ibuf + 21] / (buf[ibuf + 29] * buf[ibuf + 29]));
                t2 = (buf[ibuf + 27] / buf[ibuf + 29]) - 6.0 *
                    (buf[ibuf + 22] / (buf[ibuf + 29] * buf[ibuf + 29]));
                t3 = (buf[ibuf + 28] / buf[ibuf + 29]) - 6.0 *
                    (buf[ibuf + 23] / (buf[ibuf + 29] * buf[ibuf + 29]));
                var[ivar] = sqrt(t1 * t1 - t1 * t2 + t2 * t2 + 3.0 * t3 * t3);
                ivar++;
            }
            break;
        case VAR_MAX_STRESS:
            for (ibuf = 0; ibuf < lbuf; ibuf += ncomps) {
                t1 = (buf[ibuf + 26] / buf[ibuf + 29]) + 6.0 *
                    (buf[ibuf + 21] / (buf[ibuf + 29] * buf[ibuf + 29]));
                t2 = (buf[ibuf + 27] / buf[ibuf + 29]) + 6.0 *
                    (buf[ibuf + 22] / (buf[ibuf + 29] * buf[ibuf + 29]));
                t3 = (buf[ibuf + 28] / buf[ibuf + 29]) + 6.0 *
                    (buf[ibuf + 23] / (buf[ibuf + 29] * buf[ibuf + 29]));
                t4 = sqrt(t1 * t1 - t1 * t2 + t2 * t2 + 3.0 * t3 * t3);
                t1 = (buf[ibuf + 26] / buf[ibuf + 29]) - 6.0 *
                    (buf[ibuf + 21] / (buf[ibuf + 29] * buf[ibuf + 29]));
                t2 = (buf[ibuf + 27] / buf[ibuf + 29]) - 6.0 *
                    (buf[ibuf + 22] / (buf[ibuf + 29] * buf[ibuf + 29]));
                t3 = (buf[ibuf + 28] / buf[ibuf + 29]) - 6.0 *
                    (buf[ibuf + 23] / (buf[ibuf + 29] * buf[ibuf + 29]));
                t5 = sqrt(t1 * t1 - t1 * t2 + t2 * t2 + 3.0 * t3 * t3);
                var[ivar] = MAX(t4, t5);
                ivar++;
            }
            break;
    }
}

/*-------------------------------------------------------------------------
 * Function:    taurus_readblockvar
 *
 * Purpose:
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *     Eric Brugger, Fri Apr 28 09:13:43 PDT 1995
 *     I removed some debugging code that I accidently left in previously.
 *
 *     Jim Reus, 23 Apr 97
 *     Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static int
taurus_readblockvar (TAURUSfile *taurus, int var_id, int val_id, float *var)
{
    int            size;
    int            lbuf;
    float         *buf;
    int            ivar;
    int            n;
    int            ifile, iadd, nel, offset, ncomps;

    /*
     * When reading displacements we are reading the coordinates in
     * the first file.  This isn't very pretty but it fits the model.
     */
    if (var_id >= VAR_DISPX && var_id <= VAR_DISP_MAG) {
        ifile = 0;
        iadd = taurus->var_start[val_id];
    }
    else {
        ifile = taurus->state_file[taurus->state];
        iadd = taurus->state_loc[taurus->state] +
            taurus->var_start[val_id];
    }

    nel = taurus->var_len[val_id];
    offset = taurus->var_offset[val_id];
    ncomps = taurus->var_ncomps[val_id];

    /*
     * Allocate space for the buffer.
     */
    size = nel * ncomps;

    if (size < MAXBUF)
        lbuf = size;
    else
        lbuf = (MAXBUF / ncomps) * ncomps;

    buf = ALLOC_N(float, lbuf);

    /*
     * Read a buffer full of data at a time, and transfer it to
     * the real variable using the appropriate stride.
     */
    ivar = 0;
    while (size > 0) {
        n = MIN(size, lbuf);
        taurus_read(taurus, ifile, iadd, n * sizeof(int), (char*)buf);

        taurus_calc(taurus, buf, n, ncomps, offset, var_id, var, ivar);
        ivar += n / ncomps;
        iadd += n * sizeof(int);

        size -= n;
    }

    FREE(buf);

    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_open
 *
 * Purpose:     Open a taurus file.
 *
 * Return:      Success:        pointer to new file.
 *
 *              Failure:        NULL
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Tue Mar 28 15:03:37 PST 1995
 *    I modified the routines to read a topaz3d data file.
 *
 *    Eric Brugger, Wed Apr 26 13:14:42 PDT 1995
 *    I modified the routine to read the title in the taurus file
 *    and put it in the taurus structure.
 *
 *    Eric Brugger, Thu Jul 27 12:49:40 PDT 1995
 *    I modified the routine to handle files generated by hydra.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
TAURUSfile *
db_taur_open (char *basename)
{
    int            fd;
    int            loc, size;
    int            ctl[40];
    char           title[48];
    TAURUSfile    *taurus;

    /*
     * Create the structure to hold the open file information.
     */
    taurus = ALLOC_N(TAURUSfile, 1);

    /*
     * Open the file and read the header.
     */
    if ((fd = open(basename, O_RDONLY)) < 0) {
        FREE(taurus);
        return (NULL);
    }

    taurus->ifile = 0;
    taurus->fd = fd;
    taurus->basename = ALLOC_N(char, strlen(basename) + 1);
    strcpy(taurus->basename, basename);
    taurus->filename = ALLOC_N(char, strlen(basename) + 4);

    taurus->mesh_read = 0;

    loc = 15 * sizeof(int);

    lseek(fd, loc, SEEK_SET);
    size = 40 * sizeof(int);

    if (read(fd, ctl, size) != size) {
        FREE(taurus->basename);
        FREE(taurus->filename);
        FREE(taurus);
        close(fd);
        return (NULL);
    }

    /*
     * Do a simple check to see that this is indeed a taurus file.
     * ctl [0] should really be a 4, which indicates a 3d mesh with
     * an unpacked node list.  3 indicates a 3d mesh with a packed
     * node list, but this can still have an unpacked node list, so
     * we will assume that it is unpacked regardless of the flag.
     */
    if (!((ctl[0] == 3 || ctl[0] == 4) &&
          (ctl[2] == 1 || ctl[2] == 2 || ctl[2] == 6 || ctl[2] == 200))) {
        FREE(taurus->basename);
        FREE(taurus->filename);
        FREE(taurus);
        close(fd);
        return (NULL);
    }

    taurus->ndim = ctl[0];
    taurus->numnp = ctl[1];
    taurus->icode = ctl[2];
    taurus->nglbv = ctl[3];
    taurus->it = ctl[4];
    taurus->iu = ctl[5];
    taurus->iv = ctl[6];
    taurus->ia = ctl[7];
    taurus->nel8 = ctl[8];
    taurus->nummat8 = ctl[9];
    taurus->nv3d = ctl[12];
    taurus->nel2 = ctl[13];
    taurus->nummat2 = ctl[14];
    taurus->nv1d = ctl[15];
    taurus->nel4 = ctl[16];
    taurus->nummat4 = ctl[17];
    taurus->nv2d = ctl[18];
    taurus->activ = ctl[20];

    /*
     * if ndim is 4 then the nodelist are unpacked.  We cannot handle
     * packed values so let us assume no files exist that are packed.
     * If the values were packed they would presumably be three values
     * per integer, which means a very small number of nodes.
     */
    if (taurus->ndim == 4)
        taurus->ndim = 3;

    /*
     * Read the title.
     */
    loc = 0;
    lseek(fd, loc, SEEK_SET);
    size = 40 * sizeof(char);

    if (read(fd, title, size) != size) {
        FREE(taurus->basename);
        FREE(taurus->filename);
        FREE(taurus);
        close(fd);
        return (NULL);
    }
    title[40] = '\0';
    fix_title(title);
    strcpy(taurus->title, title);

    /*
     * Initialize the file information.
     */
    init_file_info(taurus);

    /*
     * Initialize the state information.
     */
    init_state_info(taurus);

    /*
     * Initialize the variable information.
     */
    init_var_info(taurus);

    /*
     * Initialize the material information.
     */
    init_mat_info(taurus);

    return (taurus);
}

/*-------------------------------------------------------------------------
 * Function:    db_taur_close
 *
 * Purpose:     Close a taurus file pointer and free the memory that
 *              belongs to the taurus driver.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Fri Dec  9 12:56:43 EST 1994
 *
 * Modifications:
 *    Eric Brugger, Wed Dec 20 12:04:03 PST 1995
 *    I modified the code to handle the activity data.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
int
db_taur_close (TAURUSfile *taurus)
{

    close(taurus->fd);
    FREE(taurus->basename);
    FREE(taurus->filename);
    FREE(taurus->filesize);

    FREE(taurus->state_file);
    FREE(taurus->state_loc);
    FREE(taurus->state_time);

    FREE(taurus->hex_nodelist);
    FREE(taurus->shell_nodelist);
    FREE(taurus->beam_nodelist);
    FREE(taurus->hex_facelist);
    FREE(taurus->hex_zoneno);
    FREE(taurus->hex_matlist);
    FREE(taurus->shell_matlist);
    FREE(taurus->beam_matlist);
    FREE(taurus->hex_activ);
    FREE(taurus->shell_activ);
    FREE(taurus->beam_activ);
    if (taurus->coords != NULL) {
        FREE(taurus->coords[0]);
        FREE(taurus->coords[1]);
        if (taurus->ndim > 2)
            FREE(taurus->coords[2]);
    }
    FREE(taurus->coords);
    FREE(taurus);
    return (0);
}

/*-------------------------------------------------------------------------
 * Function:    init_mesh_info
 *
 * Purpose:
 *
 * Return:      Success:        void
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Mon Aug 28 12:00:10 PDT 1995
 *    I modified the routine to not use unreferenced nodes in the
 *    mesh extent calculations.
 *
 *    Eric Brugger, Wed Dec 20 12:04:03 PST 1995
 *    I modified the code to handle the activity data.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
void
init_mesh_info (TAURUSfile *taurus)
{
    int            i;
    int            iadd;
    int            len;
    int            lbuf;
    int           *buf;
    float         *rbuf;
    int           *zones, *faces, *mats;
    int           *zoneno;
    int            nzones, nfaces;
    float          minval, maxval;
    int            ndim, numnp;
    float         *coords;
    float          xval, yval, zval;

    if (taurus->mesh_read == 1)
        return;

    taurus->nhex = 0;
    taurus->nshell = 0;
    taurus->nbeam = 0;
    taurus->coords = NULL;
    taurus->coord_state = -1;

    /*
     * Read the coordinate information.
     */
    ndim = taurus->ndim;
    numnp = taurus->numnp;

    /*
     * Allocate storage if not already allocated.
     */
    if (taurus->coords == NULL) {
        taurus->coords = ALLOC_N(float *, ndim);
        taurus->coords[0] = ALLOC_N(float, numnp);
        taurus->coords[1] = ALLOC_N(float, numnp);

        if (ndim > 2)
            taurus->coords[2] = ALLOC_N(float, numnp);
    }

    iadd = 64 * sizeof(int);

    lbuf = numnp * ndim;
    rbuf = ALLOC_N(float, lbuf);
    len = lbuf * sizeof(float);

    taurus_read(taurus, 0, iadd, len, (char*)rbuf);

    coords = taurus->coords[0];
    for (i = 0; i < numnp; i++)
        coords[i] = rbuf[i * ndim];

    coords = taurus->coords[1];
    for (i = 0; i < numnp; i++)
        coords[i] = rbuf[i * ndim + 1];

    if (ndim > 2) {
        coords = taurus->coords[2];
        for (i = 0; i < numnp; i++)
            coords[i] = rbuf[i * ndim + 2];
    }

    FREE(rbuf);

    /*
     * Set up for reading the nodelists and material data.
     */
    iadd = 64 * sizeof(int) + taurus->numnp * taurus->ndim * sizeof(float);

    lbuf = MAX(9 * taurus->nel8, MAX(5 * taurus->nel4, 6 * taurus->nel2));
    buf = ALLOC_N(int, lbuf);

    /*
     * Read the hexahedron elements.
     */
    if (taurus->nel8 > 0) {
        zones = ALLOC_N(int, 8 * taurus->nel8);
        mats = ALLOC_N(int, taurus->nel8);

        len = 9 * taurus->nel8 * sizeof(int);

        taurus_read(taurus, 0, iadd, len, (char*)buf);
        for (i = 0; i < taurus->nel8; i++) {
            zones[i * 8] = buf[i * 9] - 1;
            zones[i * 8 + 1] = buf[i * 9 + 1] - 1;
            zones[i * 8 + 2] = buf[i * 9 + 2] - 1;
            zones[i * 8 + 3] = buf[i * 9 + 3] - 1;
            zones[i * 8 + 4] = buf[i * 9 + 4] - 1;
            zones[i * 8 + 5] = buf[i * 9 + 5] - 1;
            zones[i * 8 + 6] = buf[i * 9 + 6] - 1;
            zones[i * 8 + 7] = buf[i * 9 + 7] - 1;
        }
        for (i = 0; i < taurus->nel8; i++) {
            mats[i] = buf[i * 9 + 8];
        }

        taurus->nhex = taurus->nel8;
        taurus->hex_nodelist = zones;
        taurus->hex_matlist = mats;
        taurus->hex_activ = NULL;

        if (taurus->activ >= 1000 || taurus->activ <= 1005) {
            taurus->nhex_faces = 0;
            taurus->hex_facelist = NULL;
            taurus->hex_zoneno = NULL;
        }
        else {
            db_taur_extface(zones, taurus->numnp, taurus->nel8,
                            mats, &faces, &nfaces, &zoneno);

            taurus->nhex_faces = nfaces;
            taurus->hex_facelist = faces;
            taurus->hex_zoneno = zoneno;
        }

        iadd += len;
    }

    /*
     * Read the beam elements.
     */
    if (taurus->nel2 > 0) {
        zones = ALLOC_N(int, 2 * taurus->nel2);
        mats = ALLOC_N(int, taurus->nel2);

        len = 6 * taurus->nel2 * sizeof(int);

        taurus_read(taurus, 0, iadd, len, (char*)buf);
        for (i = 0; i < taurus->nel2; i++) {
            zones[i * 2] = buf[i * 6] - 1;
            zones[i * 2 + 1] = buf[i * 6 + 1] - 1;
        }
        for (i = 0; i < taurus->nel2; i++) {
            mats[i] = buf[i * 6 + 5];
        }

        taurus->nbeam = taurus->nel2;
        taurus->beam_nodelist = zones;
        taurus->beam_matlist = mats;
        taurus->beam_activ = NULL;

        iadd += len;
    }

    /*
     * Read the shell elements.
     */
    if (taurus->nel4 > 0) {
        zones = ALLOC_N(int, 4 * taurus->nel4);
        mats = ALLOC_N(int, taurus->nel4);

        len = 5 * taurus->nel4 * sizeof(int);

        taurus_read(taurus, 0, iadd, len, (char*)buf);
        for (i = 0; i < taurus->nel4; i++) {
            zones[i * 4] = buf[i * 5] - 1;
            zones[i * 4 + 1] = buf[i * 5 + 1] - 1;
            zones[i * 4 + 2] = buf[i * 5 + 2] - 1;
            zones[i * 4 + 3] = buf[i * 5 + 3] - 1;
        }
        for (i = 0; i < taurus->nel4; i++) {
            mats[i] = buf[i * 5 + 4];
        }

        taurus->nshell = taurus->nel4;
        taurus->shell_nodelist = zones;
        taurus->shell_matlist = mats;
        taurus->shell_activ = NULL;

        iadd += len;
    }

    /*
     * Find the unreferenced coordinates.
     */
    for (i = 0; i < numnp; i++)
        buf[i] = 0;

    zones = taurus->hex_nodelist;
    nzones = taurus->nel8;
    for (i = 0; i < nzones * 8; i++)
        buf[zones[i]] = 1;

    zones = taurus->beam_nodelist;
    nzones = taurus->nel2;
    for (i = 0; i < nzones * 2; i++)
        buf[zones[i]] = 1;

    zones = taurus->shell_nodelist;
    nzones = taurus->nel4;
    for (i = 0; i < nzones * 4; i++)
        buf[zones[i]] = 1;

    for (i = 0; i < numnp && buf[i] == 0; i++)
        /* do nothing */ ;
    if (i < numnp) {
        xval = taurus->coords[0][i];
        yval = taurus->coords[1][i];
        if (ndim > 2)
            zval = taurus->coords[2][i];
    }
    else {
        xval = 0.;
        yval = 0.;
        zval = 0.;
    }

    coords = taurus->coords[0];
    for (i = 0; i < numnp; i++)
        if (buf[i] == 0)
            coords[i] = xval;
    coords = taurus->coords[1];
    for (i = 0; i < numnp; i++)
        if (buf[i] == 0)
            coords[i] = yval;
    if (ndim > 2) {
        coords = taurus->coords[2];
        for (i = 0; i < numnp; i++)
            if (buf[i] == 0)
                coords[i] = zval;
    }

    /*
     * Determine the extents of the data.  The extents will change
     * over the course of the problem but the extents at the beginning
     * will be used for all the states.
     */
    coords = taurus->coords[0];
    minval = coords[0];
    maxval = coords[0];
    for (i = 0; i < numnp; i++) {
        minval = MIN(minval, coords[i]);
        maxval = MAX(maxval, coords[i]);
    }
    taurus->min_extents[0] = minval;
    taurus->max_extents[0] = maxval;

    coords = taurus->coords[1];
    minval = coords[0];
    maxval = coords[0];
    for (i = 0; i < numnp; i++) {
        minval = MIN(minval, coords[i]);
        maxval = MAX(maxval, coords[i]);
    }
    taurus->min_extents[1] = minval;
    taurus->max_extents[1] = maxval;

    if (ndim > 2) {
        coords = taurus->coords[2];
        minval = coords[0];
        maxval = coords[0];
        for (i = 0; i < numnp; i++) {
            minval = MIN(minval, coords[i]);
            maxval = MAX(maxval, coords[i]);
        }
        taurus->min_extents[2] = minval;
        taurus->max_extents[2] = maxval;
    }
    else {
        taurus->min_extents[2] = 0.;
        taurus->max_extents[2] = 0.;
    }

    FREE(buf);

    taurus->mesh_read = 1;
}

/*-------------------------------------------------------------------------
 * Function:    init_coord_info
 *
 * Purpose:
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:
 *
 * Modifications:
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
void
init_coord_info (TAURUSfile *taurus)
{
    int            i;
    int            ndim, numnp;
    int            state_loc, state_file;
    int            loc, len, lbuf;
    float         *buf;
    float         *coords;

    ndim = taurus->ndim;
    numnp = taurus->numnp;

    /*
     * Allocate storage if not already allocated.
     */
    if (taurus->coords == NULL) {
        taurus->coords = ALLOC_N(float *, ndim);
        taurus->coords[0] = ALLOC_N(float, numnp);
        taurus->coords[1] = ALLOC_N(float, numnp);

        if (ndim > 2)
            taurus->coords[2] = ALLOC_N(float, numnp);
    }

    /*
     * Set up the addresses of where to read the data from.
     */
    if (taurus->iu == 1) {
        state_loc = taurus->state_loc[taurus->state];
        state_file = taurus->state_file[taurus->state];
        loc = state_loc + (1 + taurus->nglbv) * sizeof(float);
    }
    else {
        state_loc = 64 * sizeof(int);

        state_file = 0;
        loc = state_loc;
    }

    lbuf = taurus->numnp * taurus->ndim;
    buf = ALLOC_N(float, lbuf);

    len = lbuf * sizeof(float);

    taurus_read(taurus, state_file, loc, len, (char*)buf);

    coords = taurus->coords[0];
    for (i = 0; i < numnp; i++)
        coords[i] = buf[i * ndim];

    coords = taurus->coords[1];
    for (i = 0; i < numnp; i++)
        coords[i] = buf[i * ndim + 1];

    if (taurus->ndim > 2) {
        coords = taurus->coords[2];
        for (i = 0; i < numnp; i++)
            coords[i] = buf[i * ndim + 2];
    }

    FREE(buf);

    taurus->coord_state = taurus->state;
}

/*-------------------------------------------------------------------------
 * Function:    init_zone_info
 *
 * Purpose:
 *
 * Return:      Success:
 *
 *              Failure:
 *
 * Programmer:  Eric Brugger
 * Date:        December 20, 1995
 *
 * Modifications:
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
void
init_zone_info (TAURUSfile *taurus)
{
    int            i, j;
    int            ifile, loc;
    int           *zones, *mats;
    int           *faces, nfaces, *zoneno;

    /*
     * Check if any work needs to be done.
     */
    if (taurus->activ < 1000 && taurus->activ > 1005)
        return;

    if (taurus->state < 0 && taurus->state >= taurus->nstates)
        return;

    /*
     * Allocate storage if not already allocated.
     */
    if (taurus->hex_activ == NULL && taurus->nel8 > 0)
        taurus->hex_activ = ALLOC_N (int, taurus->nel8);

    if (taurus->beam_activ == NULL && taurus->nel2 > 0)
        taurus->beam_activ = ALLOC_N (int, taurus->nel2);

    if (taurus->shell_activ == NULL && taurus->nel4 > 0)
        taurus->shell_activ = ALLOC_N (int, taurus->nel4);

    /*
     * Read the activity data from the file if it is present.
     */
    ifile = taurus->state_file [taurus->state];
    loc  = taurus->state_loc [taurus->state];
    loc += (taurus->it * taurus->numnp +
            taurus->ndim * taurus->numnp *
            (taurus->iu + taurus->iv + taurus->ia) +
            taurus->nel8 * taurus->nv3d +
            taurus->nel4 * taurus->nv2d +
            taurus->nel2 * taurus->nv1d +
            taurus->nglbv + 1) * sizeof(int); 

    taurus_read (taurus, ifile, loc, sizeof (int)*taurus->nel8,
                 (char*)(taurus->hex_activ));
    loc += sizeof (int) * taurus->nel8;

    taurus_read (taurus, ifile, loc, sizeof (int)*taurus->nel2,
                 (char*)(taurus->beam_activ));
    loc += sizeof (int) * taurus->nel2;

    taurus_read (taurus, ifile, loc, sizeof (int)*taurus->nel4,
                 (char*)(taurus->shell_activ));
    loc += sizeof (int) * taurus->nel4;

    /*
     * Create the face list for the hex elements.
     */
    FREE (taurus->hex_facelist);
    FREE (taurus->hex_zoneno);
    zones = ALLOC_N (int, 8 * taurus->nel8);
    mats = ALLOC_N (int, taurus->nel8);
    j = 0;
    for (i = 0; i < taurus->nel8; i++) {
        if (taurus->hex_activ [i] != 0) {
            zones[j * 8] = taurus->hex_nodelist[i * 8];
            zones[j * 8 + 1] = taurus->hex_nodelist[i * 8 + 1];
            zones[j * 8 + 2] = taurus->hex_nodelist[i * 8 + 2];
            zones[j * 8 + 3] = taurus->hex_nodelist[i * 8 + 3];
            zones[j * 8 + 4] = taurus->hex_nodelist[i * 8 + 4];
            zones[j * 8 + 5] = taurus->hex_nodelist[i * 8 + 5];
            zones[j * 8 + 6] = taurus->hex_nodelist[i * 8 + 6];
            zones[j * 8 + 7] = taurus->hex_nodelist[i * 8 + 7];
            mats[j] = taurus->hex_matlist [i];
            j++;
        }
    }
    db_taur_extface(zones, taurus->numnp, j,
                    mats, &faces, &nfaces, &zoneno);
    taurus->nhex_faces = nfaces;
    taurus->hex_facelist = faces;
    taurus->hex_zoneno = zoneno;

    FREE (zones);
    FREE (mats);
}

/*-------------------------------------------------------------------------
 * Function:    taurus_readvar
 *
 * Purpose:
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:
 *
 * Modifications:
 *    Eric Brugger, Wed Apr 26 15:21:29 PDT 1995
 *    I modified the routine to return the title in the file
 *    as _fileinfo.
 *
 *    Eric Brugger, Thu Jul 27 12:49:40 PDT 1995
 *    I modified the routine to handle files generated by hydra.
 *
 *    Eric Brugger, Thu Dec 21 09:57:09 PST 1995
 *    I modified the routine to return the meshname of the variable.
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
int
taurus_readvar (TAURUSfile *taurus, char *varname, float **var, int *length,
                int *center, char *meshname)
{
    int            i;
    int            idir;
    int            ivar;
    int            var_id, val_id;

    if (taurus->icode == 1)
        idir = 8;
    else if (taurus->icode == 200)
        idir = 9;
    else
        idir = taurus->idir;

    if (idir == -1)
        return (-1);

    /*
     * Find the variable name in the variable list.
     */
    for (i = 0; taur_var_list[i].idir < idir; i++)
        /* do nothing */ ;

    for (i = i; taur_var_list[i].idir == idir &&
         strcmp(taur_var_list[i].name, varname) != 0; i++)
        /* do nothing */ ;

    if (taur_var_list[i].idir != idir)
        return (-1);

    ivar = i;
    var_id = taur_var_list[ivar].ivar;
    val_id = taur_var_list[ivar].ival;

    if (taurus->var_start[val_id] == -1)
        return (-1);

    /*
     * Set the return values.
     */
    *center = taur_var_list[ivar].centering;
    if (var_id >= VAR_SIGX && var_id <= VAR_PRINC_STRESS_3) {
        *length = taurus->nel8 + taurus->nel4;
    }
    else {
        *length = taurus->var_len[val_id];
    }
    strcpy (meshname, taur_var_list[ivar].mesh);

    /*
     * Allocate space for the variable.
     */
    *var = ALLOC_N(float, *length);

    /*
     * Read the variable.
     */
    taurus_readblockvar(taurus, var_id, val_id, *var);
    if (var_id >= VAR_SIGX && var_id <= VAR_PRINC_STRESS_3) {
        val_id += (VAL_SHELL_MID_SIGX - VAL_HEX_SIGX);
        taurus_readblockvar(taurus, var_id, val_id,
                            &(var[0][taurus->nel8]));
    }

    return (0);
}
