lex_test: lex_test.o lex2.o
	gcc -g -O2 -march=native -static build/lex_test.o build/lex2.o -o lex_test

lex_test.o: build
	gcc -g -O2 -march=native -static -c test/lex_test.c -o build/lex_test.o -iquote src/

lex2.o: build
	gcc -g -O2 -march=native -static -c src/lex2.c -o build/lex2.o

build:
	mkdir build

clean:
	rm build/*
	rm *_test

