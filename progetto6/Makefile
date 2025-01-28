##
# VM Translator
#
# @file
# @version 0.1

CFLAG = -Wall -Werror -pedantic 
DFLAG = -g -DDEBUG
SCRF  = vmt.c parse.c wf.c metavm.c
ODIR  =
BINN  = VMtranslator

vmt: vmt.o parse.o wf.o metavm.o
	gcc $(CFLAG) *.o -o $(ODIR)$(BINN)

debug: vmtd.o
	gcc $(CFLAG) $(DFLAG) *.o -o $(ODIR)$(BINN)

vmt.o: vmt.c parse.c wf.c metavm.c
	gcc $(CFLAG) -c $(SCRF)

vmtd.o: vmt.c parse.c wf.c metavm.c
	gcc $(CFLAG) $(DFLAG) -c $(SCRF)

clean:
	rm *.o $(BINN)

# end
