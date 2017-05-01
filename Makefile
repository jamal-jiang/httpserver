CC=gcc
CFLAGS=-Wall -g
BIN=tinyhttpd
OBJS=tiny.o request.o tools.o

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -f *.o $(BIN)