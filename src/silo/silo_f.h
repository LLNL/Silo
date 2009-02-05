#ifndef SILO_F_H
#define SILO_F_H

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

#define DBADDIOPT_FC     FC_FUNC (dbaddiopt,DBADDIOPT)
#define DBADDROPT_FC     FC_FUNC (dbaddropt,DBADDROPT)
#define DBADDDOPT_FC     FC_FUNC (dbadddopt,DBADDDOPT)
#define DBADDCOPT_FC     FC_FUNC (dbaddcopt,DBADDCOPT)
#define DBADDCAOPT_FC     FC_FUNC (dbaddcaopt,DBADDCAOPT)
#define DBMKOPTLIST_FC   FC_FUNC (dbmkoptlist,DBMKOPTLIST)
#define DBFREEOPTLIST_FC FC_FUNC (dbfreeoptlist,DBFREEOPTLIST)
#define DBPUTMAT_FC      FC_FUNC (dbputmat,DBPUTMAT)
#define DBPUTMSP_FC      FC_FUNC (dbputmsp,DBPUTMSP)
#define DBCALCFL_FC      FC_FUNC (dbcalcfl,DBCALCFL)
#define DBCLOSE_FC       FC_FUNC (dbclose,DBCLOSE)
#define DBCREATE_FC      FC_FUNC (dbcreate,DBCREATE)
#define DBINQLEN_FC      FC_FUNC (dbinqlen,DBINQLEN)
#define DBMKDIR_FC       FC_FUNC (dbmkdir,DBMKDIR)
#define DBOPEN_FC        FC_FUNC (dbopen,DBOPEN)
#define DBPUTFL_FC       FC_FUNC (dbputfl,DBPUTFL)
#define DBWRTFL_FC       FC_FUNC (dbwrtfl,DBWRTFL)
#define DBPUTDEFVARS_FC  FC_FUNC (dbputdefvars,DBPUTDEFVARS)
#define DBPUTMMESH_FC    FC_FUNC (dbputmmesh,DBPUTMMESH)
#define DBPUTMVAR_FC     FC_FUNC (dbputmvar,DBPUTMVAR)
#define DBPUTMMAT_FC     FC_FUNC (dbputmmat,DBPUTMMAT)
#define DBPUTPM_FC       FC_FUNC (dbputpm,DBPUTPM)
#define DBPUTPV1_FC      FC_FUNC (dbputpv1,DBPUTPV1)
#define DBPUTQM_FC       FC_FUNC (dbputqm,DBPUTQM)
#define DBPUTQV1_FC      FC_FUNC (dbputqv1,DBPUTQV1)
#define DBPUTUM_FC       FC_FUNC (dbputum,DBPUTUM)
#define DBPUTUV1_FC      FC_FUNC (dbputuv1,DBPUTUV1)
#define DBPUTZL_FC       FC_FUNC (dbputzl,DBPUTZL)
#define DBPUTZL2_FC      FC_FUNC (dbputzl2,DBPUTZL2)
#define DBRDVAR_FC       FC_FUNC (dbrdvar,DBRDVAR)
#define DBRDVARSLICE_FC  FC_FUNC (dbrdvarslice,DBRDVARSLICE)
#define DBWRITESLICE_FC  FC_FUNC (dbwriteslice,DBWRITESLICE)
#define DBSETDIR_FC      FC_FUNC (dbsetdir,DBSETDIR)
#define DBSETDIRID_FC    FC_FUNC (dbsetdirid,DBSETDIRID)
#define DBSHOWERRORS_FC  FC_FUNC (dbshowerrors,DBSHOWERRORS)
#define DBWRITE_FC       FC_FUNC (dbwrite,DBWRITE)
#define DBPUTCA_FC       FC_FUNC (dbputca,DBPUTCA)
#define DBGETCA_FC       FC_FUNC (dbgetca,DBGETCA)
#define DBPUTCURVE_FC    FC_FUNC (dbputcurve,DBPUTCURVE)
#define DBGETCURVE_FC    FC_FUNC (dbgetcurve,DBGETCURVE)
#define DBADDICMP_FC     FC_FUNC (dbaddicmp,DBADDICMP)
#define DBADDRCMP_FC     FC_FUNC (dbaddrcmp,DBADDRCMP)
#define DBADDCCMP_FC     FC_FUNC (dbaddccmp,DBADDCCMP)
#define DBMKOBJECT_FC    FC_FUNC (dbmkobject,DBMKOBJECT)
#define DBFREEOBJECT_FC  FC_FUNC (dbfreeobject,DBFREEOBJECT)
#define DBGETQV1_FC      FC_FUNC (dbgetqv1,DBGETQV1)
#define DBFGETCA_FC      FC_FUNC (dbfgetca,DBFGETCA)
#define DBINQCA_FC       FC_FUNC (dbinqca,DBINQCA)
#define DBINQFILE_FC     FC_FUNC (dbinqfile,DBINQFILE)
#define DBSET2DSTRLEN_FC FC_FUNC (dbset2dstrlen,DBSET2DSTRLEN)
#define DBGET2DSTRLEN_FC FC_FUNC (dbget2dstrlen,DBGET2DSTRLEN)
#define DBSETOVRWRT_FC   FC_FUNC (dbsetovrwrt,DBSETOVRWRT)
#define DBGETOVRWRT_FC   FC_FUNC (dbgetovrwrt,DBGETOVRWRT)
#define DBSETCKSUMS_FC   FC_FUNC (dbsetcksums,DBSETCKSUMS)
#define DBGETCKSUMS_FC   FC_FUNC (dbgetcksums,DBGETCKSUMS)
#define DBSETCOMPRESS_FC FC_FUNC (dbsetcompress,DBSETCOMPRESS)
#define DBGETCOMPRESS_FC FC_FUNC (dbgetcompress,DBGETCOMPRESS)
#define DBSETHDFNMS_FC   FC_FUNC (dbsethdfnms,DBSETHDFNMS)
#define DBGETHDFNMS_FC   FC_FUNC (dbgethdfnms,DBGETHDFNMS)
#define DBSETDEPWARN_FC  FC_FUNC (dbsetdepwarn,DBSETDEPWARN)
#define DBGETDEPWARN_FC  FC_FUNC (dbgetdepwarn,DBgETDEPWARN)
#define DBPUTCSGM_FC     FC_FUNC (dbputcsgm,DBPUTCSGM)
#define DBPUTCSGZL_FC    FC_FUNC (dbputcsgzl,DBPUTCSGZL)
#define DBPUTCSGV_FC     FC_FUNC (dbputcsgv,DBPUTCSGV)
#define DBMKMRGTREE_FC   FC_FUNC (dbmkmrgtree,DBMKMRGTREE)
#define DBFREEMRGTREE_FC FC_FUNC (dbfreemrgtree,DBFREEMRGTREE)
#define DBADDREGION_FC   FC_FUNC (dbaddregion,DBADDREGION)
#define DBADDREGIONA_FC  FC_FUNC (dbaddregiona,DBADDREGIONA)
#define DBSETCWR_FC      FC_FUNC (dbsetcwr,DBSETCWR)
#define DBPUTMRGTREE_FC  FC_FUNC (dbputmrgtree,DBPUTMRGTREE)
#define DBPUTGRPLMAP_FC  FC_FUNC (dbputgrplmap,DBPUTGRPLMAP)
#define DBMKPTR_FC       FC_FUNC (dbmkptr,DBMKPTR)
#define DBRMPTR_FC       FC_FUNC (dbrmptr,DBRMPTR)
#define DBPMRGV_FC       FC_FUNC (dbpmrgv,DBPMRGV)
#define DBERRNO_FC       FC_FUNC (dberrno, DBERRNO)

void    *DBFortranAccessPointer(int value);

#endif /* !SILO_F_H */
