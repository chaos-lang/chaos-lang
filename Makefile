FLAGS = -g -O2 -march=native -static

all: parse_test lex_test qa_test xxhash_test ht_test

# Parser tests

parse_test: parse_test.o parse.o decl.o lex2.o hash_table.o xxhash.o quick_alloc.o
	gcc $(FLAGS) build/parse_test.o build/parse.o build/decl.o build/lex2.o build/hash_table.o build/xxhash.o build/quick_alloc.o -o parse_test

parse_test.o: build
	gcc $(FLAGS) -c test/parse_test.c -o build/parse_test.o -iquote src/ -iquote lib/

parse.o: build
	gcc $(FLAGS) -c src/parse.c -o build/parse.o -iquote lib/

decl.o: build
	gcc $(FLAGS) -c src/decl.c -o build/decl.o -iquote lib/

# Lexer tests

lex_test: lex_test.o lex2.o hash_table.o xxhash.o
	gcc $(FLAGS) build/lex_test.o build/lex2.o build/hash_table.o build/xxhash.o -o lex_test

lex_test.o: build
	gcc $(FLAGS) -c test/lex_test.c -o build/lex_test.o -iquote src/ -iquote lib/

lex2.o: build
	gcc $(FLAGS) -c src/lex2.c -o build/lex2.o -iquote lib/

# Quick allocator tests

qa_test: qa_test.o quick_alloc.o
	gcc $(FLAGS) build/qa_test.o build/quick_alloc.o -o qa_test

qa_test.o: build
	gcc $(FLAGS) -c test/qa_test.c -o build/qa_test.o -iquote lib/

quick_alloc.o: build
	gcc $(FLAGS) -c lib/quick_alloc.c -o build/quick_alloc.o -iquote lib/

# XXHash tests

xxhash_test: xxhash_test.o xxhash.o
	gcc $(FLAGS) build/xxhash_test.o build/xxhash.o -o xxhash_test

xxhash_test.o: build
	gcc $(FLAGS) -c test/xxhash_test.c -o build/xxhash_test.o -iquote lib/

xxhash.o: build
	gcc $(FLAGS) -c lib/xxhash.c -o build/xxhash.o -iquote lib/

# Hash table tests

ht_test: ht_test.o hash_table.o xxhash.o
	gcc $(FLAGS) build/ht_test.o build/hash_table.o build/xxhash.o -o ht_test

ht_test.o: build
	gcc $(FLAGS) -c test/ht_test.c -o build/ht_test.o -iquote lib/

hash_table.o: build
	gcc $(FLAGS) -c lib/hash_table.c -o build/hash_table.o -iquote lib/

build:
	mkdir build

clean:
	rm build/*
	rm *_test

