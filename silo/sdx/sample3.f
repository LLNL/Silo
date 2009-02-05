************************************************************************
*
*                            Copyright 1991 - 1995
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

************************************************************************
*
*     This is a sample program which shows how to use the SDX library
*     in a typical application.  It consists of a simulation that
*     iterates 400 times moving the center of a density and pressure
*     field, as well as a material.
*
*     The mesh is a 3d ucd mesh.
*
*     The program iterates 400 times.  Every 5 cycles it tells the
*     library that it has new data to plot.
*
************************************************************************

      program sample3

      implicit none

      include "silo.inc"
      include "sdx.inc"

      integer initsdx

      integer i
      integer ier
      integer sdxid

      call initsim

      write (6, "('connecting to the remote machine')")

      sdxid = initsdx ()

      do 100 i = 1, 400

         call nextcycle

         write (6, "('cycle ',i6)") i

         if (mod (i, 5) .eq. 0) then
            ier = sdxnewdata (sdxid)
         endif
         if (mod (i, 800) .eq. 0) then
            call dumpsim
         endif

         call procevents

100   continue

      ier = sdxclose (sdxid)

      end

************************************************************************
*
*     Initialize the sdx library.
*
************************************************************************

      integer function initsdx ()

      implicit none

      include "silo.inc"
      include "sdx.inc"

      integer      nvars
      character*64 varnames (4)
      character*64 meshnames (4)
      integer      vartypes (4)
      common /toc/ nvars, varnames, meshnames, vartypes

      integer      ier
      integer      nmats, nblocks
      integer      machlen, userlen
      character*40 machname, username
      integer      sdxid

      nvars = 4

      varnames (1) = "d"
      varnames (2) = "p"
      varnames (3) = "mesh1"
      varnames (4) = "mat1"

      meshnames (1) = "mesh1"
      meshnames (2) = "mesh1"
      meshnames (3) = "mesh1"
      meshnames (4) = "mesh1"

      vartypes (1) = DB_UCDVAR
      vartypes (2) = DB_UCDVAR
      vartypes (3) = DB_UCDMESH
      vartypes (4) = DB_MATERIAL

      nmats = 2
      nblocks = 0
c
c     Get the machine address and user name.
c
      open (unit=15, file="sdxinfo", status="old")
      read (unit=15, fmt=900) machname
      read (unit=15, fmt=900) username
      close (unit=15)

      machlen = 1
100   continue
      if (machname(machlen:machlen) .eq. ' ') goto 110
      machlen = machlen + 1
      go to 100
110   continue
      machlen = machlen - 1

      userlen = 1
120   continue
      if (username(userlen:userlen) .eq. ' ') goto 130
      userlen = userlen + 1
      go to 120
130   continue
      userlen = userlen - 1

      ier = sdxopen (machname, machlen, username, userlen,
     x               "ucd_3d", 6, nvars,
     x               varnames, meshnames, vartypes,
     x               nmats, nblocks, sdxid)

      initsdx = sdxid

900   format (a40)
910   format (a40,i5)
      return
      end

************************************************************************
*
*     Process the events that come from sdx.
*
************************************************************************

      subroutine procevents

      implicit none

      include "silo.inc"
      include "sdx.inc"

      integer cycle
      common /cycle/ cycle
      real ttot
      common /ttot/ ttot
      integer      nvars
      character*64 varnames (4)
      character*64 meshnames (4)
      integer      vartypes (4)
      common /toc/ nvars, varnames, meshnames, vartypes
      integer nmat, matnos (2), matlist (2000)
      common /materials/ nmat, matnos, matlist
      real x (2646), y (2646), z (2646)
      integer ndims, nnodes, nzones
      integer zshapecnt, zshapesize, nzshapes
      integer lznodelist, znodelist (16000)
      integer nfaces, lfnodelist, fnodelist (16000), fshapecnt (4)
      integer fshapesize (4), nfshapes, fzoneno (8000), lfzoneno
      common /mesh/ x, y, z, ndims, nnodes, nzones, zshapecnt,
     x              zshapesize, nzshapes, lznodelist, znodelist,
     x              nfaces, lfnodelist, fnodelist, fshapecnt,
     x              fshapesize, nfshapes, fzoneno, lfzoneno
      real d (2646), p (2646)
      common /data/ d, p

      integer eventtype, readtype
      character*80 readvar
      integer ier, idoptl

c
c     Create an option list.
c
      ier = dbmkoptlist (2, idoptl)
      ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
      ier = dbaddropt (idoptl, DBOPT_TIME, ttot)

100   continue

c
c     Get the next event.
c
      ier = sdxnextevent (eventtype, readtype, readvar, 80)

c     write (6, "('eventtype= ',i6,' readtype= ',i6,' readvar= ',a12)")
c    x   eventtype, readtype, readvar
c
c     If their are no more events the return to the main simulation loop.
c
      if (eventtype .eq. CONTINUE) then

         go to 110

c
c     If a variable is desired then return the variable.  If the
c     variable type or variable name is not recognized the return
c     an error.
c
      else if (eventtype .eq. READV) then

         if (readtype .eq. DB_UCDMESH) then
            if (readvar .eq. "mesh1") then
               ier = sdxputum ("mesh1", 5, 3, x, y, z, "x", 1, "y", 1,
     x                         "z", 1, DB_FLOAT, nnodes, nzones,
     x                         "zl", 2, "fl", 2, idoptl)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. DB_FACELIST) then
            if (readvar .eq. "mesh1") then
               ier = sdxputfl ("mesh1", 5, nfaces, ndims, fnodelist,
     x                         lfnodelist, 1, fzoneno, fshapesize,
     x                         fshapecnt, nfshapes, 0, 0, 0)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. DB_ZONELIST) then
            if (readvar .eq. "mesh1") then
               ier = sdxputzl ("mesh1", 5, nzones, ndims, znodelist,
     x                         lznodelist, 1, zshapesize, zshapecnt, 1)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. DB_UCDVAR) then
            if (readvar .eq. "d") then
               ier = sdxputuv ("d", 1, "mesh1", 5, d, nnodes, 0, 0,
     x                         DB_FLOAT, DB_NODECENT, idoptl)
            else if (readvar .eq. "p") then
               ier = sdxputuv ("p", 1, "mesh1", 5, p, nnodes, 0, 0,
     x                         DB_FLOAT, DB_NODECENT, idoptl)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. DB_MATERIAL) then
            if (readvar .eq. "mat1") then
               ier = sdxputmat ("mat1", 4, "mesh1", 5, nmat, matnos,
     x                          matlist, nzones, 1, 0, 0, 0, 0, 0,
     x                          DB_FLOAT, idoptl)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. DB_MULTIBLOCKMESH) then
            ier = sdxmmesh (1, "mesh1", DB_UCDMESH)
         else if (readtype .eq. WMESHNAME) then
            ier = sdxchar ("mesh1", 5)
         else if (readtype .eq. WMESHTYPE) then
            if (readvar .eq. "mesh1") then
               ier = sdxinteger (DB_UCDMESH)
            else if (readvar .eq. "mat1") then
               ier = sdxinteger (DB_MATERIAL)
            else
               ier = sdxinteger (DB_UCDVAR)
            endif
         else if (readtype .eq. WVARIABLE) then
            if (readvar .eq. "time") then
               ier = sdxfloat (ttot)
            else if (readvar .eq. "cycle") then
               ier = sdxinteger (cycle)
            else if (readvar .eq. "noreg") then
               ier = sdxinteger (nmat)
            else if (readvar .eq. "_fileinfo") then
               ier = sdxchar ("3D unstructured mesh", 20)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. WVARLENGTH) then
            if (readvar .eq. "time") then
               ier = sdxinteger (1)
            else if (readvar .eq. "cycle") then
               ier = sdxinteger (1)
            else if (readvar .eq. "noreg") then
               ier = sdxinteger (1)
            else if (readvar .eq. "_fileinfo") then
               ier = sdxinteger (20)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. WVARBLENGTH) then
            if (readvar .eq. "time") then
               ier = sdxinteger (4)
            else if (readvar .eq. "cycle") then
               ier = sdxinteger (4)
            else if (readvar .eq. "noreg") then
               ier = sdxinteger (4)
            else if (readvar .eq. "_fileinfo") then
               ier = sdxinteger (20)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. TOC) then
            ier = sdxtoc (nvars, varnames, vartypes)
         else
            ier = sdxerror ()
         endif

      else if (eventtype .ne. PAUSE) then

         ier = sdxerror ()

      endif

      go to 100

110   continue

c
c     Free the option list.
c
      ier = dbfreeoptlist (idoptl)

      return
      end

************************************************************************
*
*     Initialize the simulation.  This causes the mesh to be
*     initialized.
*
************************************************************************

      subroutine initsim

      implicit none

      include "silo.inc"
      include "sdx.inc"

      integer cycle
      common /cycle/ cycle
      real ttot
      common /ttot/ ttot
      integer nmat, matnos (2), matlist (2000)
      common /materials/ nmat, matnos, matlist
      real x (2646), y (2646), z (2646)
      integer ndims, nnodes, nzones
      integer zshapecnt, zshapesize, nzshapes
      integer lznodelist, znodelist (16000)
      integer nfaces, lfnodelist, fnodelist (16000), fshapecnt (4)
      integer fshapesize (4), nfshapes, fzoneno (8000), lfzoneno
      common /mesh/ x, y, z, ndims, nnodes, nzones, zshapecnt,
     x              zshapesize, nzshapes, lznodelist, znodelist,
     x              nfaces, lfnodelist, fnodelist, fshapecnt,
     x              fshapesize, nfshapes, fzoneno, lfzoneno
      real d (2646), p (2646)
      common /data/ d, p

      integer ier
      integer i, j, k, iz
      real h, dh, theta, dtheta, r, dr
      integer origin, method

c
c     Set up miscillaneous variables.
c
      cycle = 0
      ttot  = 0.

      nmat = 2
      matnos (1) = 1
      matnos (2) = 2

c
c     Set up the mesh.
c
      ndims = 3
      nnodes = 21 * 21 * 6
      nzones = 20 * 20 * 5

      lznodelist = (20 * 20 * 5) * 8
      zshapecnt = nzones
      zshapesize = 8
      nzshapes = 1

      dh = 20. / 20.
      dtheta = (180. / 20.) * (3.1415926 / 180.)
      dr = 3. / 5.
      h = 0.
      do 100 i = 0, 20
         theta = 0
         do 110 j = 0, 20
            r = 2.
            do 120 k = 0, 5
               x (i*126+j*6+k+1) = r * cos (theta)
               y (i*126+j*6+k+1) = r * sin (theta)
               z (i*126+j*6+k+1) = h
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
               znodelist (iz)   = (i)  *126+(j)  *6+k  +1
               znodelist (iz+1) = (i)  *126+(j+1)*6+k  +1
               znodelist (iz+2) = (i)  *126+(j+1)*6+k+1+1
               znodelist (iz+3) = (i)  *126+(j)  *6+k+1+1
               znodelist (iz+4) = (i+1)*126+(j)  *6+k  +1
               znodelist (iz+5) = (i+1)*126+(j+1)*6+k  +1
               znodelist (iz+6) = (i+1)*126+(j+1)*6+k+1+1
               znodelist (iz+7) = (i+1)*126+(j)  *6+k+1+1
               iz = iz + 8
220         continue
210      continue
200   continue

      do 300 i = 1, 2000
         matlist (i) = 1
300   continue

      lfnodelist = 16000
      lfzoneno = 8000
      origin = 1
      method = 1
      ier = sdxcalcfl (znodelist, nnodes, origin, zshapesize, zshapecnt,
     x                 nzshapes, matlist, method, nfaces, fnodelist,
     x                 lfnodelist, fshapesize, fshapecnt, nfshapes,
     x                 fzoneno, lfzoneno)

      return
      end

************************************************************************
*
*     This moves the density and pressure field in the positive x
*     direction.
*
************************************************************************

      subroutine nextcycle

      implicit none

      include "silo.inc"
      include "sdx.inc"

      integer cycle
      common /cycle/ cycle
      real ttot
      common /ttot/ ttot
      real zcenter
      common /zcenter/ zcenter
      data zcenter / 0. /
      integer nmat, matnos (2), matlist (2000)
      common /materials/ nmat, matnos, matlist
      real x (2646), y (2646), z (2646)
      integer ndims, nnodes, nzones
      integer zshapecnt, zshapesize, nzshapes
      integer lznodelist, znodelist (16000)
      integer nfaces, lfnodelist, fnodelist (16000), fshapecnt (4)
      integer fshapesize (4), nfshapes, fzoneno (8000), lfzoneno
      common /mesh/ x, y, z, ndims, nnodes, nzones, zshapecnt,
     x              zshapesize, nzshapes, lznodelist, znodelist,
     x              nfaces, lfnodelist, fnodelist, fshapecnt,
     x              fshapesize, nfshapes, fzoneno, lfzoneno
      real d (2646), p (2646)
      common /data/ d, p

      integer i, iz
      integer ier
      real z1, z2
      integer origin, method

      cycle = cycle + 1
      ttot = ttot + .01

      zcenter = zcenter + .2
      if (zcenter .gt. 20) then
         zcenter = zcenter - 20.
      endif

c
c     Move the scalar fields.
c
      do 100 i = 1, 2646
         d (i) = 5. - sqrt ((z (i) - zcenter) ** 2)
         p (i) = sqrt (5.) - sqrt (sqrt ((z (i) - zcenter) ** 2))
100   continue

c
c     Move the materials
c
      iz = 1
      do 200 i = 1, 2000
         z1 = z (znodelist (iz))
         z2 = z (znodelist (iz+4))
         if (z2 .lt. zcenter - 2.) then
            matlist (i) = 1
         else if (z1 .gt. zcenter + 2.) then
            matlist (i) = 1
         else
            matlist (i) = 2
         endif
         iz = iz + 8
200   continue

      origin = 1
      method = 0
c     call sdxcalcfl (znodelist, nnodes, origin, zshapesize, zshapecnt,
c    x                nzshapes, matlist, method, nfaces, fnodelist,
c    x                lfnodelist, fshapesize, fshapecnt, nfshapes,
c    x                fzoneno, lfzoneno)

      ier = sdxpause (1)

      return
      end

************************************************************************
*
*     Write a dumpfile.
*
************************************************************************

      subroutine dumpsim

      implicit none

      include "silo.inc"

      integer cycle
      common /cycle/ cycle
      real ttot
      common /ttot/ ttot
      integer nmat, matnos (2), matlist (2000)
      common /materials/ nmat, matnos, matlist
      real x (2646), y (2646), z (2646)
      integer ndims, nnodes, nzones
      integer zshapecnt, zshapesize, nzshapes
      integer lznodelist, znodelist (16000)
      integer nfaces, lfnodelist, fnodelist (16000), fshapecnt (4)
      integer fshapesize (4), nfshapes, fzoneno (8000), lfzoneno
      common /mesh/ x, y, z, ndims, nnodes, nzones, zshapecnt,
     x              zshapesize, nzshapes, lznodelist, znodelist,
     x              nfaces, lfnodelist, fnodelist, fshapecnt,
     x              fshapesize, nfshapes, fzoneno, lfzoneno
      real d (2646), p (2646)
      common /data/ d, p

      integer ier, dbid, zlid, flid, umid, matid, did, pid
      integer idoptl
      character*80 filename

c
c     Create the file and the option list.
c
      ier = dbmkoptlist (2, idoptl)
      ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
      ier = dbaddropt (idoptl, DBOPT_TIME, ttot)

      write (filename, '("ucd", i4.4, ".silo")') cycle
      ier = dbcreate (filename, 12, 1, 0, 'ucd_dump', 8, DB_PDB,
     x                dbid)

c
c     Write out the mesh, materials and variables.
c
      ier = dbputzl (dbid, 'zl', 2, nzones, ndims, znodelist,
     x               lznodelist, 1, zshapesize, zshapecnt, nzshapes,
     x               zlid)
      ier = dbputfl (dbid, 'fl', 2, nfaces, ndims, fnodelist,
     x               lfnodelist, 1, fzoneno, fshapesize, fshapecnt,
     x               nfshapes, 0, 0, 0, flid)
      ier = dbputum (dbid, 'mesh1', 5, 3, x, y, z, 'x', 1, 'y', 1,
     x               'z', 1, DB_FLOAT, nnodes, nzones, 'zl', 2, 'fl', 2,
     x               idoptl, umid)

      ier = dbputmat (dbid, 'mat1', 4, 'mesh1', 5, nmat, matnos,
     x                matlist, nzones, 1, 0, 0, 0, 0, 0, DB_FLOAT,
     x                idoptl, matid)

      ier = dbputuv1 (dbid, 'd', 1, 'mesh1', 5, d, nnodes, 0, 0,
     x                DB_FLOAT, DB_NODECENT, idoptl, did)
      ier = dbputuv1 (dbid, 'p', 1, 'mesh1', 5, p, nnodes, 0, 0,
     x                DB_FLOAT, DB_NODECENT, idoptl, pid)

c
c     Free the option list.
c
      ier = dbfreeoptlist (idoptl)

c
c     Close the file.
c
      ier = dbclose (dbid)

      return
      end
