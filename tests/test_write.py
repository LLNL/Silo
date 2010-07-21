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
# Contract  No.   DE-AC52-07NA27344 with  the  DOE.  Neither the  United
# States Government  nor Lawrence  Livermore National Security,  LLC nor
# any of  their employees,  makes any warranty,  express or  implied, or
# assumes   any   liability   or   responsibility  for   the   accuracy,
# completeness, or usefulness of any information, apparatus, product, or
# process  disclosed, or  represents  that its  use  would not  infringe
# privately-owned   rights.  Any  reference   herein  to   any  specific
# commercial products,  process, or  services by trade  name, trademark,
# manufacturer or otherwise does not necessarily constitute or imply its
# endorsement,  recommendation,   or  favoring  by   the  United  States
# Government or Lawrence Livermore National Security, LLC. The views and
# opinions  of authors  expressed  herein do  not  necessarily state  or
# reflect those  of the United  States Government or  Lawrence Livermore
# National  Security, LLC,  and shall  not  be used  for advertising  or
# product endorsement purposes.
import Silo

db = Silo.Create("foo.silo", "test file")
db.Write("t1", 12)
db.Write("t2", "hello")
db.Write("t3", (13,))
db.Write("t4", 14.5)
db.Write("t5", (1.2, 10))
db.Write("t6", (1.2, 10.2))
db.Write("t7", (10, 1.2))
db.Write("t8", "a")
db.MkDir("a")
db.Write("a/t8", "x2")
db.SetDir("a")
db.Write("t9", "x3")
db.Write("../t10", "x4")
db.SetDir("..")
db.Write("t11", "x5")
db.Close()

db2=Silo.Open("foo.silo")
print "toc=%s"%db2.GetToc()
print "t2='%s'"%db2.GetVar("t2")
print "t6=",db2.GetVar("t6")
print "a/t8='%s'"%db2.GetVar("a/t8")
db2.SetDir("a")
print "t9=%s"%db2.GetVar("t9")
print "/t5=",db2.GetVar("../t5")
db2.Close()
