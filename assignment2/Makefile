CC = gcc
CFLAGS = -g -Wall -Wextra -pedantic
PROG = tcp_echo_server
SRCS = tcp_echo_server.c
OBJS = $(SRCS:.c=.o)

$(PROG) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG)

clean :
	rm -f core $(PROG) $(OBJS)
