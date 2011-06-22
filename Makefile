CC=g++
CFLAGS=-Wall -Wextra  \
	-I /Users/tavis.aitken/SourceCode/aif-project/cpp/taglib-1.7/include \
	-I /Users/tavis.aitken/SourceCode/aif-project/cpp/taglib-1.7 \
	-I /Users/tavis.aitken/SourceCode/aif-project/cpp/jsoncpp-src-0.5.0/include
	#-I /Users/tavis.aitken/SourceCode/aif-project/cpp/libjson/include \

LDFLAGS=-L /Users/tavis.aitken/SourceCode/aif-project/cpp/taglib-1.7/taglib -l tag \
		-L /Users/tavis.aitken/SourceCode/aif-project/cpp/jsoncpp-src-0.5.0/libs/linux-gcc-4.2.1 -l json_linux-gcc-4.2.1_libmt
		#-L /Users/tavis.aitken/SourceCode/aif-project/cpp/libjson/src -l json \
		

all: main.o 
	$(CC) $(LDFLAGS) main.o -o tagger

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

clean: clean-o clean-bin clean-aif clean-json

clean-o: 
	rm -vf *.o 

clean-bin:
	rm -vf tagger 

clean-aif:
	cp test/clean/clean.aif test/test.aif

clean-json:
	cp test/clean/clean.json test/test.json

run: clean all 
	./tagger --file test/test.aif --tags test/test.json
