#!/bin/csh -f
########################################################################
#
#                            Copyright 1991 - 1995
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

echo ""
echo "-------------------------- Conducting Silo Test ----------------------------"

#----------------------------------------------------------
#                     EXECTESTS.CSH
#
# FUNCTION:
#
#    Execute and validate the results of Silo Library test
#    cases contained in this directory.
#
#    This script program assumes the existence of a
#    `Makefile' that contains a set of known targets.
#
# COMMAND LINE:
#
#    exectests.csh
#
# RETURN VALUE:
#
#    0 --> No errors.
#          All test cases executed and validated.
#
#    1 --> At least one test case could not be validated.
#
#----------------------------------------------------------

#----------------------------------------------------------------
# Determine whether or not the Silo packages have been installed.
#----------------------------------------------------------------
#
# We should do better than "../../include/silo.h".  Maybe we should generate
# this file at configure time. (Sean Ahern - Tue Jul 30 16:38:33 PDT 1996)
if (!(-e ../../include/silo.h)) then
  exit (0)
endif

#-----------------------------
# Prepare directory for tests.
#-----------------------------

#We shouldn't need to do this. (Sean Ahern - Tue Jul 30 16:38:05 PDT 1996)
#cp ../../include/silo.inc .

set Tests = (testall array dir point quad)
set Tests = ($Tests simple ucd ucdsamp3 arrayf77 matf77)
set Tests = ($Tests pointf77 quadf77 ucdf77 testallf77)
set F77Tests = (FALSE FALSE FALSE FALSE FALSE)
set F77Tests = ($F77Tests FALSE FALSE FALSE TRUE TRUE)
set F77Tests = ($F77Tests TRUE TRUE TRUE TRUE)

#-----------------------------------------------------
# Execute the test cases.
#
# The procedure for executing a test case is---
#
#    1. Compile the test case source.
#    2. Link using the development libraries.
#    3. Run the executable.
#
# Failures on Fortran test cases are not treated as
# fatal because of a make problem concerning F77 and
# GCC.
#-----------------------------------------------------
echo " "
echo "Execution of Silo Tests in /test"
echo "`date`"

echo " "
echo "Execute silo file test generators..."

@ i = 1
foreach test ($Tests)
   echo "Executing test case $test..."

#-----------------------------------------------
#     Make and execute for development libs.
#-----------------------------------------------
   make ${test}
   ./${test}
   if ($status != 0 && $F77Tests[$i] == FALSE) then
      echo "Error executing test case $test"
      exit (1)
   endif

   @ i++
end

echo "---------------------------- Silo Test Passed ------------------------------"

exit (0)

end
