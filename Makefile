CC=cc
CFLAGS=
LDFLAGS=-Isrc

default: build

build:
	$(CC) $(CFLAGS) main.c src/*.c $(LDFLAGS)

clean:
	-@rm -f *.o *.out
