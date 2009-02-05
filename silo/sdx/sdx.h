#ifndef SDX_H
#define SDX_H

/*

                           Copyright 1991 - 1995
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
 * sdx.h: C include file for the sdx library.
 *
 * These constants and function prototypes are for either the client
 * or server side of the SDX library and do not depend on other libraries
 * like SILO.
 */

#define SDX_LEN 64

#define SDX_CNTRLINFO  805
#define SDX_TOC        806

#define SDX_CONTINUE   830
#define SDX_PAUSE      831
#define SDX_READV      832
#define SDX_CLOSE      833
#define SDX_NEWCONTROL 834
#define SDX_NEWDATA    835
#define SDX_EXECMOD    836

/*
 * This group of constants have different names in the Fortran interface
 * than in the C interface.  The C interface constants should not be
 * defined in Fortran, but the Fortran constants can be part of the C
 * interface.  Remember, mkinc removes the `SDX_' prefix.
 */
#ifndef NO_FORTRAN_DEFINE
#define NO_FORTRAN_DEFINE       /*just a marker for mkinc */
#endif
#define SDX_WVARIABLE   840
#define SDX_VAR         SDX_WVARIABLE                   NO_FORTRAN_DEFINE
#define SDX_WMESHNAME   841
#define SDX_MESHNAME    SDX_WMESHNAME                   NO_FORTRAN_DEFINE
#define SDX_WMESHTYPE   842
#define SDX_MESHTYPE    SDX_WMESHTYPE                   NO_FORTRAN_DEFINE
#define SDX_WVARLENGTH  843
#define SDX_VARLENGTH   SDX_WVARLENGTH                  NO_FORTRAN_DEFINE
#define SDX_WVARBLENGTH 844
#define SDX_VARBYTELENGTH SDX_WVARBLENGTH               NO_FORTRAN_DEFINE

#define SDX_ENOENT     201
#define SDX_ENSERVER   202
#define SDX_EMSERVER   203
#define SDX_EPROTO     204
#define SDX_EBADVAR    205
#define SDX_EAGAIN     206
#define SDX_EBADID     207

typedef struct {
    int            type;
    int            vartype;
    char          *varname;
} SDXReadEvent;

typedef struct {
    int            type;
    char          *modname;
} SDXExecModEvent;

typedef union {
    int            type;
    SDXReadEvent   read;
    SDXExecModEvent execmod;
} SDXEvent;

typedef struct {
    int            nqmesh;
    int           *nqvar;
    int            numesh;
    int           *nuvar;
    float          min_extents[3];
    float          max_extents[3];
    int            nspace;
    int            cycle;
    float          time;
    char          *name;
} MWDInfo;

extern int     sdx_errorno;



/*-------------------------------------------------------------------------
 * Function prototypes.
 *-------------------------------------------------------------------------
 */
int SDXConnect (char *, char *, char *, int, char *, char *, int, int, int);
int SDXGetControlInfo (int*, char**, int**, char**, int**, int**, int**);
int SDXGetControlType (void);
int SDXGetNewDataInfo (char**);
int SDXOpenControl (int *, char **, int **, char **, int **, int **, int **);
void SDXCloseControl (void);

int SDXChar (char*);
int SDXClose (int);
int SDXError (void);
int SDXFloat (float);
int SDXInteger (int);
int SDXMultimesh (int, char*, int*);
int SDXMultivar (int, char*, int*);
int SDXMultimat (int, char*);
int SDXMultimatspecies (int, char*);
int SDXNewData (int);
int SDXNextEvent (int*, int*, char*, int);
int SDXOpen (char*, char*, char*, int, char*, char*, int*, int, int, int);
int SDXPause (int);
int SDXToc (int, char*, int* );
int SDXPutQuadmesh (char*, char*, char*, char*, float*, float*, float*,
        int*, int, int, int, DBoptlist*);
int SDXPutQuadvar (char*, char*, float*, int*, int, float*,
        int, int, int, DBoptlist*);
int SDXPutMaterial (char*, char*, int, int*, int*, int*, int, int*, int*, int*,
        float*, int, int, DBoptlist*);
int SDXPutZonelist (char*, int, int, int*, int, int, int*, int*, int);
int SDXPutFacelist (char*, int, int, int*, int, int, int*, int*, int*, int,
        int*, int*, int);
int SDXPutUcdmesh (char*, int, float*, float*, float*, char*, char*, char*,
        int, int, int, char*, char*, DBoptlist*);
int SDXPutUcdvar (char*, char*, float*, int, float*, int, int, int,
        DBoptlist*);
int SDXCalcExternalFacelist (int*, int*, int*, int*, int*, int*, int*, int*,
        int*, int*, int*, int*, int*, int*, int*, int* );

#endif /* !SDX_H */
