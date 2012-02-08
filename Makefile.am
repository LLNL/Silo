# Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
# LLNL-CODE-425250.
# All rights reserved.
# 
# This file is part of Silo. For details, see silo.llnl.gov.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the disclaimer below.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the disclaimer (as noted
#      below) in the documentation and/or other materials provided with
#      the distribution.
#    * Neither the name of the LLNS/LLNL nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
# 
# THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
# "AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
# LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
# LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
# CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
# PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
# NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# This work was produced at Lawrence Livermore National Laboratory under
# Contract No.  DE-AC52-07NA27344 with the DOE.
# 
# Neither the  United States Government nor  Lawrence Livermore National
# Security, LLC nor any of  their employees, makes any warranty, express
# or  implied,  or  assumes  any  liability or  responsibility  for  the
# accuracy, completeness,  or usefulness of  any information, apparatus,
# product, or  process disclosed, or  represents that its use  would not
# infringe privately-owned rights.
# 
# Any reference herein to  any specific commercial products, process, or
# services by trade name,  trademark, manufacturer or otherwise does not
# necessarily  constitute or imply  its endorsement,  recommendation, or
# favoring  by  the  United  States  Government  or  Lawrence  Livermore
# National Security,  LLC. The views  and opinions of  authors expressed
# herein do not necessarily state  or reflect those of the United States
# Government or Lawrence Livermore National Security, LLC, and shall not
# be used for advertising or product endorsement purposes.
#
##############################################################################

#
# Modifications:
#
#   Mark C. Miller, Tue Aug  3 17:23:07 PDT 2010
#   Added logic to error out of there already exists a bsd dir into which we
#   wish to create a bsd tree. Added logic to remove the dir after completion.
#
#   Mark C. Miller, Wed Aug  4 15:52:52 PDT 2010
#   Added logic to handle windows project files for BSD release.
#
#   Mark C. Miller, Mon Aug  9 16:58:46 PDT 2010
#   Added creation of smalltest dist for bsd distributions.
#
#   Kathleen Bonnell, Thu Mar 3 10:34:15 PST 2011 
#   Handle MSVC9 windows project files. 
 
ACLOCAL_AMFLAGS = -I config

nodist_noinst_HEADERS = config.h

EXTRA_DIST = COPYRIGHT FAQ VERSION BSD_LICENSE_README

MAINTAINERCLEANFILES = Makefile.in\
 aclocal.m4\
 configure

## We need `.' in SUBDIRS because we want `check' to build `.' first.
SUBDIRS = . src tools tests
DIST_SUBDIRS = . config config-site \
  svn_bin docs src tests tools SiloWindows

help: config/make.help
	cat $(srcdir)/config/make.help

dist-gzip-bsd: dist-gzip
	@if test -e $(PACKAGE)-$(VERSION); then \
	    echo "A directory or file named $(PACKAGE)-$(VERSION) already exists" ;\
	    echo "Please remove before proceeding" ;\
	    exit 1 ;\
	fi
	@if test -e $(PACKAGE)-$(VERSION)-bsd; then \
	    echo "A directory or file named $(PACKAGE)-$(VERSION)-bsd already exists" ;\
	    echo "Please remove before proceeding" ;\
	    exit 1 ;\
	fi
	rm -f $(PACKAGE)-$(VERSION)-bsd.tar.gz $(PACKAGE)-$(VERSION)-bsd-smalltest.tar.gz
	gunzip < $(PACKAGE)-$(VERSION).tar.gz | tar xf -
	cp -R $(PACKAGE)-$(VERSION) $(PACKAGE)-$(VERSION)-bsd
	rm -rf $(PACKAGE)-$(VERSION)-bsd/src/hzip
	rm -rf $(PACKAGE)-$(VERSION)-bsd/src/fpzip
	rm -rf $(PACKAGE)-$(VERSION)-bsd/SiloWindows/MSVC9
	rm -rf $(PACKAGE)-$(VERSION)-bsd/SiloWindows/MSVC2010
	mv $(PACKAGE)-$(VERSION)-bsd/SiloWindows/MSVC9-BSD $(PACKAGE)-$(VERSION)-bsd/SiloWindows/MSVC9
	mv $(PACKAGE)-$(VERSION)-bsd/SiloWindows/MSVC2010-BSD $(PACKAGE)-$(VERSION)-bsd/SiloWindows/MSVC2010
	tar cf - $(PACKAGE)-$(VERSION)-bsd | gzip --best > $(PACKAGE)-$(VERSION)-bsd.tar.gz
	rm -rf $(PACKAGE)-$(VERSION)-bsd/tests/{amr3d.silo,pion0244.silo,amr2d.silo,xvers_data.tar.bz2}
	tar cf - $(PACKAGE)-$(VERSION)-bsd | gzip --best > $(PACKAGE)-$(VERSION)-bsd-smalltest.tar.gz
	rm -rf $(PACKAGE)-$(VERSION)-bsd
	rm -rf $(PACKAGE)-$(VERSION)/SiloWindows/MSVC9-BSD
	rm -rf $(PACKAGE)-$(VERSION)/SiloWindows/MSVC2010-BSD
	tar cf - $(PACKAGE)-$(VERSION) | gzip --best > $(PACKAGE)-$(VERSION).tar.gz
	rm -rf $(PACKAGE)-$(VERSION)

.PHONY: help

## Override Automake's default definition for this
am__tar = ${AMTAR} cof - "$$tardir"
