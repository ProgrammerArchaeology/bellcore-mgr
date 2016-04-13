#                        Copyright (c) 1988 Bellcore
#                            All Rights Reserved
#       Permission is granted to copy or use this program, EXCEPT that it
#       may not be sold for profit, the copyright notice must be reproduced
#       on copies, and credit should be given to Bellcore where it is due.
#       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.

#	makefile for stub blit library

include ../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS) -I..

CFILES= blit.c line.c pixel.c bitmap.c bit_size.c
OBJS= blit.o line.o pixel.o bitmap.o bit_size.o
HFILES= bitmap.h
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
	$(RMF) screen.h *.o $(LIB)

screen.h:
		$(LNS) $(SCREEN).h screen.h

$(OBJS):	screen.h

list:
	@for i in ${HFILES} ${CFILES} ${OTHER}; do \
		echo "${START}/$$i"; \
	done	

depend:		screen.h
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) $(BLITLIBFLAGS) *.c > $(MKDEPOUT)

clobber:
		$(RMF) screen.h
		$(ZILCH) $(DEPFILE)
