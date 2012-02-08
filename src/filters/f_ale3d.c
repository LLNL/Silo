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
 * ALE-3d filter.
 */
#include <assert.h>
#include <math.h>
#include "silo_private.h"
#include "filter.h"
#include "f_ale3d.h"

#if 0
#define MAXBUF          100000
#else
#define MAXBUF          1000            /*for testing */
#endif

#define false           0
#define true            1
#define TTPI            2.09439510239319549230842892219  /* 2pi/3 */
#define FTPI            4.18879020478639098461685784436  /* 4pi/3 */
#define SQR(X)          ((X)*(X))

#define DIR_ROOT          0            /* /               */
#define DIR_NODE          1            /* /node           */
#define DIR_BRICK         2            /* /brick          */
#define DIR_SHELL         3            /* /shell          */
#define DIR_BRICK_HYDRO   4            /* /brick/hydro    */
#define DIR_BRICK_STRESS  5            /* /brick/stress   */
#define DIR_SHELL_LOWER   6            /* /shell/lower    */
#define DIR_SHELL_MIDDLE  7            /* /shell/middle   */
#define DIR_SHELL_UPPER   8            /* /shell/upper    */
#define DIR_SHELL_OTHER   9            /* /shell/other    */
#define DIR_OTHER        10            /* /other          */

static char   *f_ale3d_name[DB_NFILES];
static DBfile_pub f_ale3d_cb[DB_NFILES];

/*-------------------------------------------------------------------------
 *
 * Equations used in this file.
 *
 * Definitions:
 *    sx  = xx total stress
 *    sy  = yy total stress
 *    sz  = zz total stress
 *    txy = xy total stress
 *    tyz = yz total stress
 *    tzx = zx total stress
 *    dev_sx  = xx deviatoric stress
 *    dev_sy  = yy deviatoric stress
 *    dev_sz  = zz deviatoric stress
 *    dev_txy = xy deviatoric stress
 *    dev_tyz = yz deviatoric stress
 *    dev_tzx = zx deviatoric stress
 *
 *    TTPI = 2/3 pi
 *    FTPI = 4/3 pi
 *
 * Equations:
 *    twoj = sqrt (2.*sqr(sx)  + 2.*sqr(sy)  + 2.*sqr(sz) +
 *                 2.*sqr(txy) + 2.*sqr(tyz) + 2.*sqr(tzx))
 *
 *    pressure = -(sx + sy + sz) / 3.0
 *    dev_sx  = sx  + pressure
 *    dev_sy  = sy  + pressure
 *    dev_sz  = sz  + pressure
 *    dev_txy = txy + pressure
 *    dev_tyz = tyz + pressure
 *    dev_tzx = tzx + pressure
 *    von_mises = sqrt (3. * fabs(sqr(txy) + sqr(tyz) + sqr(tzx) -
 *                dev_sx*dev_sy - dev_sy*dev_sz - dev_sz*dev_sx))
 *
 *    aa = sqr(txy) + sqr(tyz) + sqr(tzx) -
 *         dev_sx*dev_sy - dev_sy*dev_sz - dev_sz*dev_sx
 *
 *    bb = dev_sx*sqr(tyz) + dev_sy*sqr(tzx) + dev_sz*sqr(txy) -
 *         dev_sx*dev_syz*dev_szx - txy * tyz * tzx * 2.0
 *
 *    cc = -sqrt (27.0 / aa) * bb * 0.5 / aa
 *
 *    angp = acos(cc) / 3.0
 *
 *    princ_dev_stress_1 =  2.0 * sqrt(aa / 3.0) * cos(angp)
 *    princ_dev_stress_2 =  2.0 * sqrt(aa / 3.0) * cos(angp+FTPI)
 *    princ_dev_stress_3 =  2.0 * sqrt(aa / 3.0) * cos(angp+TTPI)
 *    princ_tot_stress_1 = princ_dev_stress_1 - pressure
 *    princ_tot_stress_2 = princ_dev_stress_2 - pressure
 *    princ_tot_stress_3 = princ_dev_stress_3 - pressure
 *    max_shear_stress   = 2.0 * sqrt (aa / 3.0) * (cos(angp) - cos(angp+TTPI))
 *
 *    surface_stress_1 = n_xx_normal / thickness + 6.0 *
 *                       m_xx_bending / sqr(thickness)
 *    surface_stress_2 = n_xx_normal / thickness - 6.0 *
 *                       m_xx_bending / sqr(thickness)
 *    surface_stress_3 = n_yy_normal / thickness + 6.0 *
 *                       m_yy_bending / sqr(thickness)
 *    surface_stress_4 = n_yy_normal / thickness - 6.0 *
 *                       m_yy_bending / sqr(thickness)
 *    surface_stress_5 = n_xy_normal / thickness + 6.0 *
 *                       m_xy_bending / sqr(thickness)
 *    surface_stress_6 = n_xy_normal / thickness - 6.0 *
 *                       m_xy_bending / sqr(thickness)
 *    eff_upper_stress = sqrt (sqr(surface_stress_1) +
 *                       (surface_stress_1 * surface_stress_3) +
 *                       sqr(surface_stress_3) + 3.0 * sqr(surface_stress_5)
 *    eff_lower_stress = sqrt (sqr(surface_stress_2) +
 *                       (surface_stress_2 * surface_stress_4) +
 *                       sqr(surface_stress_4) + 3.0 * sqr(surface_stress_6)
 *    eff_max_stress   = max (eff_lower_stress, eff_upper_stress)
 *
 *-------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 * This structure records dependency information about UCD variables supplied
 * by this filter (virtual variables) and the real UCD variables appearing in
 * the database.
 *
 * The `v' field is the virtual variable name.  The `r' field is a colon-
 * separated list of real variable names.  If there is only one dependency
 * in `r' then the variable is simply a change-of-name.
 *
 * The `r' field may contain more than one dependency list.  Each dependency
 * list is terminated with a null character.  The final dependency list
 * must be terminated with two null characters (the C compiler will
 * supply one automatically).  If a variable has an alias, then that
 * one-item dependency should be listed first.
 *-------------------------------------------------------------------------
 */
#define BRICK_TOT_STRESS_XX         0
#define BRICK_TOT_STRESS_YY         1
#define BRICK_TOT_STRESS_ZZ         2
#define BRICK_VON_MISES             3
#define BRICK_MAX_SHEAR_STRESS      4
#define BRICK_PRINC_DEV_STRESS_1    5
#define BRICK_PRINC_DEV_STRESS_2    6
#define BRICK_PRINC_DEV_STRESS_3    7
#define BRICK_PRINC_TOT_STRESS_1    8
#define BRICK_PRINC_TOT_STRESS_2    9
#define BRICK_PRINC_TOT_STRESS_3   10

#define SHELL_PRESSURE            0
#define SHELL_VON_MISES           1
#define SHELL_MAX_SHEAR_STRESS    2
#define SHELL_PRINC_DEV_STRESS_1  3
#define SHELL_PRINC_DEV_STRESS_2  4
#define SHELL_PRINC_DEV_STRESS_3  5
#define SHELL_PRINC_TOT_STRESS_1  6
#define SHELL_PRINC_TOT_STRESS_2  7
#define SHELL_PRINC_TOT_STRESS_3  8

#define SHELL_SURFACE_STRESS_1  0
#define SHELL_SURFACE_STRESS_2  1
#define SHELL_SURFACE_STRESS_3  2
#define SHELL_SURFACE_STRESS_4  3
#define SHELL_SURFACE_STRESS_5  4
#define SHELL_SURFACE_STRESS_6  5
#define SHELL_EFF_UPPER_STRESS  6
#define SHELL_EFF_LOWER_STRESS  7
#define SHELL_EFF_MAX_STRESS    8

static void calc_magnitude (float *result, float **data, int length, int extra);
static void calc_j2 (float *result, float **data, int length, int extra);
static void calc_brick (float *result, float **data, int length, int extra);
static void calc_shell (float *result, float **data, int length, int extra);
static void calc_shell_other (float *result, float **data, int length,
    int extra);

static struct {                 /*Variable we are intercepting  */
    char          *v;           /*Virtual (new) name   */
    char          *r;           /*Real dependencies   */
    int           dir;          /*Directory that the variable is in */
    void          (*f) (float *, float **, int, int);  /*Calc routine */
    int           extra;        /*fourth arg to `f'   */
} Intercept[] = {

    {
        "acc_x", "acc_x\0", DIR_NODE, NULL, 0
    },
    {
        "acc_y", "acc_y\0", DIR_NODE, NULL, 0
    },
    {
        "acc_z", "acc_z\0", DIR_NODE, NULL, 0
    },
    {
        "acc_magnitude", "acc_x:acc_y:acc_z\0", DIR_NODE, calc_magnitude, 0
    },
    {
        "disp_x", "disp_x\0", DIR_NODE, NULL, 0
    },
    {
        "disp_y", "disp_y\0", DIR_NODE, NULL, 0
    },
    {
        "disp_z", "disp_z\0", DIR_NODE, NULL, 0
    },
    {
        "disp_magnitude", "disp_x:disp_y:disp_z\0", DIR_NODE, calc_magnitude, 0
    },
    {
        "heatflux_x", "heatflux_x\0", DIR_NODE, NULL, 0
    },
    {
        "heatflux_y", "heatflux_y\0", DIR_NODE, NULL, 0
    },
    {
        "heatflux_z", "heatflux_z\0", DIR_NODE, NULL, 0
    },
    {
        "temperature", "heatflux_x:heatflux_y:heatflux_z\0", DIR_NODE,
        calc_magnitude, 0
    },
    {
        "vel_x", "vel_x\0", DIR_NODE, NULL, 0
    },
    {
        "vel_y", "vel_y\0", DIR_NODE, NULL, 0
    },
    {
        "vel_z", "vel_z\0", DIR_NODE, NULL, 0
    },
    {
        "vel_magnitude", "vel_x:vel_y:vel_z\0", DIR_NODE, calc_magnitude, 0
    },

    /*
     * /brick/hydro.
     */
    {
        "density", "density\0", DIR_BRICK_HYDRO, NULL, 0
    },
    {
        "equivalent_plastic_strain", "equivalent_plastic_strain\0",
        DIR_BRICK_HYDRO, NULL, 0
    },
    {
        "energy", "energy\0", DIR_BRICK_HYDRO, NULL, 0
    },
    {
        "lighting_time", "lighting_time\0", DIR_BRICK_HYDRO, NULL, 0
    },
    {
        "pressure", "pressure\0", DIR_BRICK_HYDRO, NULL, 0
    },
    {
        "relative_volume", "relative_volume\0", DIR_BRICK_HYDRO, NULL, 0
    },
    {
        "shear_modulus", "shear_modulus\0", DIR_BRICK_HYDRO, NULL, 0
    },
    {
        "shock_viscosity", "shock_viscosity\0", DIR_BRICK_HYDRO, NULL, 0
    },
    {
        "sigmts", "sigtmts\0", DIR_BRICK_HYDRO, NULL, 0
    },
    {
        "twoj", "dev_stress_xx:dev_stress_yy:dev_stress_zz:dev_stress_xy"
        ":dev_stress_xz:dev_stress_yz\0", DIR_BRICK_HYDRO, calc_j2, 0
    },
    {
        "yield", "yield\0", DIR_BRICK_HYDRO, NULL, 0
    },
    {
        "zonal_temperature", "zonal_temperature\0", DIR_BRICK_HYDRO, NULL, 0
    },

    /*
     * /brick/stress.
     */
    {
        "dev_stress_xx", "dev_stress_xx\0", DIR_BRICK_STRESS, NULL, 0
    },
    {
        "dev_stress_yy", "dev_stress_yy\0", DIR_BRICK_STRESS, NULL, 0
    },
    {
        "dev_stress_zz", "dev_stress_zz\0", DIR_BRICK_STRESS, NULL, 0
    },
    {
        "max_shear_stress", "dev_stress_xx:dev_stress_yy:dev_stress_zz"
        ":dev_stress_xy:dev_stress_yz:dev_stress_xz\0", DIR_BRICK_STRESS,
        calc_brick, BRICK_MAX_SHEAR_STRESS
    },
    {
        "princ_dev_stress_1", "dev_stress_xx:dev_stress_yy:dev_stress_zz"
        ":dev_stress_xy:dev_stress_yz:dev_stress_xz\0", DIR_BRICK_STRESS,
        calc_brick, BRICK_PRINC_DEV_STRESS_1
    },
    {
        "princ_dev_stress_2", "dev_stress_xx:dev_stress_yy:dev_stress_zz"
        ":dev_stress_xy:dev_stress_yz:dev_stress_xz\0", DIR_BRICK_STRESS,
        calc_brick, BRICK_PRINC_DEV_STRESS_2
    },
    {
        "princ_dev_stress_3", "dev_stress_xx:dev_stress_yy:dev_stress_zz"
        ":dev_stress_xy:dev_stress_yz:dev_stress_xz\0", DIR_BRICK_STRESS,
        calc_brick, BRICK_PRINC_DEV_STRESS_3
    },
    {
        "princ_tot_stress_1", "dev_stress_xx:dev_stress_yy:dev_stress_zz"
        ":dev_stress_xy:dev_stress_yz:dev_stress_xz:pressure\0",
        DIR_BRICK_STRESS, calc_brick, BRICK_PRINC_TOT_STRESS_1
    },
    {
        "princ_tot_stress_2", "dev_stress_xx:dev_stress_yy:dev_stress_zz"
        ":dev_stress_xy:dev_stress_yz:dev_stress_xz:pressure\0",
        DIR_BRICK_STRESS, calc_brick, BRICK_PRINC_TOT_STRESS_2
    },
    {
        "princ_tot_stress_3", "dev_stress_xx:dev_stress_yy:dev_stress_zz"
        ":dev_stress_xy:dev_stress_yz:dev_stress_xz:pressure\0",
        DIR_BRICK_STRESS, calc_brick, BRICK_PRINC_TOT_STRESS_3
    },
    {
        "tot_stress_xx", "dev_stress_xx:pressure\0", DIR_BRICK_STRESS,
        calc_brick, BRICK_TOT_STRESS_XX
    },
    {
        "tot_stress_yy", "dev_stress_yy:pressure\0", DIR_BRICK_STRESS,
        calc_brick, BRICK_TOT_STRESS_YY
    },
    {
        "tot_stress_zz", "dev_stress_zz:pressure\0", DIR_BRICK_STRESS,
        calc_brick, BRICK_TOT_STRESS_ZZ
    },
    {
        "tot_stress_xy", "dev_stress_xy\0", DIR_BRICK_STRESS, NULL, 0
    },
    {
        "tot_stress_yz", "dev_stress_yz\0", DIR_BRICK_STRESS, NULL, 0
    },
    {
        "tot_stress_zx", "dev_stress_xz\0", DIR_BRICK_STRESS, NULL, 0
    },
    {
        "von_mises", "dev_stress_xx:dev_stress_yy:dev_stress_zz"
        ":dev_stress_xy:dev_stress_yz:dev_stress_xz\0", DIR_BRICK_STRESS,
        calc_brick, BRICK_VON_MISES
    },

    /*
     * /shell/lower.
     */
    {
        "equivalent_plastic_strain", "low_equivalent_plastic_strain\0",
        DIR_SHELL_LOWER, NULL, 0
    },
    {
        "max_shear_stress", "low_tot_stress_xx:low_tot_stress_yy"
        ":low_tot_stress_zz:low_tot_stress_xy:low_tot_stress_yz"
        ":low_tot_stress_zx\0", DIR_SHELL_LOWER,
        calc_shell, SHELL_MAX_SHEAR_STRESS
    },
    {
        "pressure", "low_tot_stress_xx:low_tot_stress_yy"
        ":low_tot_stress_zz\0", DIR_SHELL_LOWER,
        calc_shell, SHELL_PRESSURE
    },
    {
        "princ_dev_stress_1", "low_tot_stress_xx:low_tot_stress_yy"
        ":low_tot_stress_zz:low_tot_stress_xy:low_tot_stress_yz"
        ":low_tot_stress_zx\0", DIR_SHELL_LOWER,
        calc_shell, SHELL_PRINC_DEV_STRESS_1
    },
    {
        "princ_dev_stress_2", "low_tot_stress_xx:low_tot_stress_yy"
        ":low_tot_stress_zz:low_tot_stress_xy:low_tot_stress_yz"
        ":low_tot_stress_zx\0", DIR_SHELL_LOWER,
        calc_shell, SHELL_PRINC_DEV_STRESS_2
    },
    {
        "princ_dev_stress_3", "low_tot_stress_xx:low_tot_stress_yy"
        ":low_tot_stress_zz:low_tot_stress_xy:low_tot_stress_yz"
        ":low_tot_stress_zx\0", DIR_SHELL_LOWER,
        calc_shell, SHELL_PRINC_DEV_STRESS_3
    },
    {
        "princ_tot_stress_1", "low_tot_stress_xx:low_tot_stress_yy"
        ":low_tot_stress_zz:low_tot_stress_xy:low_tot_stress_yz"
        ":low_tot_stress_zx\0", DIR_SHELL_LOWER,
        calc_shell, SHELL_PRINC_TOT_STRESS_1
    },
    {
        "princ_tot_stress_2", "low_tot_stress_xx:low_tot_stress_yy"
        ":low_tot_stress_zz:low_tot_stress_xy:low_tot_stress_yz"
        ":low_tot_stress_zx\0", DIR_SHELL_LOWER,
        calc_shell, SHELL_PRINC_TOT_STRESS_2
    },
    {
        "princ_tot_stress_3", "low_tot_stress_xx:low_tot_stress_yy"
        ":low_tot_stress_zz:low_tot_stress_xy:low_tot_stress_yz"
        ":low_tot_stress_zx\0", DIR_SHELL_LOWER,
        calc_shell, SHELL_PRINC_TOT_STRESS_3
    },
    {
        "strain_xx", "low_strain_xx\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "strain_yy", "low_strain_yy\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "strain_zz", "low_strain_zz\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "strain_xy", "low_strain_xy\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "strain_yz", "low_strain_yz\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "strain_zx", "low_strain_zx\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "tot_stress_xx", "low_tot_stress_xx\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "tot_stress_yy", "low_tot_stress_yy\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "tot_stress_zz", "low_tot_stress_zz\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "tot_stress_xy", "low_tot_stress_xy\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "tot_stress_yz", "low_tot_stress_yz\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "tot_stress_zx", "low_tot_stress_zx\0", DIR_SHELL_LOWER, NULL, 0
    },
    {
        "von_mises", "low_tot_stress_xx:low_tot_stress_yy:low_tot_stress_zz"
        ":low_tot_stress_xy:low_tot_stress_yz:low_tot_stress_zx\0",
        DIR_SHELL_LOWER, calc_shell, SHELL_VON_MISES
    },

    /*
     * /shell/middle.
     */
    {
        "equivalent_plastic_strain", "mid_equivalent_plastic_strain\0",
        DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "max_shear_stress", "mid_tot_stress_xx:mid_tot_stress_yy"
        ":mid_tot_stress_zz:mid_tot_stress_xy:mid_tot_stress_yz"
        ":mid_tot_stress_zx\0", DIR_SHELL_MIDDLE,
        calc_shell, SHELL_MAX_SHEAR_STRESS
    },
    {
        "pressure", "mid_tot_stress_xx:mid_tot_stress_yy"
        ":mid_tot_stress_zz\0", DIR_SHELL_MIDDLE,
        calc_shell, SHELL_PRESSURE
    },
    {
        "princ_dev_stress_1", "mid_tot_stress_xx:mid_tot_stress_yy"
        ":mid_tot_stress_zz:mid_tot_stress_xy:mid_tot_stress_yz"
        ":mid_tot_stress_zx\0", DIR_SHELL_MIDDLE,
        calc_shell, SHELL_PRINC_DEV_STRESS_1
    },
    {
        "princ_dev_stress_2", "mid_tot_stress_xx:mid_tot_stress_yy"
        ":mid_tot_stress_zz:mid_tot_stress_xy:mid_tot_stress_yz"
        ":mid_tot_stress_zx\0", DIR_SHELL_MIDDLE,
        calc_shell, SHELL_PRINC_DEV_STRESS_2
    },
    {
        "princ_dev_stress_3", "mid_tot_stress_xx:mid_tot_stress_yy"
        ":mid_tot_stress_zz:mid_tot_stress_xy:mid_tot_stress_yz"
        ":mid_tot_stress_zx\0", DIR_SHELL_MIDDLE,
        calc_shell, SHELL_PRINC_DEV_STRESS_3
    },
    {
        "princ_tot_stress_1", "mid_tot_stress_xx:mid_tot_stress_yy"
        ":mid_tot_stress_zz:mid_tot_stress_xy:mid_tot_stress_yz"
        ":mid_tot_stress_zx\0", DIR_SHELL_MIDDLE,
        calc_shell, SHELL_PRINC_TOT_STRESS_1
    },
    {
        "princ_tot_stress_2", "mid_tot_stress_xx:mid_tot_stress_yy"
        ":mid_tot_stress_zz:mid_tot_stress_xy:mid_tot_stress_yz"
        ":mid_tot_stress_zx\0", DIR_SHELL_MIDDLE,
        calc_shell, SHELL_PRINC_TOT_STRESS_2
    },
    {
        "princ_tot_stress_3", "mid_tot_stress_xx:mid_tot_stress_yy"
        ":mid_tot_stress_zz:mid_tot_stress_xy:mid_tot_stress_yz"
        ":mid_tot_stress_zx\0", DIR_SHELL_MIDDLE,
        calc_shell, SHELL_PRINC_TOT_STRESS_3
    },
    {
        "strain_xx", "mid_strain_xx\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "strain_yy", "mid_strain_yy\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "strain_zz", "mid_strain_zz\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "strain_xy", "mid_strain_xy\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "strain_yz", "mid_strain_yz\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "strain_zx", "mid_strain_zx\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "tot_stress_xx", "mid_tot_stress_xx\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "tot_stress_yy", "mid_tot_stress_yy\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "tot_stress_zz", "mid_tot_stress_zz\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "tot_stress_xy", "mid_tot_stress_xy\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "tot_stress_yz", "mid_tot_stress_yz\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "tot_stress_zx", "mid_tot_stress_zx\0", DIR_SHELL_MIDDLE, NULL, 0
    },
    {
        "von_mises", "mid_tot_stress_xx:mid_tot_stress_yy:mid_tot_stress_zz"
        ":mid_tot_stress_xy:mid_tot_stress_yz:mid_tot_stress_zx\0",
        DIR_SHELL_MIDDLE, calc_shell, SHELL_VON_MISES
    },

    /*
     * /shell/upper.
     */
    {
        "equivalent_plastic_strain", "upp_equivalent_plastic_strain\0",
        DIR_SHELL_UPPER, NULL, 0
    },
    {
        "max_shear_stress", "upp_tot_stress_xx:upp_tot_stress_yy"
        ":upp_tot_stress_zz:upp_tot_stress_xy:upp_tot_stress_yz"
        ":upp_tot_stress_zx\0", DIR_SHELL_UPPER,
        calc_shell, SHELL_MAX_SHEAR_STRESS
    },
    {
        "pressure", "upp_tot_stress_xx:upp_tot_stress_yy"
        ":upp_tot_stress_zz\0", DIR_SHELL_UPPER,
        calc_shell, SHELL_PRESSURE
    },
    {
        "princ_dev_stress_1", "upp_tot_stress_xx:upp_tot_stress_yy"
        ":upp_tot_stress_zz:upp_tot_stress_xy:upp_tot_stress_yz"
        ":upp_tot_stress_zx\0", DIR_SHELL_UPPER,
        calc_shell, SHELL_PRINC_DEV_STRESS_1
    },
    {
        "princ_dev_stress_2", "upp_tot_stress_xx:upp_tot_stress_yy"
        ":upp_tot_stress_zz:upp_tot_stress_xy:upp_tot_stress_yz"
        ":upp_tot_stress_zx\0", DIR_SHELL_UPPER,
        calc_shell, SHELL_PRINC_DEV_STRESS_2
    },
    {
        "princ_dev_stress_3", "upp_tot_stress_xx:upp_tot_stress_yy"
        ":upp_tot_stress_zz:upp_tot_stress_xy:upp_tot_stress_yz"
        ":upp_tot_stress_zx\0", DIR_SHELL_UPPER,
        calc_shell, SHELL_PRINC_DEV_STRESS_3
    },
    {
        "princ_tot_stress_1", "upp_tot_stress_xx:upp_tot_stress_yy"
        ":upp_tot_stress_zz:upp_tot_stress_xy:upp_tot_stress_yz"
        ":upp_tot_stress_zx\0", DIR_SHELL_UPPER,
        calc_shell, SHELL_PRINC_TOT_STRESS_1
    },
    {
        "princ_tot_stress_2", "upp_tot_stress_xx:upp_tot_stress_yy"
        ":upp_tot_stress_zz:upp_tot_stress_xy:upp_tot_stress_yz"
        ":upp_tot_stress_zx\0", DIR_SHELL_UPPER,
        calc_shell, SHELL_PRINC_TOT_STRESS_2
    },
    {
        "princ_tot_stress_3", "upp_tot_stress_xx:upp_tot_stress_yy"
        ":upp_tot_stress_zz:upp_tot_stress_xy:upp_tot_stress_yz"
        ":upp_tot_stress_zx\0", DIR_SHELL_UPPER,
        calc_shell, SHELL_PRINC_TOT_STRESS_3
    },
    {
        "strain_xx", "upp_strain_xx\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "strain_yy", "upp_strain_yy\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "strain_zz", "upp_strain_zz\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "strain_xy", "upp_strain_xy\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "strain_yz", "upp_strain_yz\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "strain_zx", "upp_strain_zx\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "tot_stress_xx", "upp_tot_stress_xx\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "tot_stress_yy", "upp_tot_stress_yy\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "tot_stress_zz", "upp_tot_stress_zz\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "tot_stress_xy", "upp_tot_stress_xy\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "tot_stress_yz", "upp_tot_stress_yz\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "tot_stress_zx", "upp_tot_stress_zx\0", DIR_SHELL_UPPER, NULL, 0
    },
    {
        "von_mises", "upp_tot_stress_xx:upp_tot_stress_yy:upp_tot_stress_zz"
        ":upp_tot_stress_xy:upp_tot_stress_yz:upp_tot_stress_zx\0",
        DIR_SHELL_UPPER, calc_shell, SHELL_VON_MISES
    },

    /*
     * /shell/other.
     */
    {
        "eff_lower_stress", "m_xx_bending:m_yy_bending:m_xy_bending"
        ":n_xx_normal:n_yy_normal:n_xy_normal:thickness\0", DIR_SHELL_OTHER,
        calc_shell_other, SHELL_EFF_LOWER_STRESS
    },
    {
        "eff_max_stress", "m_xx_bending:m_yy_bending:m_xy_bending"
        ":mn_xx_normal:n_yy_normal:n_xy_normal:thickness\0", DIR_SHELL_OTHER,
        calc_shell_other, SHELL_EFF_MAX_STRESS
    },
    {
        "eff_upper_stress", "m_xx_bending:m_yy_bending:m_xy_bending"
        ":n_xx_normal:n_yy_normal:n_xy_normal:thickness\0", DIR_SHELL_OTHER,
        calc_shell_other, SHELL_EFF_UPPER_STRESS
    },
    {
        "internal_energy", "internal_energy\0", DIR_SHELL_OTHER, NULL, 0
    },
    {
        "m_xx_bending", "m_xx_bending\0", DIR_SHELL_OTHER, NULL, 0
    },
    {
        "m_yy_bending", "m_yy_bending\0", DIR_SHELL_OTHER, NULL, 0
    },
    {
        "m_xy_bending", "m_xy_bending\0", DIR_SHELL_OTHER, NULL, 0
    },
    {
        "n_xx_normal", "n_xx_normal\0", DIR_SHELL_OTHER, NULL, 0
    },
    {
        "n_yy_normal", "n_yy_normal\0", DIR_SHELL_OTHER, NULL, 0
    },
    {
        "n_xy_normal", "n_xy_normal\0", DIR_SHELL_OTHER, NULL, 0
    },
    {
        "q_xx_shear", "q_xx_shear\0", DIR_SHELL_OTHER, NULL, 0
    },
    {
        "q_yy_shear", "q_yy_shear\0", DIR_SHELL_OTHER, NULL, 0
    },
    {
        "surface_stress_1", "m_xx_bending:n_xx_normal:thickness\0",
        DIR_SHELL_OTHER, calc_shell_other, SHELL_SURFACE_STRESS_1
    },
    {
        "surface_stress_2", "m_xx_bending:n_xx_normal:thickness\0",
        DIR_SHELL_OTHER, calc_shell_other, SHELL_SURFACE_STRESS_2
    },
    {
        "surface_stress_3", "m_yy_bending:n_yy_normal:thickness\0",
        DIR_SHELL_OTHER, calc_shell_other, SHELL_SURFACE_STRESS_3
    },
    {
        "surface_stress_4", "m_yy_bending:n_yy_normal:thickness\0",
        DIR_SHELL_OTHER, calc_shell_other, SHELL_SURFACE_STRESS_4
    },
    {
        "surface_stress_5", "m_xy_bending:n_xy_normal:thickness\0",
        DIR_SHELL_OTHER, calc_shell_other, SHELL_SURFACE_STRESS_5
    },
    {
        "surface_stress_6", "m_xy_bending:n_xy_normal:thickness\0",
        DIR_SHELL_OTHER, calc_shell_other, SHELL_SURFACE_STRESS_6
    },
    {
        "thickness", "thickness\0", DIR_SHELL_OTHER, NULL, 0
    },

};

/*-------------------------------------------------------------------------
 * Function:    satisfied
 *
 * Purpose:     Given a database ID and a virtual name, determine if all
 *              of the real dependencies exist.
 *
 * Return:      Success:        0=no, >0=yes
 *
 *              Failure:        -1 (shouldn't ever happen)
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 20:22:44 EST 1995
 *
 * Modifications:
 *
 *      Robb Matzke, Thu Mar 16 12:29:55 EST 1995
 *      Changed to support the multiple dependency lists.  Each dep list
 *      is terminated with a null character.  The final list is terminated
 *      with two null terminators.
 *
 *      Robb Matzke, Thu Mar 16 12:52:16 EST 1995
 *      The dependency list where all the items are satisfied is saved
 *      in the parameter `deplist' if not the null pointer.  If no
 *      dependency list was completely satisfied or if `name' was not
 *      found, then deplist[0]=='\0'.  If `slotno' is non-null, then it
 *      will be initialized with the Intercept slot number, or -1 if
 *      name is not found in any slot.
 *
 *      Jim Reus, 23 Apr 97
 *      Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static int
satisfied ( int id, char *name, char *deplist, int *slotno)
{
    char          *me = "satisfied";
    DBtoc         *real_toc;
    int            slot, listno, found, i;
    char          *list, _work[256], *work, *item;

    if (!f_ale3d_cb[id].toc) {
        return db_perror("missing table of contents", E_INTERNAL, me);
    }
    real_toc = f_ale3d_cb[id].toc;

    /*
     * Search the entire Intercept table until we find the requested
     * virtual variable.  Only one entry in the table will match `name',
     * so we use the first on that we find.
     */
    for (slot = 0; slot < NELMTS(Intercept); slot++) {
        if (!strcmp(Intercept[slot].v, name))
            break;
    }
    if (slot >= NELMTS(Intercept)) {
        if (deplist)
            deplist[0] = '\0';
        if (slotno)
            *slotno = -1;
        return false;
    }
    if (slotno)
        *slotno = slot;

    /*
     * Look at each dependency list until we find one where all of the
     * dependencies in that list are satisfied.  Return the 1-origin
     * list number whose dependencies were all satisfied.
     */
    for (listno = 1, list = Intercept[slot].r;
         list && *list;
         listno++, list += strlen(list) + 1) {
        strcpy(_work, list);
        work = _work;
        if (deplist)
            strcpy(deplist, list);

        /*
         * Check each dependency item.
         */
        for (work = _work, found = true;
             found && (item = strtok(work, ":"));
             work = NULL) {
            for (i = found = 0; !found && i < real_toc->nucdvar; i++) {
                found = !strcmp(item, real_toc->ucdvar_names[i]);
            }
        }

        /*
         * If every item in this dependency list was found, return
         * the 1-origin list number.
         */
        if (found)
            return listno;
    }

    if (deplist)
        deplist[0] = '\0';
    return false;
}

/*-------------------------------------------------------------------------
 * Function:    v_exists
 *
 * Purpose:     Determine if `name' is a UCD variable in the specified
 *              table of contents.
 *
 * Return:      Success:        0=no, 1=yes
 *
 *              Failure:
 *
 * Programmer:  robb@cloud
 *              Wed Mar  8 18:16:32 EST 1995
 *
 * Modifications:
 *
 *    Jim Reus, 23 Apr 97
 *    Changed to prototype form.
 *
 *-------------------------------------------------------------------------
 */
static int
v_exists ( DBtoc *toc, char *name)
{
    int            i;

    if (!toc)
        return false;
    for (i = 0; i < toc->nucdvar; i++) {
        if (!strcmp(name, toc->ucdvar_names[i]))
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_Uninstall
 *
 * Purpose:     Removes the filter from the file.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Thu Mar 16 10:37:18 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
f_ale3d_Uninstall(DBfile *dbfile)
{
    int            id;
    char          *me = "f_ale3d_Uninstall";

    if ((id = FILTER_ID(dbfile, me)))
        return -1;

    /*
     * Copy old public fields back onto file.
     */
    db_FreeToc(dbfile);
    memcpy(&(dbfile->pub), f_ale3d_cb + id, sizeof(DBfile_pub));
    free(f_ale3d_name[id]);
    f_ale3d_name[id] = NULL;
    return DBNewToc(dbfile);
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_Filters
 *
 * Purpose:     Print the filter names to the specified stream.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 11:06:17 EST 1995
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
f_ale3d_Filters(DBfile *dbfile, FILE *stream)
{
    int            id;
    char          *me = "f_ale3d_Filters";

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1;
    fprintf(stream, "%s [ALE-3d filter]\n", f_ale3d_name[id]);
    return FILTER_CALL(f_ale3d_cb[id].module, (dbfile, stream), -1, me);
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_close
 *
 * Purpose:     Close the database.  Clear entry in filter tables.
 *
 * Return:      Value of device driver
 *
 * Programmer:  robb@cloud
 *              Mon Mar  6 18:03:00 EST 1995
 *
 * Modifications:
 *
 *      Robb Matzke, 14 May 1996
 *      Fixed the return value to be -1 on error.
 *
 *-------------------------------------------------------------------------
 */
static int
f_ale3d_Close(DBfile *dbfile)
{
    int            retval;
    char          *me = "f_ale3d_close";
    int            id;

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1 ;
    retval = FILTER_CALL(f_ale3d_cb[id].close, (dbfile), (int)0, me);
    free(f_ale3d_name[id]);
    f_ale3d_name[id] = NULL;
    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_NewToc
 *
 * Purpose:     After the device driver builds the table of contents,
 *              we should store it away and build our own which contains
 *              only those items that are visible to the user.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 10:20:23 EST 1995
 *
 * Modifications:
 *
 *      Robb Matzke, 14 May 1996
 *      Fixed an assert() so as not to cause a compiler warning.
 *
 *      Eric Brugger, Mon Nov 11 09:14:13 PST 1996
 *      I modified the routine to use the new intercept data structure.
 *
 *-------------------------------------------------------------------------
 */
static int
f_ale3d_NewToc(DBfile *dbfile)
{
    int            id, nvars, i, j;
    char          *me = "f_ale3d_NewToc";
    DBtoc         *toc, *real_toc;

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1;

    /*
     * If we haven't gotten a true table of contents yet, we should
     * do that now.  The true table of contents should never change
     * since ALE-3d files don't have directories.
     */
    if (NULL == f_ale3d_cb[id].toc) {
        if (FILTER_CALL(f_ale3d_cb[id].newtoc, (dbfile), -1, me) < 0)
            return -1;
        f_ale3d_cb[id].toc = dbfile->pub.toc;
        dbfile->pub.toc = NULL;
    }
    real_toc = f_ale3d_cb[id].toc;

    /*
     * Depending on which directory is current (by directory ID) we
     * should build a fake table of contents.
     */
    db_FreeToc(dbfile);
    toc = dbfile->pub.toc = db_AllocToc();
    switch (dbfile->pub.dirid) {
        case DIR_ROOT:
            toc->nucdmesh = 5;
            toc->ucdmesh_names = ALLOC_N(char *, toc->nucdmesh);

            toc->ucdmesh_names[0] = STRDUP("beam_mesh");
            toc->ucdmesh_names[1] = STRDUP("hex_mesh");
            toc->ucdmesh_names[2] = STRDUP("hs_mesh");
            toc->ucdmesh_names[3] = STRDUP("mesh1");
            toc->ucdmesh_names[4] = STRDUP("shell_mesh");

            toc->nmat = 1;
            toc->mat_names = ALLOC_N(char *, toc->nmat);

            toc->mat_names[0] = STRDUP("mat1");

            toc->ndir = 4;
            toc->dir_names = ALLOC_N(char *, toc->ndir);

            toc->dir_names[0] = STRDUP("brick");
            toc->dir_names[1] = STRDUP("node");
            toc->dir_names[2] = STRDUP("other");
            toc->dir_names[3] = STRDUP("shell");
            break;

        case DIR_BRICK:
            toc->ndir = 2;
            toc->dir_names = ALLOC_N(char *, toc->ndir);

            toc->dir_names[0] = STRDUP("hydro");
            toc->dir_names[1] = STRDUP("stress");

            break;

        case DIR_SHELL:
            toc->ndir = 4;
            toc->dir_names = ALLOC_N(char *, toc->ndir);

            toc->dir_names[0] = STRDUP("lower");
            toc->dir_names[1] = STRDUP("middle");
            toc->dir_names[2] = STRDUP("other");
            toc->dir_names[3] = STRDUP("upper");

            break;

        case DIR_NODE:
        case DIR_BRICK_HYDRO:
        case DIR_BRICK_STRESS:
        case DIR_SHELL_LOWER:
        case DIR_SHELL_MIDDLE:
        case DIR_SHELL_UPPER:
        case DIR_SHELL_OTHER:
            /*
             * Skip over the portion of the list that doesn't
             * apply to this directory.  This logic assumes that
             * the list is sorted in increasing order of "dir".
             */
            for (i = 0; Intercept[i].dir < dbfile->pub.dirid; i++)
                /* do nothing */;

            /*
             * Form the list of variables in the current directory.
             * This coding assumes that none of the directories
             * contain more than 24 variables.
             */
            toc->ucdvar_names = ALLOC_N(char *, 24);

            toc->nucdvar = nvars = 0;
            for (; Intercept[i].dir == dbfile->pub.dirid; i++) {
                if (satisfied(id, Intercept[i].v, NULL, NULL)) {
                    toc->ucdvar_names[nvars++] = STRDUP(Intercept[i].v);
                }
            }
            toc->nucdvar = nvars;
            break;

        case DIR_OTHER:
            /*
             * All UCD variables that don't have aliases.  If a virtual variable
             * has an alias, the real variable will be listed first in the
             * dependency lists.
             */
            toc->ucdvar_names = ALLOC_N(char *, real_toc->nucdvar);

            nvars = toc->nucdvar = 0;
            for (i = 0; i < real_toc->nucdvar; i++) {
                for (j = 0; j < NELMTS(Intercept); j++) {
                    if (strchr(Intercept[j].r, ':'))
                        continue;
                    if (!strcmp(real_toc->ucdvar_names[i], Intercept[j].r)) {
                        break;
                    }
                }
                if (j >= NELMTS(Intercept)) {
                    toc->ucdvar_names[nvars++] =
                        STRDUP(real_toc->ucdvar_names[i]);
                }
            }
            toc->nucdvar = nvars;
            break;

    }

    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_SetDir
 *
 * Purpose:     The underlying file does not have directories.  This filter
 *              supports virtual directories by modifying the table of
 *              contents based on which virtual directory is current.
 *              Directories look like UNIX directories.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 12:40:44 EST 1995
 *
 * Modifications:
 *    Eric Brugger, Mon Nov 11 09:14:13 PST 1996
 *    I modified the routine to match the new directory structure.
 *
 *-------------------------------------------------------------------------
 */
static int
f_ale3d_SetDir(DBfile *dbfile, char *path)
{
    int            dirid;
    char          *me = "f_ale3d_SetDir";
    char          *s, *t, work[1024];

    if (FILTER_ID(dbfile, me) < 0)
        return -1;

    strncpy(work, path, sizeof(work));
    work[sizeof(work) - 1] = '\0';
    if ('/' == work[0]) {
        dirid = DIR_ROOT;
        s = work + 1;
    }
    else {
        dirid = dbfile->pub.dirid;
        s = work;
    }

    for ( /*void */ ; (t = strtok(s, "/")); s = NULL) {
        if (!strcmp(t, ".")) {
            /*nothing */
        }
        else if (!strcmp(t, "..")) {
            switch (dirid) {
                case DIR_ROOT:
                case DIR_NODE:
                case DIR_BRICK:
                case DIR_SHELL:
                case DIR_OTHER:
                    dirid = DIR_ROOT;
                    break;
                case DIR_BRICK_HYDRO:
                case DIR_BRICK_STRESS:
                    dirid = DIR_BRICK;
                    break;
                case DIR_SHELL_LOWER:
                case DIR_SHELL_MIDDLE:
                case DIR_SHELL_UPPER:
                case DIR_SHELL_OTHER:
                    dirid = DIR_SHELL;
                    break;
            }
        }
        else if (!strcmp(t, "node")) {
            if (DIR_ROOT != dirid) {
                db_perror(path, E_NOTDIR, me);
                return -1;
            }
            dirid = DIR_NODE;
        }
        else if (!strcmp(t, "brick")) {
            if (DIR_ROOT != dirid) {
                db_perror(path, E_NOTDIR, me);
                return -1;
            }
            dirid = DIR_BRICK;
        }
        else if (!strcmp(t, "shell")) {
            if (DIR_ROOT != dirid) {
                db_perror(path, E_NOTDIR, me);
                return -1;
            }
            dirid = DIR_SHELL;
        }
        else if (!strcmp(t, "other")) {
            if (DIR_ROOT == dirid) {
                dirid = DIR_OTHER;
            }
            else if (DIR_SHELL == dirid) {
                dirid = DIR_SHELL_OTHER;
            }
            else {
                db_perror(path, E_NOTDIR, me);
                return -1;
            }
        }
        else if (!strcmp(t, "hydro")) {
            if (DIR_BRICK != dirid) {
                db_perror(path, E_NOTDIR, me);
                return -1;
            }
            dirid = DIR_BRICK_HYDRO;
        }
        else if (!strcmp(t, "stress")) {
            if (DIR_BRICK != dirid) {
                db_perror(path, E_NOTDIR, me);
                return -1;
            }
            dirid = DIR_BRICK_STRESS;
        }
        else if (!strcmp(t, "lower")) {
            if (DIR_SHELL != dirid) {
                db_perror(path, E_NOTDIR, me);
                return -1;
            }
            dirid = DIR_SHELL_LOWER;
        }
        else if (!strcmp(t, "middle")) {
            if (DIR_SHELL != dirid) {
                db_perror(path, E_NOTDIR, me);
                return -1;
            }
            dirid = DIR_SHELL_MIDDLE;
        }
        else if (!strcmp(t, "upper")) {
            if (DIR_SHELL != dirid) {
                db_perror(path, E_NOTDIR, me);
                return -1;
            }
            dirid = DIR_SHELL_UPPER;
        }
        else {
            db_perror(path, E_NOTDIR, me);
            return -1;
        }
    }
    dbfile->pub.dirid = dirid;
    return DBNewToc(dbfile);
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_SetDirID
 *
 * Purpose:     Set the current virtual directory based on the specified
 *              directory ID number.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 12:55:28 EST 1995
 *
 * Modifications:
 *    Eric Brugger, Mon Nov 11 09:14:13 PST 1996
 *    I modified the routine to match the new directory structure.
 *
 *-------------------------------------------------------------------------
 */
static int
f_ale3d_SetDirID(DBfile *dbfile, int dirid)
{
    char          *me = "f_ale3d_SetDirID";

    if (FILTER_ID(dbfile, me) < 0)
        return -1;
    switch (dirid) {
        case DIR_ROOT:
        case DIR_NODE:
        case DIR_BRICK:
        case DIR_SHELL:
        case DIR_BRICK_HYDRO:
        case DIR_BRICK_STRESS:
        case DIR_SHELL_LOWER:
        case DIR_SHELL_MIDDLE:
        case DIR_SHELL_UPPER:
        case DIR_OTHER:
            if (dbfile->pub.dirid != dirid) {
                dbfile->pub.dirid = dirid;
                DBNewToc(dbfile);
            }
            else {
                dbfile->pub.dirid = dirid;
            }
            break;
        default:
            return db_perror(NULL, E_NOTDIR, me);
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_GetDir
 *
 * Purpose:     Return the name of the current virtual directory in a
 *              user-supplied buffer of at least 16 bytes.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Tue Mar  7 13:02:08 EST 1995
 *
 * Modifications:
 *    Eric Brugger, Mon Nov 11 09:14:13 PST 1996
 *    I modified the routine to match the new directory structure.
 *
 *-------------------------------------------------------------------------
 */
static int
f_ale3d_GetDir(DBfile *dbfile, char *path /*output */)
{
    char          *me = "f_ale3d_GetDir";

    if (FILTER_ID(dbfile, me) < 0)
        return -1;
    switch (dbfile->pub.dirid) {
        case DIR_ROOT:
            strcpy(path, "/");
            break;
        case DIR_NODE:
            strcpy(path, "/node");
            break;
        case DIR_BRICK:
            strcpy(path, "/brick");
            break;
        case DIR_SHELL:
            strcpy(path, "/shell");
            break;
        case DIR_OTHER:
            strcpy(path, "/other");
            break;
        case DIR_BRICK_HYDRO:
            strcpy(path, "/brick/hydro");
            break;
        case DIR_BRICK_STRESS:
            strcpy(path, "/brick/hydro");
            break;
        case DIR_SHELL_LOWER:
            strcpy(path, "/shell/lower");
            break;
        case DIR_SHELL_MIDDLE:
            strcpy(path, "/shell/middle");
            break;
        case DIR_SHELL_UPPER:
            strcpy(path, "/shell/upper");
            break;
        case DIR_SHELL_OTHER:
            strcpy(path, "/shell/other");
            break;
        default:
            return db_perror("internal directory error", E_NOTDIR, me);
    }
    return 0;
}

/*-------------------------------------------------------------------------
 * Function:    calc_magnitude
 *
 * Purpose:     Given the data for the three components of a vector in the
 *              `data' parameter, calculate the magnitude for the indicated
 *              number of items.
 *
 * Return:      void
 *
 * Programmer:  brugger@kickit
 *              Thu Mar 13 11:12:45 PST 1997
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Tue Nov 23 09:51:22 PST 1999
 *    Changed to ANSI-style prototype.
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
static void
calc_magnitude(float *result, float **data, int length, int extra)
{
    int            i;

    for (i = 0; i < length; i++) {
        result[i] = sqrt(data[0][i] * data[0][i] +
                         data[1][i] * data[1][i] +
                         data[2][i] * data[2][i]);
    }
}

/*-------------------------------------------------------------------------
 * Function:    calc_j2
 *
 * Purpose:     Given the data for sx, sy, sz, txy, tyz, tzx in the `data'
 *              parameter, calculate j2 for the indicated number of items.
 *
 * Return:      void
 *
 * Programmer:  brugger@kickit
 *              Thu Mar 13 11:12:45 PST 1997
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Tue Nov 23 09:51:22 PST 1999
 *    Changed to ANSI-style prototype.
 *-------------------------------------------------------------------------*/
/* ARGSUSED */
static void
calc_j2(float *result, float **data, int length, int extra)
{
    int            i;

    /*
     * data[0] = dev_stress_xx
     * data[1] = dev_stress_yy
     * data[2] = dev_stress_zz
     * data[3] = tot_stress_xy
     * data[4] = tot_stress_yz
     * data[5] = tot_stress_zx
     */
    for (i = 0; i < length; i++) {
        result[i] = sqrt(2.0 * data[0][i] * data[0][i] +
                         2.0 * data[1][i] * data[1][i] +
                         2.0 * data[2][i] * data[2][i] +
                         2.0 * data[3][i] * data[3][i] +
                         2.0 * data[4][i] * data[4][i] +
                         2.0 * data[5][i] * data[5][i]);
    }
}

/*-------------------------------------------------------------------------
 * Function:    calc_brick
 *
 * Purpose:     Given the data required to calculate a given brick
 *              variable in the `data' parameter, calculate the variable
 *              for the indicated number of items.
 *
 * Return:      void
 *
 * Programmer:  brugger@kickit
 *              Fri Mar 14 11:55:03 PST 1997
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Tue Nov 23 09:39:49 PST 1999
 *    Changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
static void
calc_brick(float *result, float **data, int length, int extra)
{
    int            i;
    double         aa, bb, cc, dd, angp;

    switch (extra) {
        case BRICK_TOT_STRESS_XX:
        case BRICK_TOT_STRESS_YY:
        case BRICK_TOT_STRESS_ZZ:
            /*
             * data[0] = dev_stress_xx
             * data[1] = pressure
             */
            for (i = 0; i < length; i++) {
                result[i] = data[0][i] - data[1][i];
            }
            break;
        case BRICK_VON_MISES:
            /*
             * data[0] = dev_stress_xx
             * data[1] = dev_stress_yy
             * data[2] = dev_stress_zz
             * data[3] = tot_stress_xy
             * data[4] = tot_stress_yz
             * data[5] = tot_stress_zx
             */
            for (i = 0; i < length; i++) {
                result[i] = sqrt(3.0 * fabs(data[3][i] * data[3][i] +
                                            data[4][i] * data[4][i] +
                                            data[5][i] * data[5][i] -
                                            data[0][i] * data[1][i] -
                                            data[1][i] * data[2][i] -
                                            data[2][i] * data[0][i]));
            }
            break;
        default:
            /*
             * data[0] = dev_stress_xx
             * data[1] = dev_stress_yy
             * data[2] = dev_stress_zz
             * data[3] = tot_stress_xy
             * data[4] = tot_stress_yz
             * data[5] = tot_stress_zx
             * data[6] = pressure
             */
            for (i = 0; i < length; i++) {
                aa = SQR(data[3][i])
                    + SQR(data[4][i])
                    + SQR(data[5][i])
                    - data[0][i] * data[1][i]
                    - data[1][i] * data[2][i]
                    - data[2][i] * data[0][i];
                bb = data[0][i] * SQR(data[4][i])
                    + data[1][i] * SQR(data[5][i])
                    + data[2][i] * SQR(data[3][i])
                    - data[0][i] * data[1][i] * data[2][i]
                    - data[3][i] * data[4][i] * data[5][i] * 2.0;

                if (aa < 1.0e-25) {
                    result[i] = 0.0;
                }
                else {
                    cc = -sqrt(27.0 / aa) * bb * 0.5 / aa;
                    cc = MAX(MIN(cc, 1.0), -1.0);
                    angp = acos(cc) / 3.0;
                    switch (extra) {
                        case BRICK_PRINC_DEV_STRESS_1:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp);
                            break;
                        case BRICK_PRINC_DEV_STRESS_2:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp + FTPI);
                            break;
                        case BRICK_PRINC_DEV_STRESS_3:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp + TTPI);
                            break;
                        case BRICK_PRINC_TOT_STRESS_1:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp) - data[6][i];
                            break;
                        case BRICK_PRINC_TOT_STRESS_2:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp + FTPI) - data[6][i];
                            break;
                        case BRICK_PRINC_TOT_STRESS_3:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp + TTPI) - data[6][i];
                            break;
                        case BRICK_MAX_SHEAR_STRESS:
                            dd = sqrt(aa / 3.0);
                            result[i] = dd * (cos(angp) - cos(angp + TTPI));
                            break;
                    }
                }
            }
            break;
    }
}

/*-------------------------------------------------------------------------
 * Function:    calc_shell
 *
 * Purpose:     Given the data required to calculate a given shell
 *              variable in the `data' parameter, calculate the variable
 *              for the indicated number of items.
 *
 * Return:      void
 *
 * Programmer:  brugger@kickit
 *              Fri Mar 14 12:46:11 PST 1997
 *
 * Modifications:
 *    Sam Wookey, Mon Jun  8 13:39:53 PDT 1998
 *    Added a loop to the pressure and von_mises sections.
 *
 *    Lisa J. Roberts, Tue Nov 23 09:39:49 PST 1999
 *    Changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
static void
calc_shell(float *result, float **data, int length, int extra)
{
    int            i;
    double         pr, aa, bb, cc, dd, angp;

    switch (extra) {
        case SHELL_PRESSURE:
            /*
             * data[0] = tot_stress_xx
             * data[1] = tot_stress_yy
             * data[2] = tot_stress_zz
             */
            for(i = 0; i < length; ++i)
            {
                result[i] = - (data[0][i] + data[1][i] + data[2][i]) / 3.0;
            }
            break;
        case SHELL_VON_MISES:
            /*
             * data[0] = tot_stress_xx
             * data[1] = tot_stress_yy
             * data[2] = tot_stress_zz
             * data[3] = tot_stress_xy
             * data[4] = tot_stress_yz
             * data[5] = tot_stress_zx
             */
            for(i = 0; i < length; ++i)
            {
                pr = - (data[0][i] + data[1][i] + data[2][i]) / 3.0;
                data[0][i] += pr;
                data[1][i] += pr;
                data[2][i] += pr;
                result[i] = sqrt(3 * fabs(data[3][i] * data[3][i] +
                                          data[4][i] * data[4][i] +
                                          data[5][i] * data[5][i] -
                                          data[0][i] * data[1][i] -
                                          data[1][i] * data[2][i] -
                                          data[2][i] * data[0][i]));
            }
            break;
        default:
            /*
             * data[0] = tot_stress_xx
             * data[1] = tot_stress_yy
             * data[2] = tot_stress_zz
             * data[3] = tot_stress_xy
             * data[4] = tot_stress_yz
             * data[5] = tot_stress_zx
             */
            for (i = 0; i < length; i++) {
                pr = (data[0][i] + data[1][i] + data[2][i]) / (-3.0);
                data[0][i] += pr;
                data[1][i] += pr;
                data[2][i] += pr;
                aa = SQR(data[3][i])
                    + SQR(data[4][i])
                    + SQR(data[5][i])
                    - data[0][i] * data[1][i]
                    - data[1][i] * data[2][i]
                    - data[2][i] * data[0][i];
                bb = data[0][i] * SQR(data[4][i])
                    + data[1][i] * SQR(data[5][i])
                    + data[2][i] * SQR(data[3][i])
                    - data[0][i] * data[1][i] * data[2][i]
                    - data[3][i] * data[4][i] * data[5][i] * 2.0;

                if (aa < 1.0e-25) {
                    result[i] = 0.0;
                }
                else {
                    cc = -sqrt(27.0 / aa) * bb * 0.5 / aa;
                    cc = MAX(MIN(cc, 1.0), -1.0);
                    angp = acos(cc) / 3.0;
                    switch (extra) {
                        case SHELL_PRINC_DEV_STRESS_1:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp);
                            break;
                        case SHELL_PRINC_DEV_STRESS_2:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp + FTPI);
                            break;
                        case SHELL_PRINC_DEV_STRESS_3:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp + TTPI);
                            break;
                        case SHELL_PRINC_TOT_STRESS_1:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp) - pr;
                            break;
                        case SHELL_PRINC_TOT_STRESS_2:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp + FTPI) - pr;
                            break;
                        case SHELL_PRINC_TOT_STRESS_3:
                            dd = 2.0 * sqrt(aa / 3.0);
                            result[i] = dd * cos(angp + TTPI) - pr;
                            break;
                        case SHELL_MAX_SHEAR_STRESS:
                            dd = sqrt(aa / 3.0);
                            result[i] = dd * (cos(angp) - cos(angp + TTPI));
                            break;
                    }
                }
            }
            break;
    }
}

/*-------------------------------------------------------------------------
 * Function:    calc_shell_other
 *
 * Purpose:     Given the data required to calculate a given shell/other
 *              variable in the `data' parameter, calculate the variable
 *              for the indicated number of items.
 *
 * Return:      void
 *
 * Programmer:  brugger@kickit
 *              Thu Mar 13 11:12:45 PST 1997
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Tue Nov 23 09:39:49 PST 1999
 *    Changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
static void
calc_shell_other(float *result, float **data, int length, int extra)
{
    int            i;
    double         t1, t2, t3, t4, t5;

    switch (extra)
    {
        case SHELL_SURFACE_STRESS_1:
        case SHELL_SURFACE_STRESS_3:
        case SHELL_SURFACE_STRESS_5:
            /*
             * data[0] = m_xx_bending, m_yy_bending, m_xy_bending
             * data[1] = n_xx_normal, n_yy_normal, n_xy_normal
             * data[2] = thickness
             */
            for (i = 0; i < length; i++) {
                result[i] = (data[1][i] / data[2][i]) + 6.0 *
                    (data[0][i] / (data[2][i] * data[2][i]));
                i++;
            }
            break;
        case SHELL_SURFACE_STRESS_2:
        case SHELL_SURFACE_STRESS_4:
        case SHELL_SURFACE_STRESS_6:
            /*
             * data[0] = m_xx_bending, m_yy_bending, m_xy_bending
             * data[1] = n_xx_normal, n_yy_normal, n_xy_normal
             * data[2] = thickness
             */
            for (i = 0; i < length; i++) {
                result[i] = (data[1][i] / data[2][i]) - 6.0 *
                    (data[0][i] / (data[2][i] * data[2][i]));
                i++;
            }
            break;
        case SHELL_EFF_UPPER_STRESS:
            /*
             * data[0] = m_xx_bending
             * data[1] = m_yy_bending
             * data[2] = m_xy_bending
             * data[3] = n_xx_normal
             * data[4] = n_yy_normal
             * data[5] = n_xy_normal
             * data[6] = thickness
             */
            for (i = 0; i < length; i++) {
                t1 = (data[3][i] / data[6][i]) + 6.0 *
                    (data[0][i] / (data[6][i] * data[6][i]));
                t2 = (data[4][i] / data[6][i]) + 6.0 *
                    (data[1][i] / (data[6][i] * data[6][i]));
                t3 = (data[5][i] / data[5][i]) + 6.0 *
                    (data[2][i] / (data[6][i] * data[6][i]));
                result[i] = sqrt(t1 * t1 - t1 * t2 + t2 * t2 + 3.0 * t3 * t3);
                i++;
            }
            break;
        case SHELL_EFF_LOWER_STRESS:
            /*
             * data[0] = m_xx_bending
             * data[1] = m_yy_bending
             * data[2] = m_xy_bending
             * data[3] = n_xx_normal
             * data[4] = n_yy_normal
             * data[5] = n_xy_normal
             * data[6] = thickness
             */
            for (i = 0; i < length; i++) {
                t1 = (data[3][i] / data[6][i]) - 6.0 *
                    (data[0][i] / (data[6][i] * data[6][i]));
                t2 = (data[4][i] / data[6][i]) - 6.0 *
                    (data[1][i] / (data[6][i] * data[6][i]));
                t3 = (data[5][i] / data[6][i]) - 6.0 *
                    (data[2][i] / (data[6][i] * data[6][i]));
                result[i] = sqrt(t1 * t1 - t1 * t2 + t2 * t2 + 3.0 * t3 * t3);
                i++;
            }
            break;
        case SHELL_EFF_MAX_STRESS:
            /*
             * data[0] = m_xx_bending
             * data[1] = m_yy_bending
             * data[2] = m_xy_bending
             * data[3] = n_xx_normal
             * data[4] = n_yy_normal
             * data[5] = n_xy_normal
             * data[6] = thickness
             */
            for (i = 0; i < length; i++) {
                t1 = (data[3][i] / data[6][i]) + 6.0 *
                    (data[0][i] / (data[6][i] * data[6][i]));
                t2 = (data[4][i] / data[6][i]) + 6.0 *
                    (data[1][i] / (data[6][i] * data[6][i]));
                t3 = (data[5][i] / data[6][i]) + 6.0 *
                    (data[2][i] / (data[6][i] * data[6][i]));
                t4 = sqrt(t1 * t1 - t1 * t2 + t2 * t2 + 3.0 * t3 * t3);
                t1 = (data[3][i] / data[6][i]) - 6.0 *
                    (data[0][i] / (data[6][i] * data[6][i]));
                t2 = (data[4][i] / data[6][i]) - 6.0 *
                    (data[1][i] / (data[6][i] * data[6][i]));
                t3 = (data[5][i] / data[6][i]) - 6.0 *
                    (data[2][i] / (data[6][i] * data[6][i]));
                t5 = sqrt(t1 * t1 - t1 * t2 + t2 * t2 + 3.0 * t3 * t3);
                result[i] = MAX(t4, t5);
                i++;
            }
            break;
    }
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_GetUcdvar
 *
 * Purpose:     Read a UCD variable.  There are three ways to read
 *              a variable.
 *
 *              (1) If the variable is not a virtual variable then it
 *              must be a real variable.  We simply call the underlying
 *              g_qv callback.
 *
 *              (2) If the variable is a virtual variable and the real
 *              dependency list contains only one item (ie, no `:') then
 *              the virtual name is just an alias for the real name.
 *
 *              (3) If the variable is a virtual variable and the
 *              real dependency list contains more than one item (ie, a `:'
 *              appears in the real name) then some calculations need
 *              to be performed in a manner similar to the Taurus driver.
 *
 * Return:      Success:        ptr to a new variable.
 *
 *              Failure:        NULL
 *
 * Programmer:  robb@cloud
 *              Wed Mar  8 17:54:34 EST 1995
 *
 * Modifications:
 *    Robb Matzke, Thu Mar 16 13:10:29 EST 1995
 *    We call `satisfied' to determine which (if any) dependency list
 *    is being used for the specified virtual variable.
 *
 *    Eric Brugger, Fri Jun  2 08:10:43 PDT 1995
 *    I made several modifications so that the sun compiler liked it.
 *
 *    Robb Matzke, Tue May 14 11:15:00 EST 1996
 *    Fixed an assert() so as not to cause a compiler warning.
 *
 *    Eric Brugger, Thu Mar 13 13:12:30 PST 1997
 *    I modified the routine to read dtime instead of time, since dtime
 *    is always written out by ale3d.  I also modified the routine to
 *    set the centering based on the directory, not always nodal as had
 *    been done before.
 *
 *-------------------------------------------------------------------------
 */
static DBucdvar *
f_ale3d_GetUcdvar(DBfile *dbfile, char *name)
{
    static int     sequence = 0;
    int            id, inter, ndeps, i, j, size, offset, slice_size, stride;
    int            slice_stride = MAXBUF, type, *types;
    char          *s, *t, *s_data[16], work[256];
    char          *me = "f_ale3d_GetUcdvar";
    DBucdvar      *uv;
    float        **data, *result;
    double        *temp_doubles = NULL;

#if 1
    dbfile->pub.r_varslice = NULL;
#endif

    /*
     * Is this one that we should handle?  If not, do things
     * the normal way.  We should also make sure that the variable
     * is in the current virtual directory.
     */
    if ((id = FILTER_ID(dbfile, me)) < 0)
        return NULL;
    if (!v_exists(dbfile->pub.toc, name)) {
        db_perror(name, E_NOTFOUND, me);
        return NULL;
    }
    if (!satisfied(id, name, work, &inter)) {
        return FILTER_CALL(f_ale3d_cb[id].g_uv, (dbfile, name),
                           (DBucdvar *) NULL, me);
    }

    /*
     * If it is found and it is simply another name for a real variable,
     * read the real variable instead.
     */
    if (!strchr(work, ':')) {
        uv = FILTER_CALL(f_ale3d_cb[id].g_uv, (dbfile, work),
                         (DBucdvar *) NULL, me);
        if (uv) {
            FREE(uv->name);
            uv->name = STRDUP(name);
        }
        return uv;
    }

    /*
     * Otherwise we must do some calculations.  We calculate in chunks so
     * we don't use so much memory.  The data for dependency variables are
     * stored as miscellaneous variables with the name `%s_data'
     */
    if (!Intercept[inter].f) {
        db_perror("no calculation function defined", E_INTERNAL, me);
        return NULL;
    }

    /*
     * Count the number of dependency (real) variables.
     */
    for (ndeps = 0, s = work; s; s = strchr(s + 1, ':'))
        ndeps++;
    if (ndeps > NELMTS(s_data)) {
        db_perror("too many dependencies", E_INTERNAL, me);
        return NULL;
    }

    /*
     * Compute the names "%s_data" of the variables and make sure they
     * are DB_FLOAT or DB_DOUBLE.  Also make sure they are all the same size.
     */
    types = ALLOC_N(int, ndeps);
    for (i = 0, s = work; i < ndeps; i++, s = NULL) {
        int ale3d_filter_internal_error = 1;
        t = strtok(s, ":");
        assert(ale3d_filter_internal_error && t != NULL);
        s_data[i] = ALLOC_N(char, strlen(t) + 6);

        sprintf(s_data[i], "%s_data", t);
    }
    for (i = 0, size = (-1); i < ndeps; i++) {
        if (DB_FLOAT != (type = DBGetVarType(dbfile, s_data[i])) &&
            DB_DOUBLE != type) {
            char           mesg[64];

            sprintf(mesg, "expecting a DB_FLOAT (%d) or DB_DOUBLE (%d) type for `%s' (got %d)",
                    DB_FLOAT, DB_DOUBLE, s_data[i], type);
            db_perror(mesg, E_INTERNAL, me);
            return NULL;
        }
        types[i] = type;
        if (size < 0) {
            size = DBGetVarLength(dbfile, s_data[i]);
        }
        else if (size != DBGetVarLength(dbfile, s_data[i])) {
            db_perror("mismatched variable sizes", E_INTERNAL, me);
            return NULL;
        }
    }

    /*
     * Build the UCD variable structure and initialize it.
     */
    uv = DBAllocUcdvar();
    uv->id = sequence++;
    uv->meshid = sequence++;
    uv->name = STRDUP(name);
    uv->units = NULL;
    uv->label = NULL;
    uv->vals = ALLOC_N(void *, 1);
    uv->nels = size;
    uv->datatype = DB_FLOAT;
    uv->nvals = 1;
    uv->ndims = 3;
    uv->origin = 0;
    if (Intercept[inter].dir == DIR_NODE)
        uv->centering = DB_NODECENT;
    else
        uv->centering = DB_ZONECENT;
    DBReadVar(dbfile, "cycle", &(uv->cycle));
    DBReadVar(dbfile, "dtime", &(uv->time));

    /*
     * Allocate work space for holding slices of the variables as well as
     * the entire final result.  If the r_varslice callback is not defined
     * then we can't be efficient about memory.  We will just read the
     * whole darn thing at once with DBReadVar.
     */
    if (!dbfile->pub.r_varslice)
        slice_stride = size;
    data = ALLOC_N(float *, ndeps);

    for (i = 0; i < ndeps; i++)
        data[i] = ALLOC_N(float, slice_stride);
    uv->vals[0] = result = ALLOC_N(float, size);

    /*
     * Do the calculations slice by slice.  A particular slice is `slice_size'
     * elements long while slices in general (all but the last) are
     * `slice_stride' elements.  Watch out! Some variables might be DB_DOUBLE
     * and should be converted to DB_FLOAT.
     */
    for (offset = 0; offset < size; offset += slice_stride) {
        slice_size = MIN(size - offset, slice_stride);
        stride = 1;
        for (i = 0; i < ndeps; i++) {
            if (dbfile->pub.r_varslice) {
                if (DB_DOUBLE == types[i]) {
                    /*
                     * Double values must be read as doubles and then
                     * converted to single.
                     */
                    if (!temp_doubles) {
                        temp_doubles = ALLOC_N(double, slice_stride);
                    }
                    if (DBReadVarSlice(dbfile, s_data[i], &offset,
                                       &slice_size, &stride, 1,
                                       temp_doubles) < 0) {
                        return NULL;
                    }
                    for (j = 0; j < slice_size; j++) {
                        data[i][j] = temp_doubles[j];
                    }
                }
                else if (DBReadVarSlice(dbfile, s_data[i], &offset,
                                        &slice_size, &stride, 1,
                                        data[i]) < 0) {
                    return NULL;
                }
            }
            else if (DB_DOUBLE == types[i]) {
                /*
                 * Double values must be read as doubles and then
                 * converted to single.
                 */
                if (!temp_doubles) {
                    temp_doubles = ALLOC_N(double, slice_stride);
                }
                if (DBReadVar(dbfile, s_data[i], temp_doubles) < 0) {
                    return NULL;
                }
                for (j = 0; j < slice_size; j++) {
                    data[i][j] = temp_doubles[j];
                }
            }
            else if (DBReadVar(dbfile, s_data[i], data[i]) < 0) {
                return NULL;
            }
        }
        (Intercept[inter].f) (result + offset, data, slice_size,
                              Intercept[inter].extra);
    }

    /*
     * Free work space.
     */
    for (i = 0; i < ndeps; i++)
        FREE(data[i]);
    FREE(data);
    FREE(temp_doubles);

    return uv;
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_InqMeshType
 *
 * Purpose:     Returns the mesh type of the specified mesh.
 *
 * Return:      Success:        mesh type
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Thu Apr 20 10:57:42 PDT 1995
 *
 * Modifications:
 *
 *      Robb Matzke, 14 May 1996
 *      Returns -1 on failure.
 *
 *-------------------------------------------------------------------------
 */
static int
f_ale3d_InqMeshType(DBfile *dbfile, char *name)
{
    int            id, inter;
    char          *me = "f_ale3d_InqMeshType";
    char           work[256], *s;

    /*
     * Is this one that we should handle?  If not, do things
     * the normal way.  We should also make sure that the variable
     * is in the current virtual directory.
     */
    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1 ;
    if (!v_exists(dbfile->pub.toc, name) ||
        !satisfied(id, name, work, &inter)) {
        return FILTER_CALL(f_ale3d_cb[id].i_meshtype, (dbfile, name), -1, me);
    }

    /*
     * If it is found and it is simply another name for a real variable,
     * return the mesh type of the real variable instead.
     */
    if (!strchr(work, ':')) {
        return FILTER_CALL(f_ale3d_cb[id].i_meshtype, (dbfile, work), -1, me);
    }

    /*
     * Otherwise, just use the first variable from the dependency list.
     */
    s = strtok(work, ":");
    return FILTER_CALL(f_ale3d_cb[id].i_meshtype, (dbfile, s), -1, me);
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_InqMeshName
 *
 * Purpose:     Returns the mesh name associated with a variable.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  matzke@viper
 *              Thu Apr 20 10:57:42 PDT 1995
 *
 * Modifications:
 *
 *      Robb Matzke, 14 May 1996
 *      Fixed the return type so as not to get a compiler warning.  Also
 *      fixed error return values to -1.
 *
 *-------------------------------------------------------------------------
 */
static int
f_ale3d_InqMeshName(DBfile *dbfile, char *name, char *meshname /*OUTPUT */)
{
    int            id, inter;
    char          *me = "f_ale3d_InqMeshName";
    char           work[256], *s;

    /*
     * Is this one that we should handle?  If not, do things
     * the normal way.  We should also make sure that the variable
     * is in the current virtual directory.
     */
    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1 ;

    if (!v_exists(dbfile->pub.toc, name) ||
        !satisfied(id, name, work, &inter)) {
        return FILTER_CALL(f_ale3d_cb[id].i_meshname, (dbfile, name, meshname),
                           -1, me);
    }

    /*
     * If it is found and it is simply another name for a real variable,
     * return the mesh type of the real variable instead.
     */
    if (!strchr(work, ':')) {
        return FILTER_CALL(f_ale3d_cb[id].i_meshname, (dbfile, work, meshname),
                           -1, me);
    }

    /*
     * Otherwise, just use the first variable from the dependency list.
     */
    s = strtok(work, ":");
    return FILTER_CALL(f_ale3d_cb[id].i_meshname, (dbfile, s, meshname), -1, me);
}

/*-------------------------------------------------------------------------
 * Function:    f_ale3d_Open
 *
 * Purpose:     Install the ALE3D filter for the specified database and
 *              initialize local table entries.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  robb@cloud
 *              Mon Mar  6 18:05:42 EST 1995
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Tue Nov 23 09:39:49 PST 1999
 *    Changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
int
f_ale3d_Open(DBfile *dbfile, char *filter_name)
{
    int            id;
    char          *me = "f_ale3d_open";

    if ((id = FILTER_ID(dbfile, me)) < 0)
        return -1;
    if (!filter_name || !*filter_name)
        filter_name = "ALE3D-FILTER";
    if (f_ale3d_name[id]) {
        char           mesg[1024];

        sprintf(mesg,
                "filter `%s' inserted into database `%s' more than once",
                filter_name, dbfile->pub.name);
        db_perror(mesg, E_NOTIMP, me);
        return -1;
    }

    f_ale3d_name[id] = safe_strdup(filter_name);
    memcpy(f_ale3d_cb + id, &(dbfile->pub), sizeof(DBfile_pub));
    f_ale3d_cb[id].toc = NULL;

    /*
     * Install conditional filters.  They are only installed
     * if that functionality previously existed.
     */
    FILTER_CB(module, f_ale3d_Filters);
    FILTER_CB(close, f_ale3d_Close);
    FILTER_CB(newtoc, f_ale3d_NewToc);
    FILTER_CB(cd, f_ale3d_SetDir);
    FILTER_CB(cdid, f_ale3d_SetDirID);
    FILTER_CB(g_dir, f_ale3d_GetDir);
    FILTER_CB(g_uv, f_ale3d_GetUcdvar);
    FILTER_CB(i_meshtype, f_ale3d_InqMeshType);
    FILTER_CB(i_meshname, f_ale3d_InqMeshName);

    /*
     * Install unconditional filters.
     */
    dbfile->pub.uninstall = f_ale3d_Uninstall;

    /*
     * Initialize the directory and table of contents.
     */
    DBNewToc(dbfile);
    DBSetDir(dbfile, "/");

    return 0;
}
