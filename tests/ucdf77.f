C***********************************************************************
C Copyright (C) 1994-2016 Lawrence Livermore National Security, LLC.
C LLNL-CODE-425250.
C All rights reserved.
C 
C This file is part of Silo. For details, see silo.llnl.gov.
C 
C Redistribution and use in source and binary forms, with or without
C modification, are permitted provided that the following conditions
C are met:
C 
C    * Redistributions of source code must retain the above copyright
C      notice, this list of conditions and the disclaimer below.
C    * Redistributions in binary form must reproduce the above copyright
C      notice, this list of conditions and the disclaimer (as noted
C      below) in the documentation and/or other materials provided with
C      the distribution.
C    * Neither the name of the LLNS/LLNL nor the names of its
C      contributors may be used to endorse or promote products derived
C      from this software without specific prior written permission.
C 
C THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
C "AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
C LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
C A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
C LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
C CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
C EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
C PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
C PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
C LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
C NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
C SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
C 
C This work was produced at Lawrence Livermore National Laboratory under
C Contract No.  DE-AC52-07NA27344 with the DOE.
C 
C Neither the  United States Government nor  Lawrence Livermore National
C Security, LLC nor any of  their employees, makes any warranty, express
C or  implied,  or  assumes  any  liability or  responsibility  for  the
C accuracy, completeness,  or usefulness of  any information, apparatus,
C product, or  process disclosed, or  represents that its use  would not
C infringe privately-owned rights.
C 
C Any reference herein to  any specific commercial products, process, or
C services by trade name,  trademark, manufacturer or otherwise does not
C necessarily  constitute or imply  its endorsement,  recommendation, or
C favoring  by  the  United  States  Government  or  Lawrence  Livermore
C National Security,  LLC. The views  and opinions of  authors expressed
C herein do not necessarily state  or reflect those of the United States
C Government or Lawrence Livermore National Security, LLC, and shall not
C be used for advertising or product endorsement purposes.
C***********************************************************************

C---------------------------------------------------------------------
C  Purpose
C
C     Demonstrate use of SILO for creating unstructured cell data
C     (UCD) objects for visualization.
C
C---------------------------------------------------------------------
      program main

      implicit none
      include "silo.inc"
      integer builducd
      integer  dbid, meshid, err, driver, nargs
      character*256 cloption

      driver = DB_PDB
      nargs = iargc()
      call getarg(1, cloption)
      if (cloption .eq. "DB_HDF5") then
          driver = DB_HDF5
      end if

C...Create file named "ucdf77.silo". Database ID is returned in 'dbid'.

      err = dbcreate("ucdf77.silo", 11, 0, DB_LOCAL,
     .               "file info", 9, driver, dbid)


C...Write out objects for visualization 

      meshid = builducd(dbid, "ucd", 3)


C...Close data file.

      err = dbclose(dbid)


      print *,'Created file: ucdf77.silo'

      stop
      end


      integer function builducd (dbid, name, lname)
C----------------------------------------------------------------------
C  Routine                                                    builducd
C
C  Purpose
C
C       Build ucd-mesh, ucd-var, facelist and zonelist, and return
C       the mesh ID.
C
C  Modifications:
C    Kathleen Bonnell, Wed Sep 2 16:12:15 PDT 20099
C    Changed 'character*8 name' to 'character*(*) name' to remove 
C    'Character length argument mismatch' compiler error.
C
C-----------------------------------------------------------------------

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

C----------------------------------------------------------------------
C       The NODES in this 2D mesh looks like this:
C
C               0       4       8
C
C               1       5       9
C                                       12
C               2       6       10
C
C               3       7       11
C
C
C       The ZONES in the mesh look like this:
C       (there are six rectangle zones and one triangle zone)
C
C
C               -----------------\
C               |   0   |   1   | \
C               -----------------  \
C               |   2   |   3   | 6 >
C               -----------------  /
C               |   4   |   5   | /
C               -----------------/
C
C
C-----------------------------------------------------------------------

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
C     real*8   ttime
      real*4   ttime
      real*4   rtime
      real     dfnew(MAXMIX)


C...Initialize material data */
      data  matlist/1,-1, 2, 1,-3, 2, 2/
      data  matnos/1, 2/
      data  mix_vf/.6, .8, .4, .2/
      data  mix_next/2, 0, 4, 0/
      data  mix_mat/1, 2, 1, 2/
      data  mix_zone/0, 0, 3, 3/

C...Initialize coordinate and zonal data.
      data  x/9.,9.,9.,9.,10.,10.,10.,10.,11.,11.,11.,11.,12./
      data  y/3.,2.,1.,0., 3., 2., 1., 0., 3., 2., 1., 0.,1.5/
      data  d/1.,2.,3.,4.,5.,6.,7./
      data  u/0.,0.,0.,0.,1.,1.,1.,1.,2.,2.,2.,2.,3./
      data  df/.58,.78,.42,.22/

C...Initialize facelist and zonelist.
      data fnodelist/0,1,1,2,2,3,3,7,7,11,11,10,10,9,9,8,8,4,4,0,8,12,
     .               12,11/
      data znodelist/0,1,5,4,4,5,9,8,1,2,6,5,5,6,10,9,2,3,7,6,6,7,11,10,
     .               8,12,11 /
      data zshapesize /4,3/     ! shape 1 has four nodes, shape 2 has three
      data zshapecnt  /6,1/     ! six zones are shape 1, one zone is shape 2

      data fshapesize /2/       ! shape 1 has two nodes
      data fshapecnt  /NFACES/  ! NFACES faces are shape 1

      ttime  = 2.345
C     rtime  = sngl(ttime)
      rtime  = ttime
      tcycle = 200
      mixlen = MAXMIX


C...Create a rudimentary option list.

      ierr = dbmkoptlist(3, optlistid)                   ! Create the option list
      ierr = dbaddiopt  (optlistid, DBOPT_CYCLE, tcycle) ! Add integer opt
      ierr = dbaddropt  (optlistid, DBOPT_TIME, rtime)   ! Add real opt


C...Start by writing the external face list. This contains all of the
C...faces which are on the external surface of the mesh. For 2D
C...meshes, this is not really necessary, but for 3D it is essential.

      err = dbputfl(dbid, 'Facelist', 8, NFACES, 2, fnodelist,
     .              LFNODELIST, 0,
     .              DB_F77NULL, fshapesize, fshapecnt, NFSHAPES,
     .              DB_F77NULL, DB_F77NULL, 0, flid)



C...Write zone list. This contains the information which describes each
C...of the zones in the mesh. The nodelist must be organized such that
C...zones of each shape are described consecutively.

      err = dbputzl(dbid, 'Zonelist', 8, NZONES, 2, znodelist,
     .              LZNODELIST, 0,
     .              zshapesize, zshapecnt, NZSHAPES, zlid)



C...Write the UCD mesh. For any arguments which are not applicable,
C...use the DB_F77NULL argument. The mesh id is returned in 'meshid'.

      err = dbputum(dbid, name, lname, 2, x, y, DB_F77NULL,
     .              "X", 1, "Y", 1, DB_F77NULLSTRING, 0, DB_FLOAT,
     .              NNODES, NZONES, 'Zonelist', 8, 'Facelist', 8,
     .              optlistid, meshid)



C...Write out a UCD variable. Use dbputuv1 for scalar (non-vector)
C...arrays. The variable ID is returned in 'varid'.

      err = dbputuv1 (dbid, "d", 1, name, lname, d, NZONES, DB_F77NULL,
     .                0, DB_FLOAT, DB_ZONECENT, DB_F77NULL, varid)

      err = dbputuv1 (dbid, "u", 1, name, lname, u, NNODES, DB_F77NULL,
     .                0, DB_FLOAT, DB_NODECENT, DB_F77NULL, varid)


      builducd = meshid
      end


