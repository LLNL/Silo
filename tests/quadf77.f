************************************************************************
*
*                            Copyright 1991 - 2009
*                 The Regents of the University of California.
*                             All rights reserved.
*
* This work was produced at the University of California, Lawrence
* Livermore National Laboratory (UC LLNL) under contract no.
* W-7405-ENG-48 (Contract 48) between the U.S. Department of Energy
* (DOE) and The Regents of the University of California (University)
* for the operation of UC LLNL.  Copyright is reserved to the University
* for purposes of controlled dissemination, commercialization through
* formal licensing, or other disposition under terms of Contract 48;
* DOE policies, regulations and orders; and U.S. statutes.  The rights
* of the Federal Government are reserved under Contract 48 subject to
* the restrictions agreed upon by DOE and University.
*
*                                 DISCLAIMER
*
* This software was prepared as an account of work sponsored by an
* agency of the United States Government. Neither the United States
* Government nor the University of California nor any of their
* employees, makes any warranty, express or implied, or assumes any
* liability or responsiblity for the accuracy, completeness, or
* usefullness of any information, apparatus, product, or process
* disclosed, or represents that its use would not infringe privately
* owned rights. Reference herein to any specific commercial products,
* process, or service by trade name, trademark, manufacturer, or
* otherwise, does not necessarily constitute or imply its endorsement,
* recommendation, or favoring by the United States Government or the
* University of California. The views and opinions of authors expressed
* herein do not necessarily state or reflect those of the United States
* Government or the University of California, and shall not be used for
* advertising or product endorsement purposes.
*
************************************************************************

c...............................................................................
c   Program
c
c       testquadf77.f
c
c   Purpose
c
c       Test program illustrating use of SILO from Fortran for
c       writing quad mesh and variable data.
c
c   Programmer
c
c       Jeff Long, NSSD/B, 11/24/92
c
c   Notes
c
c       For more info on calling sequences, see the "SILO User's Manual".
c...............................................................................


      program main

      implicit none
      include "silo.inc"

      integer  buildquad
      integer  dbid, meshid, err
      integer  iarr(20)
      real*8   darr(20)
      character*8 cname

c...Create PDB flavor file.
      err = dbcreate("quadf77.pdb", 11, DB_CLOBBER, DB_LOCAL,
     .               "file info", 9, DB_PDB, dbid)

      meshid = buildquad (dbid, "quad", 4)
      meshid = buildquad (dbid, "quad2", 5)

      call testdb (dbid)

      err = dbclose(dbid)

      print *,'Created file: quadf77.pdb'




c...Read PDB stuff.
      err = dbopen ("quadf77.pdb", 11, DB_PDB, DB_READ, dbid)

      err = dbrdvar (dbid, "hcom2", 5, iarr)
      if (err .ne. 0) print *, 'Error reading hcom2'

      err = dbrdvar (dbid, "hcom3", 5, darr)
      if (err .ne. 0) print *, 'Error reading hcom3'

      err = dbrdvar (dbid, "namebase", 8, cname)
      if (err .ne. 0) print *, 'Error reading namebase'

c     err = dbrdvar (dbid, "junk", 4, cname)
c     if (err .ne. 0) print *, 'Error reading junk'



      stop
      end

      integer function buildquad (dbid, name, lname)
c----------------------------------------------------------------------
c  Routine						      buildquad
c
c  Purpose
c
c	Build quad-mesh, quad-var and return the mesh ID.
c
c  Modifications:
c    Kathleen Bonnell, Wed Sep 2 16:12:15 PDT 20099
c    Changed 'character*8 name' to 'character*(*) name' to remove 
c    'Character length argument mismatch' compiler error.
c
c----------------------------------------------------------------------

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
      real       x(NX), y(NY), d(NZONES)
      real       ttime
      double precision dttime


c...Initializations.

      data  x/1.,2.,3.,4./
      data  y/1.,2.,3./
      data  d/1.,2.,3.,4.,5.,6./

      ttime     = 2.345
      dttime    = 2.345
      tcycle    = 200



c...Create an option list containing time and cycle information. Note
c...that the function names are different depending on the data type
c...of the option value. The variable 'optlistid' is used as a handle
c...to this option list in future function invocations.

      ierr = dbmkoptlist (5, optlistid)
      ierr = dbaddropt   (optlistid, DBOPT_TIME, ttime)    ! real
      ierr = dbaddiopt   (optlistid, DBOPT_CYCLE, tcycle)  ! integer
      ierr = dbadddopt   (optlistid, DBOPT_DTIME, dttime)  ! double

c...The following option would change the major order for multi-
c...dimensional arrays from row-major (default) to column-major.
c...If you have arrays defined like:  real  x(300,200) you might
c...need this.

c     ierr = dbaddiopt   (optlistid, DBOPT_MAJORORDER, DB_COLMAJOR)


c...Write simple 2-D quad mesh. Dims defines number of NODES. The
c...parameter 'meshid' is returned and should be used on future
c...writes of quad variables.

      ndims     = 2
      dims(1)   = NX
      dims(2)   = NY


      err = dbputqm(dbid, name, lname,
     .              "X", 1, "Y", 1, DB_F77NULLSTRING, 0,
     .              x, y, -1, dims, ndims, DB_FLOAT, DB_COLLINEAR,
     .              optlistid, meshid)


c...Write quad variable. Dims defines number of ZONES (since zone-centered).
c...Possible values for centering are: DB_ZONECENT, DB_NODECENT, DB_NOTCENT.

      ndims   = 2
      dims(1) = NX - 1
      dims(2) = NY - 1

c...Hack to make sure we don't wind up writing same variable name twice to
c...two different meshes
      if (lname .eq. 4) err = dbputqv1 (dbid, "d", 1, name,
     .                lname, d, dims, ndims,
     .                DB_F77NULL, 0, DB_FLOAT, DB_ZONECENT,
     .                optlistid, varid)
      if (lname .eq. 5) err = dbputqv1 (dbid, "e", 1, name,
     .                lname, d, dims, ndims,
     .                DB_F77NULL, 0, DB_FLOAT, DB_ZONECENT,
     .                optlistid, varid)

      buildquad = meshid

      return
      end

c************************************************************
c************************************************************
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

c                       write integer hydro common to dump
      ialen = 10
      ierr = dbwrite ( dbid, "hcom2", 5, ihfirst, ialen, 1, DB_INT )

c                       write real hydro common to dump
      ialen = 15
      ierr = dbwrite ( dbid, "hcom3", 5, rhfirst, ialen, 1,DB_DOUBLE)


      return
      end
