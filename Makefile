CC=clang
CFLAGS=-pipe
LDFLAGS=-Isrc

BUILD=-Og -g -fno-omit-frame-pointer
RELEASE=-O2 -DNDEBUG

default: build

build:
	$(CC) $(CFLAGS) $(BUILD) main.c src/*.c $(LDFLAGS)

release:
	$(CC) $(CFLAGS) $(RELEASE) main.c src/*.c $(LDFLAGS)

clean:
	-@rm -f *.o *.out *.data *.old
