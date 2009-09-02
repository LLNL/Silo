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

c---------------------------------------------------------------------
c  Purpose
c
c     Demonstrate use of SILO for creating unstructured cell data
c     (UCD) objects for viewing with MeshTV.
c
c---------------------------------------------------------------------
      program main

      implicit none
      include "silo.inc"
      integer builducd
      integer  dbid, meshid, err


c...Create file named "ucdf77.pdb". Database ID is returned in 'dbid'.

      err = dbcreate("ucdf77.pdb", 10, 0, DB_LOCAL,
     .               "file info", 9, DB_PDB, dbid)


c...Write out necessary objects for MeshTV.

      meshid = builducd(dbid, "ucd", 3)


c...Close data file.

      err = dbclose(dbid)


      print *,'Created file: ucdf77.pdb'

      stop
      end


      integer function builducd (dbid, name, lname)
c----------------------------------------------------------------------
c  Routine						      builducd
c
c  Purpose
c
c	Build ucd-mesh, ucd-var, facelist and zonelist, and return
c	the mesh ID.
c
c  Modifications:
c    Kathleen Bonnell, Wed Sep 2 16:12:15 PDT 20099
c    Changed 'character*8 name' to 'character*(*) name' to remove 
c    'Character length argument mismatch' compiler error.
c
c-----------------------------------------------------------------------

      integer  dbid             ! Database identifier
      character*(*) name        ! Name of mesh to build
      integer     lname         ! Length of name

      include "silo.inc"       ! Don't forget to include this file

      parameter  (NMATS  = 2)   ! Number of materials
      parameter  (NZONES = 7)   ! Number of zones
      parameter  (NFACES = 12)  ! Number of external faces
      parameter  (NNODES = 13)  ! Number of nodes
      parameter  (MAXMIX = 12)  ! Max length of mixed data
      parameter  (MAXOMIX= 4)
      parameter  (NZSHAPES= 2)  ! Number of zone shapes
      parameter  (NFSHAPES= 1)  ! Number of face shapes
      parameter  (NFTYPES = 0)  ! Number of face types
      parameter  (LZNODELIST= 27) ! Length of zone nodelist
      parameter  (LFNODELIST= 24) ! Length of face nodelist

c----------------------------------------------------------------------
c       The NODES in this 2D mesh looks like this:
c
c  	  	0	4	8
c
c  	  	1	5	9
c                                       12
c  	  	2	6	10
c
c  	  	3	7	11
c
c
c       The ZONES in the mesh look like this:
c       (there are six rectangle zones and one triangle zone)
c
c
c               -----------------\
c               |   0   |   1   | \
c               -----------------  \
c               |   2   |   3   | 6 >
c               -----------------  /
c               |   4   |   5   | /
c               -----------------/
c
c
c-----------------------------------------------------------------------

      integer      i, flid, zlid, meshid, varid, matid, optlistid
      integer      matlist(NZONES), matnos(NMATS)
      integer      mix_next(MAXOMIX), mix_mat(MAXOMIX),
     .             mix_zone(MAXOMIX)
      real         mix_vf(MAXOMIX)

      real         x(NNODES), y(NNODES), d(NZONES), df(MAXOMIX)
      real         u(NNODES)
      integer      fnodelist(LFNODELIST), znodelist(LZNODELIST)
      integer      zshapesize(NZSHAPES), zshapecnt(NZSHAPES)

      integer  fshapesize, fshapecnt
      integer  tcycle, mixlen
      integer  dims
c     real*8   ttime
      real*4   ttime
      real*4   rtime
      real     dfnew(MAXMIX)


c...Initialize material data */
      data  matlist/1,-1, 2, 1,-3, 2, 2/
      data  matnos/1, 2/
      data  mix_vf/.6, .8, .4, .2/
      data  mix_next/2, 0, 4, 0/
      data  mix_mat/1, 2, 1, 2/
      data  mix_zone/0, 0, 3, 3/

c...Initialize coordinate and zonal data.
      data  x/9.,9.,9.,9.,10.,10.,10.,10.,11.,11.,11.,11.,12./
      data  y/3.,2.,1.,0., 3., 2., 1., 0., 3., 2., 1., 0.,1.5/
      data  d/1.,2.,3.,4.,5.,6.,7./
      data  u/0.,0.,0.,0.,1.,1.,1.,1.,2.,2.,2.,2.,3./
      data  df/.58,.78,.42,.22/

c...Initialize facelist and zonelist.
      data fnodelist/0,1,1,2,2,3,3,7,7,11,11,10,10,9,9,8,8,4,4,0,8,12,
     .               12,11/
      data znodelist/0,1,5,4,4,5,9,8,1,2,6,5,5,6,10,9,2,3,7,6,6,7,11,10,
     .               8,12,11 /
      data zshapesize /4,3/     ! shape 1 has four nodes, shape 2 has three
      data zshapecnt  /6,1/     ! six zones are shape 1, one zone is shape 2

      data fshapesize /2/       ! shape 1 has two nodes
      data fshapecnt  /NFACES/  ! NFACES faces are shape 1

      ttime  = 2.345
c     rtime  = sngl(ttime)
      rtime  = ttime
      tcycle = 200
      mixlen = MAXMIX


c...Create a rudimentary option list.

      ierr = dbmkoptlist(3, optlistid)                   ! Create the option list
      ierr = dbaddiopt  (optlistid, DBOPT_CYCLE, tcycle) ! Add integer opt
      ierr = dbaddropt  (optlistid, DBOPT_TIME, rtime)   ! Add real opt


c...Start by writing the external face list. This contains all of the
c...faces which are on the external surface of the mesh. For 2D
c...meshes, this is not really necessary, but for 3D it is essential.

      err = dbputfl(dbid, 'Facelist', 8, NFACES, 2, fnodelist,
     .              LFNODELIST, 0,
     .              DB_F77NULL, fshapesize, fshapecnt, NFSHAPES,
     .              DB_F77NULL, DB_F77NULL, 0, flid)



c...Write zone list. This contains the information which describes each
c...of the zones in the mesh. The nodelist must be organized such that
c...zones of each shape are described consecutively.

      err = dbputzl(dbid, 'Zonelist', 8, NZONES, 2, znodelist,
     .              LZNODELIST, 0,
     .              zshapesize, zshapecnt, NZSHAPES, zlid)



c...Write the UCD mesh. For any arguments which are not applicable,
c...use the DB_F77NULL argument. The mesh id is returned in 'meshid'.

      err = dbputum(dbid, name, lname, 2, x, y, DB_F77NULL,
     .              "X", 1, "Y", 1, DB_F77NULLSTRING, 0, DB_FLOAT,
     .              NNODES, NZONES, 'Zonelist', 8, 'Facelist', 8,
     .              optlistid, meshid)



c...Write out a UCD variable. Use dbputuv1 for scalar (non-vector)
c...arrays. The variable ID is returned in 'varid'.

      err = dbputuv1 (dbid, "d", 1, name, lname, d, NZONES, DB_F77NULL,
     .                0, DB_FLOAT, DB_ZONECENT, DB_F77NULL, varid)

      err = dbputuv1 (dbid, "u", 1, name, lname, u, NNODES, DB_F77NULL,
     .                0, DB_FLOAT, DB_NODECENT, DB_F77NULL, varid)


      builducd = meshid
      end


