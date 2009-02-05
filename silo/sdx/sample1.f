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
*     iterates 100 times moving the center of a density and pressure
*     in along the positive x direction.
*
*     The mesh is a 2d quad mesh.
*
*     The program iterates 100 times.  Every 20 cycles it tells the
*     library that it has new data to plot.
*
************************************************************************

      program sample1

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

      do 100 i = 1, 4000

         call nextcycle

         write (6, "('cycle ',i6)") i

c        if (mod (i, 50) .eq. 0) then
c           ier = sdxclose (sdxid)
c           sdxid = initsdx ()
c        endif

         if (mod (i, 5) .eq. 0) then
            ier = sdxnewdata (sdxid)
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

      integer      ier
      integer      nvars
      character*64 varnames (4)
      character*64 meshnames (4)
      integer      vartypes (4)
      common /toc/ nvars, varnames, meshnames, vartypes

      integer      machlen, userlen
      character*40 machname, username
      integer      sdxid
      integer      nmats, nblocks

      nvars = 4

      varnames (1) = "d"
      varnames (2) = "p"
      varnames (3) = "mesh1"
      varnames (4) = "mat1"

      meshnames (1) = "mesh1"
      meshnames (2) = "mesh1"
      meshnames (3) = "mesh1"
      meshnames (4) = "mesh1"

      vartypes (1) = DB_QUADVAR
      vartypes (2) = DB_QUADVAR
      vartypes (3) = DB_QUADMESH
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
     x               "rect_2ds", 8, nvars,
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
      double precision x (21), y (21)
      integer nx, ny, meshdims (3), lowoffset (3), highoffset (3)
      common /mesh/ x, y, nx, ny, meshdims, lowoffset, highoffset
      double precision d (400), p (400)
      integer vardims (3)
      common /data/ d, p, vardims
      integer nmat, matnos (2), matlist (400), matdims (3)
      integer mixlen, mix_next (400), mix_mat (400), mix_zone (400)
      integer dummy
      double precision mix_vf (400)
      common /materials/ nmat, matnos, matlist, matdims, mixlen,
     x                   dummy, mix_next, mix_mat, mix_zone, mix_vf

      integer eventtype, readtype
      character*80 readvar
      integer ier, idoptl

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

         if (readtype .eq. DB_QUADMESH) then
            if (readvar .eq. "mesh1") then
               ier = dbmkoptlist (15, idoptl)
               ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
               ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
               ier = dbaddiopt (idoptl, DBOPT_COORDSYS, DB_CARTESIAN)
               ier = dbaddiopt (idoptl, DBOPT_FACETYPE, DB_RECTILINEAR)
               ier = dbaddiopt (idoptl, DBOPT_MAJORORDER, DB_ROWMAJOR)
               ier = dbaddiopt (idoptl, DBOPT_NSPACE, 2)
               ier = dbaddiopt (idoptl, DBOPT_PLANAR, DB_AREA)
               ier = dbaddiopt (idoptl, DBOPT_LO_OFFSET, lowoffset)
               ier = dbaddiopt (idoptl, DBOPT_HI_OFFSET, highoffset)
               ier = dbaddcopt (idoptl, DBOPT_XLABEL, "x", 1)
               ier = dbaddcopt (idoptl, DBOPT_YLABEL, "y", 1)
               ier = dbaddcopt (idoptl, DBOPT_XUNITS, "cm", 2)
               ier = dbaddcopt (idoptl, DBOPT_YUNITS, "cm", 2)
               ier = sdxputqm ("mesh1", 5, "x", 1, "y", 1, " ", 0,
     x               x, y, 0, meshdims, 2, DB_DOUBLE, DB_COLLINEAR,
     x               idoptl)
               ier = dbfreeoptlist (idoptl)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. DB_QUADVAR) then
            if (readvar .eq. "d") then
               ier = dbmkoptlist (6, idoptl)
               ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
               ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
               ier = dbaddcopt (idoptl, DBOPT_LABEL, "density", 7)
               ier = dbaddcopt (idoptl, DBOPT_UNITS, "g/cm**3", 7)
               ier = dbaddiopt (idoptl, DBOPT_LO_OFFSET, lowoffset)
               ier = dbaddiopt (idoptl, DBOPT_HI_OFFSET, highoffset)
               ier = sdxputqv ("d", 1, "mesh1", 5, d, vardims, 2,
     x               0, 0, DB_DOUBLE, DB_ZONECENT, idoptl)
               ier = dbfreeoptlist (idoptl)
            else if (readvar .eq. "p") then
               ier = dbmkoptlist (6, idoptl)
               ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
               ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
               ier = dbaddcopt (idoptl, DBOPT_LABEL, "pressure", 8)
               ier = dbaddcopt (idoptl, DBOPT_UNITS, "Mbar", 4)
               ier = dbaddiopt (idoptl, DBOPT_LO_OFFSET, lowoffset)
               ier = dbaddiopt (idoptl, DBOPT_HI_OFFSET, highoffset)
               ier = sdxputqv ("p", 1, "mesh1", 5, p, vardims, 2,
     x               0, 0, DB_DOUBLE, DB_ZONECENT, idoptl)
               ier = dbfreeoptlist (idoptl)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. DB_MATERIAL) then
            if (readvar .eq. "mat1") then
               ier = dbmkoptlist (3, idoptl)
               ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
               ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
               ier = dbaddcopt (idoptl, DBOPT_LABEL, "material", 8)
               ier = sdxputmat ("mat1", 4, "mesh1", 5, nmat, matnos,
     x                          matlist, matdims, 2, mix_next, mix_mat,
     x                          mix_zone, mix_vf, mixlen,
     x                          DB_DOUBLE, idoptl)
               ier = dbfreeoptlist (idoptl)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. DB_MULTIBLOCKMESH) then
            ier = sdxmmesh (1, "mesh1           ", DB_QUAD_RECT)
         else if (readtype .eq. WMESHNAME) then
            ier = sdxchar ("mesh1", 5)
         else if (readtype .eq. WMESHTYPE) then
            if (readvar .eq. "mesh1") then
               ier = sdxinteger (DB_QUAD_RECT)
            else if (readvar .eq. "mat1") then
               ier = sdxinteger (DB_MATERIAL)
            else
               ier = sdxinteger (DB_QUADVAR)
            endif
         else if (readtype .eq. WVARIABLE) then
            if (readvar .eq. "time") then
               ier = sdxfloat (ttot)
            else if (readvar .eq. "cycle") then
               ier = sdxinteger (cycle)
            else if (readvar .eq. "noreg") then
               ier = sdxinteger (nmat)
            else if (readvar .eq. "_fileinfo") then
               ier = sdxchar ("2D rectilinear mesh", 19)
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

      integer cycle
      common /cycle/ cycle
      real ttot
      common /ttot/ ttot
      double precision x (21), y (21)
      integer nx, ny, meshdims (3), lowoffset (3), highoffset (3)
      common /mesh/ x, y, nx, ny, meshdims, lowoffset, highoffset
      double precision d (400), p (400)
      integer vardims (3)
      common /data/ d, p, vardims
      integer nmat, matnos (2), matlist (400), matdims (3)
      integer mixlen, mix_next (400), mix_mat (400), mix_zone (400)
      integer dummy
      double precision mix_vf (400)
      common /materials/ nmat, matnos, matlist, matdims, mixlen,
     x                   dummy, mix_next, mix_mat, mix_zone, mix_vf

      integer i

      cycle = 0
      ttot  = 0.

c
c     Initialize the mesh information.
c
      nx = 21
      ny = 21

      meshdims (1) = 21
      meshdims (2) = 21
      meshdims (3) = 0

      do 100 i = 1, 3
         lowoffset (i) = 0
         highoffset (i) = 0
100   continue

      do 110 i = 1, 21
         x (i) = (i - 1) * (1. / 20.)
         y (i) = (i - 1) * (1. / 20.)
110   continue

c
c     Initialize the static material information.
c
      nmat = 2
      matnos (1) = 1
      matnos (2) = 2

      matdims (1) = 20
      matdims (2) = 20
      matdims (3) = 0

c
c     Initialize the static variable information.
c
      vardims (1) = 20
      vardims (2) = 20
      vardims (3) = 0

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
      double precision x (21), y (21)
      integer nx, ny, meshdims (3), lowoffset (3), highoffset (3)
      common /mesh/ x, y, nx, ny, meshdims, lowoffset, highoffset
      double precision d (400), p (400)
      integer vardims (3)
      common /data/ d, p, vardims
      integer nmat, matnos (2), matlist (400), matdims (3)
      integer mixlen, mix_next (400), mix_mat (400), mix_zone (400)
      integer dummy
      double precision mix_vf (400)
      common /materials/ nmat, matnos, matlist, matdims, mixlen,
     x                   dummy, mix_next, mix_mat, mix_zone, mix_vf

      integer i, j, k, l
      integer ier
      real angle
      real xcenter, ycenter
      real xave, yave
      real dist
      real dx, dy
      real xx (20),  yy (20)
      integer itemp (441)
      integer cnt

c
c     Increment the time and cycle.
c
      cycle = cycle + 1
      ttot = ttot + .01

c
c     Move the center of the field.
c
      angle = ttot * ((2. * 3.14159) / .36)
      xcenter = (sin (angle) * .5) + .5
      ycenter = (cos (angle) * .5) + .5

c
c     Calculate the new variables based on the center.
c
      do 100 i = 1, 20
         do 110 j = 1, 20
            xave = (x(i) + x (i+1)) / 2.
            yave = (y(j) + y (j+1)) / 2.
            dist = sqrt ((xave - xcenter) * (xave - xcenter) +
     x                   (yave - ycenter) * (yave - ycenter))
            d ((j-1)*20+i) = dist
            p ((j-1)*20+i) = 1 / (dist + .0001)
110      continue
100   continue

c
c     Calculate the new materials based on the center.
c
      mixlen = 0

c
c     Put in material 1.
c
      do 200 i = 1, 20
         do 210 j = 1, 20
            matlist ((j-1)*20+i) = 1
210      continue
200   continue

c
c     Put in material 2.
c
      do 220 i = 1, 21
         do 230 j = 1, 21
            dist = sqrt ((x(i) - xcenter) * (x(i) - xcenter) +
     x                   (y(j) - ycenter) * (y(j) - ycenter))
            if (dist .lt. 0.4) then
               itemp ((j-1)*21+i) = 1
            else
               itemp ((j-1)*21+i) = 0
            endif
230      continue
220   continue

      do 240 i = 1, 20
         do 250 j = 1, 20
            cnt = itemp ((j-1)*21+i) + itemp ((j)*21+i) +
     x            itemp ((j)*21+i+1) + itemp ((j-1)*21+i+1)
            if (cnt .eq. 0) then
c              do nothing
            else if (cnt .eq. 4) then
               matlist ((j-1)*20+i) = 2
            else
               dx = (x (i+1) - x (i)) / 20.
               dy = (y (j+1) - y (j)) / 20.
               do 260 k = 1, 20
                  xx (k) = x (i) + (dx / 2.) + (k * dx)
                  yy (k) = y (j) + (dy / 2.) + (k * dy)
260            continue
               cnt = 0
               do 270 k = 1, 20
                  do 280 l = 1, 20
                     dist = sqrt ((xx(k) - xcenter) *
     x                            (xx(k) - xcenter) +
     x                            (yy(l) - ycenter) *
     x                            (yy(l) - ycenter))
                     if (dist .lt.  0.4) then
                        cnt = cnt + 1
                     endif
280               continue
270            continue
               if (cnt .eq. 400) then
                  matlist ((j-1)*20+i) = 2
               else
                  matlist ((j-1)*20+i) = -(mixlen + 1)
                  mix_mat  (mixlen+1) = 1
                  mix_mat  (mixlen+2) = 2
                  mix_next (mixlen+1) = mixlen + 2
                  mix_next (mixlen+2) = 0
                  mix_zone (mixlen+1) = j * 20 + i
                  mix_zone (mixlen+2) = j * 20 + i
                  mix_vf   (mixlen+1) = 1. - (float (cnt) / 400.)
                  mix_vf   (mixlen+2) = (float (cnt) / 400.)
                  mixlen = mixlen + 2
               endif
            endif
250      continue
240   continue

      ier = sdxpause (1)

      return
      end
