include ./Config

TARGETS = ./libzfp.a ./libzfp.so
#OBJECTS = bitstream.o decode1f.o decode1d.o encode1f.o encode1d.o decode2f.o decode2d.o encode2f.o encode2d.o decode3f.o decode3d.o encode3f.o encode3d.o zfp.o
OBJECTS = bitstream.o decode.o encode.o zfp.o

./libzfp.a: $(OBJECTS)
	rm -f $@
	ar rc $@ $^

./libzfp.so: $(OBJECTS)
	$(CC) $(CFLAGS) -shared $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -c $<

static: ./libzfp.a

shared: ./libzfp.so

clean:
	rm -f $(TARGETS) $(OBJECTS)
