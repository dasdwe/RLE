cc=gcc
CFLAGS = 
LIBS = 

OBJ = rle.o

all: main

clean:
	rm -f *.o rle
	
.c.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

main: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o rle
