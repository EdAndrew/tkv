SRCS = tkv.c test.c
OBJS = $(SRCS:.c=.o)
PROG = tkv
CC = gcc
CFLAGS = -g

$(PROG): $(OBJS)
	$(CC)  $(CFLAGS) -o $@ $^

$(OBJS): tkv.h

clean:
	rm $(PROG) $(OBJS)
