CC = gcc
CFLAGS = -std=c23

assembler: assembler.o hashmap.o intpow.o binary.o
	$(CC) $(CFLAGS) -o bin/assembler assembler.o hashmap.o intpow.o binary.o

debug: assembler.o hashmap.o intpow.o binary.o
	$(CC) $(CFLAGS) -g -o bin/assembler assembler.c hashmap.c intpow.c binary.c

test: test.o hashmap.o intpow.o binary.o
	$(CC) $(CFLAGS) -g -o bin/test test.c hashmap.c intpow.c binary.c

test.o: test.c
	$(CC) $(CFLAGS) -g -c test.c hashmap.c intpow.c binary.c

assembler.o: assembler.h assembler.c
	$(CC) $(CFLAGS) -c assembler.c

hashmap.o: hashmap.h hashmap.c
	$(CC) $(CFLAGS) -c hashmap.c

intpow.o: intpow.h intpow.c
	$(CC) $(CFLAGS) -c intpow.c

binary.o: binary.h binary.c
	$(CC) $(CFLAGS) -c binary.c

clean:
	rm *.o bin/test
