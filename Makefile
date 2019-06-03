lex_test: lex_test.o lex.o
	gcc -g lex_test.o lex.o -o lex_test

lex_test.o:
	gcc -g -c lex_test.c -o lex_test.o

lex.o:
	gcc -g -O2 -c lex.c -o lex.o

clean:
	rm *.o
	rm *_test

