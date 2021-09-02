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

C
C   Program
C
C       testpoint
C
C   Purpose
C
C       Test program illustrating use of SILO from Fortran
C       for writing point data.
C       This particular program also uses SILO's "EZ" interface to
C       write simple scalars and arrays with.
C
C   Programmer
C
C       Jeff Long, NSSD/B, 11/02/92
C
C   Notes
C
C       For more info on calling sequences, see the "SILO User's
C       Manual".
C
C          ierr = dbcreate (filename, len_filename, filemode, target,
C                           file_info_string, len_file_info_string,
C                           filetype, DBID)
C
C          ierr = dbputpm (dbid, meshname, len_meshname, num_dims,
C                          xcoords, ycoords, zcoords, npts, datatype,
C                          option_list, MESHID)
C
C          ierr = dbputpv1 (dbid, varname, len_varname, meshname,
C                           len_meshname,
C                           var, npts, datatype, option_list, VARID)
C
C          ierr = ezwrite (silo_id, name, lname, data, len_data,
C                          datatype, ierr)
C
C

      program main

C ..Don't forget to include this file!

      include "silo.inc"

      integer  dirid, dbid, meshid, varid, silo_id, optlistid
      integer  varid1, driver, nargs
      real     x(5), y(5), z(5), foo(5)
      real     d(5), ttime
      integer  itype(5), tcycle
      real     ftype(5)
      double precision dttime
      character*256 cloption

      driver = DB_PDB
      nargs = iargc()
      call getarg(1, cloption)
      if (cloption .eq. "DB_HDF5") then
          driver = DB_HDF5
      end if

C...Generate some data to write out.

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
C...Create the file (using the SILO interface)

      ierr =  dbcreate ("pointf77.silo", 13, DB_CLOBBER, DB_LOCAL,
     .                 "file info goes here", 19, driver, dbid)
      if (ierr .ne. 0) then
         print *, 'dbcreate had error'
      else
         print *, 'created file pointf77.silo'
      endif

C...Create an option list containing time and cyle info.

      ierr = dbmkoptlist(3, optlistid)                   ! Create the option list
      ierr = dbaddiopt  (optlistid, DBOPT_CYCLE, tcycle) ! Add integer opt
      ierr = dbaddropt  (optlistid, DBOPT_TIME, ttime)   ! Add real opt
      ierr = dbadddopt  (optlistid, DBOPT_DTIME, dttime)   ! Add double opt


C...Write out the point mesh.

      ierr = dbputpm (dbid, 'pmesh', 5, 2, x, y, z, npts, DB_FLOAT,
     .                optlistid, meshid)


C...Write out the point variables

      ierr = dbputpv1 (dbid, 'd', 1, 'pmesh', 5, d, npts, DB_FLOAT,
     .                 optlistid, varid)

C     ierr = dbputpv1 (dbid, 'ftype', 5, 'pmesh', 5, ftype, npts, DB_FLOAT,
C    .                 optlistid, varid1)




      ierr = dbclose (dbid)
      if (ierr .ne. 0) print *, 'dbclose had error'


      stop
      end
