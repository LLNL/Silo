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
C   Demonstrate use of SILO for creating curve objects.
C
C-------------------------------------------------------------------------

      program main
      implicit none
      include "silo.inc"
      integer dbid, curveid, err, i, curve_id, dtype, npts
      integer driver, nargs
      real x(20), y1(20), y2(20), xin(20), yin1(20), yin2(20)
      character*256 cloption

      driver = DB_PDB
      nargs = iargc()
      call getarg(1, cloption)
      if (cloption .eq. "DB_HDF5") then
          driver = DB_HDF5
      end if

C     Create the curve x and y values...

      do i=1, 20
         x(i) = (i-1) * 3.1415927 / 20.0
         y1(i) = sin (x(i))
         y2(i) = cos (x(i))
      enddo

C     Write curve data to a PDB file...

      err = dbcreate ("curvef77.silo", 13, 0, DB_LOCAL,
     $     "Curve Data", 10, driver, dbid)

      err = dbputcurve (dbid, "sincurve", 8, x, y1, DB_FLOAT,
     $     20, DB_F77NULL, curve_id)

      err = dbputcurve (dbid, "coscurve", 8, x, y2, DB_FLOAT,
     $     20, DB_F77NULL, curve_id)

      err = dbclose (dbid)

C     Read the pdb file...

      err = dbopen ("curvef77.silo", 13, driver, DB_READ, dbid)
      err = dbgetcurve (dbid, "sincurve", 8, 20, xin, yin1,
     $     dtype, npts)
      err = dbgetcurve (dbid, "coscurve", 8, 20, xin, yin2,
     $     dtype, npts)
      err = dbclose (dbid)

C     Check for errors...

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
