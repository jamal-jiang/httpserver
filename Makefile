CC=gcc
CFLAGS=-Wall -g
BIN=webserver
OBJS=main.o

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -f *.o $(BIN)
