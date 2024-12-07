CC = gcc
CFLAGS = -std=c23

assembler: assembler.o hashmap.o intpow.o
	$(CC) $(CFLAGS) -o assembler assembler.o hashmap.o intpow.o

debug: assembler.o hashmap.o intpow.o
	$(CC) $(CFLAGS) -g -o assembler assembler.c hashmap.c intpow.c

assembler.o: assembler.h assembler.c
	$(CC) $(CFLAGS) -c assembler.c

hashmap.o: hashmap.h hashmap.c
	$(CC) $(CFLAGS) -c hashmap.c intpow.c

intpow.o: intpow.h intpow.c
	$(CC) $(CFLAGS) -c intpow.c

clean:
	rm *.o assembler
