#ifndef SILO_F_H
#define SILO_F_H

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
 * SILO FORTRAN Interface.
 */

/* I can't use DB_FCD because mkinc looks for DB* identifiers */
#ifdef CRAY
#include <fortran.h>
#define FCD_DB _fcd
#else
#define FCD_DB char*
#endif

#define FPTR(X,T)   ((!(X)||DB_F77NULL==(X))?(T)NULL:(T)(X))

#define F_DBADDIOPT     F77_ID(dbaddiopt_,dbaddiopt,DBADDIOPT)
#define F_DBADDROPT     F77_ID(dbaddropt_,dbaddropt,DBADDROPT)
#define F_DBADDDOPT     F77_ID(dbadddopt_,dbadddopt,DBADDDOPT)
#define F_DBADDCOPT     F77_ID(dbaddcopt_,dbaddcopt,DBADDCOPT)
#define F_DBMKOPTLIST   F77_ID(dbmkoptlist_,dbmkoptlist,DBMKOPTLIST)
#define F_DBFREEOPTLIST F77_ID(dbfreeoptlist_,dbfreeoptlist,DBFREEOPTLIST)
#define F_DBPUTMAT      F77_ID(dbputmat_,dbputmat,DBPUTMAT)
#define F_DBPUTMSP      F77_ID(dbputmsp_,dbputmsp,DBPUTMSP)
#define F_DBCALCFL      F77_ID(dbcalcfl_,dbcalcfl,DBCALCFL)
#define F_DBCLOSE       F77_ID(dbclose_,dbclose,DBCLOSE)
#define F_DBCREATE      F77_ID(dbcreate_,dbcreate,DBCREATE)
#define F_DBINQLEN      F77_ID(dbinqlen_,dbinqlen,DBINQLEN)
#define F_DBMKDIR       F77_ID(dbmkdir_,dbmkdir,DBMKDIR)
#define F_DBOPEN        F77_ID(dbopen_,dbopen,DBOPEN)
#define F_DBPUTFL       F77_ID(dbputfl_,dbputfl,DBPUTFL)
#define F_DBWRTFL       F77_ID(dbwrtfl_,dbwrtfl,DBWRTFL)
#define F_DBPUTDEFVARS  F77_ID(dbputdefvars_,dbputdefvars,DBPUTDEFVARS)
#define F_DBPUTMMESH    F77_ID(dbputmmesh_,dbputmmesh,DBPUTMMESH)
#define F_DBPUTMVAR     F77_ID(dbputmvar_,dbputmvar,DBPUTMVAR)
#define F_DBPUTMMAT     F77_ID(dbputmmat_,dbputmmat,DBPUTMMAT)
#define F_DBPUTPM       F77_ID(dbputpm_,dbputpm,DBPUTPM)
#define F_DBPUTPV1      F77_ID(dbputpv1_,dbputpv1,DBPUTPV1)
#define F_DBPUTQM       F77_ID(dbputqm_,dbputqm,DBPUTQM)
#define F_DBPUTQV1      F77_ID(dbputqv1_,dbputqv1,DBPUTQV1)
#define F_DBPUTUM       F77_ID(dbputum_,dbputum,DBPUTUM)
#define F_DBPUTUV1      F77_ID(dbputuv1_,dbputuv1,DBPUTUV1)
#define F_DBPUTZL       F77_ID(dbputzl_,dbputzl,DBPUTZL)
#define F_DBPUTZL2      F77_ID(dbputzl2_,dbputzl2,DBPUTZL2)
#define F_DBRDVAR       F77_ID(dbrdvar_,dbrdvar,DBRDVAR)
#define F_DBRDVARSLICE  F77_ID(dbrdvarslice_,dbrdvarslice,DBRDVARSLICE)
#define F_DBWRITESLICE  F77_ID(dbwriteslice_,dbwriteslice,DBWRITESLICE)
#define F_DBSETDIR      F77_ID(dbsetdir_,dbsetdir,DBSETDIR)
#define F_DBSETDIRID    F77_ID(dbsetdirid_,dbsetdirid,DBSETDIRID)
#define F_DBSHOWERRORS  F77_ID(dbshowerrors_,dbshowerrors,DBSHOWERRORS)
#define F_DBWRITE       F77_ID(dbwrite_,dbwrite,DBWRITE)
#define F_DBPUTCA       F77_ID(dbputca_,dbputca,DBPUTCA)
#define F_DBGETCA       F77_ID(dbgetca_,dbgetca,DBGETCA)
#define F_DBPUTCURVE	F77_ID(dbputcurve_,dbputcurve,DBPUTCURVE)
#define F_DBGETCURVE	F77_ID(dbgetcurve_,dbgetcurve,DBGETCURVE)
#define F_DBADDICMP     F77_ID(dbaddicmp_,dbaddicmp,DBADDICMP)
#define F_DBADDRCMP     F77_ID(dbaddrcmp_,dbaddrcmp,DBADDRCMP)
#define F_DBADDCCMP     F77_ID(dbaddccmp_,dbaddccmp,DBADDCCMP)
#define F_DBMKOBJECT    F77_ID(dbmkobject_,dbmkobject,DBMKOBJECT)
#define F_DBFREEOBJECT  F77_ID(dbfreeobject_,dbfreeobject,DBFREEOBJECT)
#define F_DBGETQV1      F77_ID(dbgetqv1_,dbgetqv1,DBGETQV1)
#define F_DBFGETCA      F77_ID(dbfgetca_,dbfgetca,DBFGETCA)
#define F_DBINQCA       F77_ID(dbinqca_,dbinqca,DBINQCA)
#define F_DBINQFILE     F77_ID(dbinqfile_,dbinqfile,DBINQFILE)
#define F_DBSET2DSTRLEN F77_ID(dbset2dstrlen_,dbset2dstrlen,DBSET2DSTRLEN)
#define F_DBGET2DSTRLEN F77_ID(dbget2dstrlen_,dbget2dstrlen,DBGET2DSTRLEN)
#define F_DBSETOVRWRT   F77_ID(dbsetovrwrt_,dbsetovrwrt,DBSETOVRWRT)
#define F_DBGETOVRWRT   F77_ID(dbgetovrwrt_,dbgetovrwrt,DBGETOVRWRT)
#define F_DBSETCKSUMS   F77_ID(dbsetcksums_,dbsetcksums,DBSETCKSUMS)
#define F_DBGETCKSUMS   F77_ID(dbgetcksums_,dbgetcksums,DBGETCKSUMS)

void    *DBFortranAccessPointer(int value);

#endif /* !SILO_F_H */
