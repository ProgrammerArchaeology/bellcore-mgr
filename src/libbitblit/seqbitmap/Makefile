include ../../../Configfile

CFLAGS=		$(CCFLAGS) $(CPPFLAGS) -I../$(BITBLIT)/
OBJS=		bitmapread.o bitmapwrite.o bit_load.o bit_save.o

all:		$(OBJS)

install:	all
		install -c -m 644 bitmap.5 $(MANDIR)/man5

depend:
		$(MKDEP) $(MKDEPFLAGS) $(CPPFLAGS) *.c > $(MKDEPOUT)

clean:
		$(RMF) $(OBJS)

clobber:	clean
		$(ZILCH) $(DEPFILE)

include $(DEPFILE)
