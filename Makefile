lex_test: lex_test.o lex2.o
	gcc -g -O2 -march=native -static lex_test.o lex2.o -o lex_test

lex_test.o:
	gcc -g -O2 -march=native -static -c lex_test.c -o lex_test.o

lex2.o:
	gcc -g -O2 -march=native -static -c lex2.c -o lex2.o

clean:
	rm *.o
	rm *_test

