CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic
PROG = tcp_echo_server
SRCS = tcp_echo_server.c ushout.c
OBJS = $(SRCS:.c=.o)

$(PROG) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG)

main.o : main.c
ushout.o : ushout.c ushout.h

clean :
	rm -f core $(PROG) $(OBJS)
