dnl Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
dnl LLNL-CODE-425250.
dnl All rights reserved.
dnl 
dnl This file is part of Silo. For details, see silo.llnl.gov.
dnl 
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions
dnl are met:
dnl 
dnl    * Redistributions of source code must retain the above copyright
dnl      notice, this list of conditions and the disclaimer below.
dnl    * Redistributions in binary form must reproduce the above copyright
dnl      notice, this list of conditions and the disclaimer (as noted
dnl      below) in the documentation and/or other materials provided with
dnl      the distribution.
dnl    * Neither the name of the LLNS/LLNL nor the names of its
dnl      contributors may be used to endorse or promote products derived
dnl      from this software without specific prior written permission.
dnl 
dnl THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
dnl "AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
dnl LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
dnl A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
dnl LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
dnl CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
dnl EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
dnl PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
dnl PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
dnl LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
dnl NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
dnl SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
dnl 
dnl This work was produced at Lawrence Livermore National Laboratory under
dnl Contract No.  DE-AC52-07NA27344 with the DOE.
dnl 
dnl Neither the  United States Government nor  Lawrence Livermore National
dnl Security, LLC nor any of  their employees, makes any warranty, express
dnl or  implied,  or  assumes  any  liability or  responsibility  for  the
dnl accuracy, completeness,  or usefulness of  any information, apparatus,
dnl product, or  process disclosed, or  represents that its use  would not
dnl infringe privately-owned rights.
dnl 
dnl Any reference herein to  any specific commercial products, process, or
dnl services by trade name,  trademark, manufacturer or otherwise does not
dnl necessarily  constitute or imply  its endorsement,  recommendation, or
dnl favoring  by  the  United  States  Government  or  Lawrence  Livermore
dnl National Security,  LLC. The views  and opinions of  authors expressed
dnl herein do not necessarily state  or reflect those of the United States
dnl Government or Lawrence Livermore National Security, LLC, and shall not
dnl
dnl This is the autoconf configuration file for MeshTV and company.
dnl Robb Matzke <matzke@llnl.gov>
dnl Mar 18, 1996
dnl
dnl Modifications:
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
dnl     Mark C. Miller, Wed Oct 18 17:36:59 PDT 2006
dnl     Added AC_DEFINE_UNQUOTED calls to create compile-time symbols for
dnl     Silo's version information. Added code at end to put this into
dnl     a separate 'config_vers.h' file
dnl
dnl     Thomas R. Treadway, Thu Mar 15 12:19:56 PDT 2007
dnl     Added hooks for building Macintosh application bundle
dnl     Added szlib support
dnl
dnl     Thomas R. Treadway, Mon Jul 23 10:32:06 PDT 2007
dnl     Added support for Peter Lindstrom's compression algorthims.
dnl
dnl     Thomas R. Treadway, Tue Oct 23 11:47:22 PDT 2007
dnl     Corrected HDF5 checking when szip present.
dnl     New pre-release.
dnl
dnl     Thomas R. Treadway, Thu Oct 25 16:32:40 PDT 2007
dnl     Using environment variable to over SZLIB, HDF5, SILEX, QT, 
dnl     Fortran, and readline.
dnl
dnl     Thomas R. Treadway, Thu Oct 25 16:32:40 PDT 2007
dnl     Changed release, and auspices statement
dnl
dnl     Thomas R. Treadway, Wed Nov 28 14:17:56 PST 2007
dnl     Changed release to 4.6.1
dnl     Added check for sys/stat.h
dnl     Removed src/swat directory
dnl
dnl     Mark C. Miller, Wed Jul  2 09:08:13 PDT 2008
dnl     Isolated version information to VERSION file, though configure
dnl     still needs to get re-generated with each new version.
dnl     Removed slew of extraneous refs to 'UsingXXX' variables.
dnl
dnl     Mark C. Miller, Mon Mar 30 13:29:57 PDT 2009
dnl     Fixed uname="`uname`" to UNAME="`uname`" so Darwin check would work.
dnl
dnl     Mark C. Miller, Wed Sep  2 15:21:52 PDT 2009
dnl     Making it work with '--with-hdf5' where HDF5 locations are specified
dnl     in the config-site file. Fixed problem where READLINE_LIBS was
dnl     getting populated with ALL libs not just those necessary to support
dnl     readline. Fixed problem where browser would REQUIRE readline. Still
dnl     can't handle case where user wants to specify different readline
dnl     include/lib dirs.
dnl
dnl     Mark C. Miller, Sat Dec  5 15:20:30 PST 2009
dnl     Added logic to inspect size of long and long long datatypes and, if
dnl     they are different sizes, ensuring Silo's DB_LONG_LONG enum value
dnl     is unique. Otherwise, ensuring it is the same as DB_LONG enum value.
dnl
dnl     Mark C. Miller, Wed Jul 21 09:31:24 PDT 2010
dnl     Adjusted logic to support BSD releases which will NOT include
dnl     hzip/fpzip compression features or source dirs.
dnl --------------------------------------------------------------------------

dnl
dnl The esyscmd macro invokation allows us to keep version number in
dnl VERSION file and just re-generate configure instead of having to
dnl also checkout and edit configure.ac
dnl
m4_define(SILO_VERSION, m4_esyscmd([cat VERSION | tr -d '\n']))
m4_define(SILO_NAME, silo)
m4_define(SILO_BUGS,visit-users@elist.ornl.gov)
m4_define(SILO_TOP_SRC_DIR, `pwd`)

AC_PREREQ(2.60)
AC_REVISION($Id$)dnl
AC_INIT(SILO_NAME, SILO_VERSION, SILO_BUGS, SILO_NAME)
AC_CONFIG_SRCDIR([src/silo/silo.h.in])
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
AC_COPYRIGHT([This work was performed under the auspices of the 
U.S. Department of Energy by Lawrence Livermore National Laboratory 
in part under Contract W-7405-Eng-48 and in part under 
Contract DE-AC52-07NA27344.])
# using autotest
AC_CONFIG_TESTDIR(tests)

dnl Change the default prefix (/usr/local) to `pwd`
dnl Set prefix default (install directory) to a directory in the build area.
dnl This allows multiple src-dir builds within one host.
AC_PREFIX_DEFAULT("SILO_TOP_SRC_DIR")

dnl
dnl Handle the python module right away to determine if we need shared libs.
dnl Ordinarily, we default to static libs
dnl
PYTHONMODULE=$DEFAULT_PYTHONMODULE
AC_ARG_ENABLE(pythonmodule,
    AS_HELP_STRING([--enable-pythonmodule],[build python module @<:@default=no@:>@]),
    if test "$enable_pythonmodule" = "yes"; then
        PYTHONMODULE="python"
    fi)
AC_SUBST(PYTHONMODULE)
if test -n "$PYTHONMODULE"; then
    AC_ENABLE_SHARED
else
    AC_DISABLE_SHARED
fi

dnl Determine the host type
AC_CANONICAL_HOST

# Grab the version number
AC_MSG_CHECKING(what version the package is)
AC_MSG_RESULT(SILO_VERSION)
AC_SUBST([SILO_VERS_MAJ]) SILO_VERS_MAJ="`echo SILO_VERSION | cut -d'.' -f1`"
AC_SUBST([SILO_VERS_MIN]) SILO_VERS_MIN="`echo SILO_VERSION | cut -d'.' -f2 | cut -d'-' -f1`"
AC_SUBST([SILO_VERS_PAT]) SILO_VERS_PAT="`echo SILO_VERSION | cut -d'.' -f3 | cut -d'-' -f1`"
AC_SUBST([SILO_VERS_PRE]) SILO_VERS_PRE="`echo SILO_VERSION | grep pre | cut -d'-' -f2 | cut -d'e' -f2`"
AC_SUBST([SILO_VERS_TAG]) SILO_VERS_TAG="`echo Silo_version SILO_VERSION | sed -e 's/ /_/' -e 's/\./_/g' -e 's/-/_/g'`"

dnl Configuration command invocation
AC_SUBST([CONFIG_CMD]) CONFIG_CMD="`echo $0 $ac_configure_args`"

dnl Configuration date
AC_SUBST([CONFIG_DATE]) CONFIG_DATE="`date`"

dnl User doing the configuration
AC_SUBST([CONFIG_USER]) CONFIG_USER="`whoami`@`hostname`"
if test -n "$ORGANIZATION"; then
  CONFIG_USER="$CONFIG_USER at $ORGANIZATION"
fi


dnl ----------------------------------------------------------------------

dnl
dnl Specify default settings for various options. These may be overridden
dnl in site-specific config files. Empty string means feature is disabled
dnl while non-empty string means it is enabled.
dnl
DEFAULT_PYTHONMODULE=""
DEFAULT_BROWSER="browser"
DEFAULT_SILEX=""
DEFAULT_HZIP="hzip"
DEFAULT_FPZIP="fpzip"
DEFAULT_FORTRAN="fortran"
DEFAULT_JSON=""
default_detect_readline="yes"

dnl Source any special site-specific file
hname="`hostname`"
file=$srcdir/config-site/$hname.conf
AC_MSG_CHECKING([for site config host file])
if test -f "$file"; then
    . $file
    AC_MSG_RESULT(found [$file])
else
    hname="`hostname | cut -f1 -d.`"
    file=$srcdir/config-site/$hname.conf
    if test -f "$file"; then
        . $file
        AC_MSG_RESULT(found [$file])
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

UNAME="`uname`"
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
         $UNAME; do
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

#
# MCM -- 22Jul08. I don't know why we're mixing FC and F77 stuf here.
# According to autoconf web pages, we should probably use one or the
# other but not both. Also, given that silo_f.h uses FC_FUNC macro,
# I don't see how we can get by using F77. So, it is removed and we're
# using only FC.
#

dnl handle fortran compiler.
FORTRAN=$DEFAULT_FORTRAN
AC_ARG_ENABLE(fortran,
    AC_HELP_STRING([--enable-fortran],
        [compile Fortran interface to Silo @<:@default=yes@:>@]),
    if test "$enable_fortran" = "no" ; then
        FORTRAN=""
    fi)
AC_SUBST(FORTRAN)

# Check for programs.
# We shouldn't depend on too many!
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
if test -n "$FORTRAN"; then
    AC_PROG_FC
    AC_PROG_F77
    AC_FC_LIBRARY_LDFLAGS
    AC_FC_WRAPPERS
fi
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_ISC_POSIX
# save the cache so far
AC_CACHE_SAVE

dnl C optimization flags
AC_ARG_ENABLE(optimization,
    AC_HELP_STRING([--enable-optimization], 
        [compile with optimization @<:@default=no@:>@]),
    [case "${enableval}" in
        yes) USING_DEBUG="no";;
        no)  USING_DEBUG="yes";;
        *) AC_MSG_ERROR(bad value ${enableval} for --enable-optimization) ;;
    esac],
    [USING_DEBUG="yes"])

dnl Switch to disable score lite memory header optimizations
AC_ARG_ENABLE(normal-sclite-mem-headers,
    AC_HELP_STRING([--enable-normal-sclite-mem-headers],
        [compile with normal score lite memory headers @<:@default=no@:>@]),
    if test "$enableval" = "yes" ; then
        AC_DEFINE(NORMAL_SCLITE_HEADERS,1,[SCORE lite memory header optimizations])
    fi,
    AC_DEFINE(NORMAL_SCLITE_HEADERS,0,[SCORE lite memory header optimizations]))

dnl Switch to have 'make install' install score/pdb lite header files
AC_ARG_ENABLE(install-lite-headers,
    AC_HELP_STRING([--enable-install-lite-headers],
        [Install SCORE/PDB lite header files @<:@default=no@:>@]),
    INSTLITE="$enableval")

dnl we don't need to check for netcdf driver. It is always there.
NETCDF="netcdf"
AC_DEFINE(HAVE_NETCDF_DRIVER,1,[Support for NetCDF])
AC_SUBST(NETCDF)

dnl we don't need to check for pdb driver. It is always there. 
PDB_DRV="pdb_drv"
AC_DEFINE(HAVE_PDB_DRIVER,1,[Support for PDB])
AC_SUBST(PDB_DRV)

dnl we don't need to check for taurus driver. It is always there.
TAURUS="taurus"
AC_DEFINE(HAVE_TAURUS_DRIVER,1,[Support for Taurus])
AC_SUBST(TAURUS)

dnl we don't need to check for hdf5 driver. It is always there.
HDF5_DRV="hdf5_drv"
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
    detect_readline=$default_detect_readline)

dnl
dnl handle browser.
dnl
BROWSER=$DEFAULT_BROWSER
AC_ARG_ENABLE(browser,
    AC_HELP_STRING([--enable-browser],
        [build browser @<:@default=yes@:>@]),
    if test "$enable_browser" = "no"; then
        BROWSER=""
    fi)
AC_SUBST(BROWSER)

dnl handle silex
SILEX=$DEFAULT_SILEX
AC_ARG_ENABLE(silex,
    AC_HELP_STRING([--enable-silex], 
        [build the silex browser @<:@default=no@:>@]),
    if test "$enable_silex" = "yes"; then
        SILEX="silex"
    fi)
AC_SUBST(SILEX)

dnl Handle explicit request for libz
AC_ARG_WITH(zlib, [  --with-zlib=INC,DIR     Use the GNU zlib compression @<:@default=yes@:>@])
if test "$with_zlib" = no; then
    AC_DEFINE(HAVE_ZLIB_H,0)
    AC_DEFINE(HAVE_LIBZ,0)
    ZLIB=""
elif test -n "$with_zlib" && test "$with_zlib" != yes; then
    ZLIB_INCDIR="`echo $with_zlib |cut -f1 -d,`"
    if test ! -f $ZLIB_INCDIR/zlib.h; then
        AC_MSG_ERROR(problem with directory specified for zlib includes)
    fi
    ZLIB_LIBDIR="`echo $with_zlib | cut -f2 -d, -s`"
    if test ! -f $ZLIB_LIBDIR/libz.a && test ! -f $ZLIB_LIBDIR/libz.so; then
        AC_MSG_ERROR(problem with directory specified for zlib libs)
    fi
    CPPFLAGS="$CPPFLAGS -I$ZLIB_INCDIR"
    LDFLAGS="$LDFLAGS -L$ZLIB_LIBDIR"
    LIBS="$LIBS -lz"
    AC_DEFINE(HAVE_ZLIB_H,1)
    AC_DEFINE(HAVE_LIBZ,1)
    ZLIB="zlib"
fi

dnl handle Peter Lindstrom's hzip compression stuff 
AC_MSG_CHECKING(for hzip)
if test -d $srcdir/src/hzip; then
    AC_MSG_RESULT(found)
    HZIP=$DEFAULT_HZIP
else
    AC_MSG_RESULT(hzip compression not available in BSD version)
    HZIP=""
fi
AC_ARG_ENABLE(hzip,
    AC_HELP_STRING([--enable-hzip], 
        [enable Lindstrom hex/quad mesh compression @<:@default=yes@:>@]),
    if test "$enable_hzip" = "no"; then
        HZIP=""
    fi)
AC_SUBST(HZIP)

dnl hzip does better with zlib if available. It may have been
dnl explicitly turned off, above. If not, Assume we have both
dnl header & lib. Turn it off if not.
if test -n "$HZIP" && test "$with_zlib" != no; then
    AC_DEFINE(HAVE_HZIP,1,[Support for Lindstrom hex/quad mesh compression])
    ZLIB="zlib"
    AC_CHECK_HEADERS(zlib.h,have_zlib_h="yes")
    if test -n "$have_zlib_h"; then
        AC_CHECK_LIB(z, inflateEnd)
        if test -z "`echo $LIBS | tr ' ' '\n' | grep -x -e '-lz'`"; then
            AC_DEFINE(HAVE_ZLIB_H,0)
            AC_DEFINE(HAVE_LIBZ,0)
            ZLIB=""
        fi
    fi
    AC_SUBST(ZLIB)
fi

dnl enable Peter Lindstrom's fpzip compression stuff 
AC_MSG_CHECKING(for fpzip)
if test -d $srcdir/src/fpzip; then
    AC_MSG_RESULT(found)
    FPZIP=$DEFAULT_FPZIP
else
    AC_MSG_RESULT(fpzip compression not available in BSD version)
    FPZIP=""
fi
AC_ARG_ENABLE(fpzip,
    AC_HELP_STRING([--enable-fpzip], 
        [enable Lindstrom float 1,2,3D array compression @<:@default=yes@:>@]),
    if test "$enable_fpzip" = "no"; then
        FPZIP=""
    fi)
AC_SUBST(FPZIP)
if test -n "$FPZIP"; then
    AC_DEFINE(HAVE_FPZIP,1,[Support for Lindstrom float array compression])
fi

dnl enable experimental JSON support
AC_ARG_ENABLE(json,
    AC_HELP_STRING([--enable-json], 
        [enable experimental JSON features@<:@default=no@:>@]),
    if test "$enable_json" = "yes"; then
        AC_MSG_CHECKING(Enabling json...)
        if test -e ${ac_abs_confdir}/tools/json/json-c-0.10.tar.gz; then
            if test -z "$(find tools/json/json-c-0.10 -name libjson.so -o -name libjson.a -o -name libjson.dylib)"; then
                if test ! -e ./tools/json; then
                    mkdir tools 1>/dev/null 2>&1
                    mkdir tools/json 1>/dev/null 2>&1
                fi
                pushd tools/json 1>/dev/null 2>&1
                rm -rf json-c-0.10
                gunzip < ${ac_abs_confdir}/tools/json/json-c-0.10.tar.gz | tar xf -
                cd json-c-0.10
                if test ! "${prefix}" = "NONE"; then
                    ./configure --prefix=${prefix}/json
                    make install
                else
                    ./configure
                    make
                fi
                if test $? -eq 0; then
                    AC_MSG_RESULT(json-c compiled and installed)
                    JSON="json"
                else
                    AC_MSG_RESULT(JSON support disabled due to failure to install)
                    JSON=""
                fi
                popd 1>/dev/null 2>&1
            else
                AC_MSG_RESULT(json-c compiled and installed)
                JSON="json"
            fi
        else
            AC_MSG_RESULT(JSON support not available because json-c distribution is not in this tarball)
            JSON=""
        fi
    fi)
AC_SUBST(JSON)
if test -n "$JSON"; then
    AC_DEFINE(HAVE_JSON,1,[Support for experimental JSON features])
fi

# Turn on C optimization flags
AC_MSG_CHECKING(compiler optimization)
if test "X$USING_DEBUG" = Xno; then
# use contributed GNU m4 macro library provided see:
#   autoconf-archive-2006-06-04/htmldoc/ax_cc_maxopt.html
   AX_CC_MAXOPT
   AC_MSG_RESULT(enabled)
else
   AC_MSG_RESULT(disabled)
fi

dnl
#
# MCM: I have no idea what good the following line does. I think
# all compilers accept the -D argument and any string following
# it is an argument to the -D argument. So, I think this is
# just totally bogus!
# Default to large file support
AX_CHECK_COMPILER_FLAGS("-D_LARGEFILE_SOURCE",CFLAGS="$CFLAGS -D_LARGEFILE_SOURCE";)
AX_CHECK_COMPILER_FLAGS("-D_LARGEFILE64_SOURCE",CFLAGS="$CFLAGS -D_LARGEFILE64_SOURCE";)
AX_CHECK_COMPILER_FLAGS("-D_FILE_OFFSET_BITS=64",CFLAGS="$CFLAGS -D_FILE_OFFSET_BITS=64";)
AX_CHECK_COMPILER_FLAGS("-Wdeclaration-after-statement",CFLAGS="$CFLAGS -Wdeclaration-after-statement";)

#
# Note: regardless of what the stuff above regarding large file support
# appears to do, the following test is the one that affects Silo's
# knowledge and support for large files. It may be that the result of
# the sizeof check is dependent on whether -D_LARGEFILE64_SOURCE is
# defined on the command line to the compiler though.
# Use size of off64_t to determine which form of stat() calls to make 
#
AC_CHECK_SIZEOF(off64_t)

dnl We always use the install-sh script, so don't uncomment the following line.
AC_PROG_INSTALL

dnl
dnl Checks for header files.
dnl
dnl If we don't have these header files, we can't continue.
dnl
AC_CHECK_HEADERS([sys/time.h stdarg.h sys/stat.h], [],
AC_MSG_ERROR([$0 wasn't able to find a necessary header file
(see above).  Please rectify this and rerun configure.
See the file INSTALL_FAQ in this directory for possible reasons
this might have happened.]))

AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([sys/fcntl.h])
if test ! "$ac_cv_header_fcntl_h"="yes" && test ! "$ac_cv_header_sys_fcntl_h"="yes" ; then
AC_MSG_ERROR([$0 wasn't able to find a necessary fcntl.h or
sys/fcntl.h header file.
Please rectify this and rerun configure.
See the file INSTALL_FAQ in this directory for possible reasons
this might have happened.])
fi

dnl
dnl Check for header files that we can do without.
dnl Only needed by browser
AC_CHECK_HEADERS([fnmatch.h])
dnl Only needed by silock
AC_CHECK_HEADERS([ieeefp.h])
dnl a few systems define O_RDONLY here
AC_CHECK_HEADERS([sys/types.h unistd.h])
dnl
AC_HEADER_STDC

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
AC_CHECK_FUNCS([memmove fnmatch isnan fpclass strerror])

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

dnl Check for Qt library
if test -n "$SILEX"; then
    AX_HAVE_QT
fi dnl if test -n "$SILEX"; then

dnl the hdf5 library checks (to follow) will require the -lm if its required
AC_CHECK_LIBM
LIBS="$LIBS $LIBM"

dnl ----------------------------------------------------------------------
dnl Is the szlib present? It has a library
dnl `-lsz' and their locations might be specified with the `--with-szlib'
dnl command-line switch. The value is a root directory contain the lib 
dnl directory.
dnl
AC_ARG_WITH([szlib],
            [AC_HELP_STRING([--with-szlib=DIR],
                            [Use szlib library for external szlib I/O
                             filter [default=no]])],,
            withval=default)

case $withval in
  default)
    HAVE_SZLIB="no"
    if test "$DEFAULT_SZIP" != "" ; then
      LDFLAGS="$LDFLAGS -L$DEFAULT_SZIP/lib"
      LIBS="$LIBS -lsz"
      HAVE_SZLIB="yes"
      szlib_lib="$DEFAULT_SZIP/lib"
    fi
    if test "$DEFAULT_SZIP_LIB" != "" ; then
      LDFLAGS="$LDFLAGS -L$DEFAULT_SZIP_LIB"
      LIBS="$LIBS -lsz"
      HAVE_SZLIB="yes"
      szlib_lib="$DEFAULT_SZIP_LIB"
    fi
    if test "$DEFAULT_SZIP_LIBDIR" != "" ; then
      LDFLAGS="$LDFLAGS -L$DEFAULT_SZIP_LIBDIR"
      LIBS="$LIBS -lsz"
      HAVE_SZLIB="yes"
      szlib_lib="$DEFAULT_SZIP_LIBDIR"
    fi
    if test "$HAVE_SZLIB" = "yes" ; then
      AC_MSG_CHECKING([default szlib path provided])
      AC_MSG_RESULT([$szlib_lib])
    else
      AC_MSG_CHECKING([for szlib])
      AC_MSG_RESULT([suppressed])
    fi
    ;;
  yes)
    HAVE_SZLIB="no"
    if test "$DEFAULT_SZIP" != "" ; then
      LDFLAGS="$LDFLAGS -L$DEFAULT_SZIP/lib"
      LIBS="$LIBS -lsz"
      HAVE_SZLIB="yes"
      szlib_lib="$DEFAULT_SZIP/lib"
    fi
    if test "$DEFAULT_SZIP_LIB" != "" ; then
      LDFLAGS="$LDFLAGS -L$DEFAULT_SZIP_LIB"
      LIBS="$LIBS -lsz"
      HAVE_SZLIB="yes"
      szlib_lib="$DEFAULT_SZIP_LIB"
    fi
    if test "$DEFAULT_SZIP_LIBDIR" != "" ; then
      LDFLAGS="$LDFLAGS -L$DEFAULT_SZIP_LIBDIR"
      LIBS="$LIBS -lsz"
      HAVE_SZLIB="yes"
      szlib_lib="$DEFAULT_SZIP_LIBDIR"
    fi
    if test "$HAVE_SZLIB" = "yes" ; then
      AC_MSG_CHECKING([using szlib path provided])
      AC_MSG_RESULT([$szlib_lib])
    else
      AC_MSG_ERROR([expect to find szlib environment variables])
      AC_MSG_CHECKING([for szlib])
      AC_MSG_RESULT([suppressed])
    fi
    ;;
  no)
    HAVE_SZLIB="no"
    AC_MSG_CHECKING([for szlib])
    AC_MSG_RESULT([suppressed])
    ;;
  *)
    HAVE_SZLIB="yes"
    case "$withval" in
      *)
        if test -n "$withval"; then
          szlib_lib="$withval/lib"
        fi
        ;;
    esac
    if test -n "$szlib_lib"; then
      LDFLAGS="$LDFLAGS -L$szlib_lib"
    fi

    AC_CHECK_LIB([sz], [SZ_BufftoBuffCompress],,
                 [LDFLAGS="$saved_LDFLAGS"; unset HAVE_SZLIB])

    if test -z "$HAVE_SZLIB" -a -n "$HDF5_CONFIG_ABORT"; then
      AC_MSG_ERROR([couldn't find szlib library])
    fi
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

if test -n "$HDF5_DRV"; then
   AC_MSG_CHECKING(for hdf5)
   saved_CPPFLAGS="$CPPFLAGS"
   saved_LDFLAGS="$LDFLAGS"
   saved_LIBS="$LIBS"
   AC_ARG_WITH([hdf5],
           [  --with-hdf5=INC,LIB     Location of HDF5 header and library],
	   ,
	   withval="default")
   if test "$withval" = "no" ; then
      AC_MSG_RESULT(suppressed)
      HDF5_DRV=""
   elif test "$withval" = "default" ; then
      if test "$DEFAULT_HDF5_LIBDIR" != "" ; then
         LDFLAGS="$LDFLAGS -L$DEFAULT_HDF5_LIBDIR"
         LIBS="$LIBS -lhdf5 -lz"
         hdf5_lib=$DEFAULT_HDF5_LIBDIR
         hdf5_explicit=true
      fi
      if test "$DEFAULT_HDF5_INCLUDE" != "" ; then
         CPPFLAGS="$CPPFLAGS -I$DEFAULT_HDF5_INCLUDE"
         hdf5_inc=$DEFAULT_HDF5_INCLUDE
         hdf5_explicit=true
      fi
      if test -d "$hdf5_lib"; then
         AC_MSG_RESULT([$hdf5_lib])
         AC_DEFINE(HAVE_HDF5_H,1,[hdf5 header file])
         AC_DEFINE(HAVE_LIBHDF5,1,[hdf5 library])
      else
         AC_MSG_RESULT(no)
         HDF5_DRV=""
      fi
   elif test "$withval" = "yes" ; then
      if test "$DEFAULT_HDF5_LIBDIR" != "" ; then
         LDFLAGS="$LDFLAGS -L$DEFAULT_HDF5_LIBDIR"
         LIBS="$LIBS -lhdf5 -lz"
         hdf5_lib=$DEFAULT_HDF5_LIBDIR
         hdf5_explicit=true
      fi
      if test "$DEFAULT_HDF5_INCLUDE" != "" ; then
         CPPFLAGS="$CPPFLAGS -I$DEFAULT_HDF5_INCLUDE"
         hdf5_inc=$DEFAULT_HDF5_INCLUDE
         hdf5_explicit=true
      fi
      if test -d "$hdf5_lib"; then
         AC_MSG_RESULT([$hdf5_lib])
         AC_DEFINE(HAVE_HDF5_H,1,[hdf5 header file])
         AC_DEFINE(HAVE_LIBHDF5,1,[hdf5 library])
      else
         AC_MSG_ERROR([expected to find hdf5 environment variables])
         AC_MSG_RESULT(no)
         HDF5_DRV=""
      fi
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
            if test -n "$szlib_lib"; then
               LDFLAGS="-L$hdf5_lib -L$szlib_lib $LDFLAGS"
            else
               LDFLAGS="-L$hdf5_lib $LDFLAGS"
            fi
         else
            AC_MSG_ERROR(problem with directory specified for hdf5 library)
         fi
      fi
      notfound=""
      AC_CHECK_HEADERS(hdf5.h,,notfound="hdf5.h")
      tmp_LIBS="$LIBS"
      if test -n "$szlib_lib"; then
###      LIBS="$hdf5_lib/libhdf5.a $LIBS"
         LIBS="-lhdf5 -lsz $LIBS"
      else
         LIBS="-lhdf5 $LIBS"
      fi
      AC_CHECK_FUNC(H5open,[AC_DEFINE(HAVE_LIBHDF5,1,[using HDF5])],notfound="$notfound libhdf5.a")
      if test -n "$notfound"; then
         LIBS="$tmp_LIBS"
      fi
      #
      # If the above tests did NOT find headers and lib for
      # hdf5, it could be due to zlib, so now test for zlib
      # MCM -- July, 2008: This code now looks completely useless
      # It is supposed to attempt to check if '-lz' is needed to
      # link with HDF5. However, it doesn't appear to do any of
      # that and had a useless embedded AC_ARG_WITH in it which
      # I removed and replaced all instances of 'withval' to the
      # with variable the code needs, with_zlib
      #
      if test -n "$notfound"; then
         # reset the compile and link flags
         CPPFLAGS="$saved_CPPFLAGS"
         LDFLAGS="$saved_LDFLAGS"
         AC_CHECKING(if lack of zlib caused tests for hdf5 to fail) 
         if test "no" = "$with_zlib"; then
            AC_MSG_CHECKING(for zlib)
            AC_MSG_RESULT(suppressed)
         else
            zlib_inc="`echo $with_zlibval |cut -f1 -d,`"
            if test "$with_zlibval" != "yes" && test -n "$zlib_inc"; then
               if test -d "$zlib_inc"; then
                  CPPFLAGS="-I$zlib_inc $CPPFLAGS"
               else
                  AC_MSG_ERROR(problem with directory specified for zlib includes)
               fi
            fi
            zlib_lib="`echo $with_zlibval |cut -f2 -d, -s`"
            if test "$with_zlibval" != "yes" && test -n "$zlib_lib"; then
               if test -d "$zlib_lib"; then
                  LDFLAGS="-L$zlib_lib $LDFLAGS"
               else
                  AC_MSG_ERROR(problem with directory specified for zlib library)
               fi
            fi
	    notfound=""
            dnl MCM -- 11Jul08: Here, we only need zlib to link with hdf5,
            dnl not to compile anything in Silo. Why we're checking for
            dnl headers here, I do not know.
            if test -z "`echo $LIBS | tr ' ' '\n' | grep -x -e '-lz'`"; then
                AC_CHECK_HEADERS(zlib.h,,notfound="zlib.h")
                AC_CHECK_LIB(z,compress2,,notfound="$notfound libz.a")
            fi
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
               if test -n "$szlib_lib"; then
                  LDFLAGS="-L$hdf5_lib -L$szlib_lib $LDFLAGS"
               else
                  LDFLAGS="-L$hdf5_lib $LDFLAGS"
               fi
            fi
	    notfound=""
            AC_CHECK_HEADERS(hdf5.h,,notfound="hdf5.h")
            tmp_LIBS="$LIBS"
            AC_CHECK_LIB(hdf5,H5open,,notfound="$notfound libhdf5.a")
            if test "$notfound" = ""; then
               if test -n "$szlib_lib"; then
###               LIBS="$hdf5_lib/libhdf5.a $tmp_LIBS"
                  LIBS="-lhdf5 -lsz $tmp_LIBS"
               else
                  LIBS="-lhdf5 $tmp_LIBS"
               fi
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
    # Short-term hack to deal with bogus code above (too complex) 
    # putting -lsz AHEAD of -lhdf5 in libs
    if test "$HAVE_SZLIB" = "yes"; then
        LIBS="$LIBS -lsz"
    fi
    AC_DEFINE(HAVE_HDF5_DRIVER,1,[Support for HDF5])
else
   LIBS="$saved_LIBS"
fi

AC_ARG_WITH(pdb-proper,
    AC_HELP_STRING([--with-pdb-proper],[Turns on PDB proper driver by specifying directory where PDB proper is installed @<:@default=no@:>@]),
    if test $withval != yes; then
        PDBP_DRV="pdbp_drv"
        AC_DEFINE(HAVE_PDBP_DRIVER,1,[Support for PDB Proper])
        AC_SUBST(PDBP_DRV)
        CPPFLAGS="-I$withval/include $CPPFLAGS"
        LDFLAGS="-L$withval/lib $LDFLAGS"
        LIBS="-lpdb -lpml -lscore $LIBS"
    else
        AC_MSG_ERROR(You must specify a directory for PDB proper)
    fi
    )

dnl
READLINE_LIBS=""
if test "X$detect_readline" = Xyes; then
# use contributed GNU m4 macro library provided see:
#   autoconf-archive-2006-06-04/htmldoc/vl_lib_readline.html
   saved_LIBS="$LIBS"
   VL_LIB_READLINE
   # Put only the stuff NOT already in $LIBS into READLINE_LIBS
   saved_LIBS_grep=$(echo $saved_LIBS | tr -s ' ' | sed 's% %\\|%g')
   READLINE_LIBS=$(echo $LIBS | tr -s ' ' | tr ' ' '\n' | grep -v -e $saved_LIBS_grep | tr '\n' ' ')
   LIBS="$saved_LIBS"
else
   AC_MSG_CHECKING(for readline)
   AC_MSG_RESULT(disabled)
fi
AC_SUBST(READLINE_LIBS)

dnl
dnl Check for a python installation using the Automake 
dnl provided macro, which will generate the following
dnl variables: python_PYTHON, pkgpython_PYTHON, 
dnl pyexecdir_PYTHON, and pkgpyexecdir_PYTHON.
dnl
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
   AC_MSG_RESULT([$PYTHON_CPPFLAGS])
   AC_SUBST([PYTHON_CPPFLAGS])
fi

dnl Determine how to compile MPI
# skip for now
#ACX_MPI

# Create a bundle target for Darwin
BUNDLE_TARGET=""
case $host_os in
  darwin*)
    BUNDLE_TARGET="silo.app"
    ;;
esac
AC_SUBST(BUNDLE_TARGET)

AC_CACHE_SAVE

dnl
AM_CONDITIONAL(INSTALL_LITE_HEADERS, test "$INSTLITE" = "yes")
AM_CONDITIONAL(NETCDF_NEEDED, test -n "$NETCDF")
AM_CONDITIONAL(PDB_DRV_NEEDED, test -n "$PDB_DRV")
AM_CONDITIONAL(PDBP_DRV_NEEDED, test -n "$PDBP_DRV")
AM_CONDITIONAL(TAURUS_NEEDED, test -n "$TAURUS")
AM_CONDITIONAL(HDF5_DRV_NEEDED, test -n "$HDF5_DRV")
AM_CONDITIONAL(BROWSER_NEEDED, test -n "$BROWSER")
AM_CONDITIONAL(PYTHON_NEEDED, test -n "$PYTHONMODULE")
AM_CONDITIONAL(FORTRAN_NEEDED, test -n "$FORTRAN")
AM_CONDITIONAL(SILEX_NEEDED, test -n "$SILEX")
AM_CONDITIONAL(HZIP_NEEDED, test -n "$HZIP")
AM_CONDITIONAL(FPZIP_NEEDED, test -n "$FPZIP")
AM_CONDITIONAL(JSON_NEEDED, test -n "$JSON")
AM_CONDITIONAL(ZLIB_AVAILABLE, test -n "$ZLIB")
AM_CONDITIONAL(CXX_LINK_NEEDED, test "$enable_shared" = "no" -a \( -n "$FPZIP" -o -n "$HZIP" \))
AC_CONFIG_FILES([
            Makefile
            SiloWindows/Makefile
            svn_bin/Makefile
            config/Makefile
            config-site/Makefile
            docs/Makefile
            src/Makefile
            src/score/Makefile
            src/pdb/Makefile
	    src/silo/Makefile
	    src/silo/silo.h
            src/debug/Makefile
            src/netcdf/Makefile
            src/pdb_drv/Makefile
            src/pdbp_drv/Makefile
            src/hdf5_drv/Makefile
            src/taurus/Makefile
            src/unknown/Makefile
            src/filters/Makefile
            tests/Makefile
            tools/Makefile
            tools/browser/Makefile
            tools/python/Makefile
            tools/silex/Makefile
            tools/silock/Makefile
            tools/json/Makefile
            tools/mapred/Makefile])
if test -n "$HZIP"; then
  AC_CONFIG_FILES([src/hzip/Makefile])
fi
if test -n "$FPZIP"; then
  AC_CONFIG_FILES([src/fpzip/Makefile])
fi
if test -n "$HDF5_DRV"; then
  AC_CONFIG_FILES([src/libsiloh5.settings])
else
  AC_CONFIG_FILES([src/libsilo.settings])
fi
CC_FULLPATH=$(which $(echo ${CC:-"nocc"} | cut -d' ' -f1))
CXX_FULLPATH=$(which $(echo ${CXX:-"nocxx"} | cut -d' ' -f1))
FC_FULLPATH=$(which $(echo ${FC:-"nofc"} | cut -d' ' -f1))
AC_SUBST(CC_FULLPATH)
AC_SUBST(CXX_FULLPATH)
AC_SUBST(FC_FULLPATH)
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
