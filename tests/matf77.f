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


c...This program tests the use of the UCD Fortran jacket routines, plus
c...the underlying UCD C routines.
      include 'silo.inc'
      integer driver, nargs
      character*256 cloption

      driver = DB_PDB
      nargs = iargc()
      call getarg(1, cloption)
      if (cloption .eq. "DB_HDF5") then
          driver = DB_HDF5
      end if

      call writeit ("mat.silo", driver)
      print *,'Created mat.silo'


      stop
      end

      subroutine writeit (fname, drvr)
C----------------------------------------------------------------------
C----------------------------------------------------------------------
C----------------------------------------------------------------------
C----------------------------------------------------------------------
C----------------------------------------------------------------------
Cc      implicit double precision (a-h,o-z)


C...Include SILO definitions.

      include 'silo.inc'


      character*(*) fname
      integer       drvr

      parameter (NMATS  = 2)
      parameter (NZONES = 6)
      parameter (NFACES = 10)   ! External faces only
      parameter (NNODES = 12)
      parameter (MAXMIX = NMATS * NZONES)
      parameter (MAXOMIX= NMATS * 2)
      parameter (NZSHAPES = 1)
      parameter (NFSHAPES = 1)
      parameter (NFTYPES  = 0)
      parameter (LZNODELIST = 4 * NZONES)
      parameter (LFNODELIST = 2 * NFACES)


      integer  dbid, idatatype
      integer  tcycle
      real     ttime
      real     x(NNODES), y(NNODES)
      real     d(NZONES), df(MAXOMIX), dfnew(MAXMIX)
      integer  fnodelist(LFNODELIST)
      integer  fshapesize(NFSHAPES), fshapecnt(NFSHAPES)
      integer  zshapesize(NZSHAPES), zshapecnt(NZSHAPES)
      integer  znodelist(LZNODELIST)
      integer  flid, zlid

      integer  matlist(NZONES), matlisto(NZONES)
      integer  nmix(NMATS), matnos(NMATS)
      real     vf(MAXOMIX)
      integer  mixedels(MAXOMIX)

      integer  mix_mat(MAXMIX), mix_next(MAXMIX)
      integer  mix_zone(MAXMIX)
      real     mix_vf(MAXMIX)
      character*1024 meshnms(3)
      character*1024 meshnms2
      character(len=17), allocatable :: meshnms3(:)
      integer  dims(2), err, optlist, lmeshnms(3)
      integer  meshtypes(3)

      integer btype, ol


      data matlist /2, 2, 0, 0, 1, 1/
      data mixedels/3, 4, 3, 4/
      data nmix    /2, 2/
      data vf      /.6, .8, .4, .2/
      data matnos  /1, 2/
      data ttime   /1.23/
      data tcycle  /200/
      character*1024 matnms(2)
      integer  lmatnms(2)

      data x /0., 0., 0., 0.,1.5,1.5,1.5,1.5, 3., 3., 3., 3./
      data y /3., 2., 1., 0., 3., 2., 1., 0., 3., 2., 1., 0./
      data d /1.,2.,3.,4.,5.,6./
      data df/.58,.78,.42,.22/

      data fshapesize /2/, fshapecnt/NFACES/
      data zshapesize /4/, zshapecnt/NZONES/
      data fnodelist  /0,1,1,2,2,3,3,7,7,11,11,10,10,9,9,8,8,4,4,0/
      data znodelist
     .         /0,1,5,4,4,5,9,8,1,2,6,5,5,6,10,9,2,3,7,6,6,7,11,10/

C...Here is an example of a Fortran allocatable array of strings.
C...However, I don't know how the Silo interface would currently
C...accept it. I am keeping this code here for future ref but the 
C...type it is creating is currently not used anywhere. I think the
C...issue is the two-level type involved. I don't think that 
C...translates 1:1 with anything in Silo's Fortran interface.
C     type :: varl
C        character(len=:), allocatable :: name
C     end type varl
C     type(varl), dimension(3) :: meshnms4(3)
C     allocate(character(len=11) :: meshnms4(1)%name)
C     meshnms4(1)%name = "Mandalorian"
C     allocate(character(len=8) :: meshnms4(2)%name)
C     meshnms4(2)%name = "BobaFett"
C     allocate(character(len=4) :: meshnms4(3)%name)
C     meshnms4(3)%name = "Cara"

      ttime = 2.345
Cc      idatatype = 20       ! double
      idatatype = 19       ! float

      err = dbshowerrors(DB_ABORT)
      err = dbset2dstrlen(1024)

C...Create option list for use with functions that want it.

      err = dbmkoptlist (5, optlist)
      err = dbaddiopt   (optlist, DBOPT_CYCLE, tcycle)  ! integer
      err = dbaddropt   (optlist, DBOPT_TIME,  ttime)   ! real



      err = dbcreate(fname, len(fname), 0, DB_LOCAL,
     .               "foo-info", 8, drvr, dbid)
      if (err .ne. 0) then
         print *,'Error from dbcreate'
         stop
      endif

      err = dbputfl(dbid, "facelist", 8,
     .              NFACES, 2, fnodelist, LFNODELIST, 0, DB_F77NULL,
     .              fshapesize, fshapecnt, NFSHAPES, DB_F77NULL,
     .              DB_F77NULL, 0, flid)




      err = dbputzl(dbid, "zonelist", 8,
     .              NZONES, 2, znodelist, LZNODELIST, 0,
     .              zshapesize, zshapecnt, NZSHAPES, zlid)


      err = dbaddcopt (optlist, DBOPT_XLABEL, "my-x-label", 10)

      err = dbputum(dbid, "mesh1", 5, 2, x, y, DB_F77NULL,
     .              "X", 1, "Y", 1, DB_F77NULLSTRING, 0,
     .              idatatype, NNODES, NZONES, "zonelist", 8,
     .              "facelist", 8, optlist, meshid)



C...Test material conversion functions, and write results.

      do 10 i = 1 , MAXMIX
         mix_mat(i)  = -99
         mix_next(i) = -99
         mix_zone(i) = -99
         mix_vf(i)   = -99.
 10   continue

      inmats  = NMATS
      inzones = NZONES
      dims(1) = NZONES
      ndims   = 1



      err = convtmat (mixedels, nmix, matlist, matnos, matlisto,
     .                mix_next, mix_mat, mix_zone,
     .                mixlen, idatatype, inmats, inzones,
     .                vf, mix_vf)

      err = dbputmat (dbid, "mat", 3, "mesh1", 5,
     .               inmats, matnos, matlisto,
     .               dims, ndims, mix_next, mix_mat, mix_zone, mix_vf,
     .               mixlen, idatatype, optlist, matid)


      err = convtmix (df, idatatype, mixedels, nmix, inmats,
     .                matlist, inzones, dfnew, mixlen)

      err = dbputuv1 (dbid, "d", 1, "mesh1", 5,
     .                d, inzones, dfnew, mixlen,
     .                idatatype, DB_ZONECENT, optlist, id)

C...This is a bugus multi-mesh but it tests the interface,
C...particularly with strings larger than the old default
C...of 32 chars.
      meshnms(1)  = "mesh1"
      lmeshnms(1) = 5
      meshnms(2)  = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      lmeshnms(2) = 40
      meshnms(3)  = "foobargorfo"
      lmeshnms(3) = 11
      meshtypes(1) = DB_UCDMESH
      meshtypes(2) = DB_UCDMESH
      meshtypes(3) = DB_UCDMESH
      err = dbputmmesh (dbid, "multimesh", 9, 3,
     .                  meshnms, lmeshnms, meshtypes,
     .                  DB_F77NULL, id)

C...This is really a sloppy call because its testing only half of
C...the namescheme-way of doing business.
      btype = DB_UCDMESH
      err = dbmkoptlist (5, ol)
      err = dbaddiopt   (ol, DBOPT_MB_BLOCK_TYPE, btype)  ! integer
      err = dbputmmesh (dbid, "multimesh2", 10, 3,
     .                  meshnms, lmeshnms, DB_F77NULL,
     .                  ol, id)

C...Test a compacted array of strings. Note setting 2dstrlen to zero.
      meshnms2 = "sandymarkabigail"
      lmeshnms(1) = 5
      lmeshnms(2) = 4
      lmeshnms(3) = 7
      err = dbset2dstrlen(0)
      err = dbputmmesh (dbid, "multimesh3", 10, 3,
     .                  meshnms2, lmeshnms, meshtypes,
     .                  DB_F77NULL, id)

C...Test an allocated array of strings.
      allocate(meshnms3(3))
      meshnms3(1) = "sandy"
      meshnms3(2) = "mark"
      meshnms3(3) = "abigail"
      err = dbset2dstrlen(17)
      err = dbputmmesh (dbid, "multimesh4", 10, 3,
     .                  meshnms3, lmeshnms, meshtypes,
     .                  DB_F77NULL, id)

C...Test out multi mesh. (Special case, since nmesh == 1.) Ordinarily
C...you would have to provide arrays for the 'ids', 'types' and 'dirs'.

C     meshnms(1)  = "mesh1"
C     lmeshnms(1) = 5
C
C     err = dbputmulti (dbid, "multimesh", 9, 1, meshid,
C    .                  meshnms, lmeshnms, DB_UCD, 0,
C    .                  DB_F77NULL, id)

C...Close file before quitting.

      err = dbclose(dbid)

      return
      end

C ******************************************************************************
C *  Routine                                                            convtmat
C *
C *  Purpose
C *
C *     Convert from by-material representation of material data to by-zone.
C *
C *  Programmer
C *
C *     Jeff Long, NSSD/B
C *
C *  Notes
C *
C *     The SILO material routines expect data to be stored in a by-zone
C *     fashion. This function converts from the ALE3D representation to
C *     the SILO representation.
C *
C ******************************************************************************

      function convtmat (mixed_els, nmix, matlist, matnos, matlist_out,
     .                   mix_next, mix_mat, mix_zone, mixlen, datatype,
     .                   nmats, nzones, vf, mix_vf)

                                ! Input variables
      integer  nmats            ! Number of materials
      integer  nzones           ! Number of zones
      integer  mixed_els(*)     ! [sumnmix]: mat-1 mixed els, mat-2, etc.
      integer  nmix(*)          ! [nmats]: number of mixed els per mat
      integer  matlist(*)       ! [nzone]: material id, else 0 for mixed
      integer  matnos(*)        ! [nmats]: material ID's
      real     vf(*)            ! [sumnmix]: mat-1 vf's, mat-2, etc.
      integer  datatype         ! Datatype indicator for volume fractions


                                ! Output variables
      integer  mixlen           ! Length of output mixed-data arrays
      integer  matlist_out(*)   ! [nzone]: material id, else <0 for mixed
      integer  mix_next(*)      ! [mixlen]: index of next entry for this zone
      integer  mix_mat(*)       ! [mixlen]: material ID
      integer  mix_zone(*)      ! [mixlen]: back pointer to original zone
      real     mix_vf(*)        ! [mixlen]: volume fractions in new format


      integer  i, j, iz, sumnmix
      integer  ifirst, imix
      integer  mat(1000)        ! FIX ME -- Should be dynamic, or an argument


      convtmat = 0

      ! Count total number of mixed elements
      sumnmix = 0
      do i = 1, nmats
          sumnmix = sumnmix + nmix(i)
      enddo


C     *------------------------------------------------------------
C     * Build a mixed-length variable which contains the material
C     * number for that entry.
C     *-----------------------------------------------------------

       iz = 1
       do i = 1 , nmats
          do j = 1 , nmix(i)
             mat(iz) = matnos(i)
             iz = iz+1
          enddo
       enddo


C      *------------------------------------------------------------
C      *  Loop over all mixed zones, setting mixed-data arrays in
C      *  process. Connect the various mixed entries for each zone
C      *  via the mix_next array, and modify 'matlist' to have the
C      *  negative index into mixed data arrays instead of merely
C      *  '0' to indicate mixed zones.
C      *------------------------------------------------------------

       imix = 1   ! Use 1-origin indexing

       do i = 1 , nzones

          matlist_out(i) = matlist(i)

          if (matlist(i) .le. 0) then   ! Mixed zone

               ifirst = -1

               do j = 1 , sumnmix

                    if (mixed_els(j) .eq. i) then

                         if (ifirst .eq. -1) then ! 1st el in linked list
                            ifirst = imix
                         else                   ! Set prev el to point to this
                            mix_next(imix-1) = imix
                         endif

                         mix_vf(imix)   = vf(j)
                         mix_mat(imix)  = mat(j)
                         mix_next(imix) = 0
                         mix_zone(imix) = i

                         imix = imix + 1
                    endif
               enddo
               if (ifirst .gt. 0) matlist_out(i) = -ifirst

          endif
       enddo

       mixlen = sumnmix


       return
       end


C *****************************************************************************
C *  Routine                                                            convtmix
C *
C *  Purpose
C *
C *     Convert a mixed data array from by-material to by-zone format.
C *
C *  Programmer
C *
C *     Jeff Long, NSSD/B
C *
C *  Notes
C *
C *     The SILO material routines expect data to be stored in a by-zone
C *     fashion. This function converts from the ALE3D representation to
C *     the SILO representation.
C *
C *****************************************************************************

      function convtmix (mixvar, datatype, mixed_els, nmix, nmats,
     .                   matlist, nzones, mixf, mixlen)

                                ! Input variables
       real     mixvar(*)       ! (sumnmix): mat-1 mix-data, mat-2, etc.
       integer  datatype        ! Real    or int
       integer  mixed_els(*)    ! (sumnmix): mat-1 mixed els, mat-2, etc.
       integer  nmix(*)         ! (nmats): number of mixed els per mat
       integer  nmats           ! Number of materials
       integer  matlist(*)      ! (nzone): material id, else 0 for mixed
       integer  nzones          ! Number of zones

                                ! Output variables
       real     mixf(*)         ! (mixlen): mixed data in new format
       integer  mixlen          ! Returned length of output mixed-data array


       integer  i, j, sumnmix
       integer  imix


       convtmix = 0

       ! Count total number of mixed elements
       sumnmix = 0
       do i = 1 , nmats
          sumnmix = sumnmix + nmix(i)
       enddo


C      *------------------------------------------------------------
C      *  Loop over all mixed zones, setting mixed-data array in
C      *  process.
C      *------------------------------------------------------------
       imix = 1
       do i = 1 , nzones

          if (matlist(i) .le. 0) then ! Mixed zone

             do j = 1 , sumnmix

                if (mixed_els(j) .eq. i) then
                   mixf(imix) = mixvar(j)
                   imix       = imix + 1
                endif

             enddo
          endif
       enddo

       return
       end


