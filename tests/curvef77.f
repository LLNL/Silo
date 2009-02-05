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

c-------------------------------------------------------------------------
c Purpose
c
c   Demonstrate use of SILO for creating curve objects.
c
c-------------------------------------------------------------------------

      program main
      implicit none
      include "silo.inc"
      integer dbid, curveid, err, i, curve_id, dtype, npts
      real x(20), y1(20), y2(20), xin(20), yin1(20), yin2(20)

c     Create the curve x and y values...

      do i=1, 20
         x(i) = (i-1) * 3.1415927 / 20.0
         y1(i) = sin (x(i))
         y2(i) = cos (x(i))
      enddo

c     Write curve data to a PDB file...

      err = dbcreate ("curvef77.pdb", 12, 0, DB_LOCAL,
     $     "Curve Data", 10, DB_PDB, dbid)

      err = dbputcurve (dbid, "sincurve", 8, x, y1, DB_FLOAT,
     $     20, DB_F77NULL, curve_id)

      err = dbputcurve (dbid, "coscurve", 8, x, y2, DB_FLOAT,
     $     20, DB_F77NULL, curve_id)

      err = dbclose (dbid)

c     Read the pdb file...

      err = dbopen ("curvef77.pdb", 12, DB_PDB, DB_READ, dbid)
      err = dbgetcurve (dbid, "sincurve", 8, 20, xin, yin1,
     $     dtype, npts)
      err = dbgetcurve (dbid, "coscurve", 8, 20, xin, yin2,
     $     dtype, npts)
      err = dbclose (dbid)

c     Check for errors...

      do i=1,20
         if (x(i) .ne. xin(i)) then
            write (*,*) "x mismatch error at index ", i
            stop
         endif
         if (yin1(i) .ne. y1(i)) then
            write (*,*) "y1 mismatch error at index ", i
            stop
         endif
         if (yin2(i) .ne. y2(i)) then
            write (*,*) "y2 mismatch error at index ", i
            stop
         endif
      enddo

      write (*,*) "Test successful."

      stop
      end
