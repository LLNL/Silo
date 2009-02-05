import os
import Silo

if os.access("multi_ucd3d.pdb",os.R_OK):
    file = "multi_ucd3d.pdb"
elif os.access("multi_ucd3d.h5",os.R_OK):
    file = "multi_ucd3d.h5"

db = Silo.Open(file)
print "db = ",db

try:
    print "Trying to set a bad directory"
    db.SetDir("Magrathea")
except:
    print "    -- Caught error"

try:
    print "Trying to read a bad variable"
    db.GetVar("Arkleseizure")
except:
    print "    -- Caught error"

print "Closing...."
db.Close()

print "db = ",db

try:
    print "Trying to set a directory on a closed file"
    db.SetDir("domain0")
except:
    print "    -- Caught error"

try:
    print "Trying to read a variable on a closed file"
    db.SetDir("dtime")
except:
    print "    -- Caught error"
