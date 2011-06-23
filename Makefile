CC=g++
CFLAGS=-Wall -Wextra -fPIC \
	-I /usr/include/taglib \
	-I ..//libjson/include 

LDFLAGS=-L /usr/lib64 -l tag \
		-L ../libjson/src -l json -Wl,-rpath=../libjson/src

DEBUG=-g

all: main.o 
	$(CC) $(LDFLAGS) main.o -o bin/tagger

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

clean: clean-o clean-bin clean-aif clean-json

clean-o: 
	rm -vf *.o 

clean-bin:
	rm -vf bin/*

clean-aif:
	cp test/clean/clean.aif test/test.aif

clean-json:
	cp test/clean/clean.json test/test.json

test: clean all 
	./bin/tagger --file test/test.aif --tags test/test.json
