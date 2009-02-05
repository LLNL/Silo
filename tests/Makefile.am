## Procss this file with automake to create Makefile.in
# making use of the autotest features in autoconf
# 

EXTRA_DIST= \
 exectests.csh \
 package.m4 \
 pdbcmp.csh \
 testsuite.at \
 testsuite \
 testsuitef77.at \
 testsuitef77 \
 testsuitehdf5.at \
 testsuitehdf5 \
 checksums \
 pmpio_silo_test_mesh.c

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
 -I$(srcdir)/../src/silo
quad_CPPFLAGS = -DNEW_SILO $(AM_CPPFLAGS)
TestReadMask_CPPFLAGS = -DNEW_SILO $(AM_CPPFLAGS)
AM_FFLAGS = $(AM_CPPFLAGS)

HDF5PROGS=compression grab
F77PROGS= arrayf77 curvef77 matf77 pointf77 quadf77 ucdf77 testallf77
PROGS=array dir extface multi_test partial_io point quad simple ucd \
      ucdsamp3 testall obj onehex oneprism onepyramid onetet subhex \
      TestReadMask twohex multispec sami specmix spec cpz1plt spec \
      cpz1plt group_test listtypes alltypes wave multi_file polyzl csg \
      rocket mmadjacency largefile version

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
 version 

if HDF5_DRV_NEEDED
  TESTSUITE += $(srcdir)/testsuitehdf5
  PROGS += $(HDF5PROGS)
  check_PROGRAMS += $(HDF5PROGS)
  compression_SOURCES = compression.c
  compression_LDADD = $(LDADD) 
  grab_SOURCES = grab.c
  grab_LDADD = $(LDADD) 
endif

if FORTRAN_NEEDED
  TESTSUITE += $(srcdir)/testsuitef77
  PROGS += $(F77PROGS)
  check_PROGRAMS += $(F77PROGS)
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

#TESTS = array extface multi_test partial_io point group_test \
 listtypes simple ucd ucdsamp3 testall alltypes obj onehex \
 oneprism onepyramid onetet TestReadMask subhex twohex multispec \
 specmix testpdb sami spec cpz1plt merge_block mat3d_3across \
 ucd1d wave multi_file polyzl csg rocket mmadjacency  largefile version 

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

# autotest targets
check-local: $(TESTSUITE) $(PROGS)
	list='$(TESTSUITE)'; for file in $$list; do \
	  $(SHELL) $$file; \
	done

clean-local:
	rm -f testsuite.log
	rm -rf testsuite.dir
	rm -f testsuitef77.log
	rm -rf testsuitef77.dir
	rm -f testsuitehdf5.log
	rm -rf testsuitehdf5.dir

testsuite: $(srcdir)/testsuite.at
	$(AUTOTEST) -I $(srcdir) $@.at -o $@.tmp
	-mv -f $@.tmp $@

testsuitef77: $(srcdir)/testsuitef77.at
	$(AUTOTEST) -I $(srcdir) $@.at -o $@.tmp
	-mv -f $@.tmp $@

testsuitehdf5: $(srcdir)/testsuitehdf5.at
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
