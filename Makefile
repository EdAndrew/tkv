SRCS = tkv.c test.c
OBJS = $(SRCS:.c=.o)
PROG = tkv
CC = gcc
CFLAGS = -g

$(PROG): $(OBJS)
	$(CC)  $(CFLAGS) -o $@ $^
	rm $^

$(OBJS): tkv.h

clean:
	rm $(PROG)
