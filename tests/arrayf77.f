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
c   Demonstrate use of SILO for creating compound array objects.
c
c-------------------------------------------------------------------------

      program main

      implicit none
      include "silo.inc"
      integer dbid, err

      character*32 elemnames(3)
      integer elemlengths(3), array_id, i
      real values(18)

      character*32 g_enames(10)
      integer g_elengths(10), g_nelems, g_nvalues, g_datatype,
     $     g_maxwidth

      real g_values(100)

      data elemnames/"x","y","z"/
      data elemlengths/4,6,8/

      err = dbshowerrors(DB_ABORT)

c...Create file named "robb.pdb".  Database ID is returned in 'dbid'.

      err = dbcreate("arrayf77.pdb", 12, 0, DB_LOCAL, "file info",
     $     9, DB_PDB, dbid)


      do i=1, 18
         values(i) = i/100.0
      enddo

      err = dbputca (dbid, "carray", 6, elemnames, 32,
     $ elemlengths, 3, values, DB_FLOAT, DB_F77NULL, array_id)

      err = dbclose (dbid)


CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C  Now try to read from the file...
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

      err = dbopen ("arrayf77.pdb", 12, DB_PDB, DB_READ, dbid)

      err = dbgetca (dbid, "carray", 6, 32, g_enames, g_elengths,
     $     g_nelems, g_values, g_nvalues, g_datatype)

      write (*,*) "reading via dbgetca"
      write (*,*) "number of elements..........", g_nelems
      write (*,*) "number of values............", g_nvalues
      write (*,*) "data type...................", g_datatype
      do i=1, g_nelems
         write (*,*) "simple array ", i, " name is ", g_enames(i)
      enddo
      do i=1, g_nvalues
         write (*,*) "value ", i, " is ", g_values(i)
      enddo


      g_nvalues = 0
      do i=1,100
         g_values(i) = 0
      enddo
      err = dbfgetca (dbid, "carray", 6, g_values, g_nvalues)

      write (*,*) "reading via dbfgetca"
      write (*,*) "number of values............", g_nvalues
      do i=1, g_nvalues
         write (*,*) "value ", i, " is ", g_values(i)
      enddo


      g_nelems = 0
      g_nvalues = 0
      g_datatype = 0
      err = dbinqca (dbid, "carray", 6, g_maxwidth, g_nelems, g_nvalues,
     $     g_datatype)

      write (*,*) "reading via dbinqca"
      write (*,*) "number of elements..........", g_nelems
      write (*,*) "number of values............", g_nvalues
      write (*,*) "data type...................", g_datatype
      write (*,*) "longest name................", g_maxwidth

      err = dbclose (dbid)

      stop
      end
