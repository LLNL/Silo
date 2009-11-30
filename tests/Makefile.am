## Procss this file with automake to create Makefile.in
# making use of the autotest features in autoconf
# 

# some example commands to compile ioperf plugins
# gcc -shared -I. ioperf_sec2.c -o libiop_sec2.so
# gcc -shared -I. ioperf_stdio.c -o libiop_stdio.so
# gcc -shared -I. -I/usr/gapps/silo/hdf5/1.8.1/i686_Linux_ELsmp/include ioperf_hdf5.c -o libiop_hdf5.so /usr/gapps/silo/hdf5/1.8.1/i686_Linux_ELsmp/lib/libhdf5.a

EXTRA_DIST= \
 package.m4 \
 pdbcmp.csh \
 testsuite.at \
 testsuite \
 pmpio_hdf5_test.c \
 pmpio_silo_test_mesh.c \
 add_amr_mrgtree.c \
 amr3d.silo \
 amr2d.silo \
 amr2d_mesh_config.xml \
 amr3d_mesh_config.xml \
 not_a_silo_file.h5 \
 not_a_silo_file.pdb \
 ioperf_sec2.c \
 ioperf_stdio.c \
 ioperf_hdf5.c \
 ioperf_pdb.c \
 ioperf_silo.c \
 $(check_SCRIPTS) \
 $(check_DATA)

include_HEADERS = \
 ioperf.h

# Note: use top entry in check_SCRIPTS as sole target to
# copy all scripts and data in one rule
check_SCRIPTS= \
 checksums \
 testhzip \
 testxvers \
 test_error.py \
 test_read.py \
 test_write.py \
 testsilock

check_DATA= \
 xvers_data.tar.bz2 \
 pion0244.silo

DISTCLEANFILES = atconfig
AUTOM4TE = autom4te
AUTOTEST = $(AUTOM4TE) --language=autotest
TESTSUITE = $(srcdir)/testsuite

if HDF5_DRV_NEEDED
  LDADD = ../src/libsiloh5.la
else
  LDADD = ../src/libsilo.la
endif

# paths look at install directories before sources
AM_CPPFLAGS = \
 -I$(includedir) \
 -I$(srcdir)/../src/pdb \
 -I$(srcdir)/../src/score \
 -I$(builddir)/../src/silo \
 -I$(srcdir)/../src/silo
quad_CPPFLAGS = $(AM_CPPFLAGS)
TestReadMask_CPPFLAGS = $(AM_CPPFLAGS)
AM_FFLAGS = $(AM_CPPFLAGS)

HDF5PROGS=compression grab
FCPROGS= arrayf77 curvef77 matf77 pointf77 quadf77 ucdf77 testallf77
PROGS=array dir extface multi_test partial_io point quad simple ucd \
      ucdsamp3 testall obj onehex oneprism onepyramid onetet subhex \
      TestReadMask twohex multispec sami specmix spec cpz1plt spec \
      cpz1plt group_test listtypes alltypes wave multi_file polyzl csg \
      rocket mmadjacency largefile version namescheme efcentering \
      mk_nasf_pdb ioperf

dir_SOURCES = \
 dir.c \
 testlib.c
quad_SOURCES = \
 quad.c \
 testlib.c
rocket_SOURCES = \
 rocket.cpp

check_PROGRAMS = \
 array \
 extface \
 multi_test \
 partial_io \
 point \
 group_test \
 listtypes \
 simple \
 ucd \
 ucdsamp3 \
 testall \
 alltypes \
 obj \
 onehex \
 oneprism \
 onepyramid \
 onetet \
 TestReadMask \
 subhex \
 twohex \
 multispec \
 specmix \
 testpdb \
 sami \
 spec \
 cpz1plt \
 merge_block \
 mat3d_3across \
 ucd1d \
 wave \
 multi_file \
 polyzl \
 csg \
 rocket \
 mmadjacency \
 largefile \
 dir \
 quad \
 version \
 namescheme \
 efcentering \
 mk_nasf_pdb \
 ioperf


# Deal with need to use c++ linker whenever we've got C++
# sources compiled with the silo library and the fact that
# Automake/gmake won't let me just say LINK=$(CXXLINK). 
# This is a hack to force c++ linkage when we have
# c++ code compiled into Silo. I got this hack from
# automake manaual.
if CXX_LINK_NEEDED
 nodist_EXTRA_array_SOURCES = dummy.cxx
 nodist_EXTRA_extface_SOURCES = dummy.cxx
 nodist_EXTRA_multi_test_SOURCES = dummy.cxx
 nodist_EXTRA_partial_io_SOURCES = dummy.cxx
 nodist_EXTRA_point_SOURCES = dummy.cxx
 nodist_EXTRA_group_test_SOURCES = dummy.cxx
 nodist_EXTRA_listtypes_SOURCES = dummy.cxx
 nodist_EXTRA_simple_SOURCES = dummy.cxx
 nodist_EXTRA_ucd_SOURCES = dummy.cxx
 nodist_EXTRA_ucdsamp3_SOURCES = dummy.cxx
 nodist_EXTRA_testall_SOURCES = dummy.cxx
 nodist_EXTRA_alltypes_SOURCES = dummy.cxx
 nodist_EXTRA_obj_SOURCES = dummy.cxx
 nodist_EXTRA_onehex_SOURCES = dummy.cxx
 nodist_EXTRA_oneprism_SOURCES = dummy.cxx
 nodist_EXTRA_onepyramid_SOURCES = dummy.cxx
 nodist_EXTRA_onetet_SOURCES = dummy.cxx
 nodist_EXTRA_TestReadMask_SOURCES = dummy.cxx
 nodist_EXTRA_subhex_SOURCES = dummy.cxx
 nodist_EXTRA_twohex_SOURCES = dummy.cxx
 nodist_EXTRA_multispec_SOURCES = dummy.cxx
 nodist_EXTRA_specmix_SOURCES = dummy.cxx
 nodist_EXTRA_testpdb_SOURCES = dummy.cxx
 nodist_EXTRA_sami_SOURCES = dummy.cxx
 nodist_EXTRA_spec_SOURCES = dummy.cxx
 nodist_EXTRA_cpz1plt_SOURCES = dummy.cxx
 nodist_EXTRA_merge_block_SOURCES = dummy.cxx
 nodist_EXTRA_mat3d_3across_SOURCES = dummy.cxx
 nodist_EXTRA_ucd1d_SOURCES = dummy.cxx
 nodist_EXTRA_wave_SOURCES = dummy.cxx
 nodist_EXTRA_multi_file_SOURCES = dummy.cxx
 nodist_EXTRA_polyzl_SOURCES = dummy.cxx
 nodist_EXTRA_csg_SOURCES = dummy.cxx
 nodist_EXTRA_rocket_SOURCES = dummy.cxx
 nodist_EXTRA_mmadjacency_SOURCES = dummy.cxx
 nodist_EXTRA_largefile_SOURCES = dummy.cxx
 nodist_EXTRA_dir_SOURCES = dummy.cxx
 nodist_EXTRA_quad_SOURCES = dummy.cxx
 nodist_EXTRA_version_SOURCES = dummy.cxx
 nodist_EXTRA_namescheme_SOURCES = dummy.cxx
 nodist_EXTRA_arrayf77_SOURCES = dummy.cxx
 nodist_EXTRA_curvef77_SOURCES = dummy.cxx
 nodist_EXTRA_matf77_SOURCES = dummy.cxx
 nodist_EXTRA_pointf77_SOURCES = dummy.cxx
 nodist_EXTRA_quadf77_SOURCES = dummy.cxx
 nodist_EXTRA_ucdf77_SOURCES = dummy.cxx
 nodist_EXTRA_testallf77_SOURCES = dummy.cxx
 nodist_EXTRA_compression_SOURCES = dummy.cxx
 nodist_EXTRA_grab_SOURCES = dummy.cxx
 nodist_EXTRA_efcentering_SOURCES = dummy.cxx
 nodist_EXTRA_mk_nasf_pdb_SOURCES = dummy.cxx
 nodist_EXTRA_ioperf_SOURCES = dummy.cxx
endif

ioperf_LDADD = -ldl

if HDF5_DRV_NEEDED
  PROGS += $(HDF5PROGS)
  check_PROGRAMS += $(HDF5PROGS)
  compression_SOURCES = compression.c
  compression_LDADD = $(LDADD)
  grab_SOURCES = grab.c
  grab_LDADD = $(LDADD)
endif

if FORTRAN_NEEDED
  PROGS += $(FCPROGS)
  check_PROGRAMS += $(FCPROGS)
  arrayf77_SOURCES = arrayf77.f
  arrayf77_LDADD = $(LDADD) $(FCLIBS)
  curvef77_SOURCES = curvef77.f
  curvef77_LDADD = $(LDADD) $(FCLIBS)
  matf77_SOURCES = matf77.f
  matf77_LDADD = $(LDADD) $(FCLIBS)
  pointf77_SOURCES = pointf77.f
  pointf77_LDADD = $(LDADD) $(FCLIBS)
  quadf77_SOURCES = quadf77.f
  quadf77_LDADD = $(LDADD) $(FCLIBS)
  ucdf77_SOURCES = ucdf77.f
  ucdf77_LDADD = $(LDADD) $(FCLIBS)
  testallf77_SOURCES = testallf77.f
  testallf77_LDADD = $(LDADD) $(FCLIBS)
endif

all-local:
	@echo
	@if (fmt</dev/null)>/dev/null 2>&1;then fmt="fmt -70";else fmt=cat;fi;\
	   echo 'Silo test cases are not compiled by default.  To compile and'\
	      'test them run "make check" from the silo "tests" directory.'\
	      'Otherwise you can compile them individually with "make'\
	      'TESTNAME".  The test names are: $(PROGS:=,) but "testall" is'\
	      'a good one to start with.' |\
	   $$fmt | sed 's/^/NOTICE: /'
	@echo

#
# Autotest targets.
# Since we run testsuite multiple times in different 'modes', we
# need some extra logic to contatentate all the results into a
# single, final testsuite.log. Also, it is possible for TESTARGS
# to somehow override DB_HDF5= and PYTHON= in these test inokations.
#
check-local: $(TESTSUITE) $(PROGS)
	$(SHELL) $(srcdir)/testsuite PYTHON=$(PYTHON) BROWSER=$(BROWSER) $(TESTARGS)
	@if test -e testsuite.log; then \
	    cp testsuite.log testsuite.log.pdb ; \
	fi
if HDF5_DRV_NEEDED
	$(SHELL) $(srcdir)/testsuite DB_HDF5=DB_HDF5 PYTHON=$(PYTHON) BROWSER=$(BROWSER) $(TESTARGS)
	@if test -e testsuite.log.pdb -a -e testsuite.log; then \
	    cat testsuite.log.pdb >> testsuite.log ; \
	fi
endif

clean-local:
	rm -f testsuite.log
	rm -rf testsuite.dir

testsuite: $(srcdir)/testsuite.at
	$(AUTOTEST) -I $(srcdir) $@.at -o $@.tmp
	-mv -f $@.tmp $@

$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	{					\
	  echo '# Signature of the current package.'; \
	  echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	  echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >$(srcdir)/package.m4

# deal with the CC=mpicc until I can find a better way
resample$(EXEEXT): $(resample_OBJECTS) $(resample_DEPENDENCIES)
	@rm -f resample$(EXEEXT)
	$(LIBTOOL) --tag=CC --mode=link $(MPICC) $(AM_CFLAGS) $(CFLAGS) \
		$(AM_LDFLAGS) $(LDFLAGS) -o $@ $(resample_LDFLAGS) \
		$(resample_OBJECTS) $(resample_LDADD) $(LIBS)
resample.o: resample.c
	$(MPICC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) -c -o resample.o `test -f 'resample.c' || echo '$(srcdir)/'`resample.c
resample.lo: resample.c
	$(LIBTOOL) --tag=CC --mode=compile $(MPICC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) -c -o $@ $<

tests: $(PROGS)

# Single rule to capture anything put into check_DATA or check_SCRIPTS
# this assumes the only action necessary to create is a copy
checksums: copy_cksd
copy_cksd:
	@for f in $(check_SCRIPTS) $(check_DATA); do \
	    if test ! -e $$f ; then \
	        cp $(srcdir)/$$f . ;\
	    fi ;\
	done
	touch $@
