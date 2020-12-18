CFLAGS = -std=c99 -Wall -Wextra -pedantic -Wstrict-overflow \
	-fno-strict-aliasing -Wno-missing-field-initializers -g -DDEBUGGING

OBJS =  jubil.o \
	port.o \
	reader.o


HEADERS = \
	jubil.h

all: tags jubil

tags:
	etags *.c *.h

jubil: $(OBJS) main.o
	$(CC) $(CFLAGS) -o $@ $(OBJS) main.o

*.o: Makefile $(HEADERS)

clean:
	rm -f *.o *.core jubil
