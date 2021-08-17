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
#ifndef TAURUS_H
#define TAURUS_H

#define NODAL_VAR 0
#define ZONAL_VAR 1

#define MAX_MESH  5
#define MAX_VAL  69

#define VAR_NORMAL          0
#define VAR_SIGX            1
#define VAR_SIGY            2
#define VAR_SIGZ            3
#define VAR_SIGXY           4
#define VAR_SIGYZ           5
#define VAR_SIGZX           6
#define VAR_EPS             7
#define VAR_PRESSURE        8
#define VAR_SIG_EFF         9
#define VAR_DEV_STRESS_1   10
#define VAR_DEV_STRESS_2   11
#define VAR_DEV_STRESS_3   12
#define VAR_MAX_SHEAR_STR  13
#define VAR_PRINC_STRESS_1 14
#define VAR_PRINC_STRESS_2 15
#define VAR_PRINC_STRESS_3 16
#define VAR_DISPX          17
#define VAR_DISPY          18
#define VAR_DISPZ          19
#define VAR_DISP_MAG       20
#define VAR_VEL_MAG        21
#define VAR_ACC_MAG        22
#define VAR_SURF_STRESS_1  23
#define VAR_SURF_STRESS_2  24
#define VAR_SURF_STRESS_3  25
#define VAR_SURF_STRESS_4  26
#define VAR_SURF_STRESS_5  27
#define VAR_SURF_STRESS_6  28
#define VAR_UP_STRESS      29
#define VAR_LOW_STRESS     30
#define VAR_MAX_STRESS     31
#define VAR_VORT_MAG       32

/*
 * Dyna3d and Nike3d values.
 */
#define VAL_HEX_SIGX         0
#define VAL_HEX_SIGY         1
#define VAL_HEX_SIGZ         2
#define VAL_HEX_SIGXY        3
#define VAL_HEX_SIGYZ        4
#define VAL_HEX_SIGZX        5
#define VAL_HEX_EPS_EFF      6
#define VAL_SHELL_MID_SIGX   7
#define VAL_SHELL_MID_SIGY   8
#define VAL_SHELL_MID_SIGZ   9
#define VAL_SHELL_MID_SIGXY 10
#define VAL_SHELL_MID_SIGYZ 11
#define VAL_SHELL_MID_SIGZX 12
#define VAL_SHELL_MID_EPS_EFF 13
#define VAL_SHELL_IN_SIGX   14
#define VAL_SHELL_IN_SIGY   15
#define VAL_SHELL_IN_SIGZ   16
#define VAL_SHELL_IN_SIGXY  17
#define VAL_SHELL_IN_SIGYZ  18
#define VAL_SHELL_IN_SIGZX  19
#define VAL_SHELL_IN_EPS_EFF 20
#define VAL_SHELL_OUT_SIGX  21
#define VAL_SHELL_OUT_SIGY  23
#define VAL_SHELL_OUT_SIGZ  24
#define VAL_SHELL_OUT_SIGXY 25
#define VAL_SHELL_OUT_SIGYZ 26
#define VAL_SHELL_OUT_SIGZX 27
#define VAL_SHELL_OUT_EPS_EFF 28
#define VAL_SHELL_RES1      29
#define VAL_SHELL_RES2      30
#define VAL_SHELL_RES3      31
#define VAL_SHELL_RES4      32
#define VAL_SHELL_RES5      33
#define VAL_SHELL_RES6      34
#define VAL_SHELL_RES7      35
#define VAL_SHELL_RES8      36
#define VAL_SHELL_THICKNESS 37
#define VAL_SHELL_ELDEP1    38
#define VAL_SHELL_ELDEP2    39
#define VAL_SHELL_INT_ENG   40
#define VAL_SHELL_EPSX_IN   41
#define VAL_SHELL_EPSY_IN   42
#define VAL_SHELL_EPSZ_IN   43
#define VAL_SHELL_EPSXY_IN  44
#define VAL_SHELL_EPSYZ_IN  45
#define VAL_SHELL_EPSZX_IN  46
#define VAL_SHELL_EPSX_OUT  47
#define VAL_SHELL_EPSY_OUT  48
#define VAL_SHELL_EPSZ_OUT  49
#define VAL_SHELL_EPSXY_OUT 50
#define VAL_SHELL_EPSYZ_OUT 51
#define VAL_SHELL_EPSZX_OUT 52
#define VAL_COORDX          53
#define VAL_COORDY          54
#define VAL_COORDZ          55
#define VAL_VELX            56
#define VAL_VELY            57
#define VAL_VELZ            58
#define VAL_ACCX            59
#define VAL_ACCY            60
#define VAL_ACCZ            61
#define VAL_TEMPX           62
#define VAL_TEMPY           63
#define VAL_TEMPZ           64

/*
 * Topaz3d values.
 */
#define VAL_TEMP             0
#define VAL_FLUXX            1
#define VAL_FLUXY            2
#define VAL_FLUXZ            3

/*
 * Hydra values.
 *
 * Note: VAL_VELX, VAL_VELY and VAL_VELZ already defined above.
 *
 */
#define VAL_VORTX            0
#define VAL_VORTY            1
#define VAL_VORTZ            2
#define VAL_PRESSURE         3

typedef struct {
/*
 * File information.
 */
    int            ifile;       /* The number of the currently open file */
    char           title[48];   /* The title associated with the file */
    int            fd;          /* File descriptor of currently open file */
    char          *basename;    /* The file root name */
    char          *filename;    /* The name of the currently open file */
    int            nfiles;      /* The number of files in the family */
    int           *filesize;    /* The size of each file in the family */
/*
 * State information.
 */
    int            state;       /* The current state, -1 if not in one */
    int            nstates;     /* The number of states */
    int           *state_file;  /* The file that each state is in */
    int           *state_loc;   /* The address of each state */
    float         *state_time;  /* The time of each state */
    int            idir;        /* The current directory within a state */
/*
 * Variable information.
 */
    int            var_start[MAX_VAL];  /* The starting location relative to a state
                                         */
    int            var_len[MAX_VAL];  /* The length of the variable */
    int            var_offset[MAX_VAL];  /* The offset in the vector */
    int            var_ncomps[MAX_VAL];  /* The number of components to a vector */
/*
 * Mesh information.
 */
    int            mesh_read;   /* 1 if the mesh information has been read */
    int            nhex;        /* The number of hex elements */
    int            nhex_faces;  /* The number of hex face elements */
    int            nshell;      /* The number of shell elements */
    int            nbeam;       /* The number of beam elements */
    int           *hex_nodelist;  /* The node list for the hex elements */
    int           *shell_nodelist;  /* The node list for the shell elements */
    int           *beam_nodelist;  /* The node list for the beam elements */
    int           *hex_facelist;  /* The face list for the hex elements */
    int           *hex_zoneno;  /* The zone number of each hex face */
    int           *hex_matlist; /* The material list for the hex elements */
    int           *shell_matlist;  /* The material list for the shell elements */
    int           *beam_matlist;  /* The material list for the beam elements */
    int           *hex_activ;   /* The activity data for the hex elements */
    int           *shell_activ; /* The activity data for the shell elements */
    int           *beam_activ;  /* The activity data for the beam elements */
    int            coord_state; /* The state associated with the coordinates */
    float        **coords;      /* The mesh coordinates */
    float          min_extents[3];  /* The minimum extents */
    float          max_extents[3];  /* The maximum extents */
    int            nmat;        /* The number of materials present */
    int           *matnos;      /* The material numbers present */
/*
 * Header information.
 */
    int            ndim;        /* Dimension of data */
    int            numnp;       /* The number of nodes */
    int            icode;       /* Flag for old or new database */
    int            nglbv;       /* Number of global variables for each state */
    int            it;          /* Tempuratures included flag */
    int            iu;          /* Current geometry included flag */
    int            iv;          /* Current velocity flag */
    int            ia;          /* Current acceleration flag */
    int            nel8;        /* Number of brick elements */
    int            nummat8;     /* Number of materials used by brick elements */
    int            nv3d;        /* Number of variables for each brick element */
    int            nel2;        /* Number of beam elements */
    int            nummat2;     /* Number of materials used by beam elements */
    int            nv1d;        /* Number of variables for each beam element */
    int            nel4;        /* Number of shell elements */
    int            nummat4;     /* Number of materials used by shell elements */
    int            nv2d;        /* Number of variables for each shell element */
    int            activ;       /* Element activity included flag */
} TAURUSfile;

typedef struct {
    char          *name;
    char          *mesh;
    int            idir;
    int            centering;
    int            ival;
    int            ivar;
} var_list_s;

extern var_list_s taur_var_list[];

#endif
