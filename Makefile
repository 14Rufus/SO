CC = gcc
CFLAGS = -Wall -g -I$(INCDIR)
OBJDIR = obj
SRCDIR = src
INCDIR = includes
#OBJS = $(OBJDIR)/auxs.o $(OBJDIR)/task.o
#SERVER_OBJS = $(OBJDIR)/argusd.o $(OBJDIR)/server_child.o
#CLIENT_OBJS = $(OBJDIR)/argus.o


all: server client

server: bin/aurrasd

client: bin/aurras

bin/aurrasd: obj/aurrasd.o
	gcc -g obj/aurrasd.o -o bin/aurrasd

obj/aurrasd.o: src/aurrasd.c
	gcc -Wall -g -c src/aurrasd.c obj/aurrasd.o

bin/aurras: obj/aurras.o
	gcc -g obj/aurras.o -o bin/aurras

obj/aurras.o: src/aurras.c
	gcc -Wall -g -c src/aurras.c obj/aurras.o

clean:
	rm obj/* tmp/* bin/{aurras,aurrasd}


test:
	bin/aurras samples/sample-1.mp3 tmp/sample-1.mp3
	bin/aurras samples/sample-2.mp3 tmp/sample-2.mp3

#clean:
#	rm -f $(OBJDIR)/*.o
#	rm -f aurras
#	rm -f aurrasd
#	rm -f server_client_pipe
#	rm -f client_server_pipe
	#rm -f error.txt
	#rm -f log.txt
	#rm -f log.idx
