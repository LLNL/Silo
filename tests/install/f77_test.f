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

C-------------------------------------------------------------------------
C Purpose
C
C   Demonstrate use of SILO for creating compound array objects.
C
C-------------------------------------------------------------------------

      program main

      implicit none
      include "silo.inc"
      integer dbid, err, driver, nargs
      character*256 cloption

      character*32 elemnames(3)
      integer elemlengths(3), array_id, i
      real values(18)

      character*32 g_enames(10)
      integer g_elengths(10), g_nelems, g_nvalues, g_datatype,
     $     g_maxwidth

      real g_values(100)

      data elemnames/"x","y","z"/
      data elemlengths/4,6,8/

      driver = DB_PDB
      nargs = iargc()
      call getarg(1, cloption)
      if (cloption .eq. "DB_HDF5") then
          driver = DB_HDF5
      end if

      err = dbshowerrors(DB_ABORT)

C...Create file named "robb.pdb".  Database ID is returned in 'dbid'.

      err = dbcreate("arrayf77.silo", 13, 0, DB_LOCAL, "file info",
     $     9, driver, dbid)


      do i=1, 18
         values(i) = i/100.0
      enddo

      err = dbputca (dbid, "carray", 6, elemnames, 32,
     $ elemlengths, 3, values, DB_FLOAT, DB_F77NULL, array_id)

      err = dbclose (dbid)


CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C  Now try to read from the file...
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

      err = dbopen ("arrayf77.silo", 13, DB_UNKNOWN, DB_READ, dbid)

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
