##
# VM Translator
#
# @file
# @version 0.1

CFLAG = -Wall -pedantic
DFLAG = -g -DDEBUG
SCRF  = vm.c
ODIR  = bin/
BINN  = vm

vm: vm.o
	gcc $(CFLAG) *.o -o $(ODIR)$(BINN)

debug: vmd.o
	gcc $(CFLAG) $(DFLAG) *.o -o $(ODIR)$(BINN)

vm.o: vm.c vm.h
	gcc $(CFLAG) -c $(SCRF)

vmd.o: vm.c vm.h
	gcc $(CFLAG) $(DFLAG) -c $(SCRF)

clean:
	rm *.o

# end
