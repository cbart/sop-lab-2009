CC = gcc
CFLAGS = -Wall -O0 -g -D_GNU_SOURCE

all: serwer klient

serwer: err.o graph.o ipc_msg.o thread_pool.o serwer.o
	${CC} ${CFLAGS} -lpthread -o $@ $^

klient: err.o ipc_msg.o klient.o
	${CC} ${CFLAGS} -lpthread -o $@ $^

%.o: %.c %.h
	${CC} ${CFLAGS} -c -lpthread -o $@ $<

clean:
	@rm -f *.o *~

.PHONY: clean all
