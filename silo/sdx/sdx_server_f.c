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
 * SDX Server Library
 */
#define NO_CALLBACKS
#include <silo_sdx_private.h>

#include <silo_f.h>
#include <silo_private.h>
#include <sdx_server_f.h>
#include <sdx_server.h>

/*******************************************************************
 * Procedure: SDXCHAR
*******************************************************************/

int
F_SDXCHAR(char *chars, int *nchars)
{
    char *temp = ALLOC_N (char, *nchars + 1);
    int   ret  = 0;

    strncpy(temp, chars, *nchars);
    temp[*nchars] = '\0';
    ret = SDXChar(temp);

    free(temp);

    return ret;
}

/*******************************************************************
 * Procedure: SDXCLOSE
*******************************************************************/

int
F_SDXCLOSE(int *sdxid) {
    return SDXClose(*sdxid);
}

/*******************************************************************
 * Procedure: SDXERROR
*******************************************************************/

int
F_SDXERROR(void)
{
    return SDXError();
}

/*******************************************************************
 * Procedure: SDXFLOAT
*******************************************************************/

int
F_SDXFLOAT(float *value)
{
    return SDXFloat(*value);
}

/*******************************************************************
 * Procedure: SDXINTEGER
*******************************************************************/

int
F_SDXINTEGER(int *ivalue)
{ 
    return SDXInteger(*ivalue);
}

/*******************************************************************
 * Procedure: SDXMMESH
*******************************************************************/

int
F_SDXMMESH(int *nblocks, char *meshnames, int *meshtypes)
{
    return SDXMultimesh(*nblocks, meshnames, meshtypes);
}

/*******************************************************************
 * Procedure: SDXMVAR
 * 
 * Modifications:
 *    Brooke Unger,  Tue Jul 22 16:43:43 PDT 1997
 *    I added this to handle multimesh variables.
 *
 *******************************************************************/

int
F_SDXMVAR(int *nvars, char *varnames, int *vartypes)
{
    return SDXMultivar(*nvars, varnames, vartypes); 
}

/*******************************************************************
 * Procedure: SDXMMAT
 * 
 * Modifications:
 *    Brooke Unger,  Tue Jul 22 16:43:43 PDT 1997
 *    I added this to handle multimesh materials.
 *
 *******************************************************************/

int 
F_SDXMMAT(int *nmats, char *matnames)
{
    return SDXMultimat(*nmats, matnames); 
}
    
/*******************************************************************
 * Procedure: SDXMSPEC
 * 
 * Modifications:
 *    Jeremy Meredith,  Tue Jul 22 16:43:43 PDT 1997
 *    Added to handle multimesh species.
 *
 *******************************************************************/

int 
F_SDXMSPEC(int *nspec, char *specnames)
{
    return SDXMultimatspecies(*nspec, specnames); 
}
    
/*******************************************************************
 * Procedure: SDXNEWDATA
 *******************************************************************/

int
F_SDXNEWDATA(int *sdxid)
{
    return SDXNewData(*sdxid);
}

/*******************************************************************
 * Procedure: SDXNEXTEVENT
 *******************************************************************/

int
F_SDXNEXTEVENT(int *eventtype, int *readtype, char *readvar, int *readvarlen)
{
    return SDXNextEvent(eventtype, readtype, readvar, *readvarlen);
}

/*******************************************************************
 * Procedure: SDXOPEN
 *******************************************************************/

int
F_SDXOPEN(char *f_machname, int *machnamelen, char *f_username,
          int *usernamelen, char *f_idstring, int *idstringlen,
          int *nvar, char *varnames, char *meshnames, int *vartypes,
          int *nmats, int *nblocks, int *sdxid)
{
    char      *machname=NULL, *username=NULL, *idstring=NULL;
    int       ret = 0;
    
    machname = ALLOC_N (char, *machnamelen+1);
    strncpy (machname, f_machname, *machnamelen);
    machname[*machnamelen] = '\0';
    username = ALLOC_N (char, *usernamelen+1);
    strncpy (username, f_username, *usernamelen);
    username[*usernamelen] = '\0';

    idstring = ALLOC_N (char, *idstringlen+1);
    strncpy (idstring, f_idstring, *idstringlen);
    idstring[*idstringlen] = '\0';

    ret = SDXOpen(machname, username, idstring, *nvar, varnames, 
                  meshnames, vartypes, *nmats, *nblocks, *sdxid);

    FREE (machname);
    FREE (username);
    FREE (idstring);

    return ret;
}

/***********************************************************************
*
* Purpose:  Wait delay seconds.
*
* Programmer:  Eric Brugger
* Date:        The epoch
*
* Input arguments:
*    delay    : The amount to wait in seconds.
*
* Output arguments:
*
* Input/Output arguments:
*
* Notes:
*
* Modifications:
*    Al Leibee, Tue Mar 15 11:27:22 PST 1994
*    Changed sigset to signal to span BSD and SYSTEM V.
*
*    Eric Brugger, February 1, 1995
*    I cast the second argument to signal to satisfy the prototype.
*
*    Eric Brugger, Wed Mar  1 16:55:48 PST 1995
*    I shrouded the prototypes for non-ansi compilers.
*
*    Eric Brugger, Wed Mar 15 16:39:37 PST 1995
*    I modified the routine to return an error flag.
*
***********************************************************************/

int
F_SDXPAUSE(int *delay)
{
    return SDXPause(*delay);
}

/*******************************************************************
 * Procedure: SDXTOC
 *******************************************************************/

int
F_SDXTOC(int *nvars, char *varnames, int *vartypes)
{
    return SDXToc(*nvars, varnames, vartypes); 
}

/*******************************************************************
 * Procedure: SDXPUTQM
 *
 *******************************************************************/

int
F_SDXPUTQM(char *f_name, int *lname, char *f_xname, int *lxname, char *f_yname,
           int *lyname, char *f_zname, int *lzname, float *x, float *y,
           float *z, int *dims, int *ndims, int *datatype, int *coordtype,
           int *optlist_id)
{
    char      *name=NULL, *xname=NULL, *yname=NULL, *zname=NULL;
    DBoptlist *optlist;
    int       err;

    name = ALLOC_N (char, *lname+1);
    strncpy (name, f_name, *lname);
    name[*lname] = '\0';

    xname = ALLOC_N (char, *lxname+1);
    strncpy (xname, f_xname, *lxname);
    xname[*lxname] = '\0';
    yname = ALLOC_N (char, *lyname+1);
    strncpy (yname, f_yname, *lyname);
    yname[*lyname] = '\0';
    zname = ALLOC_N (char, *lzname+1);
    strncpy (zname, f_zname, *lzname);
    zname[*lzname] = '\0';

    optlist = (DBoptlist*) DBFortranAccessPointer(*optlist_id);

    err = SDXPutQuadmesh(name, xname, yname, zname, x, y, z, dims, *ndims, 
                         *datatype, *coordtype, optlist);

    FREE (name);
    FREE (xname);
    FREE (yname);
    FREE (zname);

    return err;
}

/*******************************************************************
 * Procedure: SDXPUTQV
 *******************************************************************/

int
F_SDXPUTQV(char *f_name, int *lname, char *f_meshname, int *lmeshname,
           float *var, int *dims, int *ndims, float *mixvar, int *mixlen,
           int *datatype, int *centering, int *optlist_id)
{
    char      *name=NULL, *meshname=NULL;
    DBoptlist *optlist;
    int       err;

    name = ALLOC_N (char, *lname+1);
    strncpy (name, f_name, *lname);
    name[*lname] = '\0';

    meshname = ALLOC_N (char, *lmeshname+1);
    strncpy (meshname, f_meshname, *lmeshname);
    meshname[*lmeshname] = '\0';

    optlist = (DBoptlist*) DBFortranAccessPointer(*optlist_id);

    err = SDXPutQuadvar(name, meshname, var, dims, *ndims,
                        mixvar, *mixlen, *datatype, *centering, optlist);

    FREE (name);
    FREE (meshname);

    return err;
}

/*******************************************************************
 * Procedure: SDXPUTMAT
 *******************************************************************/

int
F_SDXPUTMAT(char *f_name, int *lname, char *f_meshname, int *lmeshname,
            int *nmat, int *matnos, int *matlist, int *dims, int *ndims,
            int *mix_next, int *mix_mat, int *mix_zone, float *mix_vf,
            int *mixlen, int *datatype, int *optlist_id)
{
    char      *name=NULL, *meshname=NULL;
    DBoptlist *optlist;
    int       err;

    name = ALLOC_N (char, *lname+1);
    strncpy (name, f_name, *lname);
    name[*lname] = '\0';

    meshname = ALLOC_N (char, *lmeshname+1);
    strncpy (meshname, f_meshname, *lmeshname);
    meshname[*lmeshname] = '\0';

    optlist = (DBoptlist*) DBFortranAccessPointer(*optlist_id);
    
    err = SDXPutMaterial(name, meshname, *nmat, matnos, matlist, dims,
                         *ndims, mix_next, mix_mat, mix_zone, mix_vf, *mixlen,
                         *datatype, optlist);

    FREE (name);
    FREE (meshname);

    return err;
}

/*******************************************************************
 * Procedure: SDXPUTZL
 *******************************************************************/

int
F_SDXPUTZL(char *f_name, int *lname, int *nzones, int *ndims, int *nodelist,
           int *lnodelist, int *origin, int *shapesize, int *shapecnt,
           int *nshapes)
{
    char      *name=NULL;
    int       err;

    name = ALLOC_N (char, *lname+1);
    strncpy (name, f_name, *lname);
    name[*lname] = '\0';

    err = SDXPutZonelist(name, *nzones, *ndims, nodelist, *lnodelist,
                         *origin, shapesize, shapecnt, *nshapes);

    FREE (name);

    return err;
}

/*******************************************************************
 * Procedure: SDXPUTFL
 *******************************************************************/

int
F_SDXPUTFL(char *f_name, int *lname, int *nfaces, int *ndims, int *nodelist,
           int *lnodelist, int *origin, int *zoneno, int *shapesize,
           int *shapecnt, int *nshapes, int *types, int *typelist, int *ntypes)
{
    char      *name=NULL;
    int       err;

    name = ALLOC_N (char, *lname+1);
    strncpy (name, f_name, *lname);
    name[*lname] = '\0';

    err = SDXPutFacelist(name, *nfaces, *ndims, nodelist, *lnodelist,
                         *origin, zoneno, shapesize, shapecnt, *nshapes, types,
                         typelist, *ntypes);

    FREE (name);

    return err;
}

/*******************************************************************
 * Procedure: SDXPUTUM
 *******************************************************************/

int
F_SDXPUTUM(char *f_name, int *lname, int *ndims, float *x, float *y, float *z,
           char *f_xname, int *lxname, char *f_yname, int *lyname,
           char *f_zname, int *lzname, int *datatype, int *nnodes, int *nzones,
           char *f_zlname, int *lzlname, char *f_flname, int *lflname,
           int *optlist_id)
{
    char      *name=NULL, *xname=NULL, *yname=NULL, *zname=NULL;
    char      *zlname=NULL, *flname=NULL;
    DBoptlist *optlist;
    int       err;

    name = ALLOC_N (char, *lname+1);
    strncpy (name, f_name, *lname);
    name[*lname] = '\0';

    xname = ALLOC_N (char, *lxname+1);
    strncpy (xname, f_xname, *lxname);
    xname[*lxname] = '\0';
    yname = ALLOC_N (char, *lyname+1);
    strncpy (yname, f_yname, *lyname);
    yname[*lyname] = '\0';
    zname = ALLOC_N (char, *lzname+1);
    strncpy (zname, f_zname, *lzname);
    zname[*lzname] = '\0';

    zlname = ALLOC_N (char, *lzlname+1);
    strncpy (zlname, f_zlname, *lzlname);
    zname[*lzlname] = '\0';
    flname = ALLOC_N (char, *lflname+1);
    strncpy (flname, f_flname, *lflname);
    zname[*lflname] = '\0';

    optlist = (DBoptlist*) DBFortranAccessPointer(*optlist_id);

    err = SDXPutUcdmesh(name, *ndims, x, y, z, xname, yname, zname,
                        *datatype, *nnodes, *nzones, zlname, flname, optlist);

    FREE (name);
    FREE (xname);
    FREE (yname);
    FREE (zname);
    FREE (zlname);
    FREE (flname);

    return err;
}

/*******************************************************************
 * Procedure: SDXPUTUV
 *******************************************************************/

int
F_SDXPUTUV(char *f_name, int *lname, char *f_meshname, int *lmeshname,
           float *var, int *nels, float *mixvar, int *mixlen, int *datatype,
           int *centering, int *optlist_id)
{
    char      *name=NULL, *meshname=NULL;
    DBoptlist *optlist;
    int       err;

    name = ALLOC_N (char, *lname+1);
    strncpy (name, f_name, *lname);
    name[*lname] = '\0';

    meshname = ALLOC_N (char, *lmeshname+1);
    strncpy (meshname, f_meshname, *lmeshname);
    meshname[*lmeshname] = '\0';

    optlist = (DBoptlist*) DBFortranAccessPointer(*optlist_id);

    err = SDXPutUcdvar(name, meshname, var, *nels, mixvar, *mixlen,
                       *datatype, *centering, optlist);

    FREE (name);
    FREE (meshname);

    return err;
}

/*******************************************************************
 * Procedure: SDXCALCFL
 *******************************************************************/

int
F_SDXCALCFL(int *znodelist, int *nnodes, int *origin, int *zshapesize,
            int *zshapecnt, int *nzshapes, int *matlist, int *bnd_method,
            int *nfaces, int *fnodelist, int *lfnodelist, int *fshapesize,
            int *fshapecnt, int *nfshapes, int *fzoneno, int *lfzoneno)
{
    int            i;
    DBfacelist    *fl;

    fl = DBCalcExternalFacelist(znodelist, *nnodes, *origin, zshapesize,
                                zshapecnt, *nzshapes,
                                (*matlist == DB_F77NULL) ? NULL : matlist,
                                *bnd_method);

    if (fl == NULL)
        return -1;

    *nfaces = fl->nfaces;
    *lfnodelist = MIN(*lfnodelist, fl->lnodelist);
    for (i = 0; i < *lfnodelist; i++)
        fnodelist[i] = fl->nodelist[i];
    *nfshapes = fl->nshapes;
    for (i = 0; i < *nfshapes; i++) {
        fshapesize[i] = fl->shapesize[i];
        fshapecnt[i] = fl->shapecnt[i];
    }
    *lfzoneno = MIN(*lfzoneno, fl->nfaces);
    for (i = 0; i < *lfzoneno; i++)
        fzoneno[i] = fl->zoneno[i];

    DBFreeFacelist(fl);

    return 0;
}

/***********************************************************************
 *
 * Modifications:
 *    Eric Brugger, Tue Jun 17 14:56:40 PDT 1997
 *    I modified the routine to use DBFortranAccessPointer to convert
 *    the optlist identifier to a pointer.
 *
 **********************************************************************/

void
process_optlist(int *optlist_id)
{
    DBoptlist* optlist;
    optlist = (DBoptlist*) DBFortranAccessPointer(*optlist_id);

    sdx_process_optlist(optlist);

    return;
}
