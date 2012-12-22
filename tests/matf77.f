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
c----------------------------------------------------------------------
c----------------------------------------------------------------------
c----------------------------------------------------------------------
c----------------------------------------------------------------------
c----------------------------------------------------------------------
cc      implicit double precision (a-h,o-z)


c...Include SILO definitions.

      include 'silo.inc'


      character*(*) fname

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
      integer  dims(2), err, optlist, lmeshnms(3)
      integer  meshtypes(3)


      data matlist /2, 2, 0, 0, 1, 1/
      data mixedels/3, 4, 3, 4/
      data nmix    /2, 2/
      data vf      /.6, .8, .4, .2/
      data matnos  /1, 2/
      data ttime   /1.23/
      data tcycle  /200/

      data x /0., 0., 0., 0.,1.5,1.5,1.5,1.5, 3., 3., 3., 3./
      data y /3., 2., 1., 0., 3., 2., 1., 0., 3., 2., 1., 0./
      data d /1.,2.,3.,4.,5.,6./
      data df/.58,.78,.42,.22/

      data fshapesize /2/, fshapecnt/NFACES/
      data zshapesize /4/, zshapecnt/NZONES/
      data fnodelist  /0,1,1,2,2,3,3,7,7,11,11,10,10,9,9,8,8,4,4,0/
      data znodelist
     .         /0,1,5,4,4,5,9,8,1,2,6,5,5,6,10,9,2,3,7,6,6,7,11,10/


      ttime = 2.345
cc      idatatype = 20       ! double
      idatatype = 19       ! float

      err = dbshowerrors(DB_ABORT)
      err = dbset2dstrlen(1024)

c...Create option list for use with functions that want it.

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



c...Test material conversion functions, and write results.

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

c...This is a bugus multi-mesh but it tests the interface,
c...particularly with strings larger than 32 chars
      meshnms(3)  = "foobargorfo"
      lmeshnms(3) = 11
      meshnms(1)  = "mesh1"
      lmeshnms(1) = 5
      meshnms(2)  = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      lmeshnms(2) = 40
      meshtypes(1) = DB_UCD
      meshtypes(2) = DB_UCD
      meshtypes(3) = DB_UCD
      err = dbputmmesh (dbid, "multimesh", 9, 3,
     .                  meshnms, lmeshnms, meshtypes,
     .                  DB_F77NULL, id)

c...Test out multi mesh. (Special case, since nmesh == 1.) Ordinarily
c...you would have to provide arrays for the 'ids', 'types' and 'dirs'.

c     meshnms(1)  = "mesh1"
c     lmeshnms(1) = 5
c
c     err = dbputmulti (dbid, "multimesh", 9, 1, meshid,
c    .                  meshnms, lmeshnms, DB_UCD, 0,
c    .                  DB_F77NULL, id)

c...Close file before quitting.

      err = dbclose(dbid)

      return
      end

c ******************************************************************************
c *  Routine                                                            convtmat
c *
c *  Purpose
c *
c * 	Convert from by-material representation of material data to by-zone.
c *
c *  Programmer
c *
c *     Jeff Long, NSSD/B
c *
c *  Notes
c *
c *     The SILO material routines expect data to be stored in a by-zone
c *     fashion. This function converts from the ALE3D representation to
c *     the SILO representation.
c *
c ******************************************************************************

      function convtmat (mixed_els, nmix, matlist, matnos, matlist_out,
     .                   mix_next, mix_mat, mix_zone, mixlen, datatype,
     .                   nmats, nzones, vf, mix_vf)

                                ! Input variables
      integer  nmats		! Number of materials
      integer  nzones		! Number of zones
      integer  mixed_els(*)	! [sumnmix]: mat-1 mixed els, mat-2, etc.
      integer  nmix(*)		! [nmats]: number of mixed els per mat
      integer  matlist(*)       ! [nzone]: material id, else 0 for mixed
      integer  matnos(*)        ! [nmats]: material ID's
      real     vf(*)		! [sumnmix]: mat-1 vf's, mat-2, etc.
      integer  datatype		! Datatype indicator for volume fractions


                                ! Output variables
      integer  mixlen		! Length of output mixed-data arrays
      integer  matlist_out(*)	! [nzone]: material id, else <0 for mixed
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


c     *------------------------------------------------------------
c     * Build a mixed-length variable which contains the material
c     * number for that entry.
c     *-----------------------------------------------------------

       iz = 1
       do i = 1 , nmats
          do j = 1 , nmix(i)
             mat(iz) = matnos(i)
             iz = iz+1
          enddo
       enddo


c      *------------------------------------------------------------
c      *  Loop over all mixed zones, setting mixed-data arrays in
c      *  process. Connect the various mixed entries for each zone
c      *  via the mix_next array, and modify 'matlist' to have the
c      *  negative index into mixed data arrays instead of merely
c      *  '0' to indicate mixed zones.
c      *------------------------------------------------------------

       imix = 1	  ! Use 1-origin indexing

       do i = 1 , nzones

  	  matlist_out(i) = matlist(i)

  	  if (matlist(i) .le. 0) then	! Mixed zone

  	       ifirst = -1

  	       do j = 1 , sumnmix

  		    if (mixed_els(j) .eq. i) then

  			 if (ifirst .eq. -1) then ! 1st el in linked list
                            ifirst = imix
  			 else			! Set prev el to point to this
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


c *****************************************************************************
c *  Routine                                                            convtmix
c *
c *  Purpose
c *
c * 	Convert a mixed data array from by-material to by-zone format.
c *
c *  Programmer
c *
c *     Jeff Long, NSSD/B
c *
c *  Notes
c *
c *     The SILO material routines expect data to be stored in a by-zone
c *     fashion. This function converts from the ALE3D representation to
c *     the SILO representation.
c *
c *****************************************************************************

      function convtmix (mixvar, datatype, mixed_els, nmix, nmats,
     . 	                 matlist, nzones, mixf, mixlen)

                                ! Input variables
       real     mixvar(*)	! (sumnmix): mat-1 mix-data, mat-2, etc.
       integer  datatype	! Real    or int
       integer  mixed_els(*)	! (sumnmix): mat-1 mixed els, mat-2, etc.
       integer  nmix(*)		! (nmats): number of mixed els per mat
       integer  nmats		! Number of materials
       integer  matlist(*)	! (nzone): material id, else 0 for mixed
       integer  nzones		! Number of zones

                                ! Output variables
       real     mixf(*)		! (mixlen): mixed data in new format
       integer  mixlen		! Returned length of output mixed-data array


       integer  i, j, sumnmix
       integer  imix


       convtmix = 0

       ! Count total number of mixed elements
       sumnmix = 0
       do i = 1 , nmats
          sumnmix = sumnmix + nmix(i)
       enddo


c      *------------------------------------------------------------
c      *  Loop over all mixed zones, setting mixed-data array in
c      *  process.
c      *------------------------------------------------------------
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


