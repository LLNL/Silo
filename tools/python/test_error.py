import Silo
file = "/usr/gapps/visit/data/multi_ucd3d.silo"

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
