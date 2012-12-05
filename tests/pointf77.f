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
* be used for advertising or product endorsement purposes.
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

      ierr =  dbcreate ("pointf77.silo", 13, DB_CLOBBER, DB_LOCAL,
     .                 "file info goes here", 19, driver, dbid)
      if (ierr .ne. 0) then
         print *, 'dbcreate had error'
      else
         print *, 'created file pointf77.silo'
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
