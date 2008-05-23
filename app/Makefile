CC      = g++
CFLAGS  = -O3 -Wall -I../lib/gringo/include -I../lib -DNDEBUG
#CFLAGS  = -O0 -Wall -g -I.
LDFLAGS = 

SOURCES = $(wildcard *.cpp program_opts/*.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))

all: ../bin/gringo

../bin/gringo: $(OBJECTS) ../bin/libgringo.a ../bin/libclasp.a
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) ../bin/libgringo.a ../bin/libclasp.a

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS)

depend: $(SOURCES)
	makedepend -Y -- $(SOURCES) &> /dev/null
	rm Makefile.bak

# DO NOT DELETE

main.o: clasp_main.h options.h program_opts/program_options.h
main.o: program_opts/value_base.h program_opts/detail/smartpointer.h timer.h
options.o: options.h program_opts/program_options.h program_opts/value_base.h
options.o: program_opts/detail/smartpointer.h program_opts/value.h
options.o: program_opts/program_options.h program_opts/errors.h
timer.o: timer.h
program_opts/program_options.o: program_opts/program_options.h
program_opts/program_options.o: program_opts/errors.h
program_opts/value.o: program_opts/value.h program_opts/program_options.h
program_opts/value.o: program_opts/value_base.h program_opts/errors.h
