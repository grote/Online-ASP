CC      = g++
CFLAGS  = -O3 -Wall -I../lib/include -DNDEBUG
#CFLAGS  = -O0 -Wall -g -I.
LDFLAGS = 

SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))

all: ../bin/gringo

../bin/gringo: $(OBJECTS) ../bin/libgringo.a
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) ../bin/libgringo.a

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS)

depend: $(SOURCES)
	makedepend -Y -- $(SOURCES) &> /dev/null
	rm Makefile.bak

# DO NOT DELETE
