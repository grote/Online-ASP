note:
	echo "U should use cmake." && echo "This Makefile file is depricated."
all: note
	make -C lib
	make -C app

doc: note
	make -C doc

clean: note
	make -C lib clean
	make -C app clean

depend: note
	make -C lib depend
	make -C app depend
	

