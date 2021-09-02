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
C Fortran test program for simple CSG mesh contributed by
C
C Author: Robert Salko, rks171@gmail.com, Penn State University
C Mon Nov  5 16:10:14 PST 2012
C
C Modifications
C   Mark C. Miller, Mon Nov  5 16:11:04 PST 2012
C   Adjusted for intergration with Silo's test suite
C-------------------------------------------------------------------------
      program csgmesh
      
      implicit none
      
      include 'silo.inc'
      
      integer :: er
      integer :: ID
      integer :: lcoeffs
      integer :: lgridname
      integer :: lnamefile
      integer :: ndim
      integer :: nbounds
      integer :: nregs
      integer :: nzones
      integer :: stat
      integer :: nargs
      integer :: driver
      
      integer, dimension(2) :: typflags
      integer, dimension(3) :: oper
      integer, dimension(3) :: leftids
      integer, dimension(3) :: rightids
      integer, dimension(1) :: zonelist
      
      character (len=12) :: namefile
      character (len=8 ) :: gridname
      character (len=256) :: cloption
      
      real, dimension(12) :: coeffs=(/0.0,0.0,0.0,5.0,
     &    -10.0, 0.0, 0.0, 1.0, 0.0, 0.0, 20.0, 4.5/)
      real*8, dimension(6) :: extents=(/-5.0,-5.0,-5.0,5.0,5.0,5.0/)
      
      driver = DB_PDB
      nargs = iargc()
      call getarg(1, cloption)
      if (cloption .eq. "DB_HDF5") then
          driver = DB_HDF5
      end if
      lcoeffs = 12
      namefile = 'csgmesh.silo'
      lnamefile = 12
      nbounds = 2
      ndim = 3
      gridname = 'CSG_mesh'
      lgridname = 8
      
      
      typflags(1) = DBCSG_SPHERE_PR
      typflags(2) = DBCSG_CYLINDER_PNLR
      
      !Report all errors
      er=dbshowerrors(db_all)
      
      !Create the silo file
      er=dbcreate(namefile,lnamefile,DB_CLOBBER,DB_LOCAL,DB_F77NULL,0,
     &            driver,ID)
      
      !Create the CSG Mesh
      er=dbputcsgm(ID,gridname,lgridname,ndim,nbounds,typflags,
     &             DB_F77NULL,coeffs,lcoeffs,DB_FLOAT,extents,
     &             'csgz1',5,DB_F77NULL,stat)

      !Create the CSG Zone list
      nregs = 3
      nzones = 1
      zonelist(1) = 2
      leftids(1) = 0
      leftids(2) = 1
      leftids(3) = 0
      rightids(1) = -1
      rightids(2) = -1
      rightids(3) =  1
      oper(1) = DBCSG_INNER
      oper(2) = DBCSG_INNER
      oper(3) = DBCSG_UNION

      er=dbputcsgzl(ID,'csgz1',5,nregs,oper,leftids,
     &              rightids,DB_F77NULL,0,DB_FLOAT,nzones,zonelist,
     &              DB_F77NULL,stat)
      
      er=dbclose(ID);
      end program csgmesh
