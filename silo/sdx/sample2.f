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
*     Copied from sample1.f by Brooke Aspen Unger.  
*     Modified to utilize multi block features.
*    
************************************************************************

      program sample2

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
      integer      nvars, nblocks, nmats
      character*64 varnames (4)
      character*64 meshnames (4)
      integer      vartypes (4), meshtypes (4)
      common /toc/ nvars, nblocks, nmats, varnames, meshnames, 
     x             vartypes, meshtypes

      integer      machlen, userlen
      character*40 machname, username
      integer      sdxid

      nmats = 2
      nvars = 4
      
      varnames (1) = "multimesh"
      varnames (2) = "d"
      varnames (3) = "p"
      varnames (4) = "multimat"

      vartypes (1) = DB_MULTIMESH
      vartypes (2) = DB_MULTIVAR
      vartypes (3) = DB_MULTIVAR
      vartypes (4) = DB_MULTIMAT

      nblocks = 2

      meshnames (1) = "mesh1"
      meshnames (2) = "mesh2"

      meshtypes (1) = DB_QUADMESH
      meshtypes (2) = DB_QUADMESH

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
      integer      nvars, nblocks, nmats
      character*64 varnames (4)
      character*64 meshnames (4)
      integer      vartypes (4), meshtypes (4)
      common /toc/ nvars, nblocks, nmats, varnames, meshnames, 
     x             vartypes, meshtypes
      double precision x1 (341), y1 (341), x2 (961), y2 (961)
      integer nx1, ny1, mdims1 (3), lo_off1 (3), hi_off1 (3)
      integer nx2, ny2, mdims2 (3), lo_off2 (3), hi_off2 (3)
      common /mesh/ x1, y1, x2, y2, mdims1, lo_off1, hi_off1,
     x              nx1, ny1, nx2, ny2, mdims2, lo_off2, hi_off2
      double precision d1 (300), p1 (300), d2 (900), p2 (900)
      integer vardims1 (3), vardims2 (3)
      integer dmvars, pmvars
      character*64 dmvarnames (2), pmvarnames (2)
      integer dmvartypes (2), pmvartypes (2)
      common /data/ d1, p1, d2, p2, vardims1, vardims2,
     x              dmvars, dmvarnames, dmvartypes,
     x              pmvars, pmvarnames, pmvartypes
      integer mixlen, matnos (2)
      integer matlist1 (1200), matdims1 (3)
      integer matlist2 (1200), matdims2 (3)
      character*64 matnames (2)
      common /materials/ mixlen, matnos, matlist1, matdims1,
     x                   matlist2, matdims2, matnames

      integer        eventtype, readtype
      character*80   readvar
      integer        ier, idoptl


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

         if (readtype .eq. DB_MULTIMESH) then
            if (readvar .eq. "multimesh") then
               ier = sdxmmesh(nblocks, meshnames, meshtypes)
            else 
               ier = sdxerror()
            endif
         else if (readtype .eq. DB_MULTIVAR) then
            if (readvar .eq. "d") then
               ier = sdxmvar(dmvars, dmvarnames, dmvartypes)
            else if (readvar .eq. "p") then
               ier = sdxmvar(pmvars, pmvarnames, pmvartypes)
            else
               ier = sdxerror ()
            endif 
         else if (readtype .eq. DB_MULTIMAT) then
            if (readvar .eq. "multimat") then
               ier = sdxmmat(nmats, matnames)
            else 
               ier = sdxerror ()
            endif
         else if (readtype .eq. DB_MATERIAL) then
            ier = dbmkoptlist (5, idoptl)
            ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
            ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
            if (readvar .eq. "mat1") then
               ier = sdxputmat ("mat1", 4, "mesh1", 5, nmats, matnos,
     x              matlist1, matdims1, 2, DB_F77NULL, DB_F77NULL,
     x              DB_F77NULL, DB_F77NULL, mixlen,
     x              DB_DOUBLE, idoptl)
            else if (readvar .eq. "mat2") then
               ier = sdxputmat ("mat2", 4, "mesh2", 5, nmats, matnos,
     x              matlist2, matdims2, 2, DB_F77NULL, DB_F77NULL,
     x              DB_F77NULL, DB_F77NULL, mixlen,
     x              DB_DOUBLE, idoptl)
            endif
            ier = dbfreeoptlist (idoptl)
         else if (readtype .eq. DB_QUADMESH) then
            if (readvar .eq. "mesh1") then
               ier = dbmkoptlist (15, idoptl)
               ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
               ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
               ier = dbaddiopt (idoptl, DBOPT_COORDSYS, DB_CARTESIAN)  
               ier = dbaddiopt (idoptl, DBOPT_FACETYPE, DB_RECTILINEAR)
               ier = dbaddiopt (idoptl, DBOPT_MAJORORDER, DB_COLMAJOR)
               ier = dbaddiopt (idoptl, DBOPT_NSPACE, 2)
               ier = dbaddiopt (idoptl, DBOPT_PLANAR, DB_AREA)        
               ier = dbaddiopt (idoptl, DBOPT_LO_OFFSET, lo_off1)
               ier = dbaddiopt (idoptl, DBOPT_HI_OFFSET, hi_off1)
               ier = dbaddcopt (idoptl, DBOPT_XLABEL, "x", 1)
               ier = dbaddcopt (idoptl, DBOPT_YLABEL, "y", 1)
               ier = dbaddcopt (idoptl, DBOPT_XUNITS, "cm", 2)
               ier = dbaddcopt (idoptl, DBOPT_YUNITS, "cm", 2)
               ier = sdxputqm ("mesh1", 5, "x", 1, "y", 1, " ", 0,
     x               x1, y1, 0, mdims1, 2, DB_DOUBLE, DB_NONCOLLINEAR,
     x               idoptl)
               ier = dbfreeoptlist (idoptl)
            else if (readvar .eq. "mesh2") then
               ier = dbmkoptlist (15, idoptl) 
               ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
               ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
               ier = dbaddiopt (idoptl, DBOPT_COORDSYS, DB_CARTESIAN)  
               ier = dbaddiopt (idoptl, DBOPT_FACETYPE, DB_RECTILINEAR)
               ier = dbaddiopt (idoptl, DBOPT_MAJORORDER, DB_COLMAJOR)
               ier = dbaddiopt (idoptl, DBOPT_NSPACE, 2)
               ier = dbaddiopt (idoptl, DBOPT_PLANAR, DB_AREA)        
               ier = dbaddiopt (idoptl, DBOPT_LO_OFFSET, lo_off1)
               ier = dbaddiopt (idoptl, DBOPT_HI_OFFSET, hi_off1)
               ier = dbaddcopt (idoptl, DBOPT_XLABEL, "x", 1)
               ier = dbaddcopt (idoptl, DBOPT_YLABEL, "y", 1)
               ier = dbaddcopt (idoptl, DBOPT_XUNITS, "cm", 2)
               ier = dbaddcopt (idoptl, DBOPT_YUNITS, "cm", 2)
               ier = sdxputqm ("mesh2", 5, "x", 1, "y", 1, " ", 0,
     x               x2, y2, 0, mdims2, 2, DB_DOUBLE, DB_NONCOLLINEAR,
     x               idoptl)
               ier = dbfreeoptlist (idoptl)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. DB_QUADVAR) then
            if (readvar .eq. "d1") then
               ier = dbmkoptlist (7, idoptl)
               ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
               ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
               ier = dbaddcopt (idoptl, DBOPT_LABEL, "density", 7)
               ier = dbaddiopt (idoptl, DBOPT_MAJORORDER, DB_COLMAJOR)
               ier = dbaddcopt (idoptl, DBOPT_UNITS, "g/cm**3", 7)
               ier = dbaddiopt (idoptl, DBOPT_LO_OFFSET, lo_off1)
               ier = dbaddiopt (idoptl, DBOPT_HI_OFFSET, hi_off1)
               ier = sdxputqv ("d1", 1, "mesh1", 5, d1, vardims1, 2,
     x               0, 0, DB_DOUBLE, DB_ZONECENT, idoptl)
               ier = dbfreeoptlist (idoptl) 
            else if (readvar .eq. "d2") then
               ier = dbmkoptlist (7, idoptl)
               ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
               ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
               ier = dbaddcopt (idoptl, DBOPT_LABEL, "density", 7)
               ier = dbaddiopt (idoptl, DBOPT_MAJORORDER, DB_COLMAJOR)
               ier = dbaddcopt (idoptl, DBOPT_UNITS, "g/cm**3", 7)
               ier = dbaddiopt (idoptl, DBOPT_LO_OFFSET, lo_off2)
               ier = dbaddiopt (idoptl, DBOPT_HI_OFFSET, hi_off2)
               ier = sdxputqv ("d2", 2, "mesh2", 5, d2, vardims2, 2,
     x               0, 0, DB_DOUBLE, DB_ZONECENT, idoptl)
               ier = dbfreeoptlist (idoptl)
            else if (readvar .eq. "p1") then
               ier = dbmkoptlist (7, idoptl)
               ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
               ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
               ier = dbaddcopt (idoptl, DBOPT_LABEL, "pressure", 8)
               ier = dbaddiopt (idoptl, DBOPT_MAJORORDER, DB_COLMAJOR)
               ier = dbaddcopt (idoptl, DBOPT_UNITS, "Mbar", 4)
               ier = dbaddiopt (idoptl, DBOPT_LO_OFFSET, lo_off1)
               ier = dbaddiopt (idoptl, DBOPT_HI_OFFSET, hi_off1)
               ier = sdxputqv ("p1", 1, "mesh1", 5, p1, vardims1, 2,
     x               0, 0, DB_DOUBLE, DB_ZONECENT, idoptl)
               ier = dbfreeoptlist (idoptl)
            else if (readvar .eq. "p2") then
               ier = dbmkoptlist (7, idoptl)
               ier = dbaddiopt (idoptl, DBOPT_CYCLE, cycle)
               ier = dbaddropt (idoptl, DBOPT_TIME, ttot)
               ier = dbaddcopt (idoptl, DBOPT_LABEL, "pressure", 8)
               ier = dbaddiopt (idoptl, DBOPT_MAJORORDER, DB_COLMAJOR)
               ier = dbaddcopt (idoptl, DBOPT_UNITS, "Mbar", 4)
               ier = dbaddiopt (idoptl, DBOPT_LO_OFFSET, lo_off2)
               ier = dbaddiopt (idoptl, DBOPT_HI_OFFSET, hi_off2)
               ier = sdxputqv ("p2", 2, "mesh2", 5, p2, vardims2, 2,
     x               0, 0, DB_DOUBLE, DB_ZONECENT, idoptl)
               ier = dbfreeoptlist (idoptl)
            else 
               ier = sdxerror ()
            endif
         else if (readtype .eq. WMESHNAME) then
            if (readvar .eq. "mesh1") then
               ier = sdxchar ("mesh1", 5)
            else if (readvar .eq. "mesh2") then
               ier = sdxchar ("mesh2", 5)
            else if (readvar .eq. "d") then
               ier = sdxchar ("multimesh", 9)
            else if (readvar .eq. "d1") then
               ier = sdxchar ("mesh1", 5)
            else if (readvar .eq. "d2") then
               ier = sdxchar ("mesh2", 5)
            else if (readvar .eq. "p") then
               ier = sdxchar ("multimesh", 9)
            else if (readvar .eq. "p1") then
               ier = sdxchar ("mesh1", 5)
            else if (readvar .eq. "p2") then
               ier = sdxchar ("mesh2", 5)
            else if (readvar .eq. "mat1") then
               ier = sdxchar ("mesh1", 5)
            else if (readvar .eq. "mat2") then
               ier = sdxchar ("mesh2", 5)
            else if (readvar .eq. "multimat") then
               ier = sdxchar ("multimesh", 5) 
            else
               ier = sdxerror ()
           endif
         else if (readtype .eq. WMESHTYPE) then
            if (readvar .eq. "multimesh") then
               ier = sdxinteger (DB_MULTIMESH)
            else if (readvar .eq. "mesh1") then
               ier = sdxinteger (DB_QUAD_RECT)
            else if (readvar .eq. "mesh2") then
               ier = sdxinteger (DB_QUAD_RECT)
 	    else if (readvar .eq. "d") then
 	       ier = sdxinteger (DB_MULTIVAR)
 	    else if (readvar .eq. "d1") then
	       ier = sdxinteger (DB_QUADVAR) 
 	    else if (readvar .eq. "d2") then
	       ier = sdxinteger (DB_QUADVAR)              
 	    else if (readvar .eq. "p") then
	       ier = sdxinteger (DB_MULTIVAR)
 	    else if (readvar .eq. "p1") then
	       ier = sdxinteger (DB_QUADVAR) 
 	    else if (readvar .eq. "p2") then
	       ier = sdxinteger (DB_QUADVAR)              
            else if (readvar .eq. "multimat") then
               ier = sdxinteger (DB_MULTIMAT)
            else if (readvar .eq. "mat1") then
               ier = sdxinteger (DB_MATERIAL)
            else if (readvar .eq. "mat2") then
               ier = sdxinteger (DB_MATERIAL)
            else
               ier = sdxerror ()
            endif
         else if (readtype .eq. WVARIABLE) then
            if (readvar .eq. "time") then
               ier = sdxfloat (ttot)
            else if (readvar .eq. "cycle") then
               ier = sdxinteger (cycle)
            else if (readvar .eq. "noreg") then
               ier = sdxinteger (nmats)
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
            write (6, "('readtype is ', readtype6)")
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

      include "silo.inc"
      include "sdx.inc"

      integer cycle
      common /cycle/ cycle
      real ttot
      common /ttot/ ttot
      integer      nvars, nblocks, nmats
      character*64 varnames (4)
      character*64 meshnames (4)
      integer      vartypes (4), meshtypes (4)
      common /toc/ nvars, nblocks, nmats, varnames, meshnames, 
     x             vartypes, meshtypes
      double precision x1 (341), y1 (341)
      double precision x2 (961), y2 (961)
      integer nx1, ny1, mdims1 (3), lo_off1 (3), hi_off1 (3)
      integer nx2, ny2, mdims2 (3), lo_off2 (3), hi_off2 (3)
      common /mesh / x1, y1, x2, y2, mdims1, lo_off1, hi_off1,
     x               nx1, ny1, nx2, ny2, mdims2, lo_off2, hi_off2
      double precision d1 (300), p1 (300), d2 (900), p2 (900)
      integer vardims1 (3), vardims2 (3)
      integer dmvars, pmvars
      character*64 dmvarnames (2), pmvarnames (2)
      integer dmvartypes (2), pmvartypes (2)
      common /data/ d1, p1, d2, p2, vardims1, vardims2,
     x              dmvars, dmvarnames, dmvartypes,
     x              pmvars, pmvarnames, pmvartypes
      integer mixlen, matnos (2)
      integer matlist1 (1200), matdims1 (3)
      integer matlist2 (1200), matdims2 (3)
      character*64 matnames (2)
      common /materials/ mixlen, matnos, matlist1, matdims1, 
     x                   matlist2, matdims2, matnames

      integer i, j
      real r, dr, theta, dtheta, nx
      real xave, yave, dist, xcenter, ycenter

      cycle = 0
      ttot  = 0.

c
c     Initialize the mesh information. nx1 and nx2 should be the same
c     

      nx1 = 31 
      ny1 = 11
      nx2 = 31
      ny2 = 31
 
      nx = nx1

      mdims1 (1) = nx1
      mdims1 (2) = ny1
      mdims1 (3) = 0

      mdims2 (1) = nx2
      mdims2 (2) = ny2
      mdims2 (3) = 0

      do 100 i = 1, 3
         lo_off1 (i) = 0
         hi_off1 (i) = 0
         lo_off2 (i) = 0
         hi_off2 (i) = 0
100   continue

      dr = 3. / ((ny1 - 1) + (ny2 - 1))
      theta = 0.0
      dtheta = (180. / (nx - 1)) * (3.1415926 / 180.)

c
c     Mesh 1 
c
      do 120 i = 1, nx1
         r = 2.0
         do 110 j = 1, ny1
            x1 ((j - 1) + (i - 1) * (ny1) + 1) = r * cos (theta)
            y1 ((j - 1) + (i - 1) * (ny1) + 1) = r * sin (theta)
            r = r + dr
110      continue
         theta = theta + dtheta
120   continue
      
c
c     Mesh 2
c
      theta = 0
      do 140 i = 1, nx2
         r = 2.75
         do 130 j = 1, ny2
            x2 ((j - 1) + (i - 1) * (ny2) + 1) = r * cos (theta)
            y2 ((j - 1) + (i - 1) * (ny2) + 1) = r * sin (theta)
            r = r + dr
130      continue
         theta = theta + dtheta
140   continue

c
c     Create the density and pressure arrays
c

      vardims1 (1) = nx1 - 1
      vardims1 (2) = ny1 - 1 
      vardims1 (3) = 0

      vardims2 (1) = nx2 - 1
      vardims2 (2) = ny2 - 1 
      vardims2 (3) = 0

      dmvars = 2
      dmvarnames (1) = "d1"
      dmvarnames (2) = "d2"
      dmvartypes (1) = DB_QUADVAR
      dmvartypes (2) = DB_QUADVAR  

      pmvars = 2
      pmvarnames (1) = "p1"
      pmvarnames (2) = "p2"
      pmvartypes (1) = DB_QUADVAR
      pmvartypes (2) = DB_QUADVAR  

      xcenter = 0.0
      ycenter = 0.0

      do 200 i = 1, nx - 1
         do 190 j = 1, ny1 - 1 
            xave =(x1 ((j - 1) + (i - 1) * ny1 + 1) +
     x           x1 ((j - 1) + (i) * ny1 + 1) +
     x           x1 ((j) + (i) * ny1 + 1) +
     x           x1 ((j) + (i - 1) * ny1 + 1)) / 4.0 
            yave =(y1 ((j - 1) + (i - 1) * ny1 + 1) +
     x           y1 ((j - 1) + (i) * ny1 + 1) +
     x           y1 ((j) + (i) * ny1 + 1) +
     x           y1 ((j) + (i - 1) * ny1 + 1)) / 4.0
            dist = sqrt ((xave - xcenter) * (xave - xcenter) +
     x           (yave - ycenter) * (yave - ycenter)) 
            d1 ((j - 1) + (i - 1) * (ny1 - 1) + 1) = dist
            p1 ((j - 1) + (i - 1) * (ny1 - 1) + 1) = 1. / (dist + .0001)
190   continue
200   continue
            
      do 230 i = 1, nx - 1 
         do 220 j = 1, ny2 - 1
            xave =(x2 ((j - 1) + (i - 1) * ny2 + 1) +
     x           x2 ((j - 1) + (i) * ny2 + 1) +
     x           x2 ((j) + (i) * ny2 + 1) +
     x           x2 ((j) + (i - 1) * ny2 + 1)) / 4.0 
            yave =(y2 ((j - 1) + (i - 1) * ny2 + 1) +
     x           y2 ((j - 1) + (i) * ny2 + 1) +
     x           y2 ((j) + (i) * ny2 + 1) +
     x           y2 ((j) + (i - 1) * ny2 + 1)) / 4.0
            dist = sqrt ((xave - xcenter) * (xave - xcenter) +
     x           (yave - ycenter) * (yave - ycenter)) 
            d2 ((j - 1) + (i - 1) * (ny2 - 1) + 1) = dist
            p2 ((j - 1) + (i - 1) * (ny2 - 1) + 1) = 1. / (dist + .001)
220   continue
230   continue
           

c
c     Create 2 materials 
c
      
      matnames (1) = "mat1"
      matnames (2) = "mat2"

      mixlen = 0
      matnos (1) = 1
      matnos (2) = 2

      matdims1 (1) = nx1
      matdims1 (2) = ny1
      
      matdims2 (1) = nx2
      matdims2 (2) = ny2

      do 250 i = 1, nx1 * ny1
         matlist1 (i) = 1
250   continue

      do 260 i = 1, nx2 * ny2
         matlist2 (i) = 2
260   continue

      return
      end

************************************************************************
*
*     Increments the cycle and calls a pause.
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

      integer ier

c
c     Increment the time and cycle.
c
      cycle = cycle + 1
      ttot = ttot + .01

      ier = sdxpause (1)

      return
      end
