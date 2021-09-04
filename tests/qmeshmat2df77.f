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

      program testmat


C...This program tests the use of the UCD Fortran jacket routines, plus
C...the underlying UCD C routines.
      include 'silo.inc'
      integer driver, nargs
      character*256 cloption

      driver = DB_PDB
      nargs = iargc()
      call getarg(1, cloption)
      if (cloption .eq. "DB_HDF5") then
          driver = DB_HDF5
      end if

      call writeit ("quad_mat.silo", driver)
      print *,'Created quad_mat.silo'

      stop
      end

      subroutine writeit (fname, drvr)
C----------------------------------------------------------------------
C----------------------------------------------------------------------
C----------------------------------------------------------------------
C----------------------------------------------------------------------
C Writes the following 2x3 quadmesh (note colmajor ordering)
C
C    ^Y axis
C    |
C   3-     1--------4--------7-------10
C    |     |        |        |       |
C    |     |   1    |   3    |   5   |
C   2-     2--------5--------8-------11
C    |     |        |        |       |
C    |     |   2    |   4    |   6   |
C   1-     3--------6--------9-------12
C    |
C   0------1--------2--------3-------4-->X axis
C    
C with 2-material composition. . .
C
C          +--------+-------/+-------+
C          |   1    |  1   / |       |
C          |        |     / 2|   2   |
C          +--------+----/---+-------+
C          |   1    | 1 /    |       |
C          |        |  /  2  |   2   |
C          +--------+-/------+-------+
C
C using 2D Fortran arrays A(#rows,#cols) stored column major (e.g.
C row index varies fastest as you march through A's memory). Note
C that #rows is size in Y direction while #cols is size in X
C direction. So, A is dimensioned (nY,nX). In Fortran, the left-most
C dimension in an array declaration varies fastest when marching 
C through array elements in memory. However, the 'dims' array in
C Silo's DBPut calls is arranged such that dims[0] is the size in
C X, dims[1] the size in Y and dims[2], if applicable, the size in Z.
C So, although we have declarations for A(3,4), the dims array is
C passed in as dims[0] = 4, dims[1] = 3
C----------------------------------------------------------------------
Cc      implicit double precision (a-h,o-z)


C...Include SILO definitions.

      include 'silo.inc'

      character*(*) fname
      integer       drvr

      integer  dbid, dbstat
      integer  tcycle
      real     ttime
      real     x(3,4), y(3,4)
C...For co-linear case
      real xcl(4), ycl(3)

      integer  matlist(2,3), matlistcl(3,2)
      integer  dims(2), dimscl(2)
      integer  zdims(2), zdimscl(2)

      integer  matnos(2)
      integer  mix_next(4)
      integer  mix_zone(4)
      integer  mix_mat(4)
      real     mix_vf(4)

      data matlist/1, 1, -1, -3, 2, 2/
C      data matlistcl/1, -1, 2, 1, -3, 2/
      data matlistcl/1, -3, 2, 1, -1, 2/
C      data dims/4,3/
      data dims/3,4/
      data dimscl/4,3/
C      data zdims/3,2/
      data zdims/2,3/
      data zdims/2,3/
      data zdimscl/3,2/
      data x/1.0,1.0,1.0,2.0,2.0,2.0,3.0,3.0,3.0,4.0,4.0,4.0/
      data y/3.0,2.0,1.0,3.0,2.0,1.0,3.0,2.0,1.0,3.0,2.0,1.0/
      data xcl/1.0,2.0,3.0,4.0/
      data ycl/1.0,2.0,3.0/
      data mix_next/2,0,4,0/
      data mix_zone/2,2,4,4/
      data mix_mat/1,2,1,2/
      data mix_vf/0.7,0.3,0.4,0.6/
      data matnos  /1, 2/
      data ttime   /1.23/
      data tcycle  /200/

      ttime = 2.345

      err = dbshowerrors(DB_ABORT)

      err = dbcreate(fname, len(fname), 0, DB_LOCAL,
     .               "foo-info", 8, drvr, dbid)
      if (err .ne. 0) then
         print *,'Error from dbcreate'
         stop
      endif

      err = dbputqm(dbid, "qmesh", 5, "x", 1, "y", 1,
     . DB_F77NULLSTRING, 0,
     . x, y, DB_F77NULL, dims, 2, DB_FLOAT, DB_NONCOLLINEAR,
     . DB_F77NULL, dbstat)

      err = dbputqm(dbid, "qmeshcl", 7, "xcl", 3, "ycl", 3,
     . DB_F77NULLSTRING, 0,
     . xcl, ycl, DB_F77NULL, dimscl, 2, DB_FLOAT, DB_COLLINEAR,
     . DB_F77NULL, dbstat)

      err = dbputmat (dbid, "mat", 3, "qmesh", 5,
     . 2, matnos, matlist, zdims, 2,
     . mix_next, mix_mat, mix_zone, mix_vf, 4,
     . DB_FLOAT, DB_F77NULL, dbstat)

      err = dbputmat (dbid, "matcl", 5, "qmeshcl", 7,
     . 2, matnos, matlistcl, zdimscl, 2,
     . mix_next, mix_mat, mix_zone, mix_vf, 4,
     . DB_FLOAT, DB_F77NULL, dbstat)

C...Close file before quitting.

      err = dbclose(dbid)

      return
      end
