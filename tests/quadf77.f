!***********************************************************************
! Copyright (C) 1994-2016 Lawrence Livermore National Security, LLC.
! LLNL-CODE-425250.
! All rights reserved.
! 
! This file is part of Silo. For details, see silo.llnl.gov.
! 
! Redistribution and use in source and binary forms, with or without
! modification, are permitted provided that the following conditions
! are met:
! 
!    * Redistributions of source code must retain the above copyright
!      notice, this list of conditions and the disclaimer below.
!    * Redistributions in binary form must reproduce the above copyright
!      notice, this list of conditions and the disclaimer (as noted
!      below) in the documentation and/or other materials provided with
!      the distribution.
!    * Neither the name of the LLNS/LLNL nor the names of its
!      contributors may be used to endorse or promote products derived
!      from this software without specific prior written permission.
! 
! THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
! "AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
! LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
! A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
! LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
! CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
! EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
! PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
! PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
! LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
! NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
! SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
! 
! This work was produced at Lawrence Livermore National Laboratory under
! Contract No.  DE-AC52-07NA27344 with the DOE.
! 
! Neither the  United States Government nor  Lawrence Livermore National
! Security, LLC nor any of  their employees, makes any warranty, express
! or  implied,  or  assumes  any  liability or  responsibility  for  the
! accuracy, completeness,  or usefulness of  any information, apparatus,
! product, or  process disclosed, or  represents that its use  would not
! infringe privately-owned rights.
! 
! Any reference herein to  any specific commercial products, process, or
! services by trade name,  trademark, manufacturer or otherwise does not
! necessarily  constitute or imply  its endorsement,  recommendation, or
! favoring  by  the  United  States  Government  or  Lawrence  Livermore
! National Security,  LLC. The views  and opinions of  authors expressed
! herein do not necessarily state  or reflect those of the United States
! Government or Lawrence Livermore National Security, LLC, and shall not
! be used for advertising or product endorsement purposes.
!***********************************************************************

!...............................................................................
!   Program
!
!       testquadf77.f
!
!   Purpose
!
!       Test program illustrating use of SILO from Fortran for
!       writing quad mesh and variable data.
!
!   Programmer
!
!       Jeff Long, NSSD/B, 11/24/92
!
!   Notes
!
!       For more info on calling sequences, see the "SILO User's Manual".
!...............................................................................


      program main

      implicit none
      include "silo.inc"

      integer  buildquad
      integer  dbid, meshid, err, driver, nargs, anint
      integer  iarr(20)
      real*8   darr(20)
      character*8 cname
      character*256 cloption

      driver = DB_PDB
      nargs = iargc()
      call getarg(1, cloption)
      if (cloption .eq. "DB_HDF5") then
          driver = DB_HDF5
      end if

      err = dbset2dstrlen(1024)

!...Create file.
      err = dbcreate("quadf77.silo", 12, DB_CLOBBER, DB_LOCAL,
     .               "file info", 9, driver, dbid)

      meshid = buildquad (dbid, "quad", 4)
      meshid = buildquad (dbid, "quad2", 5)

      call testdb (dbid)

      err = dbclose(dbid)

      print *,'Created file: quadf77.silo'




!...Read stuff.
      err = dbopen ("quadf77.silo", 12, driver, DB_READ, dbid)

      err = dbinqlen(dbid, "quad_coord0", 11, anint)
      if (err .ne. 0) print *, 'Unable to read quad_coord0 length'
      if (anint .ne. 4) print *, 'Error reading quad_coord0 length'

      err = dbinqdtyp(dbid, "quad_coord0", 11, anint)
      if (err .ne. 0) print *, 'Unable to read quad_coord0 data type'
      if (anint .ne. DB_FLOAT)
     .    print *, 'Error reading quad_coord0 data type'

      err = dbrdvar (dbid, "hcom2", 5, iarr)
      if (err .ne. 0) print *, 'Error reading hcom2'

      err = dbrdvar (dbid, "hcom3", 5, darr)
      if (err .ne. 0) print *, 'Error reading hcom3'

      err = dbrdvar (dbid, "namebase", 8, cname)
      if (err .ne. 0) print *, 'Error reading namebase'

!     err = dbrdvar (dbid, "junk", 4, cname)
!     if (err .ne. 0) print *, 'Error reading junk'



      stop
      end

      integer function buildquad (dbid, name, lname)
!----------------------------------------------------------------------
!  Routine                                                    buildquad
!
!  Purpose
!
!       Build quad-mesh, quad-var and return the mesh ID.
!
!  Modifications:
!    Kathleen Bonnell, Wed Sep 2 16:12:15 PDT 20099
!    Changed 'character*8 name' to 'character*(*) name' to remove 
!    'Character length argument mismatch' compiler error.
!
!----------------------------------------------------------------------

      integer  dbid, lname
      character*(*) name

      parameter  (NX     = 4)
      parameter  (NY     = 3)
      parameter  (NZONES = 6)
      parameter  (NNODES = 12)

      include 'silo.inc'

      integer    i, meshid, varid
      integer    tcycle, mixlen, optlistid
      integer    dims(3), ndims
      real       x(NX), y(NY), d(NZONES), f(2*NZONES)
      real       ttime
      double precision dttime
      character*1024 vnames(2)
      integer lvnames(2)


!...Initializations.

      data  x/1.,2.,3.,4./
      data  y/1.,2.,3./
      data  d/1.,2.,3.,4.,5.,6./
      data  f/1.0,1.1,1.2,1.3,1.4,1.5,2.0,2.1,2.2,2.3,2.4,2.5/

      ttime     = 2.345
      dttime    = 2.345
      tcycle    = 200



!...Create an option list containing time and cycle information. Note
!...that the function names are different depending on the data type
!...of the option value. The variable 'optlistid' is used as a handle
!...to this option list in future function invocations.

      ierr = dbmkoptlist (5, optlistid)
      ierr = dbaddropt   (optlistid, DBOPT_TIME, ttime)    ! real
      ierr = dbaddiopt   (optlistid, DBOPT_CYCLE, tcycle)  ! integer
      ierr = dbadddopt   (optlistid, DBOPT_DTIME, dttime)  ! double

!...The following option would change the major order for multi-
!...dimensional arrays from row-major (default) to column-major.
!...If you have arrays defined like:  real  x(300,200) you might
!...need this.

!     ierr = dbaddiopt   (optlistid, DBOPT_MAJORORDER, DB_COLMAJOR)


!...Write simple 2-D quad mesh. Dims defines number of NODES. The
!...parameter 'meshid' is returned and should be used on future
!...writes of quad variables.

      ndims     = 2
      dims(1)   = NX
      dims(2)   = NY


      err = dbputqm(dbid, name, lname,
     .              "X", 1, "Y", 1, DB_F77NULLSTRING, 0,
     .              x, y, -1, dims, ndims, DB_FLOAT, DB_COLLINEAR,
     .              optlistid, meshid)


!...Write quad variable. Dims defines number of ZONES (since zone-centered).
!...Possible values for centering are: DB_ZONECENT, DB_NODECENT, DB_NOTCENT.

      ndims   = 2
      dims(1) = NX - 1
      dims(2) = NY - 1

!...Hack to make sure we don't wind up writing same variable name twice to
!...two different meshes
      if (lname .eq. 4) err = dbputqv1 (dbid, "d", 1, name,
     .                lname, d, dims, ndims,
     .                DB_F77NULL, 0, DB_FLOAT, DB_ZONECENT,
     .                optlistid, varid)
      if (lname .eq. 5) err = dbputqv1 (dbid, "e", 1, name,
     .                lname, d, dims, ndims,
     .                DB_F77NULL, 0, DB_FLOAT, DB_ZONECENT,
     .                optlistid, varid)
      vnames(1) = "FooBar"
      lvnames(1) = 6
      vnames(2) = "gorfo"
      lvnames(2) = 5
      err = dbputqv(dbid, "f", 1, name, lname, 2, vnames, lvnames,
     .          f, dims, ndims, DB_F77NULL, 0, DB_FLOAT, DB_ZONECENT,
     .          optlistid, varid)

      buildquad = meshid

      return
      end

!************************************************************
!************************************************************
      subroutine testdb (dbid)
      integer dbid

      include 'silo.inc'

      character*8 namebase
      integer  ilen, ihfirst(10)
      real*8   rhfirst(15)

      do i = 1 , 10
         ihfirst(i) = i
      enddo
      do i = 1 , 15
         rhfirst(i) = dble(i)
      enddo
      namebase = 'abcdefgh'
      ilen     = 10


      ierr = dbwrite ( dbid, "namebase", 8, namebase, 8, 1, DB_CHAR )
      ierr = dbwrite ( dbid, "ilen",     4, ilen,     1, 1, DB_INT )

!                       write integer hydro common to dump
      ialen = 10
      ierr = dbwrite ( dbid, "hcom2", 5, ihfirst, ialen, 1, DB_INT )

!                       write real hydro common to dump
      ialen = 15
      ierr = dbwrite ( dbid, "hcom3", 5, rhfirst, ialen, 1,DB_DOUBLE)


      return
      end
