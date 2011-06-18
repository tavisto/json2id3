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

clean: clean-o clean-bin clean-aif clean-json clean-images

clean-o: 
	rm -vf *.o 

clean-bin:
	rm -vf bin/*

clean-images:
	cp test/clean/attilas_id3logo.jpg test/test.jpg
	cp test/clean/id3v2.png test/test.png

clean-aif:
	cp test/clean/clean.aif test/test.aif
	#cp test/clean/big.aif test/big.aif

clean-json:
	cp test/clean/clean.json test/test.json

test: clean all 
	./bin/tagger --file test/test.aif --tags test/test.json
	#./bin/tagger --file test/big.aif --tags test/test.json
