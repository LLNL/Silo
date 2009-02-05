PROGRAM = ex2silo
SRC     = ex2silo.C
OBJ     = ex2silo.o

CC      = CC

# Serial IRIX
#CFLAGS  = -g -LANG:ansi-for-init-scope=ON -ptused -no_prelink
#LDFLAGS =  -ptused -no_prelink

# Parallel IRIX
CFLAGS  = -g -LANG:ansi-for-init-scope=ON -DPARALLEL -ptused -no_prelink
LDFLAGS =  -ptused -no_prelink

# Parallel on Sandia's RED machine:
#CC      = pgCC -cougar
#CFLAGS  = -g -D__BOOL_DEFINED -DDUMB_COMPILER -DPARALLEL


#
# Modilfy the pathnames below to match the locations of silo and
# exodus libraries and include files on your system.
#
# Silo libraries
SLPATH   = ../../../lib
# Exodus libraries
ELPATH   = /usr/local/appsn32/lib
# Silo include files
SILOHDIR = ../../../include
# Exodus include files
EXOHDIR	 = /usr/local/appsn32/include


# 
# If you are lucky, from this point down nothing should need to be changed.
#
EXLIBS	 = -lexoIIv2c \
           -lnetcdf

SILOLIBS = -lsilo \
           -lm
LIBS = -lmpi

$(PROGRAM):   $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -L$(SLPATH) -L$(ELPATH) $(SILOLIBS) $(EXLIBS) $(LIBS) -o $(PROGRAM)

$(OBJ):   $(SRC)
	$(CC) $(CFLAGS) -I$(SILOHDIR) -I$(EXOHDIR) -c $(SRC)

clean:
	rm $(PROGRAM) $(OBJ)

