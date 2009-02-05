!cccccccccccccccccccccccccccccccccccccccccccccccccccccccc -*- fortran -*-
!
!     SDX include file for Fortan applications.
!
! Programmer:   Robb Matzke
!       Wed Nov 30 11:40:46 PST 1994
!
! WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
!    This file is generated automatically from C header files in the
!    SILO distribution.  Do not make changes to this file manually,
!    as those changes will disappear.
! WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
!
! This file was built by saying:
!       mv sdx.inc sdx.inc~
!       cat sdx.h sdx_server.h | mkinc > sdx.inc
!
!cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc


!...Functions.
      external sdxcalcfl, sdxchar, sdxclose, sdxerror, sdxfloat
      external sdxinteger, sdxmmat, sdxmmesh, sdxmspec, sdxmvar
      external sdxnewdata, sdxnextevent, sdxopen, sdxpause, sdxputfl
      external sdxputmat, sdxputqm, sdxputqv, sdxputum, sdxputuv
      external sdxputzl, sdxtoc

      integer  sdxcalcfl, sdxchar, sdxclose, sdxerror, sdxfloat
      integer  sdxinteger, sdxmmat, sdxmmesh, sdxmspec, sdxmvar
      integer  sdxnewdata, sdxnextevent, sdxopen, sdxpause, sdxputfl
      integer  sdxputmat, sdxputqm, sdxputqv, sdxputum, sdxputuv
      integer  sdxputzl, sdxtoc


!...Parameters.
      integer  CLOSE, CNTRLINFO, CONTINUE, EAGAIN, EBADID, EBADVAR
      integer  EMSERVER, ENOENT, ENSERVER, EPROTO, EXECMOD, LEN
      integer  NEWCONTROL, NEWDATA, PAUSE, READV, TOC, WMESHNAME
      integer  WMESHTYPE, WVARBLENGTH, WVARIABLE, WVARLENGTH

      parameter (LEN                       = 64             )
      parameter (CNTRLINFO                 = 805            )
      parameter (TOC                       = 806            )
      parameter (CONTINUE                  = 830            )
      parameter (PAUSE                     = 831            )
      parameter (READV                     = 832            )
      parameter (CLOSE                     = 833            )
      parameter (NEWCONTROL                = 834            )
      parameter (NEWDATA                   = 835            )
      parameter (EXECMOD                   = 836            )
      parameter (WVARIABLE                 = 840            )
      parameter (WMESHNAME                 = 841            )
      parameter (WMESHTYPE                 = 842            )
      parameter (WVARLENGTH                = 843            )
      parameter (WVARBLENGTH               = 844            )
      parameter (ENOENT                    = 201            )
      parameter (ENSERVER                  = 202            )
      parameter (EMSERVER                  = 203            )
      parameter (EPROTO                    = 204            )
      parameter (EBADVAR                   = 205            )
      parameter (EAGAIN                    = 206            )
      parameter (EBADID                    = 207            )


! End.
