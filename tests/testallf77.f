************************************************************************
* Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
* LLNL-CODE-425250.
* All rights reserved.
* 
* This file is part of Silo. For details, see silo.llnl.gov.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 
*    * Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the disclaimer below.
*    * Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the disclaimer (as noted
*      below) in the documentation and/or other materials provided with
*      the distribution.
*    * Neither the name of the LLNS/LLNL nor the names of its
*      contributors may be used to endorse or promote products derived
*      from this software without specific prior written permission.
* 
* THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
* "AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
* LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
* LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
* CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
* PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
* NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* This work was produced at Lawrence Livermore National Laboratory under
* Contract No.  DE-AC52-07NA27344 with the DOE.
* 
* Neither the  United States Government nor  Lawrence Livermore National
* Security, LLC nor any of  their employees, makes any warranty, express
* or  implied,  or  assumes  any  liability or  responsibility  for  the
* accuracy, completeness,  or usefulness of  any information, apparatus,
* product, or  process disclosed, or  represents that its use  would not
* infringe privately-owned rights.
* 
* Any reference herein to  any specific commercial products, process, or
* services by trade name,  trademark, manufacturer or otherwise does not
* necessarily  constitute or imply  its endorsement,  recommendation, or
* favoring  by  the  United  States  Government  or  Lawrence  Livermore
* National Security,  LLC. The views  and opinions of  authors expressed
* herein do not necessarily state  or reflect those of the United States
* Government or Lawrence Livermore National Security, LLC, and shall not
* ***********************************************************************

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
      integer  dbid, meshid, err, driver, nargs
      character*256 cloption 

      integer buildrect2d
      integer builducd

c...Create file named "rectf77.silo". Database ID is returned in 'dbid'.

      driver = DB_PDB
      nargs = iargc()
      call getarg(1, cloption)
      if (cloption .eq. "DB_HDF5") then
          driver = DB_HDF5
      end if

      err = dbcreate("rectf77.silo", 12, 0, DB_LOCAL,
     .               "file info", 9, driver, dbid)


c...Write out necessary objects for MeshTV.

      err = buildrect2d(dbid, "rect", 4)


c...Close data file.

      err = dbclose(dbid)


      print *,'Created file: rectf77.silo'

c...Create file named "ucdf77.silo". Database ID is returned in 'dbid'.

      err = dbcreate("ucdf77.silo", 11, 0, DB_LOCAL,
     .               "file info", 9, driver, dbid)


c...Write out necessary objects for MeshTV.

      err = builducd(dbid, "ucd", 3)


c...Close data file.

      err = dbclose(dbid)


      print *,'Created file: ucdf77.silo'

      stop
      end

      integer function buildrect2d (dbid, name, lname)
c----------------------------------------------------------------------
c  Routine                                                 buildrect2d
c
c  Purpose
c
c       Build rect-mesh, rect-var, and return the mesh ID.
c
c  Modifications:
c    Kathleen Bonnell, Wed Sep 2 16:12:15 PDT 20099
c    Changed 'character*8 name' to 'character*(*) name' to remove 
c    'Character length argument mismatch' compiler error.
c
c-----------------------------------------------------------------------

      implicit none

      integer        dbid            ! Database identifier
      character*(*)  name            ! Name of mesh to build
      integer        lname           ! Length of name

      include        "silo.inc"      ! Don't forget to include this file

      integer        err, ierr
      integer        cycle
      real           time
      real           x(31), y(41)
      integer        dims(2)
      integer        ndims

      integer        optlistid

      integer        i

c
c     Create the mesh.
c
      ndims = 2
      dims (1) = 30
      dims (2) = 40
      do 100 i = 1, 31
          x(i) = (i - 1) * (1. / 30)
100   continue
      do 110 i = 1, 41
          y(i) = (i - 1) * (1. / 40)
110   continue


c
c     Write out the variables.
c
      cycle = 48
      time = 4.8

      ierr = dbmkoptlist(8, optlistid)
      ierr = dbaddiopt  (optlistid, DBOPT_CYCLE, cycle)
      ierr = dbaddropt  (optlistid, DBOPT_TIME, time)
      ierr = dbaddcopt  (optlistid, DBOPT_XLABEL, "R", 1)
      ierr = dbaddcopt  (optlistid, DBOPT_YLABEL, "Z", 1)
      ierr = dbaddcopt  (optlistid, DBOPT_XUNITS, "cm", 2)
      ierr = dbaddcopt  (optlistid, DBOPT_YUNITS, "cm", 2)

      err = dbputqm (dbid, "mesh1", 5, "xcoords", 7, "ycoords", 7,
     .               "zcoords", 7, x, y, DB_F77NULL, dims, ndims,
     .               DB_FLOAT, DB_COLLINEAR, optlistid, ierr)

      buildrect2d = 0

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

      implicit none

      integer        dbid            ! Database identifier
      character*(*)  name            ! Name of mesh to build
      integer        lname           ! Length of name

      include        "silo.inc"      ! Don't forget to include this file

      integer        err, ierr
      integer        cycle
      real           time
      real           x(2646), y(2646), z(2646)
      integer        ndims
      integer        nzones, nnodes
      integer        lzonelist
      integer        zshapesize, zshapecnt
      integer        zonelist(16000)

      real           d(2646), p(2646), u(2646), v(2646), w(2646)

      integer        nmats
      integer        matnos(3)
      integer        matlist(2000)
      integer        mixlen
      integer        mix_next(900), mix_mat(900), mix_zone(900)
      real           mix_vf(900)

      integer        optlistid
      integer        flid, one

      integer        i, j, k
      integer        iz
      real           xcenter, ycenter
      real           theta, dtheta
      real           r, dr
      real           h, dh
      real           dist

      character*1024 mnames(3)
      integer        lmnames(3)
      character*1024 colors(3)
      integer        lcolors(3)
c     Create the mesh.
c
      ndims = 3
      nnodes = 21 * 21 * 6
      nzones = 20 * 20 * 5

      lzonelist = (20 * 20 * 5) * 8
      zshapecnt = nzones
      zshapesize = 8

      dh = 20. / 20.
      dtheta = (180. / 20.) * (3.1415926 / 180.)
      dr = 3. / 5.
      h = 0.
      do 100 i = 0, 20
         theta = 0.
         do 110 j = 0, 20
            r = 2.
            do 120 k = 0, 5
               x(i * 126 + j * 6 + k + 1) = r * cos(theta)
               y(i * 126 + j * 6 + k + 1) = r * sin(theta)
               z(i * 126 + j * 6 + k + 1) = h
               r = r + dr
120         continue
            theta = theta + dtheta
110      continue
         h = h + dh
100   continue

      iz = 1
      do 200 i = 0, 19
         do 210 j = 0, 19
            do 220 k = 0, 4
               zonelist(iz) = (i) * 126 + (j) * 6 + k
               zonelist(iz + 1) = (i) * 126 + (j + 1) * 6 + k
               zonelist(iz + 2) = (i) * 126 + (j + 1) * 6 + k + 1
               zonelist(iz + 3) = (i) * 126 + (j) * 6 + k + 1
               zonelist(iz + 4) = (i + 1) * 126 + (j) * 6 + k
               zonelist(iz + 5) = (i + 1) * 126 + (j + 1) * 6 + k
               zonelist(iz + 6) = (i + 1) * 126 + (j + 1) * 6 + k + 1
               zonelist(iz + 7) = (i + 1) * 126 + (j) * 6 + k + 1
               iz = iz + 8
220         continue
210      continue
200   continue

c
c     Create the density and pressure arrays.
c
      xcenter = 0.
      ycenter = 0.
      do 300 i = 0, 20
         do 310 j = 0, 20
            do 320 k = 0, 5
               dist = sqrt((x(i * 126 + j * 6 + k + 1) - xcenter) *
     .                     (x(i * 126 + j * 6 + k + 1) - xcenter) +
     .                     (y(i * 126 + j * 6 + k + 1) - ycenter) *
     .                     (y(i * 126 + j * 6 + k + 1) - ycenter))
               d(i * 126 + j * 6 + k + 1) = dist
               p(i * 126 + j * 6 + k + 1) = 1. / (dist + .0001)
320         continue
310      continue
300   continue

c
c     Create the velocity component arrays.
c
      xcenter = 0.
      ycenter = 0.
      do 400 i = 0, 20
         do 410 j = 0, 20
            do 420 k = 0, 5
               dist = sqrt((x(i+1) - xcenter) * (x(i+1) - xcenter) +
     .                     (y(j+1) - ycenter) * (y(j+1) - ycenter))
               u(i * 126 + j * 6 + k + 1) = (x(i+1) - xcenter) / dist
               v(i * 126 + j * 6 + k + 1) = (y(j+1) - ycenter) / dist
               w(i * 126 + j * 6 + k + 1) = 0.
420         continue
410      continue
400   continue

c
c     Create the material array.
c
      nmats = 3
      matnos(1) = 1
      matnos(2) = 2
      matnos(3) = 3
      mixlen = 0

c
c     Put in the material in 3 shells.
c
      do 500 i = 0, 19
         do 510 j = 0, 19
            do 520 k = 0, 1
               matlist(i * 100 + j * 5 + k + 1) = 1
520         continue
            do 530 k = 2, 3
               matlist(i * 100 + j * 5 + k + 1) = 2
530         continue
            do 540 k = 4, 4
               matlist(i * 100 + j * 5 + k + 1) = 3
540         continue
510      continue
500   continue

c
c     Write out the variables.
c
      cycle = 48
      time = 4.8
      one = 1

      ierr = dbset2dstrlen(1024)
      ierr = dbmkoptlist(10, optlistid)                  ! Create the option list
      ierr = dbaddiopt  (optlistid, DBOPT_CYCLE, cycle) ! Add integer opt
      ierr = dbaddropt  (optlistid, DBOPT_TIME, time)   ! Add real opt

      err = dbcalcfl (zonelist, nnodes, 0, zshapesize, zshapecnt, 1,
     .                matlist, 1, flid) 
      err = dbwrtfl (dbid, "facelist", 8, flid, ierr)

      err = dbputzl (dbid, "zonelist", 8, nzones, ndims, zonelist,
     .               lzonelist, 0, zshapesize, zshapecnt, 1, ierr)

      err = dbputum (dbid, "mesh1", 5, ndims, x, y, z,
     .               "xcoords", 7, "ycoords", 7, "zcoords", 7, DB_FLOAT,
     .               nnodes, nzones, "zonelist", 8, "facelist", 8,
     .               optlistid, ierr)

      err = dbputuv1 (dbid, "d", 1, "mesh1", 5, d, nnodes, DB_F77NULL,
     .                0, DB_FLOAT, DB_NODECENT, optlistid, ierr)

      err = dbputuv1 (dbid, "p", 1, "mesh1", 5, p, nnodes, DB_F77NULL,
     .                0, DB_FLOAT, DB_NODECENT, optlistid, ierr)

      err = dbputuv1 (dbid, "u", 1, "mesh1", 5, u, nnodes, DB_F77NULL,
     .                0, DB_FLOAT, DB_NODECENT, optlistid, ierr)

      err = dbputuv1 (dbid, "v", 1, "mesh1", 5, v, nnodes, DB_F77NULL,
     .                0, DB_FLOAT, DB_NODECENT, optlistid, ierr)

      err = dbputuv1 (dbid, "w", 1, "mesh1", 5, w, nnodes, DB_F77NULL,
     .                0, DB_FLOAT, DB_NODECENT, optlistid, ierr)

      ierr = dbaddropt  (optlistid, DBOPT_ALLOWMAT0, one)
      mnames(1) = "Ear Wax"
      mnames(2) = "Drool"
      mnames(3) = "Vaporware"
      lmnames(1) = 7 
      lmnames(2) = 5 
      lmnames(3) = 9
      ierr = dbaddcaopt  (optlistid, DBOPT_MATNAMES,
     .                nmats, mnames, lmnames)
      colors(1) = "Green"
      colors(2) = "Red"
      colors(3) = "Baby Blue"
      lcolors(1) = 5
      lcolors(2) = 3
      lcolors(3) = 9 
      ierr = dbaddcaopt  (optlistid, DBOPT_MATCOLORS,
     .                nmats, colors, lcolors)

      err = dbputmat (dbid, "mat1", 4, "mesh1", 5, nmats, matnos,
     .                matlist, nzones, 1, mix_next, mix_mat,
     .                mix_zone, mix_vf, mixlen, DB_FLOAT, optlistid,
     .                ierr)

      builducd = 0

      end

