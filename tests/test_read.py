import os
import Silo

# ---- file 1 ----
if os.access("multi_ucd3d.pdb",os.R_OK):
    file = "multi_ucd3d.pdb"
elif os.access("multi_ucd3d.h5",os.R_OK):
    file = "multi_ucd3d.h5"

db = Silo.Open(file)
print "db = ",db
print "db.filename = '%s'"%db.filename

toc = db.GetToc()
print "\n-- TOC --\n",toc

print "cycle='%d'"%db.GetVar("cycle")
print "dtime='%f'"%db.GetVar("dtime")
print "_fileinfo='%s'"%db.GetVar("_fileinfo")
print "d_vartypes=",db.GetVar("d_vartypes")
print "d_varnames=",db.GetVar("d_varnames")

db.SetDir("block0")
toc = db.GetToc()
print "mesh1_coord0=",db.GetVar("mesh1_coord0")[1:10]
print "v_data=",db.GetVar("v_data")[1:10]

db.SetDir("../block7")
toc = db.GetToc()
print "mesh1_coord0=",db.GetVar("mesh1_coord0")[1:10]
print "v_data=",db.GetVar("v_data")[1:10]

db.Close()

print "\n"

# ---- file 2 ----
if os.access("multi_rect3d.pdb",os.R_OK):
    file = "multi_rect3d.pdb"
elif os.access("multi_rect3d.h5",os.R_OK):
    file = "multi_rect3d.h5"

db = Silo.Open(file)
print "db = ",db

toc = db.GetToc()
print "\n-- TOC --\n",toc

print "cycle='%d'"%db.GetVar("cycle")
print "time='%f'"%db.GetVar("time")
print "_fileinfo='%s'"%db.GetVar("_fileinfo")
print "defvars_defns='%s'"%db.GetVar("defvars_defns")

db.SetDir("block0")
toc = db.GetToc()
print "mesh1_coord0=",db.GetVar("mesh1_coord0")

db.SetDir("/block7")
toc = db.GetToc()
print "mesh1_coord0=",db.GetVar("mesh1_coord0")

db.SetDir("/")
toc = db.GetToc()
