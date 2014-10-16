# BASIC MUA/MTA										Makefile
# ----------------------------------------------------------
INCDIR	= inc
SRCDIR	= src
OBJDIR	= obj

CC 		= gcc
LD		= gcc
# get ride of that -Wno-missing-field-initializers and fix the problem
#CFLAGS	= -g -Wall -Wextra -I$(INCDIR) -Wno-missing-field-initializers #-O1 -Wuninitialized
CFLAGS	= -g -I$(INCDIR) -Wno-missing-field-initializers #-O1 -Wuninitialized
LDFLAGS	= -lsqlite3

CLTSRC		= $(SRCDIR)/client.c
SRVSRC		= $(SRCDIR)/server.c
CLTOBJ		= $(OBJDIR)/client.o
SRVOBJ		= $(OBJDIR)/server.o
CLTBIN		= client
SRVBIN		= server

SRCS		= $(filter-out $(SRVSRC) $(CLTSRC), $(wildcard $(SRCDIR)/*.c) )
OBJS		= $(addprefix $(OBJDIR)/,  $(notdir $(SRCS:.c=.o)) )
INCS		= $(wildcard $(INCDIR)/*.h)
BINS		= $(CLTBIN) $(SRVBIN)

all : $(CLTBIN) $(SRVBIN)
#all : $(SRVBIN)
run : $(SRVBIN)
	@echo running server
	./$(SRVBIN)

client : $(OBJS) $(CLTOBJ)
	$(LD) $(LDFLAGS) $^ -o $@

server : $(OBJS) $(SRVOBJ)
	$(LD) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $<
	mv -f *.o $(OBJDIR)

# TAG : make tag
clean :
	rm -f $(BIN)

mrproper : clean
	rm -f $(OBJDIR)/*.o

.PHONY : all clean mrproper

depend:
	makedepend -I$(INCDIR) -Y --$(CFLAGS) --$(SRCS) $(CLTSRC) $(SRVSRC)
# DO NOT DELETE

src/mail.o: inc/wsock.h inc/err.h inc/util.h inc/io.h inc/mail.h inc/common.h
src/net.o: inc/net.h
src/util.o: inc/util.h
src/wsock.o: inc/wsock.h inc/net.h
