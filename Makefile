SRCS = tkv.c server.c test.c
OBJS = $(SRCS:.c=.o)
PROG = tkv
CC = gcc
CFLAGS = -g

$(PROG): $(OBJS)
	$(CC)  $(CFLAGS) -o $@ $^
	rm $^

$(OBJS): tkv.h server.h

clean:
	rm $(PROG)
