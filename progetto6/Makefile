##
# VM Translator
#
# @file
# @version 0.1

CFLAG = -Wall -Werror -pedantic 
DFLAG = -g -DDEBUG
SCRF  = vm.c parse.c wf.c
ODIR  =
BINN  = VMtranslator

vm: vm.o parse.o wf.o
	gcc $(CFLAG) *.o -o $(ODIR)$(BINN)

debug: vmd.o
	gcc $(CFLAG) $(DFLAG) *.o -o $(ODIR)$(BINN)

vm.o: vm.c parse.c wf.c
	gcc $(CFLAG) -c $(SCRF)

vmd.o: vm.c parse.c wf.c
	gcc $(CFLAG) $(DFLAG) -c $(SCRF)

clean:
	rm *.o $(BINN)

# end
