CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -pedantic

assembler: assembler.o hashmap.o intpow.o binary.o
	$(CC) $(CFLAGS) -Os -o assembler assembler.o hashmap.o intpow.o binary.o

debug: assembler.o hashmap.o intpow.o binary.o
	$(CC) $(CFLAGS) -g -o assembler assembler.c hashmap.c intpow.c binary.c

test: test.o hashmap.o intpow.o binary.o
	$(CC) $(CFLAGS) -g -o bin/test test.c hashmap.c intpow.c binary.c

assembler.o: assembler.h assembler.c
	$(CC) $(CFLAGS) -c assembler.c

hashmap.o: hashmap.h hashmap.c
	$(CC) $(CFLAGS) -c hashmap.c

intpow.o: intpow.h intpow.c
	$(CC) $(CFLAGS) -c intpow.c

binary.o: binary.h binary.c
	$(CC) $(CFLAGS) -c binary.c

clean:
	rm *.o
