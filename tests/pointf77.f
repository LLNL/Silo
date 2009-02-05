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

c
c   Program
c
c       testpoint
c
c   Purpose
c
c       Test program illustrating use of SILO from Fortran
c       for writing point data.
c       This particular program also uses SILO's "EZ" interface to
c       write simple scalars and arrays with.
c
c   Programmer
c
c       Jeff Long, NSSD/B, 11/02/92
c
c   Notes
c
c       For more info on calling sequences, see the "SILO User's
c       Manual".
c
c          ierr = dbcreate (filename, len_filename, filemode, target,
c                           file_info_string, len_file_info_string,
c                           filetype, DBID)
c
c          ierr = dbputpm (dbid, meshname, len_meshname, num_dims,
c                          xcoords, ycoords, zcoords, npts, datatype,
c                          option_list, MESHID)
c
c          ierr = dbputpv1 (dbid, varname, len_varname, meshname,
c                           len_meshname,
c                           var, npts, datatype, option_list, VARID)
c
c          ierr = ezwrite (silo_id, name, lname, data, len_data,
c                          datatype, ierr)
c
c

      program main

c ..Don't forget to include this file!

      include "silo.inc"

      integer  dirid, dbid, meshid, varid, silo_id, optlistid
      integer  varid1
      real     x(5), y(5), z(5), foo(5)
      real     d(5), ttime
      integer  itype(5), tcycle
      real     ftype(5)
      double precision dttime

c...Generate some data to write out.

      do i = 1 , 5
         x(i) = i
         y(i) = i
         z(i) = i
         d(i) = 10 * i
         itype(i) = i
         ftype(i) = i
         foo(i)   = i
      enddo

      npts = 5
      lfoo = 5

      ttime  = 0.123
      tcycle = 123
      dttime = 0.123
c...Create the file (using the SILO interface)

      ierr =  dbcreate ("pointf77.pdb", 12, DB_CLOBBER, DB_LOCAL,
     .                 "file info goes here", 19, DB_PDB, dbid)
      if (ierr .ne. 0) then
         print *, 'dbcreate had error'
      else
         print *, 'created file pointf77.pdb'
      endif

c...Create an option list containing time and cyle info.

      ierr = dbmkoptlist(3, optlistid)                   ! Create the option list
      ierr = dbaddiopt  (optlistid, DBOPT_CYCLE, tcycle) ! Add integer opt
      ierr = dbaddropt  (optlistid, DBOPT_TIME, ttime)   ! Add real opt
      ierr = dbadddopt  (optlistid, DBOPT_DTIME, dttime)   ! Add double opt


c...Write out the point mesh.

      ierr = dbputpm (dbid, 'pmesh', 5, 2, x, y, z, npts, DB_FLOAT,
     .                optlistid, meshid)


c...Write out the point variables

      ierr = dbputpv1 (dbid, 'd', 1, 'pmesh', 5, d, npts, DB_FLOAT,
     .                 optlistid, varid)

c     ierr = dbputpv1 (dbid, 'ftype', 5, 'pmesh', 5, ftype, npts, DB_FLOAT,
c    .                 optlistid, varid1)




      ierr = dbclose (dbid)
      if (ierr .ne. 0) print *, 'dbclose had error'


      stop
      end
