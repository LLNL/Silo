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
#ifndef SILO_F_H
#define SILO_F_H


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
#define DBADDIAOPT_FC    FC_FUNC (dbaddiaopt,DBADDIAOPT)
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
#define DBPUTQV_FC       FC_FUNC (dbputqv,DBPUTQV)
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
#define DBREGFOPTS_FC	 FC_FUNC (dbregfopts, DBREGFOPTS)
#define DBUNREGFOPTS_FC  FC_FUNC (dbunregfopts, DBUNREGFOPTS)
#define DBUNREGAFOPTS_FC FC_FUNC (dbunregafopts, DBUNREGAFOPTS)

void    *DBFortranAccessPointer(int value);

#endif /* !SILO_F_H */
