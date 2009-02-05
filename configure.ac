dnl Process this file with autoconf to produce configure.
dnl 
dnl 
dnl 
dnl This work was produced at the University of California, Lawrence Livermore
dnl National Laboratory (UC LLNL)  under contract no.  W-7405-ENG-48 (Contract
dnl 48) between  the U.S. Department of Energy  (DOE)  and The Regents  of the
dnl University  of  California  (University)  for  the  operation of  UC LLNL.
dnl The  rights of  the Federal  Government  are reserved  under  Contract  48
dnl subject  to  the restrictions agreed  upon  by  the DOE and  University as
dnl allowed under DOE Acquisition Letter 97-1.
dnl
dnl                                 DISCLAIMER
dnl
dnl This work  was prepared as an  account  of work sponsored by an  agency of
dnl the United States Government. Neither the United States Government nor the
dnl University  of California nor any of  their employees, makes any warranty,
dnl express or implied,  or assumes  any liability  or responsiblity  for  the
dnl accuracy, completeness, or    usefullness of any  information,  apparatus,
dnl product, or  process   disclosed, or represents that  its   use would  not
dnl infringe privately    owned  rights. Reference   herein  to  any  specific
dnl commercial  products, process,  or    service by trade   name,  trademark,
dnl manufacturer, or otherwise, does  not necessarily constitute or imply  its
dnl endorsement, recommendation, or  favoring by the United  States Government
dnl or the   University  of  California.  The views   and  opinions of authors
dnl expressed herein do not  necessarily state or  reflect those of the United
dnl States Government  or the University of  California, and shall not be used
dnl for advertising or product endorsement purposes.
dnl
dnl                       NOTIFICATION OF COMMERCIAL USE
dnl
dnl Commercialization  of this  product is  prohibited  without notifying  the
dnl Department  of  Energy  (DOE) or  Lawrence  Livermore National  Laboratory
dnl (LLNL).
dnl

dnl
dnl This is the autoconf configuration file for MeshTV and company.
dnl Robb Matzke <matzke@llnl.gov>
dnl Mar 18, 1996
dnl
dnl Modifications:
dnl
dnl     Robb Matzke, 5 Mar 1997
dnl     Replaced `touch X' with `echo something > X' because touch doesn't
dnl     update the mtime when touching an empty file on some old BSD systems
dnl     (and neither do other commands that open an empty file for read/write
dnl     access).
dnl
dnl     Robb Matzke, 6 Mar 1997
dnl     Added checks for `-lXpm' and `-lXext' needed by meshtv and meshtvx.
dnl
dnl     Robb Matzke, 19 Mar 1997
dnl     Added a check for a site configuration file based on the host
dnl     name.  If `hostname` returns X, then the configuration file
dnl     ./cofig-site/X.conf is sourced.  The file contains envronment
dnl     variable settings which would have normally appeared on the
dnl     command line so that when make reruns configure the appropriate
dnl     variables are set.  For instance, when I work on maya.nuance.mdn.com
dnl     I use the GNU gcc compiler and I want to turn on warnings and generate
dnl     debug tables.  This is done by adding the line
dnl
dnl           export CFLAGS="-Wall -g"
dnl
dnl     to the ./config-site/maya.nuance.mdn.com.conf file.  These personal
dnl     site configuration files should not be part of a distribution.
dnl
dnl     Robb Matzke, 20 Mar 1997
dnl     The builtin `pwd' command fails on some systems, so we use
dnl     `/bin/pwd' instead if available.
dnl
dnl     Robb Matzke, 20 Mar 1997
dnl     Added the `-f /dev/null' option to the GNU make test in case
dnl     a valid makefile accidently exists.
dnl
dnl     Robb Matzke, 2 Apr 1997
dnl     Removed the AC_FUNC_MMAP check since nothing uses it.
dnl
dnl     Robb Matzke, 15 May 1997
dnl     Removed OS_TYPE as a config.h constant since it was never added to
dnl     acconfig.h anyway.
dnl
dnl     Quincey Koziol, 15 May 1997
dnl     Added link checking for FreeBSD.
dnl
dnl     Sean Ahern, 18 June 1997
dnl     Added the ability to have a config-site configuration file that is
dnl     named the same as the operating system.  Configure now checks
dnl     automatically for this (using uname(1)).  For example, I added
dnl     config-site/OSF1.conf for DEC machines.  Any configuration needed for
dnl     that operating system should go in that file.
dnl
dnl     Robb Matzke, 19 Jun 1997
dnl     Changed the default from `test -L' to `test -h'.
dnl
dnl	Robb Matzke, 27 Aug 1997
dnl	Added -I/usr/local/include and -L/usr/local/lib to the compile
dnl	command so we have a better chance of finding the readline
dnl	library.
dnl
dnl     Robb Matzke, 23 Dec 1997
dnl	Added ability to turn off checking for the GNU readline since lots
dnl     of systems have the software but it's misconfigured, causing
dnl     configure to detect it's presence and causing make to fail.  Say
dnl     `--without-readline' to turn detection off.
dnl
dnl	Robb Matzke, 4 Feb 1998
dnl	Combined GNU target file with non-GNU target file and put the results
dnl	in include/make-targets.in with the other targets.
dnl
dnl     Eric Brugger, Wed Aug 12 15:50:27 PDT 1998
dnl     I made the libraries load static on Linux since most people do not
dnl     have Motif.  Added a check for the Motif includes and libraries
dnl     being found without an explicit include or lib path and added one
dnl     one if it didn't work.  I changed the CPPFLAGS and LDFLAGS to
dnl     append /usr/local/include and /usr/local/lib instead of setting it
dnl     if they weren't already set.  I modified LDFLAGS to contain $tmplib
dnl     at the beginning so that libraries will be found in the local lib
dnl     directory before the public lib directories.
dnl
dnl     Eric Brugger, Thu Aug 13 13:18:43 PDT 1998
dnl     I corrected a bug with the checking of the knowledge of the Motif
dnl     libraries where the test always returned false.
dnl
dnl     Eric Brugger, Fri Aug 28 16:43:46 PDT 1998
dnl     I added support for parallel meshtv.
dnl
dnl     Sean Ahern, Wed Dec  2 11:29:54 PST 1998
dnl     Added a check for "clearmake".  If we're running clearmake,
dnl     set the verbosity to 0.  Also, create a line for the
dnl     compile target in the Makefiles so that clearmake only
dnl     winks in object files that have been created on the same OS
dnl     type.
dnl
dnl     Eric Brugger, Mon Dec 28 09:51:18 PST 1998
dnl     I removed checks for basename and dirname.  The behavior of both
dnl     is poorly defined and varies between OSes.  We could check for a
dnl     specific behavior and then use the system one in that case that
dnl     is a lot of work when using our own is much simpler.
dnl
dnl	Robb Matzke, Tue Feb  9 10:38:54 EST 1999
dnl	Check for location of hdf5 headers and libraries.
dnl
dnl	Robb Matzke, Tue Mar  2 16:35:48 EST 1999
dnl	Check for location of vbt, dsl, and dmf headers and libraries.
dnl
dnl     Eric Brugger, Mon Mar 15 11:39:40 PST 1999
dnl     Made the silo netcdf, pdb and taurus drivers optional.
dnl
dnl	Mark Miller, Thu Mar 25 17:11:34 PST 1999
dnl     Made dmf and hdf5 drivers optional as Eric did above
dnl
dnl     Jeremy Meredith, Thu May 20 15:56:45 PDT 1999
dnl     Added a check for a site config with the first field of hostname
dnl     removed.  This allows a single site config for node-addressable
dnl     machines such as the IBM SP2.  Also reordered the hostname and
dnl     OSname config checks so that a single machine can override the
dnl     OS-based default configuration.
dnl
dnl     Brad Whitlock, Wed Jun 23 15:30:34 PST 1999
dnl     Added a check for the VDL libraries. If the environment variable
dnl     "VDL" is set, then the CFLAGS of meshtvx and Spi are modified
dnl     to include -DVDL and -I$(VDL)/include and -L$(VDL)/lib.
dnl
dnl     Jeremy Meredith, Tue Jul 27 18:27:37 PDT 1999
dnl     Added code to clear MPI_LIBS if we did not --enable-parallel.
dnl
dnl     Eric Brugger, Wed Aug 11 13:11:45 PDT 1999
dnl     I removed the check for -lXpm.  I modified the script to only
dnl     check for -lXext on Linux.  I modified the script to only check
dnl     for -lnsl and -lsocket on Solaris.
dnl
dnl     Brad Whitlock, Wed Jul 7 15:29:45 PST 1999
dnl     Added a check for parallel when the VDL libraries are being 
dnl     checked. If parallel is enabled at that time, then the VDLP
dnl     driver will be compiled.
dnl
dnl     Eric Brugger, Fri Oct  8 14:12:27 PDT 1999
dnl     I added a check for -lXp on Linux.
dnl
dnl     Brad Whitlock, Fri Nov 5 14:53:17 PST 1999
dnl     Removed the GL driver from SpiLib.
dnl
dnl     Jeremy Meredith, Thu Nov 11 13:35:02 PST 1999
dnl     Added ability to have dynamic libraries on Linux systems.
dnl
dnl     Brad Whitlock, Thu Oct 14 10:23:34 PDT 1999
dnl     Added build target for the spectrum widget.
dnl
dnl     Jeremy Meredith and Sean Ahern, Mon Dec 13 14:24:28 PST 1999
dnl     Allow override of MPI_LIBS by external configuration.
dnl
dnl     Jeremy Meredith, Fri Jan  7 10:19:39 PST 2000
dnl     Check for existence of Mesa.  If it's not there: error if we need it,
dnl     don't try to build it if we don't need it.
dnl
dnl     Brad Whitlock, Thu Feb 17 15:17:41 PST 2000
dnl     I added -lpt to the meshtvx link line when using VDL. This makes
dnl     meshtv work with the new VDL library.
dnl
dnl     Sean Ahern, Mon Jan 10 16:26:31 PST 2000
dnl     I put in logic to read the version number in from a file.
dnl
dnl     Brad Whitlock, Thu Feb 10 16:47:50 PST 2000
dnl     Added --disable-sdx flag. Also added a variable called
dnl     tflops_override which will be set to yes on TFLOPS systems.
dnl     (ASCI Red) This bypasses some function tests. Added --disable_gui
dnl     flag. This disables all X and motif stuff from meshtvx and Spi.
dnl     Added --disable-browser so it does not have to be compiled.
dnl
dnl     Brad Whitlock, Fri Mar 31 16:41:33 PST 2000
dnl     Fixed a bug that caused a parallel compiler to be used all the time
dnl     on some platforms.
dnl
dnl     Eric Brugger, Mon May 15 12:02:59 PDT 2000
dnl     I changed references of -lpt to -lptutil to account for the name
dnl     change of the library.
dnl
dnl     Mark Miller, Wed Aug 23 15:38:27 PDT 2000 
dnl     I changed the logic for lib hdf5 to deal correctly with presence or
dnl     absence of zlib. 
dnl
dnl     Eric Brugger, Fri Apr  6 17:52:30 PDT 2001
dnl     I added -lXext to the meshtvx link line when using VDL. This makes
dnl     meshtv work with the new VDL library.
dnl
dnl     Brad Whitlock, Tue Jan 22 16:42:30 PST 2002
dnl     I added options for building JPEG support.
dnl
dnl     Eric Brugger, Tue Mar 12 10:44:02 PST 2002
dnl     I made the tests for jpeg and glr support more robust.  I also added
dnl     a check which adds the dynamic link library (-ldl) on Linux.
dnl
dnl     Jeremy Meredith, Wed Apr 24 11:17:24 PDT 2002
dnl     I changed the SunOS linker flags for the jpeg library.
dnl
dnl     Mark Miller, Tue Oct  1 12:27:07 PDT 2002
dnl     I removed the old dmf-driver checks 
dnl	I fixed the check for hdf5 libs to only change $LIB if HDF5 was
dnl	requested AND found.
dnl	I removed the checks for GLR
dnl
dnl     Eric Brugger, Fri Nov  8 15:15:26 PST 2002
dnl     I removed the logic that statically loaded the libraries on Linux
dnl     systems.
dnl
dnl     Eric Brugger, Thu Dec  5 13:27:13 PST 2002 
dnl     I reordered some of the tests to avoid a problem with hdf5 and the
dnl     size tests.
dnl
dnl     Eric Brugger, Thu Sep  4 13:05:39 PDT 2003
dnl     I corrected a bug with --without-exodus not disabling the exodus
dnl     driver.
dnl
dnl     Eric Brugger, Tue Mar  2 12:41:32 PST 2004
dnl     I modified the hdf5 library logic to statically link the hdf5
dnl     library to avoid LD_LIBRARY_PATH problems and having to distribute
dnl     the shared hdf5 library with binary distributions.
dnl
dnl     Jeremy Meredith, Tue Jul 12 16:34:00 PDT 2005
dnl     Added a Python wrapper for Silo.  This required adding
dnl     checks for the Python include files, adding support
dnl     for shared libraries, and adding support for C++ builds.
dnl 
dnl     Eric Brugger, Thu Mar  9 08:49:31 PST 2006
dnl     I added AC_PROG_EGREP to avoid some problems handling uid and gid
dnl     on some platforms.
dnl
dnl     Thomas Treadway, Mon May 22 11:30:52 PDT 2006
dnl     Initial configuration file for Silo, after MeshTV split.
dnl     Based on Mark Miller's "meshtv_mark_silo_451_enhancments" view.
dnl     Reformated to support recent versions autoconf, and automake.
dnl
dnl     Thomas Treadway, Mon Jun  1 13:11:00 PDT 2006
dnl     Bugs go to visit-help@llnl.gov
dnl
dnl     Thomas Treadway, Mon Jun 12 10:24:53 PDT 2006
dnl     Conversion to libtool seems to be working pretty well.
dnl     Currently using autoreconfig via config/bootstrap to use
dnl     autoheaders, libtoolize, aclocal, automake, autoconf, and
dnl     autotest.
dnl
dnl     Thomas Treadway, Mon Jun 26 13:06:39 PDT 2006
dnl     The building of tools/python is conditional and performed only on
dnl     explicit request.
dnl
dnl     Thomas Treadway, Wed Sep 13 12:45:01 PDT 2006
dnl     Changed defaults: fortran compiler, optimization, and largefiles.
dnl
dnl     Thomas Treadway, Fri Oct  6 13:50:01 PDT 2006
dnl     re-enabled config-site files, added version information file
dnl     src/libsilo.settings or src/libsiloh5.settings, changed version
dnl     back to 4.6 (expect 5.0 after re-factoring of the driver code into
dnl     an abstract I/O layer.
dnl
dnl --------------------------------------------------------------------------

dnl
dnl Configuration header stuff...
dnl NOTE: Don't forget to change the version number here when we do a
dnl release!!!
dnl
m4_define(SILO_NAME, silo)
m4_define(SILO_VERSION, 4.6.0b)
m4_define(SILO_DATE, 2006/10/10)
m4_define(SILO_TIME, 13:34:00)
m4_define(SILO_DATETIME, [SILO_DATE SILO_TIME])
m4_define(SILO_BUGS,visit-help@llnl.gov)
m4_define(SILO_TOP_SRC_DIR, `pwd`)

AC_PREREQ(2.60)
AC_REVISION($Id$)dnl
AC_INIT(SILO_NAME, SILO_VERSION, SILO_BUGS, SILO_NAME)
AC_CONFIG_SRCDIR([src/silo/silo.h])
AC_CONFIG_AUX_DIR(config)
dnl AM_INIT_AUTOMAKE takes a list of options that should be applied to
dnl every Makefile.am when automake is run.
AM_INIT_AUTOMAKE([1.9 foreign no-installinfo no-installman no-texinfo.tex])

AC_CONFIG_HEADER([config.h:config/config.h.in])
dnl AM_MAINTAINER_MODE turns off "rebuild rules" that contain dependencies
dnl for Makefiles, configure, src/H5config.h, etc.  If AM_MAINTAINER_MODE
dnl is *not* included here, these files will be rebuilt if out of date.
dnl This is a problem because if users try to build on a machine with
dnl the wrong versions of autoconf and automake, these files will be
dnl rebuilt with the wrong versions and bad things can happen.
dnl Also, Clearcase doesn't preserve dependencies between timestamps
dnl when doing (group) checkin's, so extra care is needed to manually
dnl verify timestamps (see, FAQ) so that Makefiles doesn't think rebuilding 
dnl needs to occur when it doesn't.
dnl Developers should './configure --enable-maintainer-mode' to turn on
dnl rebuild rules.
AM_MAINTAINER_MODE
AC_COPYRIGHT([This work was produced at the University of California,
Lawrence Livermore National Laboratory (UC LLNL) under contract
no. W-7405-ENG-48 (Contract 48) between the U.S. Department of Energy
(DOE) and The Regents of the University of California (University) for
the operation of UC LLNL. The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
the DOE and University as allowed under DOE Acquisition Letter 97-1.])
# using autotest
AC_CONFIG_TESTDIR(tests)

dnl Change the default prefix (/usr/local) to `pwd`
dnl Set prefix default (install directory) to a directory in the build area.
dnl This allows multiple src-dir builds within one host.
AC_PREFIX_DEFAULT("SILO_TOP_SRC_DIR")
UsingSrcDir="SILO_TOP_SRC_DIR"
AC_SUBST([UsingSrcDir])

# Turn off shared libraries by default, since build process takes longer.
AC_DISABLE_SHARED

dnl Determine the host type
AC_CANONICAL_HOST

# Grab the version number
AC_MSG_CHECKING(what version the package is)
#VERSION=`cat VERSION`
#AC_DEFINE_UNQUOTED(VERSION,"$VERSION")
AC_MSG_RESULT(SILO_VERSION)

dnl Configuration date
AC_SUBST([CONFIG_DATE]) CONFIG_DATE="`date`"

dnl User doing the configuration
AC_SUBST([CONFIG_USER]) CONFIG_USER="`whoami`@`hostname`"
if test -n "$ORGANIZATION"; then
  CONFIG_USER="$CONFIG_USER at $ORGANIZATION"
fi

dnl Configuration command
AC_SUBST([CONFIG_CMD]) CONFIG_CMD="$*"

dnl ----------------------------------------------------------------------

dnl Source any special site-specific file
hname="`hostname`"
file=$srcdir/config-site/$hname.conf
AC_MSG_CHECKING([for site config host file])
if test -f "$file"; then
    . $file
    AC_MSG_RESULT([$file])
else
    hname="`hostname | cut -f2- -d.`"
    file=$srcdir/config-site/$hname.conf
    if test -f "$file"; then
        . $file
        AC_MSG_RESULT([$file])
    else
        AC_MSG_RESULT([no])
    fi
fi

dnl Source any special files that we need.  These files normally aren't
dnl present but can be used by the maintainers to fine tune things like
dnl turning on debug or profiling flags for the compiler.  The search order
dnl is:
dnl
dnl     CPU-VENDOR-OS
dnl     VENDOR-OS
dnl     CPU-OS
dnl     CPU-VENDOR
dnl     OS
dnl     VENDOR
dnl     CPU
dnl     uname
dnl
dnl If the `OS' ends with a version number then remove it. For instance,
dnl `freebsd3.1' would become `freebsd'
case $host_os in
  aix4.*)
    host_os_novers=aix4.x
    ;;
  aix5.*)
    host_os_novers=aix5.x
    ;;
  freebsd*)
    host_os_novers=freebsd
    ;;
  irix5.*)
    host_os_novers=irix5.x
    ;;
  irix6.*)
    host_os_novers=irix6.x
    ;;
  osf4.*)
    host_os_novers=osf4.x
    ;;
  osf5.*)
    host_os_novers=osf5.x
    ;;
  solaris2.*)
    host_os_novers=solaris2.x
    ;;
  *)
    host_os_novers=$host_os
    ;;
esac

uname="`uname`"
host_config="none"
for f in $host_cpu-$host_vendor-$host_os \
         $host_cpu-$host_vendor-$host_os_novers \
         $host_vendor-$host_os \
         $host_vendor-$host_os_novers \
         $host_cpu-$host_os \
         $host_cpu-$host_os_novers \
         $host_cpu-$host_vendor \
         $host_os \
         $host_os_novers \
         $host_vendor \
         $host_cpu \
         $uname; do
  AC_MSG_CHECKING([for config-site $f])
  if test -f "$srcdir/config-site/$f.conf"; then
    host_config=$srcdir/config-site/$f.conf
    AC_MSG_RESULT([found])
    break
  fi
  AC_MSG_RESULT([no])
done
if test "X$host_config" != "Xnone"; then
  . $host_config
fi

# This F77 stuff is still embeaded in libtools, it should go away
# If F77 is set, and FC isn't, then set FC to whatever F77 was set to.
if test "${F77+set}" = set -a "${FC+set}" != set; then
  FC=$F77
  if test "${FFLAGS+set}" = set -a "${FCFLAGSset}" != set; then
    FCFLAGS=$FFLAGS
  fi
fi
if test "${FC+set}" = set -a "${F77set}" != set; then
  F77=$FC
  if test "${FCFLAGS+set}" = set -a "${FFLAGSset}" != set; then
    FFLAGS=$FCFLAGS
  fi
fi

dnl C optimization flags
AC_ARG_ENABLE(optimization,
AC_HELP_STRING([--enable-optimization], 
  [compile with optimization @<:@default=no@:>@]),
[case "${enableval}" in
  yes) USING_DEBUG="no";;
  no)  USING_DEBUG="yes";;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-optimization) ;;
esac],[USING_DEBUG="yes"])

# Check for the netcdf driver.
AC_MSG_CHECKING(for silo's netcdf driver)
if test -d $srcdir/src/netcdf; then
    AC_MSG_RESULT(found)
    AC_DEFINE(HAVE_NETCDF_DRIVER,1,[Support for NetCDF])
    NETCDF="netcdf"
else
    AC_MSG_RESULT(not found - SILO will not have NETCDF)
    NETCDF=""
fi
AC_SUBST(NETCDF)

# Check for the pdb driver.
AC_MSG_CHECKING(for silo's pdb driver)
if test -d $srcdir/src/pdb_drv; then
    AC_MSG_RESULT(found)
    AC_DEFINE(HAVE_PDB_DRIVER,1,[Support for PDB])
    PDB_DRV="pdb_drv"
else
    AC_MSG_RESULT(not found - SILO will not have PDB)
    PDB_DRV=""
fi
AC_SUBST(PDB_DRV)

# Check for the taurus driver.
AC_MSG_CHECKING(for silo's taurus driver)
if test -d $srcdir/src/taurus; then
    AC_MSG_RESULT(found)
    AC_DEFINE(HAVE_TAURUS_DRIVER,1,[Support for Taurus])
    TAURUS="taurus"
else
    AC_MSG_RESULT(not found - SILO will not have TAURUS)
    TAURUS=""
fi
AC_SUBST(TAURUS)

dnl
dnl Check for the hdf5 driver.
dnl
AC_MSG_CHECKING(for silo's hdf5 driver)
if test -d $srcdir/src/hdf5_drv; then
    AC_MSG_RESULT(found)
    HDF5_DRV="hdf5_drv"
else
    AC_MSG_RESULT(not found - SILO will not have HDF5)
    HDF5_DRV=""
fi
AC_SUBST(HDF5_DRV)

# Some sites have the GNU readline header files and libraries installed
# incorrectly which causes programs that use them to not link or possibly
# not run.  This configure doesn't check for misconfigured systems, but
# it does allow the user to prevent the GNU readline files from being
# detected.
AC_ARG_WITH(readline,
	AC_HELP_STRING([--with-readline],
            [enable detection of GNU readline @<:@default=yes@:>@]),
	detect_readline=$withval,
	detect_readline=yes)

dnl
dnl Check if we should build python module.
dnl
AC_MSG_CHECKING(if we should build python module)
AC_ARG_ENABLE(pythonmodule,
        AC_HELP_STRING([--enable-pythonmodule],
            [build python module @<:@default=no@:>@]),
        BuildPythonmodule="$enable_pythonmodule",
        BuildPythonmodule=no)
if test "$BuildPythonmodule" = "yes"; then
    AC_MSG_RESULT(yes)
    PYTHONMODULE=python
else
    AC_MSG_RESULT(no)
    PYTHONMODULE=""
fi
AC_SUBST(PYTHONMODULE)

dnl
dnl Check if we should build browser.
dnl
AC_MSG_CHECKING(if we should build silo's command line browser)
AC_ARG_ENABLE(browser,
        AC_HELP_STRING([--enable-browser],
            [build browser @<:@default=yes@:>@]),
        BuildBrowser="$enable_browser",
        BuildBrowser=yes)
if test "X$BuildBrowser" = "Xyes"; then
    AC_MSG_RESULT(yes)
    BROWSER="browser"
    UsingBrowser="yes"
else
    AC_MSG_RESULT(no)
    BROWSER=""
    UsingBrowser="no"
    detect_readline=no
fi
AC_SUBST(BROWSER)
AC_SUBST(UsingBrowser)


dnl enable the silex broswer
AC_MSG_CHECKING(if we should build silo's silex GUI browser)
AC_ARG_ENABLE(silex,
AC_HELP_STRING([--enable-silex], 
  [build the silex browser @<:@default=no@:>@]),
              UseSilex="$enable_silex", UseSilex=no)
if test "X$UseSilex" = "Xyes"; then
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi

dnl
dnl Check if we should use the fortran compiler.
dnl
AC_MSG_CHECKING(if we want Fortran)
AC_ARG_ENABLE(fortran,
        AC_HELP_STRING([--enable-fortran],
            [compile Fortran examples @<:@default=no@:>@]),
        UsingFortran="$enable_fortran",
        UsingFortran=no)
if test "$UsingFortran" = "yes"; then
    AC_MSG_RESULT(yes)
    USEFORTRAN="yes"
else
    AC_MSG_RESULT(no)
    USEFORTRAN=""
fi
AC_SUBST(USEFORTRAN)

# Check for programs.
# We shouldn't depend on too many!
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
if test -n "$USEFORTRAN"; then
    AC_PROG_FC
# this too
    AC_PROG_F77
# Fortran portability
    AC_FC_LIBRARY_LDFLAGS
    AC_FC_WRAPPERS
fi
AC_SUBST(UsingFortran)
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_ISC_POSIX
# save the cache so far
AC_CACHE_SAVE

# Obsolute
# Add an option for disabling the stat checks.
##AC_MSG_CHECKING(if we should disable filesystem checks)
##AC_ARG_ENABLE(filesystem-checks,
##        AC_HELP_STRING([--disable-filesystem-checks],
##            [build without filesystem checks]),
##        UseStat="$enable_filesystem_checks", 
##        UseStat=yes)
##if test "$UseStat" = "no"; then
##    AC_MSG_RESULT(yes)
##    AC_DEFINE(DISABLE_FILESYSTEM_CHECKS,1,[Disable stat checks])
##else
##    AC_MSG_RESULT(no)
##fi

# Turn on C optimization flags
AC_MSG_CHECKING(compiler optimization)
if test "X$USING_DEBUG" = Xno; then
# use contributed GNU m4 macro library provided see:
#   autoconf-archive-2006-06-04/htmldoc/ax_cc_maxopt.html
   AX_CC_MAXOPT
   UsingOpt="yes"
   AC_MSG_RESULT(enabled)
else
   UsingOpt="no"
   AC_MSG_RESULT(disabled)
fi
AC_SUBST(UsingOpt)

dnl
# Default to large file support
AX_CHECK_COMPILER_FLAGS("-D_LARGEFILE64_SOURCE",CFLAGS="$CFLAGS -D_LARGEFILE64_SOURCE"; Using64bits=yes, Using64bits=no)
AC_SUBST(Using64bits)
#
# Use size of off64_t to determine which form of stat() calls to make 
#
AC_CHECK_SIZEOF(off64_t)

# Check to see if we're running clearmake.
# If we are, set the build verbosity to 0.  Also, set the
# compilation rule to include the OS, so that we won't wink in object
# files across OSes.
#
dnl Note that the first character in the OS_RULE is a tab.  Don't change this 
dnl to spaces; it's used in the Makefiles.
AC_MSG_CHECKING(for clearmake)
if test "`${MAKE-make} -version 2>/dev/null | cut -d' ' -f1`" = "clearmake"; then
    AC_MSG_RESULT(yes)
    CLEARCASE_BLD_VERBOSITY=0; export CLEARCASE_BLD_VERBOSITY
    OS_RULE="	: $OS_type"
else
    AC_MSG_RESULT(no)
fi
AC_SUBST(OS_RULE)

dnl We always use the install-sh script, so don't uncomment the following line.
AC_PROG_INSTALL

dnl
dnl Checks for header files.
dnl
AC_HEADER_STDC
AC_HEADER_TIME

dnl
dnl If we don't have these header files, we can't continue.
dnl
AC_CHECK_HEADERS(fcntl.h limits.h sys/time.h sys/types.h unistd.h stdarg.h, ,
AC_MSG_ERROR([$0 wasn't able to find a necessary header file
(see above).  Please rectify this and rerun configure.
See the file INSTALL_FAQ in this directory for possible reasons
this might have happened.]))

dnl
dnl Check for header files that we can do without.
dnl Only needed by browser
AC_CHECK_HEADERS(fnmatch.h)
dnl Only needed by silock
AC_CHECK_HEADERS(ieeefp.h)

dnl
dnl Check for various functions in the STDIO header files.
dnl
dnl Note that we are calling two "changequote" macros.  This is because the
dnl default quoting mechanism is to use `[' and `]'.  Since we have to use
dnl these in the "tr" call, we need to change them to something else
dnl temporarily.
dnl
dnl The following `if false' statement is *only* so autoheader can pick up
dnl the names of the constants and insert them into the config.h.in file. If
dnl the AC_DEFINE statements are removed then autoheader never sees the
dnl constants.
if false; then
    AC_DEFINE(HAVE_FCLOSE_POINTER,1,[System provides fclose prototypes])
    AC_DEFINE(HAVE_FFLUSH_POINTER,1,[System provides fflush prototypes])
    AC_DEFINE(HAVE_FOPEN_POINTER,1,[System provides fopen prototypes])
    AC_DEFINE(HAVE_FPRINTF_POINTER,1,[System provides fprintf prototypes])
    AC_DEFINE(HAVE_FREAD_POINTER,1,[System provides fread prototypes])
    AC_DEFINE(HAVE_FSEEK_POINTER,1,[System provides fseek prototypes])
    AC_DEFINE(HAVE_SETVBUF_POINTER,1,[System provides setvbuf prototypes])
    AC_DEFINE(HAVE_FTELL_POINTER,1,[System provides ftell prototypes])
    AC_DEFINE(HAVE_FWRITE_POINTER,1,[System provides fwrite prototypes])
fi

for function in fclose fflush fopen fprintf fread fseek setvbuf ftell fwrite
do
    changequote(<<,>>)
    cap_func="`echo ${function}|tr [a-z] [A-Z]`"
    changequote([,])
    cache_name="`echo vc_cv_${function}_pointer`"
    have_name="`echo HAVE_${cap_func}_POINTER`"
    AC_MSG_CHECKING(for $function function pointer)
    AC_CACHE_VAL($cache_name,
        AC_TRY_COMPILE([#include <stdio.h>],
        [ int (*f)() = (int(*)())$function; ],
            [eval $cache_name=yes ],
            [eval $cache_name=no ]
        )
    )
    if eval "test \"`echo '$''{'$cache_name'}'`\" = no"; then
        AC_MSG_RESULT(no, using a replacement.)
    else
        AC_DEFINE_UNQUOTED(${have_name},1,[System provides prototype])
        AC_MSG_RESULT(yes)
    fi
done

dnl
dnl Check for library functions that can work around, or that we have
dnl replacements for.
dnl
AC_CHECK_FUNCS(memmove fnmatch isnan fpclass strerror)

dnl
dnl On Paragon/TeraFLOP systems there are "buggy" versions of
dnl `setjmp' and `longjmp' that print an error message to use
dnl `_setjmp' and `_longjmp' instead and exit with a non-zero
dnl status.
dnl
AC_MSG_CHECKING(if setjmp and longjmp work)
if test -n "$SETJMP_OVERRIDE"; then
   AC_MSG_RESULT(no.  Replacing with _setjmp and _longjmp.)
   AC_DEFINE(setjmp,_setjmp,[Override setjmp])
   AC_DEFINE(longjmp,_longjmp,[Override longjmp])
else
   AC_MSG_RESULT(yes)
fi

dnl
dnl save the cache
dnl
AC_CACHE_SAVE

dnl the hdf5 library checks (to follow) will require the -lm if its required
AC_CHECK_LIBM
LIBS="$LIBS $LIBM"


dnl
dnl We only need to check for -ldl if we're on SunOS.
dnl
AC_MSG_CHECKING(for Linux/SunOS and -ldl)
if test "`uname 2>/dev/null`" = "Linux" || test "`uname 2>/dev/null`" = "SunOS"; then
    AC_CHECK_LIB(dl, main,   LIBS="-ldl $LIBS")
else
    AC_MSG_RESULT(no)
fi
case "${host}" in
  hppa*-hp-hpux*)
   AC_CHECK_LIB(nsl, main, LIBS="-lnsl $LIBS")
   ;;
  sparc-sun-solaris2*)
   AC_CHECK_LIB(dl, main,   LIBS="-ldl $LIBS")
   AC_CHECK_LIB(socket, main, LIBS="-lsocket $LIBS")
   AC_CHECK_LIB(nsl, main, LIBS="-lnsl $LIBS")
   ;;
  *-*-linux-gnu)
   AC_CHECK_LIB(dl, main,   LIBS="-ldl $LIBS")
   ;;
  *)
   ;;
esac

# Is HDF5 present (it's needed for the silo/hdf5 driver for pmesh/alec SAMI
# files larger than 2GB until SAF is in place)?  Normally check for
# hdf5.h and libhdf5.a but skip the test if `--without-hdf5' is specified.
# Alternate directories can be specified for hdf5.h and/or libhdf5.a by giving
# the names like this `--with-hdf5=INC,LIB' (if only the LIB directory is
# given then it must still be preceded by a comma).
#
# Checking for hdf5 is complicated by the fact that hdf5 may or may not
# in turn depend on zlib compression lib. In and of itself, Silo does NOT
# depend on zlib. So, we don't want -lz on the link line if zlib is either
# not needed or, worse, not present as this generates warning messages
# or fatal errors. So, we first test for hdf5 without -lz on the link line.
# If this succeeds, we conclude hdf5 is present and we can build the hdf5
# driver. If hdf5 fails, it may be that hdf5 was compiled with zlib. So, next
# we make an independent test for zlib. If zlib fails, we conclude that the
# failure on the hdf5 test was due to something other than lack of zlib and
# decide libhdf5.a is NOT present and we cannot build the hdf5 driver. If
# zlib succeeds, we re-try the hdf5 test with -lz on the link line. If that
# succeeds, we conclude we need -lz on the link line and we can build the
# hdf5 driver. Again, if it fails, we neither put -lz on the link line nor
# build the hdf5 driver. One other possibility that could be tested for
# and configure could make a reasonable comment on is to see if only the
# binaries for hdf5 were installed AND they have zlib dependences which
# are NOT resolvable on the platform they are installed on. This would be
# a boneheaded thing to do, but nonetheless something that configure could
# inform the user has gone wrong and what to do.
#

UsingHDF5="no"
if test -n "$HDF5_DRV"; then
   saved_CPPFLAGS="$CPPFLAGS"
   saved_LDFLAGS="$LDFLAGS"
   saved_LIBS="$LIBS"
   AC_ARG_WITH(hdf5,
           [  --with-hdf5=INC,LIB     Location of HDF5 header and library],
	   ,
	   withval="no")
   if test "$withval" = "no" ; then
      AC_MSG_CHECKING(for hdf5)
      AC_MSG_RESULT(suppressed)
      HDF5_DRV=""
   else
      AC_CHECKING(for hdf5 and supporting libraries)
      hdf5_inc="`echo $withval |cut -f1 -d,`"
      if test "$withval" != "yes" && test -n "$hdf5_inc"; then
	 hdf5_explicit=true
         if test -d "$hdf5_inc"; then
            CPPFLAGS="-I$hdf5_inc $CPPFLAGS"
         else
            AC_MSG_ERROR(problem with directory specified for hdf5 includes)
         fi
      fi
      hdf5_lib="`echo $withval |cut -f2 -d, -s`"
      if test "$withval" != "yes" && test -n "$hdf5_lib"; then
	 hdf5_explicit=true
         if test -d "$hdf5_lib"; then
            LDFLAGS="-L$hdf5_lib $LDFLAGS"
         else
            AC_MSG_ERROR(problem with directory specified for hdf5 library)
         fi
      fi
      notfound=""
      AC_CHECK_HEADERS(hdf5.h,,notfound="hdf5.h")
      tmp_LIBS="$LIBS"
###   LIBS="$hdf5_lib/libhdf5.a $LIBS"
      LIBS="-lhdf5 $LIBS"
      AC_CHECK_FUNC(H5open,[AC_DEFINE(HAVE_LIBHDF5,1,[using HDF5])],notfound="$notfound libhdf5.a")
      if test -n "$notfound"; then
         LIBS="$tmp_LIBS"
      fi
      #
      # If the above tests did NOT find headers and lib for
      # hdf5, it could be due to zlib, so now test for zlib
      #
      if test -n "$notfound"; then
         # reset the compile and link flags
         CPPFLAGS="$saved_CPPFLAGS"
         LDFLAGS="$saved_LDFLAGS"
         AC_CHECKING(if lack of zlib caused tests for hdf5 to fail) 
         AC_ARG_WITH(zlib,
                 [  --with-zlib=INC,LIB     Use the GNU zlib compression],
                     ,withval=yes)
         if test "no" = "$withval"; then
            AC_MSG_CHECKING(for zlib)
            AC_MSG_RESULT(suppressed)
         else
            zlib_inc="`echo $withval |cut -f1 -d,`"
            if test "$withval" != "yes" && test -n "$zlib_inc"; then
               if test -d "$zlib_inc"; then
                  CPPFLAGS="-I$zlib_inc $CPPFLAGS"
               else
                  AC_MSG_ERROR(problem with directory specified for zlib includes)
               fi
            fi
            zlib_lib="`echo $withval |cut -f2 -d, -s`"
            if test "$withval" != "yes" && test -n "$zlib_lib"; then
               if test -d "$zlib_lib"; then
                  LDFLAGS="-L$zlib_lib $LDFLAGS"
               else
                  AC_MSG_ERROR(problem with directory specified for zlib library)
               fi
            fi
	    notfound=""
            AC_CHECK_HEADERS(zlib.h,,notfound="zlib.h")
            AC_CHECK_LIB(z,compress2,,notfound="$notfound libz.a")
         fi
         #
         # If libz was not found, the failure of hdf5 must be something else.
         #
         if test -n "$notfound"; then
            CPPFLAGS="$saved_CPPFLAGS"
            LDFLAGS="$saved_LDFLAGS"
            HDF5_DRV=""
	    if test -n "$hdf5_explicit"; then 
               AC_MSG_ERROR([
You have made an explicit request for hdf5 with the --with-hdf option
and $0 is unable to find hdf5 and its supporting libs, if any.
Please see INSTALL_FAQ for possible causes.])
   	    else
	       AC_MSG_WARN(hdf5 driver disabled)
	    fi
         else
            # we unset these to defeat autoconf's cache
            unset ac_cv_header_hdf5_h
            unset ac_cv_lib_hdf5_H5open
            #
            # Now, retest for hdf5 with zlib
            #
            if test -n "$hdf5_inc"; then
               CPPFLAGS="-I$hdf5_inc $CPPFLAGS"
            fi
            if test -n "$hdf5_lib"; then
               LDFLAGS="-L$hdf5_lib $LDFLAGS"
            fi
	    notfound=""
            AC_CHECK_HEADERS(hdf5.h,,notfound="hdf5.h")
            tmp_LIBS="$LIBS"
            AC_CHECK_LIB(hdf5,H5open,,notfound="$notfound libhdf5.a")
            if test "$notfound" = ""; then
###            LIBS="$hdf5_lib/libhdf5.a $tmp_LIBS"
               LIBS="-lhdf5 $tmp_LIBS"
            fi
            if test -n "$notfound"; then
               CPPFLAGS="$saved_CPPFLAGS"
               LDFLAGS="$saved_LDFLAGS"
               HDF5_DRV=""
	       if test -n "$hdf5_explicit"; then 
                  AC_MSG_ERROR([
You have made an explicit request for hdf5 with the --with-hdf option
and $0 is unable to find hdf5 and its supporting libs, if any.
Please see INSTALL_FAQ for possible causes.])
	       else
                  AC_MSG_WARN(hdf5 driver disabled)
	       fi
            fi
         fi # if test -n "$notfound" (for second hdf5 test)
      fi # if test -n "$notfound" (for zlib)
   fi # if test "no" = "$withval"
fi # if test -n "$HDF5_DRV"

# Check to see if the results of the above tests found the HDF5 libraries
if test -n "$HDF5_DRV"; then
    UsingHDF5="yes"
    AC_DEFINE(HAVE_HDF5_DRIVER,1,[Support for HDF5])
else
   LIBS="$saved_LIBS"
fi
AC_SUBST(UsingHDF5)

dnl
UsingReadline="no"
if test "X$detect_readline" = Xyes; then
# use contributed GNU m4 macro library provided see:
#   autoconf-archive-2006-06-04/htmldoc/vl_lib_readline.html
   VL_LIB_READLINE
   UsingReadline="yes"
else
   AC_MSG_CHECKING(for readline)
   AC_MSG_RESULT(disabled)
fi
AC_SUBST(UsingReadline)

dnl
dnl If Silex is desired then Qt is required
dnl
UsingSilex="no"
if test "X$UseSilex" = "Xyes"; then
dnl search for the required Qt
   AC_MSG_CHECKING(for Qt)
#   autoconf-archive-2006-08-20/htmldoc/bnv_have_qt.html
    BNV_HAVE_QT
    if test "X$have_qt" = "Xyes"; then
        AC_MSG_RESULT(Qt found - Silex enabled)
        UsingSilex="yes"
        SILEX="silex"
    else
        AC_MSG_RESULT(Qt not found - Silex disabled)
        SILEX=""
    fi
fi
AC_SUBST(UsingSilex)
AC_SUBST(SILEX)

dnl
dnl Check for a python installation using the Automake 
dnl provided macro, which will generate the following
dnl variables: python_PYTHON, pkgpython_PYTHON, 
dnl pyexecdir_PYTHON, and pkgpyexecdir_PYTHON.
dnl
UsingPython="no"
UsingCxx="no"
if test -n "$PYTHONMODULE"; then
   AM_PATH_PYTHON(2.1,, :)
# Check for Python include path
#
   AC_MSG_CHECKING([for Python include path])
   if test -z "$PYTHON_CPPFLAGS"; then
      python_path=`$PYTHON -c "import distutils.sysconfig; \
         print distutils.sysconfig.get_python_inc();"`
      if test -n "${python_path}"; then
         python_path="-I$python_path"
      fi
      PYTHON_CPPFLAGS=$python_path
   fi
   UsingPython="yes"
   UsingCxx="yes"
   AC_MSG_RESULT([$PYTHON_CPPFLAGS])
   AC_SUBST([PYTHON_CPPFLAGS])
fi
AC_SUBST(UsingPython)
AC_SUBST(UsingCxx)

UsingShared="no"
if test "$enable_shared" = yes; then
   UsingShared="yes"
fi
AC_SUBST(UsingShared)
UsingStatic="no"
if test "$enable_static" = yes; then
   UsingStatic="yes"
fi
AC_SUBST(UsingStatic)

dnl Determine how to compile MPI
# skip for now
#ACX_MPI

AC_CACHE_SAVE

dnl
AM_CONDITIONAL(NETCDF_NEEDED, test -n "$NETCDF")
AM_CONDITIONAL(PDB_DRV_NEEDED, test -n "$PDB_DRV")
AM_CONDITIONAL(TAURUS_NEEDED, test -n "$TAURUS")
AM_CONDITIONAL(HDF5_DRV_NEEDED, test -n "$HDF5_DRV")
AM_CONDITIONAL(BROWSER_NEEDED, test -n "$BROWSER")
AM_CONDITIONAL(PYTHON_NEEDED, test -n "$PYTHONMODULE")
AM_CONDITIONAL(FORTRAN_NEEDED, test -n "$USEFORTRAN")
AM_CONDITIONAL(SILEX_NEEDED, test -n "$SILEX")
AC_CONFIG_FILES([
            Makefile
            clearcase_bin/Makefile
            config/Makefile
            config-site/Makefile
            docs/Makefile
            src/Makefile
            src/score/Makefile
            src/pdb/Makefile
            src/swat/Makefile
	    src/silo/Makefile
            src/debug/Makefile
            src/netcdf/Makefile
            src/pdb_drv/Makefile
            src/hdf5_drv/Makefile
            src/taurus/Makefile
            src/unknown/Makefile
            src/filters/Makefile
            tests/Makefile
            tools/Makefile
            tools/browser/Makefile
            tools/python/Makefile
            tools/silock/Makefile])
if test -n "$HDF5_DRV"; then
  AC_CONFIG_FILES([src/libsiloh5.settings])
else
  AC_CONFIG_FILES([src/libsilo.settings])
fi
AC_OUTPUT
dnl ----------------------------------------------------------------------
dnl Print out a summary of what we are going to build.
dnl   
echo;echo
if test -n "$HDF5_DRV"; then
  cat src/libsiloh5.settings | sed 's|\\n||' | sed 's|"||g'
else
  cat src/libsilo.settings | sed 's|\\n||' | sed 's|"||g'
fi
