#!/bin/csh -f
########################################################################
#
#                            Copyright (c) 1991 - 2009
#                 The Regents of the University of California.
#                             All rights reserved.
#
# This work was produced at the University of California, Lawrence
# Livermore National Laboratory (UC LLNL) under contract no.
# W-7405-ENG-48 (Contract 48) between the U.S. Department of Energy
# (DOE) and The Regents of the University of California (University)
# for the operation of UC LLNL.  Copyright is reserved to the University
# for purposes of controlled dissemination, commercialization through
# formal licensing, or other disposition under terms of Contract 48;
# DOE policies, regulations and orders; and U.S. statutes.  The rights
# of the Federal Government are reserved under Contract 48 subject to
# the restrictions agreed upon by DOE and University.
#
#                                 DISCLAIMER
#
# This software was prepared as an account of work sponsored by an
# agency of the United States Government. Neither the United States
# Government nor the University of California nor any of their
# employees, makes any warranty, express or implied, or assumes any
# liability or responsiblity for the accuracy, completeness, or
# usefullness of any information, apparatus, product, or process
# disclosed, or represents that its use would not infringe privately
# owned rights. Reference herein to any specific commercial products,
# process, or service by trade name, trademark, manufacturer, or
# otherwise, does not necessarily constitute or imply its endorsement,
# recommendation, or favoring by the United States Government or the
# University of California. The views and opinions of authors expressed
# herein do not necessarily state or reflect those of the United States
# Government or the University of California, and shall not be used for
# advertising or product endorsement purposes.
#
########################################################################

set host = `uname`
if ($host == IRIX) then
   set pdbdiff = /usr/local/bin/pdbdiff
else if ($host == SunOs) then
   set pdbdiff = /usr/local/bin/pdbdiff
else
   set pdbdiff = pdbdiff
endif

set result = 0

if ($#argv == 0) then
   echo USAGE: pdbcmp file1_set1 file2_set1 ... file1_set2 file2_set2 ...
   exit (1)
endif
set files = ($argv[*])

@ num_files = $#files / 2

set fileset1 = ( )
set fileset2 = ( )

@ ifile = 1
while ($ifile <= $num_files)
   set fileset1 = ($fileset1 $files[$ifile])

   @ ifileset2  = $ifile + $num_files
   set fileset2 = ($fileset2 $files[$ifileset2])

   @ ifile = $ifile + 1
end

@ ifile = 1
while ($ifile <= $num_files)
   echo "Comparing $fileset1[$ifile] and $fileset2[$ifile] ..."

   $pdbdiff $fileset1[$ifile] $fileset2[$ifile]

   if ($status) then
      echo Files did not compare.
      set result = 1
   else
      echo Files compared ok.
   endif

   @ ifile = $ifile + 1
end

exit ($result)

end
