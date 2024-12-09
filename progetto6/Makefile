##
# VM Translator
#
# @file
# @version 0.1

CFLAG = -std=c23 -Wall -pedantic
DFLAG = -g -DDEBUG
SCRF  = vm.c
ODIR  = bin/
LDIR  = lib/
BINN  = vm

vm: vm.o
	gcc $(CFLAG) *.o -o $(ODIR)$(BINN)

debug: vmd.o libsd
	gcc $(CFLAG) $(DFLAG) *.o -o $(ODIR)$(BINN)

libs: $(LDIR)/*
	gcc $(CFLAG) -c $(LDIR)*.c

libsd: $(LDIR)/*
	gcc $(CFLAG) $(DFLAG) -c $(LDIR)*.c

vm.o: vm.c vm.h
	gcc $(CFLAG) -c $(SCRF)

vmd.o: vm.c vm.h
	gcc $(CFLAG) $(DFLAG) -c $(SCRF)

clean:
	rm *.o

# end
