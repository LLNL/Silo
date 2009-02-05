#ifndef SDX_SERVER_F_H
#define SDX_SERVER_F_H

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
 * SDX Server Header.
 *
 */

#include <sdx.h>

#define F_SDXCHAR       F77_ID(sdxchar_,sdxchar,SDXCHAR)
#define F_SDXCLOSE      F77_ID(sdxclose_,sdxclose,SDXCLOSE)
#define F_SDXERROR      F77_ID(sdxerror_,sdxerror,SDXERROR)
#define F_SDXFLOAT      F77_ID(sdxfloat_,sdxfloat,SDXFLOAT)
#define F_SDXINTEGER    F77_ID(sdxinteger_,sdxinteger,SDXINTEGER)
#define F_SDXMMESH      F77_ID(sdxmmesh_,sdxmmesh,SDXMMESH)
#define F_SDXMVAR       F77_ID(sdxmvar_,sdxmvar,SDXMVAR)
#define F_SDXMMAT       F77_ID(sdxmmat_,sdxmmat,SDXMMAT)
#define F_SDXMSPEC      F77_ID(sdxmspec_,sdxmspec,SDXMSPEC)
#define F_SDXNEWDATA    F77_ID(sdxnewdata_,sdxnewdata,SDXNEWDATA)
#define F_SDXNEXTEVENT  F77_ID(sdxnextevent_,sdxnextevent,SDXNEXTEVENT)
#define F_SDXOPEN       F77_ID(sdxopen_,sdxopen,SDXOPEN)
#define F_SDXPAUSE      F77_ID(sdxpause_,sdxpause,SDXPAUSE)
#define F_SDXTOC        F77_ID(sdxtoc_,sdxtoc,SDXTOC)
#define F_SDXPUTQM      F77_ID(sdxputqm_,sdxputqm,SDXPUTQM)
#define F_SDXPUTQV      F77_ID(sdxputqv_,sdxputqv,SDXPUTQV)
#define F_SDXPUTMAT     F77_ID(sdxputmat_,sdxputmat,SDXPUTMAT)
#define F_SDXPUTZL      F77_ID(sdxputzl_,sdxputzl,SDXPUTZL)
#define F_SDXPUTFL      F77_ID(sdxputfl_,sdxputfl,SDXPUTFL)
#define F_SDXPUTUM      F77_ID(sdxputum_,sdxputum,SDXPUTUM)
#define F_SDXPUTUV      F77_ID(sdxputuv_,sdxputuv,SDXPUTUV)
#define F_SDXCALCFL     F77_ID(sdxcalcfl_,sdxcalcfl,SDXCALCFL)

void process_optlist (int *);

#endif /* !SDX_SERVER_F_H */
