CC = gcc
CFLAGS = -Wall -O0 -g

all: graph.o ipc_msg.o thread_pool.o err.o

%.o: %.c %.h
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	@rm -f *.o *~

.PHONY: clean all
