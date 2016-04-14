#                        Copyright (c) 1988 Bellcore
#                            All Rights Reserved
#       Permission is granted to copy or use this program, EXCEPT that it
#       may not be sold for profit, the copyright notice must be reproduced
#       on copies, and credit should be given to Bellcore where it is due.
#       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.

#	makefile for stub blit library

include ../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS) -I..

CFILES= blit.c line.c pixel.c bitmap.c bit_size.c bit_blit.c bit_on.c palette.c screen.c
OBJS= blit.o line.o pixel.o bitmap.o bit_size.o bit_blit.o bit_on.o palette.o screen.o
HFILES= bitmap.h screen.h
OTHER=Makefile README
LIB=blitlib.a
START=.
MAKE=make

all:		$(LIB)

# bitmap library

$(LIB): $(OBJS)
	ar rv $(LIB) $(OBJS)
	ranlib $(LIB)

clean:
	$(RMF) *.o $(LIB)

list:
	@for i in ${HFILES} ${CFILES} ${OTHER}; do \
		echo "${START}/$$i"; \
	done	

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) $(BLITLIBFLAGS) *.c > $(MKDEPOUT)

clobber:
		$(ZILCH) $(DEPFILE)
