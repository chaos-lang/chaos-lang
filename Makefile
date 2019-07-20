FLAGS = -g -O2 -march=native -static

lex_test: lex_test.o lex2.o
	gcc $(FLAGS) build/lex_test.o build/lex2.o -o lex_test

lex_test.o: build
	gcc $(FLAGS) -c test/lex_test.c -o build/lex_test.o -iquote src/

lex2.o: build
	gcc $(FLAGS) -c src/lex2.c -o build/lex2.o

qa_test: qa_test.o quick_alloc.o
	gcc $(FLAGS) build/qa_test.o build/quick_alloc.o -o qa_test

qa_test.o: build
	gcc $(FLAGS) -c test/qa_test.c -o build/qa_test.o -iquote lib/

quick_alloc.o: build
	gcc $(FLAGS) -c lib/quick_alloc.c -o build/quick_alloc.o -iquote lib/

xxhash_test: xxhash_test.o xxhash.o
	gcc $(FLAGS) build/xxhash_test.o build/xxhash.o -o xxhash_test

xxhash_test.o: build
	gcc $(FLAGS) -c test/xxhash_test.c -o build/xxhash_test.o -iquote lib/

xxhash.o: build
	gcc $(FLAGS) -c lib/xxhash.c -o build/xxhash.o -iquote lib/

build:
	mkdir build

clean:
	rm build/*
	rm *_test

