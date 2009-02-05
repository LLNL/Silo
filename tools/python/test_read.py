import Silo

# ---- file 1 ----
file = "/usr/gapps/visit/data/multi_ucd3d.silo"

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

print "\n"

# ---- file 2 ----
file = "/usr/gapps/visit/data/globe.silo"

db = Silo.Open(file)
print "db = ",db

toc = db.GetToc()
print "\n-- TOC --\n",toc

print "cycle='%d'"%db.GetVar("cycle")
print "time='%f'"%db.GetVar("time")
print "_fileinfo='%s'"%db.GetVar("_fileinfo")
print "mesh1_coord0=",db.GetVar("mesh1_coord0")[1:10]
print "fl_zoneno=",db.GetVar("fl_zoneno")[1:10]
